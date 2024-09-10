# Lua API

The Lua API for camlib is under development. You can view the current source code
for the bindings [here on Github](https://github.com/petabyt/camlib/blob/master/src/lua.c).

Currently, the Lua bindings take advantage of the fact that camlib already can convert almost
every PTP data structure to JSON - and is able to convert the JSON to Lua tables using `lua-cjson`.

## `ptp.getDeviceInfo()`
Returns a structure about the device:
```
{
	model = "Canon Rebel Blah"
	propsSupported = {12345, 12345, 12345}
	...
}
```
## `ptp.takePicture()`
Triggers a complete capture.
## `ptp.sendOperation(opcode, params, payload)`
Send a custom opcode request to the camera. Have up to 5 parameters, and an optional
payload in bytes (0-255).
**Only use this if you know what you're doing.**
Sending bad data can easily brick cameras.

# Examples
```
-- Timelapse script
TAKE_X_PICS = 10
MS_BETWEEN_PICS = 100

for i = 0,TAKE_X_PICS,1 do
	rc = ptp.takePicture()

	if rc == ptp.IO_ERR then
		uiToast("IO Error taking picture")
		break
	elseif rc == ptp.UNSUPPORTED then
		uiToast("Remote capture is unsupported")
		break
	elseif rc then
		uiToast("Error: " + tostring(rc))
		break
	else
		uiToast("Took " .. tostring(i) .. "picture(s)")
	end

	msleep(MS_BETWEEN_PICS);
end
```
