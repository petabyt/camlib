# Documentation For Camlib
- PDF Reference for PTP and MTP: https://www.usb.org/document-library/media-transfer-protocol-v11-spec-and-mtp-v11-adopters-agreement

## Quick Overview of PTP Standard
1. The lib can send a command packet with up to 5 parameters
2. An optional data packet (the data phase)
3. A response packet is recieved from the camera

For data to be sent to the device, a data packet can be sent following  
the command packet. The camera should know when to expect this.  

- Each packet sent to the camera has a unique transaction ID (see PtpBulkContainer.transaction)
- The operation code (OC) is an ID for each command, and determines how the camera will expect and send data.
- In a response packet, the response code (RC) is placed in the (see PtpBulkContainer.code) field

## JSON Bindings
Camlib provides a JSON frontend that can easily be ported to higher level programming languages,
like Java, Javascript, or Python.
### bind_run
```
int bind_run(struct PtpRuntime *r, char *req, char *buffer, int max);
```
- req is a formatted request.
- Response JSON will be written to buffer
- max is the size of the buffer, `PTP_BIND_DEFAULT_SIZE` is recommended
- It's up to the caller to figure out how to return the value of PtpRuntime->data back to caller
The formatted request:
- Runs a formatted request and returns an error code.
- It does the entire operation (generate packet, send, and recieve data)
- It doesn't parse the response - this is up to the caller
- The end of each section in the request is marked by a `;` semicolon.
- The request starts with an ASCII request name, followed by a semicolon.
- For parameters, base 10 numbers seperated by a single `,` comma follow.
- The end of the parameters is marked with another `;` semicolon.
```
ptp_connect;
ptp_drive_lens;-1;
ptp_custom_send;4097,66,66,66,66,66;
```
