cmake_minimum_required(VERSION 3.16)

option(SKIP_AUTOMATE_VCPKG "When ON, you will need to built the packages required by eveloader2 on \
your own or supply your own vcpkg toolchain.")

if (NOT SKIP_AUTOMATE_VCPKG)
    include(cmake/automate-vcpkg.cmake)

    vcpkg_bootstrap()
    vcpkg_install_packages(easyhook loguru)
endif()

project(eveloader2)

set(CMAKE_CXX_STANDARD 17)

set(eveloader2_src
        src/main.cpp
        include/eveloader.h
        src/configuration.cpp
        src/configuration.h
        src/ini.cpp
        src/ini.h
        src/INIReader.cpp
        src/INIReader.h
        src/blue_patcher.cpp
        src/blue_patcher.h
        src/overlay.cpp
        src/overlay.h
        src/patches.h
        src/crc32.cpp
        src/crc32.h
        include/eveloader_util.hpp
        )

set(eveloader2_dll_src
        dllsrc/dllmain.cpp
        dllsrc/main.cpp
        include/eveloader.h
        dllsrc/pyloader.cpp
        dllsrc/pyloader.h
        dllsrc/hook_manager.cpp
        dllsrc/hook_manager.h
        dllsrc/hooks.cpp
        dllsrc/hooks.h
        )

add_executable(eveloader2 ${eveloader2_src})
add_library(eveloader2_dll SHARED ${eveloader2_dll_src})

target_include_directories(eveloader2 PUBLIC SYSTEM "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include" "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/src/")
target_link_directories(eveloader2 PRIVATE "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")
target_link_libraries(eveloader2 PRIVATE "easyhook32.lib")

target_include_directories(eveloader2_dll PUBLIC SYSTEM "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include" "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/deps/python/include" "${CMAKE_SOURCE_DIR}/deps/python/include/Stackless")
target_link_directories(eveloader2_dll PRIVATE "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" "${CMAKE_SOURCE_DIR}/deps/python/lib")
target_link_libraries(eveloader2_dll PRIVATE  "easyhook32.lib" "psapi.lib")

add_dependencies(eveloader2 eveloader2_dll)

if (CMAKE_BUILD_TYPE EQUAL "Debug")
    target_link_libraries(eveloader2_dll PRIVATE  "python27_d.lib")
    target_compile_definitions(eveloader2_dll DEBUG)
    target_compile_definitions(eveloader DEBUG)
else()
    target_link_libraries(eveloader2_dll PRIVATE  "python27.lib")
endif()

add_custom_target(eveloader2_release_zip COMMAND
        ${CMAKE_COMMAND} -E tar "cfv" "release.zip" --format=zip
        "${CMAKE_CURRENT_BINARY_DIR}/eveloader2.exe"
        "${CMAKE_CURRENT_BINARY_DIR}/eveloader2_dll.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/EasyHook32.dll"
        )

add_dependencies(eveloader2_release_zip eveloader2)
