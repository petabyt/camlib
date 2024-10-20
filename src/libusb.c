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

// TODO: If this is accidentally called in the middle of a connection, it will cause a huge fault
static int ptp_comm_init(struct PtpRuntime *r) {
	ptp_reset(r);

	// libusb 1.0 has no specificed limit for reads/writes
	r->max_packet_size = 512 * 4;

	if (r->comm_backend == NULL) {
		r->comm_backend = malloc(sizeof(struct LibUSBBackend));
		memset(r->comm_backend, 0, sizeof(struct LibUSBBackend));

		struct LibUSBBackend *backend = (struct LibUSBBackend *)r->comm_backend;

		ptp_verbose_log("Initializing libusb...\n");
		libusb_init(&(backend->ctx));

		//libusb_set_option(backend->ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);
	}

	return 0;
}

struct PtpDeviceEntry *ptpusb_device_list(struct PtpRuntime *r) {
	if (r->comm_backend == NULL) {
		ptp_verbose_log("comm_backend is NULL\n");
		return NULL;
	}

	if (!r->io_kill_switch) {
		ptp_verbose_log("Connection is active\n");
		return NULL;
	}

	ptp_mutex_lock(r);

	struct LibUSBBackend *backend = (struct LibUSBBackend *)r->comm_backend;

	libusb_device **list;
	ssize_t count = libusb_get_device_list(backend->ctx, &list);

	struct libusb_device_descriptor desc;
	struct libusb_config_descriptor *config;
	const struct libusb_interface *interf;
	const struct libusb_interface_descriptor *interf_desc;

	struct PtpDeviceEntry *curr_ent = malloc(sizeof(struct PtpDeviceEntry));
	memset(curr_ent, 0, sizeof(struct PtpDeviceEntry));

	struct PtpDeviceEntry *orig_ent = curr_ent;

	if (count == 0) {
		ptp_mutex_unlock(r);
		return NULL;
	}

	int valid_devices = 0;
	for (int d = 0; d < (int)count; d++) {
		libusb_device *dev = list[d];
		int rc = libusb_get_device_descriptor(dev, &desc);
		if (rc) {
			perror("libusb_get_device_descriptor");
			ptp_mutex_unlock(r);
			return NULL;
		}

		if (desc.bNumConfigurations == 0) {
			continue;
		} 

		rc = libusb_get_config_descriptor(dev, 0, &config);
		if (rc) {
			perror("libusb_get_config_descriptor");
			ptp_mutex_unlock(r);
			return NULL;
		}

		if (config->bNumInterfaces == 0) {
			continue;
		}

		interf = &config->interface[0];
		if (interf->num_altsetting == 0) {
			continue;
		}

		// TODO: check altsetting length (garunteed to be >=1?)

		interf_desc = &interf->altsetting[0];

		// Only accept imaging class devices
		if (interf_desc->bInterfaceClass != LIBUSB_CLASS_IMAGE) {
			continue;
		}

		// We require in/out/int endpoints
		const struct libusb_endpoint_descriptor *ep = interf_desc->endpoint;
		if (interf_desc->bNumEndpoints < 2) {
			continue;
		}

		// Init next/curr member of linked list
		if (valid_devices != 0) {
			struct PtpDeviceEntry *new_ent = malloc(sizeof(struct PtpDeviceEntry));
			memset(new_ent, 0, sizeof(struct PtpDeviceEntry));

			curr_ent->next = new_ent;
			new_ent->prev = curr_ent;

			curr_ent = new_ent;
		}

		valid_devices++;

		ptp_verbose_log("Vendor ID: %X, Product ID: %X\n", desc.idVendor, desc.idProduct);

		curr_ent->id = d;
		curr_ent->vendor_id = desc.idVendor;
		curr_ent->product_id = desc.idProduct;

		for (int i = 0; i < interf_desc->bNumEndpoints; i++) {
			if (ep[i].bmAttributes == LIBUSB_ENDPOINT_TRANSFER_TYPE_BULK) {
				if (ep[i].bEndpointAddress & LIBUSB_ENDPOINT_IN) {
					curr_ent->endpoint_in = ep[i].bEndpointAddress;
					ptp_verbose_log("Endpoint IN addr: 0x%X\n", ep[i].bEndpointAddress);
				} else {
					curr_ent->endpoint_out = ep[i].bEndpointAddress;
					ptp_verbose_log("Endpoint OUT addr: 0x%X\n", ep[i].bEndpointAddress);
				}
			} else if (ep[i].bmAttributes == LIBUSB_ENDPOINT_TRANSFER_TYPE_INTERRUPT) {
				curr_ent->endpoint_int = ep[i].bEndpointAddress;
				ptp_verbose_log("Endpoint INT addr: 0x%X\n", ep[i].bEndpointAddress);	
			}
		}

		curr_ent->device_handle_ptr = dev;
		
		libusb_device_handle *handle = NULL;
		rc = libusb_open(dev, &handle);
		if (rc) {
			perror("usb_open() failure");
			ptp_mutex_unlock(r);
			return NULL;
		}

		char buffer[64];
		rc = libusb_get_string_descriptor_ascii(handle, desc.iProduct, (unsigned char *)buffer, sizeof(buffer));
		if (rc < 0) {
			strcpy(curr_ent->name, "?");
		} else {
			strncpy(curr_ent->name, buffer, sizeof(curr_ent->name) - 1);
			ptp_verbose_log("Device name: %s\n", curr_ent->name);
		}

		rc = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, (unsigned char *)buffer, sizeof(buffer));
		if (rc < 0) {
			strcpy(curr_ent->manufacturer, "?");
		} else {
			strncpy(curr_ent->manufacturer, buffer, sizeof(curr_ent->name) - 1);
			ptp_verbose_log("Manufacturer: %s\n", curr_ent->manufacturer);
		}

