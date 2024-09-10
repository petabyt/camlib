# Camlib C API

Camlib read and writes all data in a single buffer. When incoming data is parsed, it's also parsed in this buffer.
This reduces the number of memory allocations needed for a single transaction (generally it will be zero), and simplifies memory management.

In a multithreaded application, this buffer must be protected by a mutex. See `ptp_mutex_` functions.
