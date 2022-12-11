// This file implements Picture Transfer Protocol (ISO 15740)
// Written by Daniel Cook, licensed under GPL2.0 or later

#ifndef PTP_H
#define PTP_H

#include <stdint.h>

#pragma pack(push, 1)

// PTP Packet container types
#define PTP_PACKET_TYPE_COMMAND 	0x1
#define PTP_PACKET_TYPE_DATA		0x2
#define PTP_PACKET_TYPE_RESPONSE	0x3
#define PTP_PACKET_TYPE_EVENT		0x4

// TODO: convert to params[5]; ?

struct PtpBulkContainer {
	uint32_t length; // length of packet, in bytes
	uint16_t type; // See PACKET_TYPE_*
	uint16_t code; // See PTP_OC_*
	uint32_t transaction;

	// Parameters are only included in command packets,
	// Skipped in data packets
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
#define PTP_OC_GetStorageInfo		0x1005
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

#define PTP_OC_NIKON_Capture		0x90C0

// Non EOS (Canon point and shoot) operation codes
#define PTP_OC_CANON_ViewFinderOn		0x900b
#define PTP_OC_CANON_ViewFinderOff		0x900c
#define PTP_OC_CANON_GetViewFinderImage	0x901d
#define PTP_OC_CANON_LockUI				0x9004
#define PTP_OC_CANON_UnlockUI			0x9005

// EOS specific
#define PTP_OC_EOS_GetStorageIDs		0x9101
#define PTP_OC_EOS_InitiateViewfinder	0x9151
#define PTP_OC_EOS_TerminateViewfinder	0x9152
#define PTP_OC_EOS_GetViewFinderData	0x9153
#define PTP_OC_EOS_RemoteReleaseOn		0x9128
#define PTP_OC_EOS_RemoteReleaseOff		0x9129
#define PTP_OC_EOS_SetDevicePropValueEx	0x9110
#define PTP_OC_EOS_PCHDDCapacity		0x911A
#define PTP_OC_EOS_SetEventMode			0x9115
#define PTP_OC_EOS_SetRemoteMode		0x9114
#define PTP_OC_EOS_DriveLens			0x9155
#define PTP_OC_EOS_KeepDeviceOn			0x911D
#define PTP_OC_EOS_GetEvent				0x9116
#define PTP_OC_EOS_GetDevicePropValue	0x9127

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

// MTP (Microsoft) extension
#define PTP_RC_UndefinedMTP				0xA800
#define PTP_RC_InvalidObjPropCode		0xA801
#define PTP_RC_InvalidObjPropCodeFormat	0xA802
#define PTP_RC_InvalidObjPropCodeValue	0xA803
#define PTP_RC_InvalidObjReference		0xA804
#define PTP_RC_InvalidDataset			0xA806
#define PTP_RC_GroupSpecUnsupported		0xA807
#define PTP_RC_DepthSpecUnsupported		0xA808
#define PTP_RC_ObjectTooLarge			0xA809
#define PTP_RC_ObjectPropUnsupported	0xA80A

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

/* Canon EOS events */
#define PTP_EC_EOS_RequestGetEvent			0xC101
#define PTP_EC_EOS_ObjectAddedEx			0xC181
#define PTP_EC_EOS_ObjectRemoved			0xC182
#define PTP_EC_EOS_RequestGetObjectInfoEx	0xC183
#define PTP_EC_EOS_StorageStatusChanged		0xC184
#define PTP_EC_EOS_StorageInfoChanged		0xC185
#define PTP_EC_EOS_RequestObjectTransfer	0xC186
#define PTP_EC_EOS_ObjectInfoChangedEx		0xC187
#define PTP_EC_EOS_ObjectContentChanged		0xC188
#define PTP_EC_EOS_PropValueChanged			0xC189
#define PTP_EC_EOS_AvailListChanged			0xC18A
#define PTP_EC_EOS_CameraStatusChanged		0xC18B
#define PTP_EC_EOS_WillSoonShutdown			0xC18D
#define PTP_EC_EOS_ShutdownTimerUpdated		0xC18E
#define PTP_EC_EOS_RequestCancelTransfer	0xC18F
#define PTP_EC_EOS_RequestObjectTransferDT	0xC190
#define PTP_EC_EOS_RequestCancelTransferDT	0xC191
#define PTP_EC_EOS_StoreAdded				0xC192
#define PTP_EC_EOS_StoreRemoved				0xC193
#define PTP_EC_EOS_BulbExposureTime			0xC194
#define PTP_EC_EOS_RecordingTime			0xC195
#define PTP_EC_EOS_RequestObjectTransferTS	0xC1A2
#define PTP_EC_EOS_AfResult					0xC1A3

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
#define PTP_OF_3GP				0xb984
#define PTP_OF_MP2				0xb983

#define PTP_OF_CANON_CRW	0xb101
#define PTP_OF_CANON_CR2	0xb103
#define PTP_OF_CANON_MOV	0xb104

// Association types
#define PTP_AT_Folder	0x1
#define PTP_AT_Album	0x1

// Standard Property codes
#define PTP_PC_BatteryLevel		0x5001
#define PTP_PC_FunctionalMode	0x5002
#define PTP_PC_FocalLength		0x5008
#define PTP_PC_FocalDistance	0x5009
#define PTP_PC_FocusMode		0x500A
#define PTP_PC_DateTime			0x5011

// Canon (Not EOS) Property Codes
#define PTP_PC_CANON_BeepCode		0xD001
#define PTP_PC_CANON_ViewFinderMode	0xD003
#define PTP_PC_CANON_ImageQuality	0xD006
#define PTP_PC_CANON_ImageSize		0xD008
#define PTP_PC_CANON_FlashMode		0xD00a
#define PTP_PC_CANON_TvAvSetting	0xD00c
#define PTP_PC_CANON_MeteringMode	0xd010
#define PTP_PC_CANON_MacroMode		0xd011
#define PTP_PC_CANON_FocusingPoint	0xd012
#define PTP_PC_CANON_WhiteBalance	0xd013
#define PTP_PC_CANON_ISOSpeed		0xd01c
#define PTP_PC_CANON_Aperture		0xd01c
#define PTP_PC_CANON_ShutterSpeed	0xd01e
#define PTP_PC_CANON_ExpComp		0xd01f
#define PTP_PC_CANON_Zoom			0xd02a
#define PTP_PC_CANON_SizeQuality	0xd02c
#define PTP_PC_CANON_FlashMemory	0xd031
#define PTP_PC_CANON_CameraModel	0xd032
#define PTP_PC_CANON_CameraOwner	0xd033
#define PTP_PC_CANON_UnixTime		0xd032
#define PTP_PC_CANON_ViewFinderOut	0xD036
#define PTP_PC_CANON_RealImageWidth	0xD039
#define PTP_PC_CANON_PhotoEffect	0xD040
#define PTP_PC_CANON_AssistLight	0xD041

// EOS Device Property Codes
#define PTP_PC_EOS_Aperture				0xD101
#define PTP_PC_EOS_ShutterSpeed			0xD102
#define PTP_PC_EOS_ISOSpeed				0xD103
#define PTP_PC_EOS_ExpCompensation		0xD104
#define PTP_PC_EOS_AutoExposureMode		0xD105
#define PTP_PC_EOS_DriveMode			0xD106
#define PTP_PC_EOS_MeteringMode			0xD107 
#define PTP_PC_EOS_FocusMode			0xD108
#define PTP_PC_EOS_WhiteBalance			0xD109
#define PTP_PC_EOS_ColorTemperature		0xD10A
#define PTP_PC_EOS_WhiteBalanceAdjustA	0xD10B
#define PTP_PC_EOS_WhiteBalanceAdjustB	0xD10C
#define PTP_PC_EOS_WhiteBalanceXA		0xD10D
#define PTP_PC_EOS_WhiteBalanceXB		0xD10E
#define PTP_PC_EOS_ColorSpace			0xD10F
#define PTP_PC_EOS_PictureStyle			0xD110
#define PTP_PC_EOS_BatteryPower			0xD111
#define PTP_PC_EOS_BatterySelect		0xD112
#define PTP_PC_EOS_CameraTime			0xD113
#define PTP_PC_EOS_Owner				0xD115
#define PTP_PC_EOS_ModelID				0xD116
#define PTP_PC_EOS_PTPExtensionVersion	0xD119
#define PTP_PC_EOS_DPOFVersion			0xD11A
#define PTP_PC_EOS_AvailableShots		0xD11B
#define PTP_PC_EOS_CaptureDestination	0xD11C
#define PTP_PC_EOS_ImageFormat			0xD120
#define PTP_PC_EOS_ImageFormatCF		0xD121
#define PTP_PC_EOS_ImageFormatSD		0xD122
#define PTP_PC_EOS_ImageFormatExtHD		0xD123
#define PTP_PC_EOS_VF_Output			0xD1B0
#define PTP_PC_EOS_EVFMode				0xD1B1
#define PTP_PC_EOS_DOFPreview			0xD1B2
#define PTP_PC_EOS_VFSharp				0xD1B3

// Storage Types
#define PTP_ST_Undefined	0x0
#define PTP_ST_FixedROM		0x1
#define PTP_ST_RemovableROM	0x2
#define PTP_ST_FixedRAM		0x3
#define PTP_ST_RemovableRAM	0x4

// Filesystem Type
#define PTP_FT_Undefined	0x0
#define PTP_FT_GenericFlat	0x1
#define PTP_FT_GenericHei	0x2
#define PTP_FT_DCF			0x3

// Access Capability
#define PTP_AC_ReadWrite	0x0
#define PTP_AC_Read			0x1
#define PTP_AC_ReadDelete	0x2

// Device type codes
#define PTP_TC_UNDEF	0x0
#define PTP_TC_INT8		0x1
#define PTP_TC_UINT8	0x2
#define PTP_TC_INT16	0x3
#define PTP_TC_UINT16	0x4
#define PTP_TC_INT32	0x5
#define PTP_TC_UINT32	0x6
#define PTP_TC_INT64	0x7
#define PTP_TC_UINT64	0x8
#define PTP_TC_INT128	0x9
#define PTP_TC_UINT128	0xA
#define PTP_TC_UINT8ARRAY	0x4002
#define PTP_TC_UINT16ARRAY	0x4004
#define PTP_TC_UINT32ARRAY	0x4006
#define PTP_TC_UINT64ARRAY	0x4008
#define PTP_TC_STRING	0xFFFF

#define PTPIP_INIT_COMMAND_REQ	0x1
#define PTPIP_INIT_COMMAND_ACK	0x2
#define PTPIP_INIT_EVENT_REQ	0x3
#define PTPIP_INIT_EVENT_ACK	0x4
#define PTPIP_INIT_FAIL			0x5
#define PTPIP_COMMAND_REQUEST	0x6
#define PTPIP_COMMAND_RESPONSE	0x7
#define PTPIP_EVENT				0x8
#define PTPIP_DATA_PACKET_START	0x9
#define PTPIP_DATA_PACKET		0xA
#define PTPIP_CANCEL_TRANSACTION	0xB
#define PTPIP_DATA_PACKET_END	0xC
#define PTPIP_PING				0xD
#define PTPIP_PONG				0xE

// Standard interface Class ID for PTP.
// See https://en.wikipedia.org/wiki/USB#Device_classes
#define PTP_CLASS_ID 6

// ISO number for PTP/IP, seems to be standard (?)
#define PTP_IP_PORT 15740

// Vendor init/USB codes, not specifically PTP
#define USB_REQ_RESET			0x66
#define USB_REQ_STATUS			0x67
#define USB_REQ_GET_STATUS		0x00
#define USB_REQ_CLEAR_FEATURE	0x01
#define USB_REQ_SET_FEATURE		0x03

// Data phase host to device, device to host
#define USB_DP_HTD				0x0
#define USB_DP_DTH				0x80

#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#ifndef USB_TYPE_CLASS
#define USB_TYPE_CLASS 0x20
#endif

#pragma pack(pop)

#endif
