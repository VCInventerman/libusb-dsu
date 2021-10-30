#define EVG_LIB_BOOST
#include <evglib>

#ifdef EVG_PLATFORM_WIN
#pragma warning( push )
#pragma warning( disable : 4200 )
#include <libusb.h>
#pragma warning( pop )
#endif


int main(int, char**) {
    std::cout << "Hello, world!\n";
    
}
