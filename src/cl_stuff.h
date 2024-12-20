#ifndef CL_STUFF_H
#define CL_STUFF_H

int ptp_list_devices(void);

struct PtpRuntime *ptp_connect_from_id(int id);

int ptp_dump_device(int dev_id);

#endif
