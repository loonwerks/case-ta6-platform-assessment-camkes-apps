#
# Copyright 2020, Collins Aerospace
#
# This software may be distributed and modified according to the terms of
# the BSD 3-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD3.txt" for details.
#

cmake_minimum_required(VERSION 3.7.2)

project(case-uav-step5 C)

includeGlobalComponents()
find_package(camkes-vm REQUIRED)
find_package(camkes-vm-images REQUIRED)
find_package(camkes-arm-vm REQUIRED)
find_package(camkes-vm-linux REQUIRED)
camkes_arm_vm_import_project()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/hexdump)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/camkes_log_queue)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/queue)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/components/AttestationGate)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/components/AutopilotSerialServer)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/components/WaypointManager)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vmRadio)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vmUxAS)

CAmkESAddImportPath(exynos5422)

DeclareCAmkESRootserver(case-uav-step5.camkes)

# Add linux kernel image to file server
AddToFileServer("linux" "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/linux")

DefineCAmkESVMFileServer()

