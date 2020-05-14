#
# Copyright 2020, Collins Aerospace
#
# This software may be distributed and modified according to the terms of
# the BSD 3-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD3.txt" for details.
#

cmake_minimum_required(VERSION 3.7.2)

project(case-uav-step3 C)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/hexdump)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/queue)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/components/AutopilotSerialServer)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/components/WaypointManager)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vmRadio)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vmUxAS)

DeclareCAmkESRootserver(case-uav-step3.camkes)
