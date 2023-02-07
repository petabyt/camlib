// Bindings to libusb
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <sys/ioctl.h>
#include <errno.h>
#include <usb.h>
#include <linux/usbdevice_fs.h>
#include <stdio.h>

#include <camlib.h>
#include <ptp.h>
#include <backend.h>

// Apperantly usb-dev_handle must be defined?
struct usb_dev_handle {
	int fd;
};

struct PtpBackend {
	uint32_t endpoint_in;
	uint32_t endpoint_out;
	uint32_t endpoint_int;

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
		while (dev != NULL) {
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
	PTPLOG("Vendor ID: %d, Product ID: %d\n", dev->descriptor.idVendor, dev->descriptor.idProduct);

	for (int i = 0; i < endpoints; i++) {
		if (ep[i].bmAttributes == USB_ENDPOINT_TYPE_BULK) {
			if ((ep[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK)) {
				ptp_backend.endpoint_in = ep[i].bEndpointAddress;
				PTPLOG("Endpoint IN addr: 0x%X\n", ep[i].bEndpointAddress);
			} else {
				ptp_backend.endpoint_out = ep[i].bEndpointAddress;
				PTPLOG("Endpoint OUT addr: 0x%X\n", ep[i].bEndpointAddress);
			}
		} else {
			if (ep[i].bmAttributes == USB_ENDPOINT_TYPE_INTERRUPT) {
				ptp_backend.endpoint_int = ep[i].bEndpointAddress;
				PTPLOG("Endpoint INT addr: 0x%X\n", ep[i].bEndpointAddress);	
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
			return PTP_OPEN_FAIL;
		}
		if (usb_claim_interface(ptp_backend.devh, 0)) {
			perror("usb_claim_interface() failure");
			usb_close(ptp_backend.devh);
			return PTP_OPEN_FAIL;
		}
	}

	r->active_connection = 1;

	return 0;
}

int ptp_device_close(struct PtpRuntime *r) {
	if (usb_release_interface(ptp_backend.devh, ptp_backend.dev->config->interface->altsetting->bInterfaceNumber)) {
		return 1;
	}

	if (usb_reset(ptp_backend.devh)) {
		return 1;
	}

	if (usb_close(ptp_backend.devh)) {
		return 1;
	}

	r->active_connection = 0;

	return 0;
}

int ptp_device_reset(struct PtpRuntime *r) {
	return usb_control_msg(ptp_backend.devh, USB_TYPE_CLASS | USB_RECIP_INTERFACE, USB_REQ_RESET, 0, 0, NULL, 0, PTP_TIMEOUT);
}

int ptp_send_bulk_packet(void *to, int length) {
	return usb_bulk_write(
		ptp_backend.devh,
		ptp_backend.endpoint_out,
		(char *)to, length, PTP_TIMEOUT);
}

int ptp_recieve_bulk_packet(void *to, int length) {
	return usb_bulk_read(
		ptp_backend.devh,
		ptp_backend.endpoint_in,
		(char *)to, length, PTP_TIMEOUT);
}

int ptp_recieve_int(void *to, int length) {
	int x = usb_bulk_read(
		ptp_backend.devh,
		ptp_backend.endpoint_int,
		(char *)to, length, 10);

	// Error generally means pipe is empty
	if (x == -110 || x == -16 || x == -5) {
		return 0;
	} else if (x == -19) {
		return PTP_IO_ERR;
	}

	return x;
}

int reset_int() {
	return usb_control_msg(ptp_backend.devh, USB_RECIP_ENDPOINT, USB_REQ_CLEAR_FEATURE,
		0, ptp_backend.endpoint_int, NULL, 0, PTP_TIMEOUT);
}
