let main = function() {
	let x = ptp_device_init();
	if (x) {
		print("Couldn't find device");
		return 0;
	} else {
		print("Connected");
	}
	
	x = ptp_device_info_json();
	if (typeof("x") === "string") {
		print(x);
	} else {
		print("Device info error");
	}
};

main();
