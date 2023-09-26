// Bindings to libusb v1.0 (2008)
// Copyright 2022 by Daniel C (https://github.com/petabyt/camlib)

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>
#include <string.h>

#include <camlib.h>
#include <ptp.h>

// Private struct
struct LibUSBBackend {
	uint32_t endpoint_in;
	uint32_t endpoint_out;
	uint32_t endpoint_int;

	int fd;
	libusb_context *ctx;
	libusb_device_handle *handle;
};

int ptpusb_device_list(struct PtpRuntime *r) {
	
}

int ptp_device_init(struct PtpRuntime *r) {
	ptp_generic_reset(r);
	r->comm_backend = malloc(sizeof(struct LibUSBBackend));
	memset(r->comm_backend, 0, sizeof(struct LibUSBBackend));

	struct LibUSBBackend *backend = (struct LibUSBBackend *)r->comm_backend;

	ptp_verbose_log("Initializing USB...\n");
	libusb_init(r->comm_backend);

	libusb_device **list;
	ssize_t count = libusb_get_device_list(backend->ctx, &list);

	// TODO: allow API to loop through different devices

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

		ptp_verbose_log("Vendor ID: %X, Product ID: %X\n", desc.idVendor, desc.idProduct);

		if (interf_desc->bInterfaceClass == LIBUSB_CLASS_IMAGE) {
			dev = list[i];
			break;
		}

		libusb_free_config_descriptor(config);
	}

	if (dev == NULL) {
		return PTP_NO_DEVICE;
	}

	// libusb 1.0 has no specificed limit for reads/writes
	r->max_packet_size = 512*4;

	const struct libusb_endpoint_descriptor *ep = interf_desc->endpoint;
	for (int i = 0; i < interf_desc->bNumEndpoints; i++) {
		if (ep[i].bmAttributes == LIBUSB_ENDPOINT_TRANSFER_TYPE_BULK) {
			if (ep[i].bEndpointAddress & LIBUSB_ENDPOINT_IN) {
				backend->endpoint_in = ep[i].bEndpointAddress;
				ptp_verbose_log("Endpoint IN addr: 0x%X\n", ep[i].bEndpointAddress);
			} else {
				backend->endpoint_out = ep[i].bEndpointAddress;
				ptp_verbose_log("Endpoint OUT addr: 0x%X\n", ep[i].bEndpointAddress);
			}
		} else if (ep[i].bmAttributes == LIBUSB_ENDPOINT_TRANSFER_TYPE_INTERRUPT) {
			backend->endpoint_int = ep[i].bEndpointAddress;
			ptp_verbose_log("Endpoint INT addr: 0x%X\n", ep[i].bEndpointAddress);	
		}
	}

	if (interf_desc->bNumEndpoints < 2) {
		return PTP_OPEN_FAIL;
	}

	libusb_free_config_descriptor(config);
	int rc = libusb_open(dev, &(backend->handle));
	libusb_free_device_list(list, 0);

	if (rc) {
		perror("usb_open() failure");
		return PTP_OPEN_FAIL;
	}

	char buffer[64];
	rc = libusb_get_string_descriptor_ascii(backend->handle, desc.iProduct, buffer, sizeof(buffer));
	ptp_verbose_log("Name: %s\n", buffer);

	if (libusb_set_auto_detach_kernel_driver(backend->handle, 0)) {
		perror("libusb_set_auto_detach_kernel_driver");
		return PTP_OPEN_FAIL;
	}

	if (libusb_claim_interface(backend->handle, 0)) {
		perror("usb_claim_interface() failure");
		libusb_close(backend->handle);
		return PTP_OPEN_FAIL;
	}

	r->active_connection = 1;

	return 0;
}

int ptp_device_close(struct PtpRuntime *r) {
	struct LibUSBBackend *backend = (struct LibUSBBackend *)r->comm_backend;
	if (libusb_release_interface(backend->handle, 0)) {
		return 1;
	}

	libusb_close(backend->handle);

	r->active_connection = 0;

	return 0;
}

int ptp_device_reset(struct PtpRuntime *r) {
	return 0;
}

int ptp_cmd_write(struct PtpRuntime *r, void *to, int length) {
	struct LibUSBBackend *backend = (struct LibUSBBackend *)r->comm_backend;
	if (backend == NULL) return -1;
	int transferred;
	int rc = libusb_bulk_transfer(
		backend->handle,
		backend->endpoint_out,
		(unsigned char *)to, length, &transferred, PTP_TIMEOUT);
	if (rc) {
		return -1;
	}

	return transferred;
}

int ptp_cmd_read(struct PtpRuntime *r, void *to, int length) {
	struct LibUSBBackend *backend = (struct LibUSBBackend *)r->comm_backend;
	if (backend == NULL) return -1;
	int transferred = 0;
	int rc = libusb_bulk_transfer(
		backend->handle,
		backend->endpoint_in,
		(unsigned char *)to, length, &transferred, PTP_TIMEOUT);
	if (rc) {
		return -1;
	}

	return transferred;
}

int ptp_read_int(struct PtpRuntime *r, void *to, int length) {
	struct LibUSBBackend *backend = (struct LibUSBBackend *)r->comm_backend;
	if (backend == NULL) return -1;
	int transferred = 0;
	int rc = libusb_bulk_transfer(
		backend->handle,
		backend->endpoint_int,
		(unsigned char *)to, length, &transferred, 10);
	if (rc == LIBUSB_ERROR_NO_DEVICE) {
		return PTP_IO_ERR;
	} else if (rc == LIBUSB_ERROR_TIMEOUT) {
		return 0;
	}

	return transferred;
}

int reset_int(struct PtpRuntime *r) {
	return -1;
}
