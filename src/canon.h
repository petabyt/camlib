#ifndef	CANON_H
#define CANON_H

// Canon EvProc Vendor commands
#define PTP_OC_CANON_ActivateOperations			0x9050
#define PTP_OC_CANON_TerminateEventProc_051		0x9051
#define PTP_OC_CANON_ExecuteEventProc			0x9052
#define PTP_OC_CANON_GetEventProcReturnData		0x9053
#define PTP_OC_CANON_IsEventProcRunning			0x9057
#define PTP_OC_CANON_GetSizeOfTransparentMemory	0x9058
#define PTP_OC_CANON_LoadTransparentMemory		0x9059
#define PTP_OC_CANON_SaveTransparentMemory		0x905a
#define PTP_OC_CANON_QuickLoadTransparentMemory	0x905b
#define PTP_OC_CANON_InitiateEventProc1			0x905c
#define PTP_OC_CANON_TerminateEventProc_05D		0x905d

// Property Codes
#define PTP_PC_CANON_BeepCode		0xd001
#define PTP_PC_CANON_ViewFinderMode	0xd003
#define PTP_PC_CANON_ImageQuality	0xd006
#define PTP_PC_CANON_ImageSize		0xd008
#define PTP_PC_CANON_FlashMode		0xd00a
#define PTP_PC_CANON_TvAvSetting	0xd00c
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
#define PTP_PC_CANON_ViewFinderOut	0xd036
#define PTP_PC_CANON_RealImageWidth	0xd039
#define PTP_PC_CANON_PhotoEffect	0xd040
#define PTP_PC_CANON_AssistLight	0xd041

#endif
