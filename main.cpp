#include <atomic>
#include <thread>
#include <iostream>

#ifdef EVG_PLATFORM_WIN
#pragma warning( push )
#pragma warning( disable : 4200 )
#include <libusb.h>
#pragma warning( pop )
#else
#include <libusb-1.0/libusb.h>
#endif

#include <hidapi.h>

#include "CommandEval.h"

std::atomic_bool programActive = true;

int main(int, char**)
{
    int res;
    unsigned char buf[256];
    #define MAX_STR 255
    wchar_t wstr[MAX_STR];

    std::cout << "Welcome to libusb-dsu! Try help\n";

    if (hid_init())
        return -1;

    hid_device* handle;
    handle = hid_open(0x0738, 0x4161, NULL);
    if (!handle) {
        printf("unable to open device\n");
        return 1;
    }
    
    hid_set_nonblocking(handle, 1);


    // Read the Manufacturer String
    wstr[0] = 0x0000;
    res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
    if (res < 0)
        printf("Unable to read manufacturer string\n");
    printf("Manufacturer String: %ls\n", wstr);

    // Read the Product String
    wstr[0] = 0x0000;
    res = hid_get_product_string(handle, wstr, MAX_STR);
    if (res < 0)
        printf("Unable to read product string\n");
    printf("Product String: %ls\n", wstr);

    // Read the Serial Number String
    wstr[0] = 0x0000;
    res = hid_get_serial_number_string(handle, wstr, MAX_STR);
    if (res < 0)
        printf("Unable to read serial number string\n");
    printf("Serial Number String: (%d) %ls", wstr[0], wstr);
    printf("\n");

    // Read Indexed String 1
    wstr[0] = 0x0000;
    res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
    if (res < 0)
        printf("Unable to read indexed string 1\n");
    printf("Indexed String 1: %ls\n", wstr);




    res = 0;
    int i = 0;
    while (res == 0) {
        res = hid_read(handle, buf, sizeof(buf));
        if (res == 0)
            printf("waiting...\n");
        if (res < 0)
            printf("Unable to read()\n");
#ifdef _WIN32
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
#else
        usleep(500 * 1000);
#endif
    }

    printf("Data read:\n   ");
    // Print out the returned buffer.
    for (i = 0; i < res; i++)
        printf("%02x ", (unsigned int)buf[i]);
    printf("\n");




    CommandEval eval;
    eval.addCommand("help", [](std::string_view) 
        { std::cout << "Of course this isn't implemented\n"; return CommandEval::ReturnCode::Success;});
    eval.addCommand("exit", [](std::string_view) 
        { programActive = false; return CommandEval::ReturnCode::Success; });


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
    hid_exit();
    exit(0);
}
