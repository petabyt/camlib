# Documentation For Camlib
- PDF Reference for PTP and MTP: https://www.usb.org/document-library/media-transfer-protocol-v11-spec-and-mtp-v11-adopters-agreement
- PTP/IP has no free public reference :(

Terminology:
- initiator: the computer that sends commands (laptop, phone, PC)
- responder: the computer that responds to commands (the camera)

## Quick Overview of PTP Standard
### To issue a command to the device
1. Send a *command packet* with up to 5 parameters
2. An optional data packet (the data phase)
3. A response packet(s) is recieved from the camera.

For data to be sent to the camera, a data packet can be sent following  
the command packet. The camera should know when to expect this.  

- Each packet sent to the camera has a unique transaction ID (see PtpBulkContainer.transaction)
- The operation code (OC) is an ID for each command, and determines how the camera will expect and send back data.
- In a response packet, the response code (RC) is placed in the PtpBulkContainer.code field

## PTP/IP
PTP/IP is the variant of PTP designed to be used on TCP.

A standard request to the responder is as follows:
1. Command request (`PTPIP_COMMAND_REQUEST`) send to responder
2. If sending data payload:
	- Set the `data_phase` field in the initial `PTPIP_COMMAND_REQUEST` packet
	- Send data start packet (`PTPIP_DATA_PACKET_START`), contains data length
	- Send data end packet (`PTPIP_DATA_PACKET_END`), which includes the actual data
2. If not sending payload to responder, then, wait for response

Response is either:
- `PTPIP_DATA_PACKET_START` packet followed by `PTPIP_DATA_PACKET_END`, and finally a `PTPIP_COMMAND_RESPONSE` packet
- Just a `PTPIP_COMMAND_RESPONSE` packet
