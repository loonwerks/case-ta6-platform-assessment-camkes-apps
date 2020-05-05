#
# Copyright 2018, Data61
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# ABN 41 687 119 230.
#
# Copyright 2020 Collins Aerospace
# Modifications made to original
#
# This software may be distributed and modified according to the terms of
# the BSD 3-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD3.txt" for details.
#

cmake_minimum_required(VERSION 3.7.2)

project(ComponentAutopilotSerialServer C)

if(KernelArchARM)
    set(CAmkESDTS ON CACHE BOOL "" FORCE)
endif()

set(CAmkESCPP ON CACHE BOOL "" FORCE)

# ARM platforms have similar serial configurations in their DTSes and can share
# similar headers and code
if(KernelArchARM)
    set(PlatPrefix "arm_common")
else()
    set(PlatPrefix "${KernelPlatform}")
endif()

DeclareCAmkESComponent(
    AutopilotSerialServer
    SOURCES
    src/autopilot_serial_server.c
    src/sentinel_serial_buffer.c
    src/serial.c
    src/plat.c
    src/plat/${PlatPrefix}/plat.c
    INCLUDES
    include
    LIBS
    queue
)

CAmkESAddCPPInclude("${CMAKE_CURRENT_LIST_DIR}/include/plat/${PlatPrefix}/")

