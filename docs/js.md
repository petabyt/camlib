# Camlib.js

The user interface for CamControl is written entirely in HTML/CSS/JS. Requests are made through a JS 'mutex' and routed to camlib bindings.
Camlib.js is open-source, you can find it here: [https://github.com/clutchlink/camlibjs](https://github.com/clutchlink/camlibjs)

This is mostly used internally, as every one of these functions *require* `await` as they are async. This makes for a clunky and arkward API.

## `ptp.getDeviceInfo()`

Returns device info as JSON. Note that device info is stored in `ptp.info` when the device is connected.

## `ptp.disconnect()`

Disconnects the device abruptly. All tasks should die after this.

## `ptp.driveLens()`

Drives the lens, if possible. For EOS cameras, you can use range `-3`-`3`.

## `ptp.getLiveViewFrame()`

Internal function used by CamControl (Linux, Windows) to get JSON raw bytes from a liveview frame.
This is done internally on the backend.

## `ptp.getDeviceType()`

Returns the current device type - type enums are stored in `ptp.devs`:

```javascript
devs: {
    EMPTY: 0,
    EOS: 1,
    CANON: 2,
    NIKON: 3,
    SONY: 4,
    FUJI: 5,
    PANASONIC: 6,
}
```


## `ptp.getRetCode()`
Gets the return code from the last operation.

## `ptp.getStorageIDs()`
Return a list of storage IDs (32 bit integers) from the camera.

## `getStorageInfo(id)`
Returns a JSON structure with information on the requested storage ID.

## `ptp.getObjectHandles(id, root)`
Returns a list of object handles from a storage ID. Root can either be 0 for the top directory, or a handle to a folder.

## `ptp.getObjectInfo()`
Gets information on a particular object. Could be a folder, file, or even an album.

## `ptp.getEvents()`
Used only by CamControl. Gets a list of changes the camera has made since the last call.

## `ptp.customCmd(opcode, params)`
(Not implemented in v0.1.0)
Sends a custom command opcode (with no data phase) to the camera.

## `ptp.getPartialObject(handle, offset, max)`
Data is returned in the same way that thumbnail JPEG data is returned.
