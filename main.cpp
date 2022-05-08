#include <iostream>
#include <exception>

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4200 )
#include <libusb.h>
#pragma warning( pop )
#else
#include <libusb-1.0/libusb.h>
#endif
#include <spdlog/spdlog.h>
#include <boost/range/irange.hpp>

#include "CommandEval.h"

unsigned char packetStartOneS[] = { 0x05, 0x20, 0x00, 0x01, 0x00 };
unsigned char packetCrash[] = { 0x01, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0E };
unsigned char packetShortRumble[] = { 0x09, 0x08, 0x00, 0x02, 0x00, 0x0f, 0x04, 0x04 };

std::atomic_bool programActive = true;


class Device
{
public:
    using This = Device;


    constexpr static size_t MAX_SERIALNAME_SIZE = 100;
    constexpr static size_t IBUF_SIZE = 256;

    libusb_device* device = nullptr;
    libusb_device_handle* handle = nullptr;
    libusb_device_descriptor desc = {};

    char serialName[MAX_SERIALNAME_SIZE];
    size_t serialNameLength = 0;

    libusb_transfer* iTransfer = {};
    char iBuf[IBUF_SIZE];
    bool iTransferActive = false;



    Device(libusb_device* device)
    {
        this->device = device;

        constexpr auto check = [](const int ec)
        {
            if (ec)
            {
                spdlog::error(libusb_error_name(ec));
                throw std::runtime_error(libusb_error_name(ec));
            }
        };

        check(libusb_open(device, &handle));
        check(libusb_claim_interface(handle, 0));

        createDetails();

        std::cout << "Active: " << libusb_kernel_driver_active(handle, 0) << '\n';
    }

    void createDetails()
    {
        libusb_get_device_descriptor(device, &desc);

        serialName[0] = '\0';
        serialNameLength = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber,
            (unsigned char*)serialName, MAX_SERIALNAME_SIZE);

        int currentConfig = -2;
        libusb_get_configuration(handle, &currentConfig);
        std::cout << "Current config: " << currentConfig << '\n';
    }

    Device(Device&) = delete;
    Device(Device&& other)
    {
        device = other.device;
        other.device = nullptr;

        handle = other.handle;
        other.handle = nullptr;

        createDetails();
    }

    static void transferCb(libusb_transfer* transfer)
    {
        This& t = *(This*)(transfer->user_data);
        t.iTransferActive = false;

        static int count = 0;
        spdlog::info("Got transfer {}", count);


    }

    ~Device()
    {
        if (handle != nullptr)
        { 
            libusb_release_interface(handle, 0);
            libusb_close(handle); 
        }
    }
};

void runEventLoop(std::vector<Device>* devicesPtr, libusb_context* const context)
{
    std::vector<Device>& devices = *devicesPtr;

    for (auto& i : devices)
    {
        i.iTransfer = libusb_alloc_transfer(0);
    }

    while (programActive)
    {
        for (auto& i : devices)
        {
            if (i.iTransferActive)
            {
                libusb_fill_interrupt_transfer(i.iTransfer, i.handle, (0x82 | LIBUSB_ENDPOINT_IN),
                    (unsigned char*)(i.iBuf), Device::IBUF_SIZE,
                    Device::transferCb, (void*)&i, 0);

                libusb_submit_transfer(i.iTransfer);
            }
        }

        libusb_handle_events(context);
    }
}

