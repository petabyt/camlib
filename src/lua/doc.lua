--- Runs PTP_OC_GetDeviceInfo and returns a structure
--- @return table
function ptp.getDeviceInfo()
end
--- Connect to the first available PTP device
--- @return number
function ptp.connect()
end
--- Perform a PTP operation with custom options
--- @param params table have a list of up to 5 integer parameters.
--- @param payload table payload to be sent to device.
--- @return table structure such as {"error": 0, "code": 0x2001, "payload": [0, 1, 2]}
function ptp.sendOperation(opcode, params, payload)
end
--- Run PTP_OC_SetDevicePropValue
--- @param propCode number PTP_DPC_*
--- @param value number value to be set, as a 32 bit integer
function ptp.setProperty(propCode, value)
end
--- Trigger the camera to take a picture
--- @return number Camlib error code
function ptp.takePicture()
end