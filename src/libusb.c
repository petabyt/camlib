// Bindings to libusb v1.0 (2008)
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <errno.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>

#include <camlib.h>
#include <ptp.h>

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
		int rc = libusb_get_device_descriptor(list[i], &desc);
		if (rc) {
			perror("libusb_get_device_descriptor");
			return PTP_NO_DEVICE;
		}

		if (desc.bNumConfigurations == 0) {
			continue;
		} 

		rc = libusb_get_config_descriptor(list[i], 0, &config);
		if (rc) {
			perror("libusb_get_config_descriptor");
			return PTP_NO_DEVICE;
		}

		if (config->bNumInterfaces == 0) {
			continue;
		}

		interf = &config->interface[0];
		if (interf->num_altsetting == 0) {
			continue;
		}

		interf_desc = &interf->altsetting[0];

		PTPLOG("Vendor ID: %X, Product ID: %X\n", desc.idVendor, desc.idProduct);

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

	const struct libusb_endpoint_descriptor *ep = interf_desc->endpoint;
	for (int i = 0; i < interf_desc->bNumEndpoints; i++) {
		if (ep[i].bmAttributes == LIBUSB_ENDPOINT_TRANSFER_TYPE_BULK) {
			if (ep[i].bEndpointAddress & LIBUSB_ENDPOINT_IN) {
				ptp_backend.endpoint_in = ep[i].bEndpointAddress;
				PTPLOG("Endpoint IN addr: 0x%X\n", ep[i].bEndpointAddress);
			} else {
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
	if (ptp_backend.handle == NULL) return -1;
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
	if (ptp_backend.handle == NULL) return -1;
	int transferred = 0;
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
	int transferred = 0;
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
