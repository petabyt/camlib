// This file implements PTP (ISO 15740)
#ifndef PTP_H
#define PTP_H

#include <stdint.h>

struct PtpBulkContainer {
	uint32_t length; // length of packet, in bytes
	uint16_t type; // See PACKET_TYPE_*
	uint16_t code; // See PTP_OC_*
	uint32_t transaction;

	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
	uint32_t param4;
	uint32_t param5;

	// Payload data follows, if any
};

struct PtpEventContainer {
	uint32_t length;
	uint16_t type;
	uint16_t code;
	uint32_t transaction;
	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
};

// Standard PTP Operation Codes (OC)
#define PTP_OC_GetDeviceInfo		0x1001
#define PTP_OC_OpenSession			0x1002
#define PTP_OC_CloseSession			0x1003
#define PTP_OC_GetStorageIDs		0x1004
#define PTP_OC_GetNumObjects		0x1006
#define PTP_OC_GetObjectHandles		0x1007
#define PTP_OC_GetObjectInfo		0x1008
#define PTP_OC_GetObject			0x1009
#define PTP_OC_GetThumb				0x100A
#define PTP_OC_DeleteObject			0x100B
#define PTP_OC_SendObjectInfo		0x100C
#define PTP_OC_SendObject			0x100D
#define PTP_OC_InitiateCapture		0x100E
#define PTP_OC_FormatStore			0x100F
#define PTP_OC_ResetDevice			0x1010
#define PTP_OC_SelfTest				0x1011
#define PTP_OC_SetObjectProtection	0x1012
#define PTP_OC_PowerDown			0x1013
#define PTP_OC_GetDevicePropDesc	0x1014
#define PTP_OC_GetDevicePropValue	0x1015
#define PTP_OC_SetDevicePropValue	0x1016
#define PTP_OC_ResetDevicePropValue	0x1017
#define PTP_OC_TerminateOpenCapture	0x1018
#define PTP_OC_MoveObject			0x1019
#define PTP_OC_CopyObject			0x101A
#define PTP_OC_GetPartialObject		0x101B
#define PTP_OC_InitiateOpenCapture	0x101C

// Return codes (RC)
#define PTP_RC_Undefined				0x2000
#define PTP_RC_OK						0x2001
#define PTP_RC_GeneralError				0x2002
#define PTP_RC_SessionNotOpen			0x2003
#define PTP_RC_InvalidTransactionID		0x2004
#define PTP_RC_OperationNotSupported	0x2005
#define PTP_RC_ParameterNotSupported	0x2006
#define PTP_RC_IncompleteTransfer		0x2007
#define PTP_RC_InvalidStorageId			0x2008
#define PTP_RC_InvalidObjectHandle		0x2009
#define PTP_RC_DevicePropNotSupported	0x200A
#define PTP_RC_InvalidObjectFormatCode	0x200B
#define PTP_RC_StoreFull				0x200C
#define PTP_RC_ObjectWriteProtected		0x200D
#define PTP_RC_StoreReadOnly			0x200E
#define PTP_RC_AccessDenied				0x200F
#define PTP_RC_NoThumbnailPresent		0x2010
#define PTP_RC_SelfTestFailed			0x2011
#define PTP_RC_PartialDeletion			0x2012
#define PTP_RC_StoreNotAvailable		0x2013
#define PTP_RC_SpecByFormatUnsupported	0x2014
#define PTP_RC_NoValidObjectInfo		0x2015
#define PTP_RC_InvalidCodeFormat		0x2016
#define PTP_RC_UnknownVendorCode		0x2017
#define PTP_RC_CaptureAlreadyTerminated	0x2018
#define PTP_RC_DeviceBusy				0x2019
#define PTP_RC_InvalidParentObject		0x201A
#define PTP_RC_InvalidDevicePropFormat	0x201B
#define PTP_RC_InvalidDevicePropValue	0x201C
#define PTP_RC_InvalidParameter			0x201D
#define PTP_RC_SessionAlreadyOpened		0x201E
#define PTP_RC_TransactionCanceled		0x201F
#define PTP_RC_SpecOfDestinationUnsupported	0x2020

#define PTP_OC_FUJI_SendObjectInfo	0x900C
#define PTP_OC_FUJI_Unknown1		0x900D
#define PTP_OC_FUJI_SendObject		0x901D

// Event Codes (EC)
#define PTP_EC_Undefined			0x4000
#define PTP_EC_CancelTransaction	0x4001
#define PTP_EC_ObjectAdded			0x4002
#define PTP_EC_ObjectRemoved		0x4003
#define PTP_EC_StoreAdded			0x4004
#define PTP_EC_StoreRemoved			0x4005
#define PTP_EC_DevicePropChanged	0x4006
#define PTP_EC_ObjectInfoChanged	0x4007
#define PTP_EC_DeviceInfoChanged	0x4008
#define PTP_EC_RequestObjectTransfer 0x4009
#define PTP_EC_StoreFull			0x400A
#define PTP_EC_DeviceReset			0x400B
#define PTP_EC_StorageInfoChanged	0x400C
#define PTP_EC_CaptureComplete		0x400D
#define PTP_EC_UnreportedStatus		0x400E

// Object Formats (OF)
#define PTP_OF_Undefined		0x3000
#define PTP_OF_Association		0x3001
#define PTP_OF_Script			0x3002
#define PTP_OF_Executable		0x3003
#define PTP_OF_Text				0x3004
#define PTP_OF_HTML				0x3005
#define PTP_OF_DPOF				0x3006
#define PTP_OF_AIFF				0x3007
#define PTP_OF_WAV				0x3008
#define PTP_OF_MP3				0x3009
#define PTP_OF_AVI				0x300A
#define PTP_OF_MPEG				0x300B
#define PTP_OF_ASF				0x300C
#define PTP_OF_UndefinedImage	0x300D
#define PTP_OF_EXIF				0x300E
#define PTP_OF_TIFF_EP			0x300F
#define PTP_OF_FlashPix			0x3010
#define PTP_OF_BMP				0x3011
#define PTP_OF_CIFF				0x3012
#define PTP_OF_Reserved0		0x3013
#define PTP_OF_GIF				0x3014
#define PTP_OF_JFIF				0x3015
#define PTP_OF_CD				0x3016
#define PTP_OF_PICT				0x3017
#define PTP_OF_PNG				0x3018
#define PTP_OF_Reserved1		0x3019
#define PTP_OF_TIFF_IT			0x301A
#define PTP_OF_JP2				0x301B
#define PTP_OF_JPX				0x301C
#define PTP_OF_Firmware			0xb802
#define PTP_OF_WIF				0xb881
#define PTP_OF_Audio			0xb900
#define PTP_OF_WMA				0xb901
#define PTP_OF_OGG				0xb902
#define PTP_OF_AAC				0xb903
#define PTP_OF_Audible			0xb904
#define PTP_OF_FLAC				0xb906
#define PTP_OF_SamsungPlaylist	0xb909
#define PTP_OF_Video			0xb980
#define PTP_OF_WMV				0xb981
#define PTP_OF_MP4				0xb982
#define PTP_OF_MP2				0xb983
#define PTP_OF_3GP				0xb984


// Vendor init/USB codes
#define VENDOR_CANON 1193

// PTP Packet types
#define PACKET_TYPE_COMMAND 1
#define PACKET_TYPE_DATA 2
#define PACKET_TYPE_RESPONSE 3
#define PACKET_TYPE_EVENT 4

#endif
