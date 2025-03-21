cmake_minimum_required(VERSION 3.15)

project(pict)

option(PTP_CANON_ADV "Include implementation for advanced Canon features" ON)
option(PTP_STUFF "Include extra functions for debugging/testing" ON)
option(PTP_DEFAULT_LOGGING "Include default log/panic/error implementation" ON)
option(PTP_INCLUDE_CLI "Include CLI program" OFF)
option(PTP_USE_LIBUSB "Use LibUSB backend on UNIX platforms" ON)
option(PTP_INCLUDE_EXAMPLES "Include examples" OFF)
option(PTP_INCLUDE_VCAM_TEST "Include vcam test" OFF)
option(PTP_INCLUDE_LIBUSB_LIBS "make libusb libraries PUBLIC" OFF)
option(PTP_INCLUDE_BIND "Include optional bind.c" ON)
option(PTP_NO_USB "Compile src/no_usb.c in place of libusb/libwpd" OFF)
option(PTP_NO_TCP "Compile no_ip.c in place of TCP code" OFF)
option(PTP_INCLUDE_WINDOWS_TEST "Pull in libwpd and test windows compilation" OFF)

set(PICT_CORE
    src/operations.c
    src/packet.c
    src/enums.c
    src/data.c
    src/enum_dump.c
    src/lib.c
    src/canon.c
    src/liveview.c
    src/ml.c
    src/conv.c
    src/generic.c
    src/transport.c
)
if(NOT PTP_NO_TCP)
    list(APPEND PICT_CORE src/ip.c)
else()
    list(APPEND PICT_CORE src/no_ip.c)
endif()
if(PTP_INCLUDE_BIND)
    list(APPEND PICT_CORE src/bind.c)
endif()
if(PTP_CANON_ADV)
    list(APPEND PICT_CORE src/canon_adv.c)
endif()
if(PTP_STUFF)
    list(APPEND PICT_CORE src/stuff.c)
endif()
if(PTP_DEFAULT_LOGGING)
    list(APPEND PICT_CORE src/log.c)
endif()

if(UNIX AND PTP_USE_LIBUSB AND NOT PTP_NO_USB)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
    set(PLATFORM_LIB_FILES src/libusb.c)
    set(PLATFORM_LIBS ${LIBUSB_LIBRARIES})
    set(PLATFORM_DIRS ${LIBUSB_INCLUDE_DIRS})
elseif(WIN32 AND NOT PTP_NO_USB)
    set(PLATFORM_LIB_FILES src/libwpd.c)
    set(PLATFORM_LIBS wpd_static)
endif()

if(PTP_NO_USB)
    set(PLATFORM_LIB_FILES src/no_usb.c)
endif()

add_library(libpict STATIC ${PICT_CORE} ${PLATFORM_LIB_FILES})
target_compile_options(libpict
    PRIVATE -Wall -Wshadow -Wcast-qual -Wpedantic -Werror=incompatible-pointer-types -Werror=deprecated-declarations -Wstrict-aliasing=3 -D VERBOSE
)
target_include_directories(libpict PUBLIC ${PROJECT_SOURCE_DIR}/src ${PLATFORM_DIRS})
target_link_libraries(libpict PRIVATE ${PLATFORM_LIBS}) # Specify our libraries so flags/include dirs will be added
set_target_properties(libpict PROPERTIES OUTPUT_NAME pict) # libpict.a

if(PTP_INCLUDE_CLI)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBPCAP REQUIRED libpcap)

    add_executable(pict
        src/cli.c
        src/dec/main.c
    )
    target_include_directories(pict PUBLIC ${PROJECT_SOURCE_DIR}/src ${LIBPCAP_INCLUDE_DIRS})
    target_link_libraries(pict libpict ${LIBUSB_LIBRARIES} ${LIBPCAP_LIBRARIES})
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

if(WIN32 AND PTP_INCLUDE_WINDOWS_TEST)
    option(PTP_LIBWPD_FROM_GIT "Use git repo for libwpd" ON)
    if(PTP_LIBWPD_FROM_GIT)
        include(FetchContent)
        FetchContent_Declare(libwpd GIT_REPOSITORY https://github.com/petabyt/libwpd GIT_TAG master GIT_SHALLOW 1)
        FetchContent_MakeAvailable(libwpd)
    else()
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../libwpd ${CMAKE_CURRENT_BINARY_DIR}/libwpd)
    endif()
    add_executable(wintest examples/wintest.c)
    target_link_libraries(wintest wpd_static libpict -lpthread -lurlmon -luuid -lws2_32)
endif()

if(PTP_INCLUDE_VCAM_TEST)
    option(PTP_VCAM_FROM_GIT "Use git repo for vcam" ON)
    if(PTP_VCAM_FROM_GIT)
        include(FetchContent)
        FetchContent_Declare(vcam GIT_REPOSITORY https://github.com/petabyt/vcam GIT_TAG master GIT_SHALLOW 1)
        FetchContent_MakeAvailable(vcam)
    else()
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../vcam ${CMAKE_CURRENT_BINARY_DIR}/vcam)
    endif()

    add_executable(test test/test.c test/data.c)
    target_link_libraries(test libpict usb-vcam)
    #add_subdirectory(/home/daniel/Pulled/hello_imgui/ ${CMAKE_CURRENT_BINARY_DIR}/hello_imgui)
endif()
