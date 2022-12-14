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
#include <wia.h>

#include <winptp.h>

#include <camlib.h>
#include <backend.h>
#include <ptp.h>

#define MAX_DATA_IN 0x10000
#define MAX_DATA_OUT 0x1000000

struct WiaBackend {
	IWiaDevMgr *wia;
	IEnumWIA_DEV_INFO *info;

	IUnknown *dev_connected;
	IWiaEventCallback *callback;

	IWiaPropertyStorage *storage;
	IWiaItem *item;
	IWiaItemExtras *extras;

	PTP_VENDOR_DATA_IN *pDataIn;
	PTP_VENDOR_DATA_OUT *pDataOut;

	DWORD dwActualDataOutSize;
}wia_backend;

HRESULT _ImageEventCallback(IWiaEventCallback *This,const GUID *pEventGUID,BSTR bstrEventDescription,BSTR bstrDeviceID,BSTR bstrDeviceDescription,DWORD dwDeviceType,BSTR bstrFullItemName,ULONG *pulEventType,ULONG ulReserved) {
	PTPLOG("_ImageEventCallback\n");
	return 1;
}
HRESULT _QueryInterface(IWiaEventCallback *This,REFIID riid,void **ppvObject) {
	PTPLOG("_QueryInterface\n");
	return E_NOINTERFACE;
}
ULONG _AddRef(IWiaEventCallback *This) {
	PTPLOG("_AddRef\n");
	return 1;
}
ULONG _Release(IWiaEventCallback *This) {
	PTPLOG("_Release\n");
	return 1;
}

int ptp_device_init(struct PtpRuntime *r) {
	memset(&wia_backend, 0, sizeof(wia_backend));

	// Data sent to device
	wia_backend.pDataIn = (PTP_VENDOR_DATA_IN *)malloc(MAX_DATA_IN);
	if (!wia_backend.pDataIn) {
		return PTP_IO_ERR;
	}

	// Data recieved from device
	wia_backend.pDataOut = (PTP_VENDOR_DATA_OUT *)malloc(MAX_DATA_OUT);
	if (!wia_backend.pDataOut) {
		return PTP_IO_ERR;
	}

	HRESULT x = CoInitialize(NULL);
	PTPLOG("CoInitialize(): %d\n", x);

	x = CoCreateInstance(
		&CLSID_WiaDevMgr,
		NULL,
		CLSCTX_LOCAL_SERVER,
		&IID_IWiaDevMgr,
		(LPVOID *)&wia_backend.wia
	);

	PTPLOG("CoCreateInstance(): %d\n", x);
	if (x != S_OK) return PTP_NO_DEVICE;

#if 0
	// Set up a PTP event listener	
	IWiaEventCallback *callback;
	callback->lpVtbl->ImageEventCallback = _ImageEventCallback;
	callback->lpVtbl->QueryInterface = _QueryInterface;
	callback->lpVtbl->AddRef = _AddRef;
	callback->lpVtbl->Release = _Release;
	x = wia_backend.wia->lpVtbl->RegisterEventCallbackInterface(
		wia_backend.wia,
		0,
		NULL,
		&WIA_EVENT_DEVICE_CONNECTED,
		callback,
		&wia_backend.dev_connected
	);

	PTPLOG("RegisterEventCallbackInterface(): %d\n", x);
	if (x != S_OK) return PTP_NO_DEVICE;
#endif

	x = wia_backend.wia->lpVtbl->EnumDeviceInfo(wia_backend.wia, WIA_DEVINFO_ENUM_LOCAL, &wia_backend.info);
	printf("EnumDeviceInfo(): %d\n", x);
	if (x != S_OK) return PTP_NO_DEVICE;

	int curr = 0;
	while (1) {
		x = wia_backend.info->lpVtbl->Next(wia_backend.info, 1, &wia_backend.storage, NULL);
		printf("Next(): %d\n", x);
		if (x != S_OK) break;
		
		PROPSPEC PropSpec[2] = {0};
		PROPVARIANT PropVar[2] = {0};

		PropSpec[0].ulKind = PRSPEC_PROPID;
		PropSpec[0].propid = WIA_DIP_DEV_ID;

		PropSpec[1].ulKind = PRSPEC_PROPID;
		PropSpec[1].propid = WIA_DIP_DEV_NAME;
		
		x = wia_backend.storage->lpVtbl->ReadMultiple(
			wia_backend.storage, 2, PropSpec, PropVar);
		if (PropVar[0].vt == VT_BSTR)
			printf("Device ID: %ws\n", PropVar[0].bstrVal);
		
		if (PropVar[1].vt == VT_BSTR)
			printf("Device Name: %ws\n", PropVar[1].bstrVal);
		
		PTPLOG("Building item tree...");
		x = wia_backend.wia->lpVtbl->CreateDevice(
			wia_backend.wia, PropVar[0].bstrVal, &wia_backend.item);
		printf("CreateDevice: %d\n", x);

		x = wia_backend.item->lpVtbl->QueryInterface(
			wia_backend.item, &IID_IWiaItemExtras, (void **)&wia_backend.extras);
		printf("QueryInterface: %X\n", x);

		//FreePropVariantArray(2, PropVar);
		return 0;
	}

	return PTP_NO_DEVICE;
}

