# Technical Info
The user interface for CamControl is written entirely in HTML/CSS/JS. This may draw slightly more  
battery power, but it allows the same app to be run on every platform without rewriting an entire codebase.

The backend of CamControl is CamLib, which is written in C99. This is compiled into the Android app with JNI.  
CamLib provides easy to access bindings, which can accept a limited set of parameters. Binding requests can also  
be sent in string form, as documented in the CamLib docs. This allows commands to be sent over a GET request in the  
browser based version.

# The `ptp` API
Each function returns a JSON object. The object will always have a `error` key with
a CamLib error code. Use `ptp.throwErr(code)` to throw an error message for it.
The object might have a `resp` key which could hold any type or structure which would
be returned by the operation. A `code` parameter will sometimes be added for the operation
response code.

If the `error` key is not zero, an error will be thrown.

Scripts **must** be run asynchronously, with the `await`/`async` keywords. For example:
```
var deviceInfo = (await ptp.getDeviceInfo()).resp;
```

For scripts to be able to run in the background, the code must be a little more complex:
```
{
	name: "demo",
	init: async function() {
		console.log("Test for demo task");
	},

	loop: async function() {
		console.log("This is run at 15fps by default")
	},
};
```

`loop` will be called at a certain FPS. All script tasks are run at 5fps.

Since the script is run as a seperate worker task, it must send IO errors to the main task so  
the connection can be killed.
```
try {
	var deviceInfo = (await ptp.getDeviceInfo()).resp;	
} catch (e) {
	ui.log("Error in demo task:" + String(e));
	ptp.kill();
}
```


ADB inspect element is enabled on release build. It can be accessed with `about://inspect` on Chromium based browsers.

## `ptp.getDeviceInfo()`
Returns device info as JSON. Note that device info is stored in `ptp.info` when the device is connected.
## `ptp.disconnect()`
Disconnects the device abruptly. All tasks should die after this.
## `ptp.driveLens()`
Drives the lens, if possible. For EOS cameras, you can use range `-3`-`3`
## `ptp.getLiveViewFrame()`
Internal function used by CamControl (Linux, Windows) to get JSON raw bytes from a liveview frame.
This is done internally on the backend.
## `ptp.getDeviceType()`
Returns the current device type - type enums are stored in `ptp.devs`:
```
devs: {
	EMPTY: 0,
	EOS: 1,
	CANON: 2,
	NIKON: 3,
	SONY: 4,
	FUJI: 5,
	PANASONIC: 6,
},
```
## `ptp.getRetCode()`
Gets the return code from the last operation.
## `ptp.getStorageIDs()`
Return a list of storage IDs (32 bit integers) from the camera
## `getStorageInfo(id)`
Returns a JSON structure with information on the requested storage ID.
## `ptp.getObjectHandles(id, root)`
Returns a list of object handles from a storage ID. Root can either be 0 for the top directory, or a handle to a folder.
## `ptp.getObjectInfo()`
Gets information on a particular object. Could be a folder, file, or even an album.
## `ptp.getEvents()`
Used only by CamControl. Gets a list of changes the camera has made since the last call.
## `ptp.customCmd(opcode, params)`
(*Not avilable in v0.1.0*)
Sends a custom CMD request (with no data sent) to the camera.
