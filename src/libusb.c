// Bindings to libusb
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <errno.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>

#include <camlib.h>
#include <ptp.h>
#include <ptpbackend.h>

struct PtpBackend {
	uint32_t endpoint_in;
	uint32_t endpoint_out;
	uint32_t endpoint_int;

	int fd;
	libusb_context *ctx;
	libusb_device_handle *handle;
}ptp_backend = {
	0, 0, 0, 0, NULL, NULL
};

int ptp_device_init(struct PtpRuntime *r) {
	PTPLOG("Initializing USB...\n");
	libusb_init(&ptp_backend.ctx);

	libusb_device **list;
	ssize_t count = libusb_get_device_list(ptp_backend.ctx, &list);

	struct libusb_device_descriptor desc;
	struct libusb_config_descriptor *config;
	const struct libusb_interface *interf;
	const struct libusb_interface_descriptor *interf_desc;

	libusb_device *dev = NULL;
	for (int i = 0; i < (int)count; i++) {
		int r = libusb_get_device_descriptor(list[i], &desc);

		if (desc.bNumConfigurations == 0) {
			continue;
		} 

		r = libusb_get_config_descriptor(list[i], 0, &config);
		if (config->bNumInterfaces == 0) {
			continue;
		}

		interf = &config->interface[0];
		if (interf->num_altsetting == 0) {
			continue;
		}

		interf_desc = &interf->altsetting[0];

		PTPLOG("Vendor ID: %d, Product ID: %d, Class %d\n",
			desc.idVendor, desc.idProduct, interf_desc->bInterfaceClass);

		if (interf_desc->bInterfaceClass == LIBUSB_CLASS_IMAGE) {
			dev = list[i];

			break;
		}

		libusb_free_config_descriptor(config);
	}

	if (dev == NULL) {
		return PTP_NO_DEVICE;
	}

	r->max_packet_size = 512;

	struct libusb_endpoint_descriptor *ep = interf_desc->endpoint;
	int endpoints = interf_desc->bNumEndpoints;

	for (int i = 0; i < endpoints; i++) {
		if (ep[i].bmAttributes == LIBUSB_ENDPOINT_TRANSFER_TYPE_BULK) {
			if (ep[i].bEndpointAddress & LIBUSB_ENDPOINT_IN) {
				ptp_backend.endpoint_in = ep[i].bEndpointAddress;
				PTPLOG("Endpoint IN addr: 0x%X\n", ep[i].bEndpointAddress);
			} else {
				if (ep[i].bEndpointAddress == 0) break; // ?
				ptp_backend.endpoint_out = ep[i].bEndpointAddress;
				PTPLOG("Endpoint OUT addr: 0x%X\n", ep[i].bEndpointAddress);
			}
		} else if (ep[i].bmAttributes == LIBUSB_ENDPOINT_TRANSFER_TYPE_INTERRUPT) {
			ptp_backend.endpoint_int = ep[i].bEndpointAddress;
			PTPLOG("Endpoint INT addr: 0x%X\n", ep[i].bEndpointAddress);	
		}
	}

	libusb_free_config_descriptor(config);
	int rc = libusb_open(dev, &ptp_backend.handle);
	libusb_free_device_list(list, 0);
	if (rc) {
		perror("usb_open() failure");
		return PTP_OPEN_FAIL;
	} else {
		if (libusb_set_auto_detach_kernel_driver(ptp_backend.handle, 0)) {
			perror("libusb_set_auto_detach_kernel_driver");
			return PTP_OPEN_FAIL;
		}

		// if (libusb_set_configuration(ptp_backend.handle, 1)) {
			// perror("usb_set_configuration() failure");
			// libusb_close(ptp_backend.handle);
			// return PTP_OPEN_FAIL;
		// }

		if (libusb_claim_interface(ptp_backend.handle, 0)) {
			perror("usb_claim_interface() failure");
			libusb_close(ptp_backend.handle);
			return PTP_OPEN_FAIL;
		}
	}

	r->active_connection = 1;

	return 0;
}

int ptp_device_close(struct PtpRuntime *r) {
	if (libusb_release_interface(ptp_backend.handle, 0)) {
		return 1;
	}

	libusb_close(ptp_backend.handle);

	r->active_connection = 0;

	return 0;
}

int ptp_device_reset(struct PtpRuntime *r) {
	return -1;
}

int ptp_send_bulk_packet(void *to, int length) {
	int transferred;
	int r = libusb_bulk_transfer(
		ptp_backend.handle,
		ptp_backend.endpoint_out,
		(unsigned char *)to, length, &transferred, PTP_TIMEOUT);
	if (r) {
		return -1;
	}

	return transferred;
}

int ptp_recieve_bulk_packet(void *to, int length) {
	int transferred;
	int r = libusb_bulk_transfer(
		ptp_backend.handle,
		ptp_backend.endpoint_in,
		(unsigned char *)to, length, &transferred, PTP_TIMEOUT);
	if (r) {
		return -1;
	}

	return transferred;
}

int ptp_recieve_int(void *to, int length) {
	int transferred;
	int r = libusb_bulk_transfer(
		ptp_backend.handle,
		ptp_backend.endpoint_out,
		(unsigned char *)to, length, &transferred, 10);
	if (r == LIBUSB_ERROR_NO_DEVICE) {
		return PTP_IO_ERR;
	} else if (r == LIBUSB_ERROR_TIMEOUT) {
		return 0;
	}

	return transferred;
}

int reset_int() {
	return -1;
}
