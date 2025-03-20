cmake_minimum_required(VERSION 2.4.4...3.5.0)

project(pict)

option(PTP_CANON_ADV "Include implementation for advanced Canon features" ON)
option(PTP_STUFF "Include extra functions for debugging/testing" ON)
option(PTP_DEFAULT_LOGGING "Include default log/panic/error implementation" ON)
option(PTP_INCLUDE_CLI "Include CLI program" OFF)
option(PTP_USE_LIBUSB "Use LibUSB backend on UNIX platforms" ON)
option(PTP_INCLUDE_EXAMPLES "Include examples" ON)

set(PICT_CORE
    src/operations.c
    src/packet.c
    src/enums.c
    src/data.c
    src/enum_dump.c
    src/lib.c
    src/canon.c
    src/liveview.c
    src/bind.c
    src/ip.c
    src/ml.c
    src/conv.c
    src/generic.c
    src/transport.c
)
if(PTP_CANON_ADV)
    list(APPEND PICT_CORE src/canon_adv.c)
endif()
if(PTP_STUFF)
    list(APPEND PICT_CORE src/stuff.c)
endif()
if(PTP_DEFAULT_LOGGING)
    list(APPEND PICT_CORE src/log.c)
endif()

if(UNIX AND PTP_USE_LIBUSB)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
    set(PLATFORM_LIB_FILES src/libusb.c)
    set(PLATFORM_LIBS ${LIBUSB_LIBRARIES})
    set(PLATFORM_DIRS ${LIBUSB_INCLUDE_DIRS})
elseif(WIN32)
    set(PLATFORM_LIB_FILES src/libwpd.c)
    set(PLATFORM_LIBS /usr/x86_64-w64-mingw32/lib/libwpd.a)
endif()

add_library(libpict STATIC ${PICT_CORE} ${PLATFORM_LIB_FILES})
target_compile_options(libpict
    PRIVATE -Wall -Wshadow -Wcast-qual -Wpedantic -Werror=incompatible-pointer-types -Werror=deprecated-declarations -Wstrict-aliasing=3 -D VERBOSE
)
target_include_directories(libpict PUBLIC ${PROJECT_SOURCE_DIR}/src ${PLATFORM_DIRS})
target_link_libraries(libpict PRIVATE ${PLATFORM_LIBS})
set_target_properties(libpict PROPERTIES OUTPUT_NAME pict) # libpict.a

if(PTP_INCLUDE_CLI)
    add_executable(pict
        src/cli.c
        src/dec/main.c
    )
    target_include_directories(pict PUBLIC ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(pict libpict ${LIBUSB_LIBRARIES} pcap)
endif()

macro(add_example exe_name source)
    add_executable(${exe_name} ${source})
    target_include_directories(${exe_name} PUBLIC ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(${exe_name} libpict ${LIBUSB_LIBRARIES})
endmacro()

if(PTP_INCLUDE_EXAMPLES)
    add_example(info examples/info.c)
    add_example(optest examples/optest.c)
    add_example(directprint examples/directprint.c)
    add_example(eos examples/eos.c)
    add_example(evtest examples/evtest.c)
    add_example(live examples/live.c)
    add_example(ml examples/ml.c)
    add_example(pktest examples/pktest.c)
    add_example(storage examples/storage.c)
    add_example(usb examples/usb.c)
    add_example(wifi examples/wifi.c)
endif()
