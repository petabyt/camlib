set_project("camlib")

local files = {"src/libusb.c", "src/operations.c", "src/packet.c", "src/deviceinfo.c"}

add_ldflags("-lusb")
add_includedirs("src/", {public = true})

target("pktest")
	set_kind("binary")
	add_files("pktest.c")
	add_files(files)

target("optest")
	add_files("optest.c")
	add_files(files)
	set_kind("binary")

target("testlive")
	add_files(files, "test-live.c")
	set_kind("binary")
