#ifndef	CANON_H
#define CANON_H

// Non EOS (point and shoot) operation codes
#define PTP_OC_CANON_ViewFinderOn		0x900b
#define PTP_OC_CANON_ViewFinderOff		0x900c
#define PTP_OC_CANON_GetViewFinderImage	0x901d

// EOS specific
#define PTP_OC_EOS_InitiateViewfinder	0x9151
#define PTP_OC_EOS_TerminateViewfinder	0x9152
#define PTP_OC_EOS_GetViewFinderData	0x9153
#define PTP_OC_EOS_RemoteReleaseOn		0x9128
#define PTP_OC_EOS_RemoteReleaseOff		0x9129
#define PTP_OC_EOS_SetDevicePropValueEx	0x9110

// Property Codes
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
#define PTP_PC_CANON_EOS_VF_Output	0xD1B0
#define PTP_PC_CANON_EOS_EVFMode	0xD1B1
#define PTP_PC_CANON_EOS_DOFPrev	0xD1B2
#define PTP_PC_CANON_EOS_VFSharp	0xD1B3
#define PTP_PC_EOS_CaptureDest		0xD11C

#endif
