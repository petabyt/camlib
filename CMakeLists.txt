cmake_minimum_required(VERSION 2.4.4...3.5.0)

project(camlib)

set(CMAKE_C_FLAGS "-fpic -Wall -Wshadow -Wcast-qual -Wpedantic -Werror=incompatible-pointer-types -Werror=deprecated-declarations -Wstrict-aliasing=3 -D CAMLIB_NO_COMPAT -D VERBOSE")

# Camlib core source files
set(CAMLIB_CORE
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
    src/log.c
	src/canon_adv.c
)

if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

    set(PLATFORM_LIB_FILES ${CAMLIB_CORE} src/libusb.c)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LIBUSB_CFLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${LIBUSB_LDFLAGS}")
elseif(WIN32)
    set(PLATFORM_LIB_FILES ${CAMLIB_CORE} src/libwpd.c)
    set(MINGW_LIBS
        -lwpd -luser32 -lkernel32 -lgdi32 -lcomctl32 -luxtheme -lmsimg32 -lcomdlg32
        -ld2d1 -ldwrite -lole32 -loleaut32 -loleacc -lstdc++ -lgcc -lpthread -lssp -lurlmon -luuid -lws2_32
    )
    set(CMAKE_SHARED_LINKER_FLAGS "${MINGW_LIBS}")
endif()

add_library(libcamlib SHARED ${PLATFORM_LIB_FILES})
target_include_directories(libcamlib PUBLIC ${PROJECT_SOURCE_DIR}/src)
set_target_properties(libcamlib PROPERTIES OUTPUT_NAME libcamlib)

add_executable(camlib
    src/cli.c
    src/dec/main.c
)
target_include_directories(camlib PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(camlib libcamlib ${LIBUSB_LIBRARIES} pcap)