		libusb_free_config_descriptor(config);
		
		libusb_close(handle);
	}

	//libusb_free_device_list(list, 0);

	ptp_mutex_unlock(r);

	if (valid_devices == 0) {
		return NULL;
	}

	return orig_ent;
}

int ptp_device_open(struct PtpRuntime *r, struct PtpDeviceEntry *entry) {
	ptp_mutex_lock(r);
	if (r->comm_backend == NULL) {
		ptp_verbose_log("comm_backend is NULL\n");
		ptp_mutex_unlock(r);
		return PTP_OPEN_FAIL;
	}

	if (!r->io_kill_switch) {
		ptp_verbose_log("Connection is active\n");
		return PTP_OPEN_FAIL;
	}

	struct LibUSBBackend *backend = (struct LibUSBBackend *)r->comm_backend;

	backend->endpoint_in = entry->endpoint_in;
	backend->endpoint_out = entry->endpoint_out;
	backend->endpoint_int = entry->endpoint_int;

	int rc = libusb_open(entry->device_handle_ptr, &(backend->handle));
	if (rc) {
		perror("usb_open() failure");
		ptp_mutex_unlock(r);
		return PTP_OPEN_FAIL;
	}

	if (libusb_set_auto_detach_kernel_driver(backend->handle, 0)) {
		perror("libusb_set_auto_detach_kernel_driver");
		ptp_mutex_unlock(r);
		return PTP_OPEN_FAIL;
	}

	if (libusb_claim_interface(backend->handle, 0)) {
		perror("usb_claim_interface() failure");
		libusb_close(backend->handle);
		ptp_mutex_unlock(r);
		return PTP_OPEN_FAIL;
	}

	r->io_kill_switch = 0;
	r->operation_kill_switch = 0;

	ptp_mutex_unlock(r);
	return 0;
}

void ptpusb_free_device_list_entry(void *ptr) {
	// TODO: free libusb_device
}

int ptp_device_init(struct PtpRuntime *r) {
	ptp_comm_init(r);
	struct LibUSBBackend *backend = (struct LibUSBBackend *)r->comm_backend;

	struct PtpDeviceEntry *list = ptpusb_device_list(r);

	if (list == NULL) {
		return PTP_NO_DEVICE;
	}

	backend->endpoint_in = list->endpoint_in;
	backend->endpoint_out = list->endpoint_out;
	backend->endpoint_int = list->endpoint_int;

	int rc = libusb_open(list->device_handle_ptr, &(backend->handle));
	if (rc) {
		perror("usb_open() failure");
		return PTP_OPEN_FAIL;
	}

	if (libusb_set_auto_detach_kernel_driver(backend->handle, 0)) {
		perror("libusb_set_auto_detach_kernel_driver");
		return PTP_OPEN_FAIL;
	}

	if (libusb_claim_interface(backend->handle, 0)) {
		perror("usb_claim_interface() failure");
		libusb_close(backend->handle);
		return PTP_OPEN_FAIL;
	}

	r->io_kill_switch = 0;
	r->operation_kill_switch = 0;

	return 0;
}

int ptp_device_close(struct PtpRuntime *r) {
	r->io_kill_switch = 1;
	r->operation_kill_switch = 1;
	struct LibUSBBackend *backend = (struct LibUSBBackend *)r->comm_backend;
	if (libusb_release_interface(backend->handle, 0)) {
		return 1;
	}

	libusb_close(backend->handle);

	return 0;
}

int ptp_device_reset(struct PtpRuntime *r) {
	return 0;
}

int ptp_cmd_write(struct PtpRuntime *r, void *to, int length) {
	const struct LibUSBBackend *backend = (struct LibUSBBackend *)r->comm_backend;

	if (backend == NULL || r->io_kill_switch) {
		return -11;
	}

	int transferred;
	int rc = libusb_bulk_transfer(
		backend->handle,
		backend->endpoint_out,
		(unsigned char *)to, length, &transferred, PTP_TIMEOUT);
	if (rc) {
		perror("libusb_bulk_transfer");
		return -1;
	}

	return transferred;
}

int ptp_cmd_read(struct PtpRuntime *r, void *to, int length) {
	const struct LibUSBBackend *backend = (struct LibUSBBackend *)r->comm_backend;
	if (backend == NULL || r->io_kill_switch) return -1;
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
	if (backend == NULL || r->io_kill_switch) return -1;
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