int ptp_device_close(struct PtpRuntime *r) {
	wia_backend.extras->lpVtbl->Release(wia_backend.extras);
	wia_backend.item->lpVtbl->Release(wia_backend.item);
	wia_backend.storage->lpVtbl->Release(wia_backend.storage);
	wia_backend.info->lpVtbl->Release(wia_backend.info);
	wia_backend.wia->lpVtbl->Release(wia_backend.wia);
	r->active_connection = 0;
	return 0;
}

int ptp_send_bulk_packets(struct PtpRuntime *r, int length) {
	PTPLOG("ptp_send_bulk_packets()\n");
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);

	DWORD dwDataInSize = sizeof(PTP_VENDOR_DATA_IN) - 1;
	DWORD dwDataOutSize = sizeof(PTP_VENDOR_DATA_OUT) - 1 + MAX_DATA_OUT;
	
	printf("Type: %X\n", bulk->type);

	if (bulk->type == PTP_PACKET_TYPE_DATA) {
		if (wia_backend.pDataIn->NextPhase == PTP_NEXTPHASE_READ_DATA) {
			PTPLOG("Data phase after command\n");
		}

		wia_backend.pDataIn->NextPhase = PTP_NEXTPHASE_WRITE_DATA;
	} else if (bulk->type == PTP_PACKET_TYPE_COMMAND) {
		memset(wia_backend.pDataIn, 0, dwDataInSize);
		memset(wia_backend.pDataOut, 0, dwDataOutSize);

		wia_backend.pDataIn->OpCode = bulk->code;
		wia_backend.pDataIn->SessionId = 0;
		wia_backend.pDataIn->TransactionId = 0;

		wia_backend.pDataIn->NextPhase = PTP_NEXTPHASE_READ_DATA;

		wia_backend.pDataIn->NumParams = (bulk->length - 12) / 4;
		wia_backend.pDataOut->Params[0] = bulk->param1;
		wia_backend.pDataOut->Params[1] = bulk->param2;
		wia_backend.pDataOut->Params[2] = bulk->param3;
		wia_backend.pDataOut->Params[3] = bulk->param4;
		wia_backend.pDataOut->Params[4] = bulk->param5;
		
		PTPLOG("Nparams: %d\n", wia_backend.pDataIn->NumParams);
	}

	return length;
}

int ptp_recieve_bulk_packets(struct PtpRuntime *r) {
	PTPLOG("ptp_recieve_bulk_packets()\n");
	struct PtpBulkContainer *bulk = (struct PtpBulkContainer*)(r->data);

	DWORD dwDataInSize = sizeof(PTP_VENDOR_DATA_IN) - 1;
	DWORD dwDataOutSize = sizeof(PTP_VENDOR_DATA_OUT) - 1 + MAX_DATA_OUT;

	HRESULT x = wia_backend.extras->lpVtbl->Escape(
		wia_backend.extras, ESCAPE_PTP_VENDOR_COMMAND,
		(BYTE *)wia_backend.pDataIn, dwDataInSize,
		(BYTE *)wia_backend.pDataOut, dwDataOutSize,
		&wia_backend.dwActualDataOutSize
	);

	PTPLOG("Escape(): %X\n", x);
	if (x != S_OK) {
		return PTP_IO_ERR;
	}

	bulk->length = wia_backend.dwActualDataOutSize - 18;
	bulk->code = wia_backend.pDataOut->ResponseCode;
	bulk->transaction = wia_backend.pDataOut->TransactionId;
	if (bulk->length == 12) {
		bulk->type = PTP_PACKET_TYPE_RESPONSE;
	} else {
		bulk->type = PTP_PACKET_TYPE_DATA;
		
		// Copy in the 
		struct PtpBulkContainer *bulk2 = (struct PtpBulkContainer*)(r->data + bulk->length);
		bulk2->length = 12;
		bulk2->transaction = wia_backend.pDataOut->TransactionId;
		bulk2->type = PTP_PACKET_TYPE_RESPONSE;
		bulk2->code = wia_backend.pDataOut->ResponseCode;
		
		bulk2->param1 = wia_backend.pDataOut->Params[0];
		bulk2->param2 = wia_backend.pDataOut->Params[1];
		bulk2->param3 = wia_backend.pDataOut->Params[2];
		bulk2->param4 = wia_backend.pDataOut->Params[3];
		bulk2->param5 = wia_backend.pDataOut->Params[4];
	}

	PTPLOG("Recieved %d bytes, code %X\n", bulk->length, wia_backend.pDataOut->ResponseCode);

	memcpy((uint8_t *)(bulk) + 12, wia_backend.pDataOut->VendorReadData, bulk->length - 12);

	return 0;
}

int ptp_recieve_int(char *to, int length) {
	return 0;
}
