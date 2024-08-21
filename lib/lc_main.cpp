#define BUILD_DLL
#define CAMLIB_INCLUDE_IMPL

#include "lc_main.h"
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <libusb.h>
#include <camlib.h>

/*
#include <lib.c>
#include <libusb.c>
#include <log.c>
#include <no_ip.c>            //Include CygWin Library under Windows
#include <packet.c>
#include <transport.c>
*/

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
