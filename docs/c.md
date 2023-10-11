# Camlib C API

Camlib uses *no macros*, so it's easy to use camlib from any language that has a basic C FFI. It would be trivial to
write a binding for Rust.

Camlib also uses a single-buffer design. This means that all data, whether it comes in or out, is read, written, packed, and unpacked in a *single multi-megabyte buffer*.
This is not done to reduce memory usage, but to decrease complexity and reduce the number of `malloc()` calls. It also helps prevent rogue memory leaks, which
is crucial for apps, which have a very low memory limit.

This means that functions processing or using this data *must* keep the operation mutex locked until processing is done, so long as
the caller isn't making the application thread-safe. In a single-threaded application, there is no need for it to be thread-safe.

Camlib was designed to run on a single thread, through a thread-safe server returning `bind` requests. This works well
in many applications, but I'm slowly working on making it thread-safe for higher speeds applications.

## `void ptp_generic_init(struct PtpRuntime *r);`
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
