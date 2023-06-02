#ifndef FUJI_H
#define FUJI_H

#include <stdint.h>

#pragma pack(push, 1)

#define FUJI_PROTOCOL_VERSION 0x8f53e4f2

struct FujiInitPacket {
	uint32_t length;
	uint32_t type;
	uint32_t version;
	uint32_t guid1;
	uint32_t guid2;
	uint32_t guid3;
	uint32_t guid4;
	char device_name[54];
};

#define PTP_OC_FUJI_SendObjectInfo	0x900c
#define PTP_OC_FUJI_SendObject		0x901d

#define PTP_PC_Fuji_Unlocked 0xd212
#define PTP_PC_Fuji_TransferMode 0xdf22
#define PTP_PC_Fuji_Mode 0xdf01
#define PTP_MODE_PHOTO 0x200


#pragma pack(pop)

#endif
