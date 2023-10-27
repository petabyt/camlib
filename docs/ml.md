# Magic Lantern PTP Ext (WIP)

Magic Lantern had a custom PTP opcode, (`0xA1E8`) but it's been left disabled since around 2012. This is a specification
for a new opcode extension for ML, using only opcode `0x1998` (after CHDK's `0x9999`).

## First Parameter
```
#define ML_GetExtVersion   0
#define ML_GetCamInfo      1
#define ML_SendFileInfo    2
#define ML_UploadFile      2
#define ML_GetLiveviewData 3
#define ML_GetMenuBmpData  4
#define ML_GetBmpSpecs     5
```

## ML_GetExtVersion
Returns `uint32_t`: version, currently `0x00000001`

## ML_GetCamInfo
Returns packed struct:
```
struct CamInfo {
	uint8_t cpu_digic;
	uint8_t module_api_ver;
	char build_version[32];
	char build_id[32];
	char build_date[32];
	char build_user[32];
}
```

## ML_SendFileInfo
Similar to `SendObjectInfo`. Requires data phase from initiator:
```
struct {
	uint16_t format_code;
	char filename[32];
}
```

## ML_UploadFile
Requires data phase from initiator as follows:

## ML_GetLiveviewData
Returns unprocessed data stored lvram. Format depends on the DIGIC generation, which can be retrieved from `ML_GetExtVersion`.

## ML_GetMenuBmpData
Returns unprocessed BMP data - stores both Canon and Magic Lantern menus. Format depends on the DIGIC generation.
Each pixel is a single byte. To process this data, `ML_GetBmpSpecs` must be used to get the width, height, and color palette
for each pixel.

## ML_GetBmpSpecs
Returns info on how to process the BMP liveview data, starting with info on the pitch and width:
```
struct Info {
	uint32_t lv_pitch;
	uint32_t lv_width;
	uint32_t palette[256];
}
```
The BMP palette tells which each byte (0-256) maps to in RGBA format.
