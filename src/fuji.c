#include <string.h>
#include <camlib.h>
#include <ptp.h>

int ptpip_fuji_init(struct PtpRuntime *r, char *device_name) {
	struct FujiInitPacket *p = (struct FujiInitPacket *)r->data;
	memset(p, 0, sizeof(struct FujiInitPacket));
	p->length = 0x52;
	p->type = PTPIP_INIT_COMMAND_REQ;

	p->version = FUJI_PROTOCOL_VERSION;

	p->guid1 = 0x5d48a5ad;
	p->guid2 = 0xb7fb287;
	p->guid3 = 0xd0ded5d3;
	p->guid4 = 0x0;

	ptp_write_unicode_string(p->device_name, device_name);

	if (ptpip_cmd_write(r, r->data, p->length) != p->length) return PTP_IO_ERR;

	// Read the packet size, then recieve the rest
	int x = ptpip_cmd_read(r, r->data, 4);
	if (x < 0) return PTP_IO_ERR;
	x = ptpip_cmd_read(r, r->data + 4, p->length - 4);
	if (x < 0) return PTP_IO_ERR;

	if (ptp_get_return_code(r) == 0x0) {
		return 0;
	} else {
		return PTP_IO_ERR;
	}
}

// Dummy pinger
int ptp_fuji_ping(struct PtpRuntime *r) {
	int rc = ptp_get_prop_value(r, PTP_PC_Fuji_Unlocked);
	return rc;
}

int ptpip_fuji_wait_unlocked(struct PtpRuntime *r) {
	while (1) {
		int rc = ptp_get_prop_value(r, PTP_PC_Fuji_Unlocked);
		if (rc) {
			return rc;
		}

		struct PtpDevPropDesc *pc = (struct PtpDevPropDesc *)(ptp_get_payload(r));
		if (pc->code == 0xFFFF) {
			return PTP_IO_ERR;
		} else if (pc->code == 0x2) {
			return 0;
		}

		CAMLIB_SLEEP(100);
	}
}
