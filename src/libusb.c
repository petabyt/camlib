// LibUsb Bindings
#include <sys/ioctl.h>
#include <errno.h>
#include <usb.h>

struct libusb_bulk {
	uint32_t endpoint;
	uint32_t len;
	uint32_t timeout;
	void *data;
};

struct PtpLibUsb {
	uint32_t endpoint_in;
	uint32_t endpoint_out;

	int fd;
}ptp_libusb;

int ptp_bulk_write(struct PtpRuntime *r, int length) {
	struct libusb_bulk bulk;
	bulk.endpoint = ptp_libusb.endpoint_out;
	bulk.timeout = PTP_TIMEOUT;

	int sent = 0;

	while (length < sent) {
		bulk.len = length - sent;
		if (bulk.len > r->max_packet_size) {
			bulk.len = r->max_packet_size;
		}

		bulk.data = r->data + sent;

		if (ioctl(ptp_libusb.fd, IOCTL_USB_BULK, &bulk) < 0) {
			return -1;
		}
	}

	return 0;
}

int usb_bulk_read() {
	
}