int main(int, char**)
{
    std::cout << "Welcome to libusb-dsu! Try help\n";

    libusb_context* context = nullptr;
    libusb_device** deviceList = nullptr;
    std::vector<Device> devices;
    int returnCode = 0;

    returnCode = libusb_init(&context);
    if (returnCode) { std::cout << "libusb error " << __LINE__ << " : " << libusb_error_name(returnCode) << '\n'; }

    libusb_set_option(context, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);

    size_t count = (size_t)libusb_get_device_list(context, &deviceList);
    std::cout << "Found " << count << " devices.\n";

    // Connect devices
    for (auto i : boost::irange(count))
    {
        try
        {
            Device device(deviceList[i]);
            devices.emplace_back(std::move(device));
        }
        catch (...) { }
    }

    libusb_free_device_list(deviceList, 1);


    int actual = -2;

    //libusb_bulk_transfer(devices[0].handle, (1 | LIBUSB_ENDPOINT_OUT), rumblePacket, sizeof(rumblePacket), &actual, 0);

    //libusb_interrupt_transfer(devices[0].handle, (0x02 | LIBUSB_ENDPOINT_OUT), rumblePacket, sizeof(rumblePacket), &actual, 100);
    //libusb_interrupt_transfer(devices[0].handle, (0x02 | LIBUSB_ENDPOINT_OUT), packetStartOneS, sizeof(packetStartOneS), &actual, 0);
    //std::cout << "Data sent size: " << actual << '\n';

    //libusb_interrupt_transfer(devices[0].handle, (0x02 | LIBUSB_ENDPOINT_OUT), packetShortRumble, sizeof(packetShortRumble), &actual, 0);
    //std::cout << "Data sent size: " << actual << '\n';


    std::thread eventLoop(runEventLoop, &devices, context);


    CommandEval eval;
    eval.addCommand("help", [](std::string_view)
        { std::cout << "Of course this isn't implemented\n"; return CommandEval::ReturnCode::Success; });
    eval.addCommand("exit", [](std::string_view)
        { programActive = false; return CommandEval::ReturnCode::Success; });
    eval.addCommand("list", [&devices](std::string_view)
        {
            for (auto i = devices.cbegin(); i != devices.cend(); i++)
            {
                spdlog::info("{}: Vendor:Device:Serial {:X}:{:X}:{}",
                    std::distance(devices.cbegin(), i), i->desc.idVendor, i->desc.idProduct, i->serialName);
            }
            return CommandEval::ReturnCode::Success;
        });
    eval.addCommand("reset", [&devices](std::string_view command)
        {
            if (std::count(command.cbegin(), command.cend(), ' ') < 1) { return CommandEval::ReturnCode::InvalidInput; }

            int device = -2;
            auto res = std::from_chars(command.data() + command.find(' ') + 1,
                command.data() + command.size(),
                device);

            libusb_reset_device(devices[device].handle);
        });

    eval.addCommand("send", [&devices](std::string_view command)
        {
            if (std::count(command.cbegin(), command.cend(), ' ') < 2) { return CommandEval::ReturnCode::InvalidInput; }

            int device = -2;
            auto res = std::from_chars(command.data() + command.find(' ') + 1,
                command.data() + command.size(),
                device);

            if (res.ec != std::errc() || device > devices.size())
            {
                std::cout << "Invalid device name!\n";
                return CommandEval::ReturnCode::InvalidInput;
            }

            uint8_t* bytes = (uint8_t*)alloca(command.size() / 2); // Rough upper bound
            memset(bytes, 0, command.size());
            size_t bytesIndex = 0;

            // Process rest of string
            for (size_t i = findNthChar(command, ' ', 1); i < command.size(); i++)
            {
                char digit = command[i];

                if (!std::isxdigit(digit))
                {
                    continue;
                }

                if (std::isdigit(digit))
                {
                    bytes[bytesIndex / 2] |= (digit - 0x30) << (((bytesIndex % 2) ^ 0x1) * 4);
                }
                else
                {
                    digit |= 0x20; // Force lowercase letters
                    bytes[bytesIndex / 2] |= (digit - 0x57) << (((bytesIndex % 2) ^ 0x1) * 4);
                }

                bytesIndex++;
            }

            //Device& device = devices[]

            //libusb_fill_interrupt_transfer(i.iTransfer, i.handle, (0x82 | LIBUSB_ENDPOINT_IN), 
            //    (unsigned char*)(i.iBuf), Device::IBUF_SIZE,
            //    Device::transferCb, (void*)&i, 0);

            //libusb_submit_transfer(i.iTransfer);

            return CommandEval::ReturnCode::Success;
        });


    std::string currentCommand;
    while (programActive)
    {
        std::cout << "> ";
        std::getline(std::cin, currentCommand, '\n');

        CommandEval::ReturnCode code = eval.exec(currentCommand);

        if (code == CommandEval::ReturnCode::CommandNotFound)
        {
            std::cout << "Command not found! Try /help\n";
        }
        else if (code == CommandEval::ReturnCode::InvalidInput)
        {
            std::cout << "Invalid input.\n";
        }
    }


    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Try and give event handler a chance to clean up
    libusb_exit(context);
    exit(0);
}