# Camlib C API

Camlib read and writes all data in a single buffer. When incoming data is parsed, it's also parsed in this buffer.
This reduces the number of memory allocations needed for a single transaction (generally it will be zero), and simplifies memory management.

In a multithreaded application, this buffer must be protected by a mutex. See `ptp_mutex_` functions.

## `void ptp_init(struct PtpRuntime *r);`
Initializes a `struct PtpRuntime`. The struct is pretty small. This allocates the a large buffer to `r.data`.
You may also set r.connection_type to one of `enum PtpConnType`.
## `int ptp_device_init(struct PtpRuntime *r);`
Attempts to connect to the first valid PTP device found, over USB. Returns `enum CamlibError`.
## `int ptp_open_session(struct PtpRuntime *r);`
Opens session
## `int ptp_close_session(struct PtpRuntime *r);`
Closes session, (typically shuts down camera)
## `int ptp_get_device_info(struct PtpRuntime *r, struct PtpDeviceInfo *di);`
Recieves device info into [`struct PtpDeviceInfo`](https://github.com/petabyt/camlib/blob/8291304dc94a00a57200e33a19a497f1a6ccc5b6/src/cl_data.h#L32).
