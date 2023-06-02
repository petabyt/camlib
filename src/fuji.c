#include <string.h>
#include <camlib.h>
#include <ptp.h>

#include <fuji.h>

int ptp_fuji_init(struct PtpRuntime *r, char *device_name) {
	struct FujiInitPacket *p = (struct FujiInitPacket *)r->data;
	memset(p, 0, sizeof(struct FujiInitPacket));
	p->length = 0x52;
	p->type = PTPIP_INIT_COMMAND_REQ;

	p->version = FUJI_PROTOCOL_VERSION;

	p->guid1 = 0x5d48a5ad;
	p->guid2 = 0xb7fb287;
	p->guid3 = 0xd0ded5d3;
	p->guid4 = 0x0;

	int i;
	for (i = 0; device_name[i] != '\0'; i++) {
		p->device_name[i * 2] = device_name[i];
		p->device_name[i * 2 + 1] = '\0';
	}
	p->device_name[i * 2 + 1] = '\0';

	if (ptpip_send_bulk_packet(r, r->data, p->length) != p->length) return PTP_IO_ERR;

	// Read the packet size, then recieve the rest
	int x = ptpip_recieve_bulk_packet(r, r->data, 4);
	if (x < 0) return PTP_IO_ERR;
	x = ptpip_recieve_bulk_packet(r, r->data + 4, p->length - 4);
	if (x < 0) return PTP_IO_ERR;

	if (ptp_get_return_code(r) == 0x0) {
		return 0;
	} else {
		return PTP_IO_ERR;
	}
}

int ptp_fuji_wait_unlocked(struct PtpRuntime *r) {
	while (1) {
		ptp_get_prop_value(r, PTP_PC_Fuji_Unlocked);

		struct PtpDevPropDesc *pc = (struct PtpDevPropDesc *)(ptp_get_payload(r));
		if (pc->code == 0x0) {
			// retry
		} else if (pc->code == 0xFFFF) {
			return PTP_IO_ERR;
		} else if (pc->code == 0x2) {
			return 0;
		}

		CAMLIB_SLEEP(1000);
	}
}
