# Examples
```
-- Timelapse script
TAKE_X_PICS = 10
MS_BETWEEN_PICS = 100

for i = 0,TAKE_X_PICS,1 do
	rc = ptp.takePicture()

	if rc == ptp.IO_ERR then
		setStatusText("IO Error taking picture")
		break
	elseif rc == ptp.UNSUPPORTED then
		setStatusText("Remote capture is unsupported")
		break
	elseif rc then
		setStatusText("Error: " + tostring(rc))
		break
	else
		setStatusText("Took " .. tostring(i) .. "picture(s)")
	end

	msleep(MS_BETWEEN_PICS);
end
```

```
-- Basic UI script
win = ui.popup("Astro Mode")
if win.addButton("Take pic") then
	rc = ptp.takePicture()
endif
```
