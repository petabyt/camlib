// Bindings to libusb

#include <sys/ioctl.h>
#include <errno.h>
#include <usb.h>
#include <linux/usbdevice_fs.h>
#include <stdio.h>

#include "camlib.h"
#include "ptp.h"
#include "backend.h"

// Apperantly usb-dev_handle must be defined?
struct usb_dev_handle {
	int fd;
};

struct PtpBackend {
	uint32_t endpoint_in;
	uint32_t endpoint_out;

	int fd;
	struct usb_dev_handle *devh;
	struct usb_device *dev;
}ptp_backend;

struct usb_device *ptp_search() {
	PTPLOG("Initializing USB...\n");
	usb_init();
	usb_find_busses();
	usb_find_devices();

	struct usb_bus *bus = usb_get_busses();
	struct usb_device *dev;
	while (bus != NULL) {
		dev = bus->devices;
		while (dev->next != NULL) {
			PTPLOG("Trying %s\n", dev->filename);
			if (dev->config->interface->altsetting->bInterfaceClass == PTP_CLASS_ID) {
				PTPLOG("Found PTP device %s\n", dev->filename)
				return dev;
			}

			dev = dev->next;
		}

		bus = bus->next;
	}

	PTPLOG("No PTP devices found\n");

	return NULL;
}
int ptp_device_init(struct PtpRuntime *r) {
	struct usb_device *dev = ptp_search();
	ptp_backend.dev = dev;
	if (dev == NULL) {
		return PTP_NO_DEVICE;
	}

	r->max_packet_size = 512;

	struct usb_endpoint_descriptor *ep = dev->config->interface->altsetting->endpoint;
	int endpoints = dev->config->interface->altsetting->bNumEndpoints;

	PTPLOG("Device has %d endpoints.\n", endpoints);

	for (int i = 0; i < endpoints; i++) {
		if (ep[i].bmAttributes == USB_ENDPOINT_TYPE_BULK) {
			if ((ep[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK)) {
				ptp_backend.endpoint_in = ep[i].bEndpointAddress;
				PTPLOG("Endpoint IN addr: 0x%X\n", ep[i].bEndpointAddress);
			} else {
				ptp_backend.endpoint_out = ep[i].bEndpointAddress;
				PTPLOG("Endpoint OUT addr: 0x%X\n", ep[i].bEndpointAddress);
			}
		}
	}

	ptp_backend.devh = usb_open(dev);
	if (ptp_backend.devh == NULL) {
		perror("usb_open() failure");
		return PTP_OPEN_FAIL;
	} else {
		if (usb_set_configuration(ptp_backend.devh, dev->config->bConfigurationValue)) {
			perror("usb_set_configuration() failure");
			usb_close(ptp_backend.devh);
			return 1;
		}
		if (usb_claim_interface(ptp_backend.devh, 0)) {
			perror("usb_claim_interface() failure");
			usb_close(ptp_backend.devh);
			return 1;
		}
	}

	r->active_connection = 1;

	return 0;
}

int ptp_device_close(struct PtpRuntime *r) {
	usb_release_interface(ptp_backend.devh, ptp_backend.dev->config->interface->altsetting->bInterfaceNumber);
	usb_reset(ptp_backend.devh);
	usb_close(ptp_backend.devh);
	r->active_connection = 0;
}

void print_bytes(uint8_t *bytes, int n);

int ptp_send_bulk_packets(struct PtpRuntime *r, int length) {
	int x = usb_bulk_write(
		ptp_backend.devh,
		ptp_backend.endpoint_out,
		(char*)r->data, length, PTP_TIMEOUT);
	if (x < 0) {
		perror("usb_bulk_write()");
	}

	PTPLOG("usb_bulk_write(%d, %d)\n", x, ptp_backend.endpoint_out);
	print_bytes(r->data, x);

	return x;
}

int ptp_recieve_bulk_packets(struct PtpRuntime *r) {
	int read = 0;

	while (1) {
		int x = usb_bulk_read(
			ptp_backend.devh,
			ptp_backend.endpoint_in,
			(char*)r->data + read, r->max_packet_size, PTP_TIMEOUT);
		PTPLOG("usb_bulk_read: %d bytes, endpoint %X, %d so far\n", x, ptp_backend.endpoint_in, read);
		read += x;

		if (read >= r->data_length - r->max_packet_size) {
			PTPLOG("Not enough memory");
			return PTP_OUT_OF_MEMORY;
		}

		if (x < 0) {
			PTPLOG("ptp_bulk_read < 0, IO error");
			return PTP_IO_ERROR;
		} else if (x != r->max_packet_size) {
			// No more more packets to read
			return read;
		}

	}

	return read;
}
