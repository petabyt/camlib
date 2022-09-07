// LibUsb Bindings
#include <sys/ioctl.h>
#include <errno.h>
#include <usb.h>

struct usb_bulktransfer {
	uint32_t ep;
	uint32_t len;
	uint32_t timeout;
	void *data;
};

struct usb_dev_handle {
	int fd;
	struct usb_bus *bus;
	struct usb_device *device;
	int config;
	int interface;
	int altsetting;
	void *impl_info;
};


int usb_bulk_write() {
	
}

int usb_bulk_read() {
	
}
