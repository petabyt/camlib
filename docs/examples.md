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
