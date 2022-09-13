// LibUsb Bindings
#include <sys/ioctl.h>
#include <errno.h>
#include <usb.h>
#include <stdio.h>

#include "piclib.h"
#include "ptp.h"

struct PtpLibUsb {
	uint32_t endpoint_in;
	uint32_t endpoint_out;

	int fd;
}ptp_libusb;

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

int ptp_connect() {
	struct usb_device *dev = ptp_search();
	if (dev == NULL) {
		return PTP_NO_DEVICE;
	}

	struct usb_endpoint_descriptor *ep = dev->config->interface->altsetting->endpoint;
	int endpoints = dev->config->interface->altsetting->bNumEndpoints;

	PTPLOG("Device has %d endpoints.", endpoints);

	for (int i = 0; i < endpoints; i++) {
		if (ep[i].bmAttributes == USB_ENDPOINT_TYPE_BULK) {
			if ((ep[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK)) {
				ptp_libusb.endpoint_in = ep[i].bEndpointAddress;
			} else {
				ptp_libusb.endpoint_out = ep[i].bEndpointAddress;
			}
		}
		printf("%X\n", ep[i].bEndpointAddress);
	}

	return 0;
}

#define IOCTL_USB_BULK _IOWR('U', 2, struct usb_bulktransfer)

struct usb_bulktransfer {
	/* keep in sync with usbdevice_fs.h:usbdevfs_bulktransfer */
	unsigned int ep;
	unsigned int len;
	unsigned int timeout; /* in milliseconds */

	/* pointer to data */
	void *data;
};

int ptp_bulk_write(struct PtpRuntime *r, int length) {
}

//int usb_bulk_read() {
	
//}

