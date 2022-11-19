## Quick Overview of PTP Standard
1. The lib can send a command packet with up to 5 parameters
2. An optional data packet (the data phase)
3. A response packet is recieved from the camera

For data to be sent to the device, a data packet can be sent following  
the command packet. The camera should know when to expect this.  

- Each packet sent to the camera has a unique transaction ID (see PtpBulkContainer.transaction)
- The operation code (OC) is an ID for each command, and determines how the camera will expect and send data.
- In a response packet, the response code (RC) is placed in the (see PtpBulkContainer.code) field
