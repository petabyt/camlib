#ifndef	CANON_H
#define CANON_H

#define PTP_OC_CANON_ViewFinderOn		0x900b
#define PTP_OC_CANON_ViewFinderOff		0x900c
#define PTP_OC_CANON_GetViewFinderImage	0x901d

#define PTP_OC_EOS_GetViewFinderData	0x9153
#define PTP_OC_EOS_RemoteReleaseOn		0x9128
#define PTP_OC_EOS_RemoteReleaseOff		0x9129

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
