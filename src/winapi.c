// Bindings to WinAPI Wia (Windows Image Acquision)
// https://learn.microsoft.com/en-us/windows/win32/wia/-wia-startpage
// This is an obsolete API, but it's well implemented in early versoins of
// Windows, as well as Wine and ReactOS.
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

// Tested for MinGW

// ISO PTP CMD packet: 12 bytes
// Windows wrapper CMD packet: 30 bytes

#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <winbase.h>
#include <devguid.h>
#include <regstr.h>
#include <winuser.h>
#include <devpkey.h>
#include <initguid.h>

//#include <winptp.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>

#include <PortableDeviceApi.h>      // Include this header for Windows Portable Device API interfaces
#include <PortableDevice.h>         // Include this header for Windows Portable Device definitions
#include <WpdMtpExtensions.h>

#pragma comment(lib,"PortableDeviceGUIDs.lib")

#define CLIENT_NAME         L"WPD Services Sample Application"
#define CLIENT_MAJOR_VER    1
#define CLIENT_MINOR_VER    0
#define CLIENT_REVISION     0

HRESULT OpenDevice(LPCWSTR wszPnPDeviceID) {
    HRESULT                hr = S_OK;
    IPortableDeviceValues* pClientInformation = NULL;
    IPortableDevice* pDevice = NULL;

    if ((wszPnPDeviceID == NULL))
    {
        hr = E_INVALIDARG;
        return hr;
    }

    HRESULT ClientInfoHR = S_OK;

    // CoCreate an IPortableDeviceValues interface to hold the client information.
    hr = CoCreateInstance(CLSID_PortableDeviceValues,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IPortableDeviceValues,
        (VOID**)&pClientInformation);
    if (SUCCEEDED(hr))
    {

        // Attempt to set all properties for client information. If we fail to set
        // any of the properties below it is OK. Failing to set a property in the
        // client information isn't a fatal error.
        ClientInfoHR = pClientInformation->SetStringValue(WPD_CLIENT_NAME, CLIENT_NAME);
        if (FAILED(ClientInfoHR))
        {
            // Failed to set WPD_CLIENT_NAME
            printf("Error %d\n", GetLastError());
        }

        ClientInfoHR = pClientInformation->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, CLIENT_MAJOR_VER);
        if (FAILED(ClientInfoHR))
        {
            // Failed to set WPD_CLIENT_MAJOR_VERSION
        }

        ClientInfoHR = pClientInformation->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, CLIENT_MINOR_VER);
        if (FAILED(ClientInfoHR))
        {
            // Failed to set WPD_CLIENT_MINOR_VERSION
        }

        ClientInfoHR = pClientInformation->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, CLIENT_REVISION);
        if (FAILED(ClientInfoHR))
        {
            // Failed to set WPD_CLIENT_REVISION
            printf("Error %d\n", GetLastError());
        }
    }
    else
    {
        // Failed to CoCreateInstance CLSID_PortableDeviceValues for client information
        printf("Error %d\n", GetLastError());
    }

    ClientInfoHR = pClientInformation->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION);
    if (FAILED(ClientInfoHR))
    {
        // Failed to set WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE
        printf("Error %d\n", GetLastError());
    }

    if (SUCCEEDED(hr))
    {
        // CoCreate an IPortableDevice interface
        hr = CoCreateInstance(CLSID_PortableDeviceFTM,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IPortableDevice,
            (VOID**)&pDevice);

        if (SUCCEEDED(hr))
        {
            // Attempt to open the device using the PnPDeviceID string given
            // to this function and the newly created client information.
            // Note that we're attempting to open the device the first 
            // time using the default (read/write) access. If this fails
            // with E_ACCESSDENIED, we'll attempt to open a second time
            // with read-only access.
            hr = pDevice->Open(wszPnPDeviceID, pClientInformation);
            if (hr == E_ACCESSDENIED)
            {
                // Attempt to open for read-only access
                pClientInformation->SetUnsignedIntegerValue(
                    WPD_CLIENT_DESIRED_ACCESS,
                    GENERIC_READ);
                hr = pDevice->Open(wszPnPDeviceID, pClientInformation);
            }
            if (SUCCEEDED(hr))
            {
                printf("Success opening\n");

                HANDLE foo = CreateEvent(nullptr, false, false, nullptr);

                PROPERTYKEY opType = WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ;

                IPortableDeviceValues* pDevValues;
                hr = CoCreateInstance(CLSID_PortableDeviceValues,
                    nullptr,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&pDevValues));

                hr = pDevValues->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                    opType.fmtid);

                hr = pDevValues->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                    opType.pid);

                hr = pDevValues->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE,
                    (ULONG)0x1001);

                if (SUCCEEDED(hr))
                {
                    printf("Success opcode\n");
                }

                IPortableDevicePropVariantCollection* spMtpParams = nullptr;
                hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&spMtpParams));

                if (SUCCEEDED(hr))
                {
                    printf("Success param\n");
                }

                hr = pDevValues->SetIPortableDevicePropVariantCollectionValue(
                    WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, spMtpParams);

                if (SUCCEEDED(hr))
                {
                    printf("Success set param\n");
                }

                IPortableDeviceValues* spResults = nullptr;

                hr = pDevice->SendCommand(0, pDevValues, &spResults);

                if (SUCCEEDED(hr))
                {
                    printf("Success send\n");

                    ULONG cbOptimalDataSize = 0;
                    hr = spResults->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_TOTAL_DATA_SIZE,
                        &cbOptimalDataSize);
                    printf("Data size: %d\n", cbOptimalDataSize);
                }
            }
        }
        else
        {
            // Failed to CoCreateInstance CLSID_PortableDevice
            printf("Error %d\n", GetLastError());
        }
    }

    // Release the IPortableDevice when finished
    if (pDevice != NULL)
    {
        pDevice->Release();
        pDevice = NULL;
    }

    // Release the IPortableDeviceValues that contains the client information when finished
    if (pClientInformation != NULL)
    {
        pClientInformation->Release();
        pClientInformation = NULL;
    }

    return hr;
}


int main() {
    CoInitialize(nullptr);

    IPortableDevice *m_device;
    IPortableDeviceManager* pPortableDeviceManager = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceManager,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pPortableDeviceManager));

    if (SUCCEEDED(hr)) {
        puts("Yay");
    }
    else {
        printf("Error %d\n", GetLastError());
    }
    DWORD numDevices = 0;

    hr = pPortableDeviceManager->GetDevices(NULL, &numDevices);

    if (SUCCEEDED(hr)) {
        puts("Yay");
    } // https://github.com/dougforpres/SonyCamera/blob/b4249942cd5dd1817da395856b514e1103e8c854/SonyMTPCamera/CameraManager.cpp
    else {
        printf("Error %d\n", GetLastError());
    }

    printf("Devices: %d\n", numDevices);

    for (int i = 0; i < numDevices; i++) {
        PWSTR pPnpDeviceIDs[1];
        DWORD len = numDevices;
        hr = pPortableDeviceManager->GetDevices(pPnpDeviceIDs, &len);
        if (SUCCEEDED(hr)) {
            puts("Yay");
            printf("%d\n", len);

            std::wstring deviceId = (WCHAR*)pPnpDeviceIDs[0];
            printf("Device ID: %ls\n", deviceId.c_str());
            OpenDevice((LPCWSTR)deviceId.c_str());
        }
        else {
            printf("Error %d\n", GetLastError());
        }
    }

}
