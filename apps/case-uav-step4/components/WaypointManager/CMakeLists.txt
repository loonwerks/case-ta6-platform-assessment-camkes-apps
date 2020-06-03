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

project(ComponentWaypointManager C)

DeclareCAmkESComponent(WaypointManager
    SOURCES
    src/waypoint_manager.c
    src/CMASI/AddressAttributedMessage.c
    src/CMASI/AirVehicleState.c
    src/CMASI/AutomationResponse.c
    src/CMASI/EntityConfiguration.c
    src/CMASI/EntityState.c
    src/CMASI/KeyValuePair.c
    src/CMASI/lmcp.c
    src/CMASI/Location3D.c
    src/CMASI/MissionCommand.c
    src/CMASI/PayloadAction.c
    src/CMASI/PayloadConfiguration.c
    src/CMASI/PayloadState.c
    src/CMASI/VehicleAction.c
    src/CMASI/VehicleActionCommand.c
    src/CMASI/Waypoint.c
    src/CMASI/LineSearchTask.c
    src/CMASI/SearchTask.c
    src/CMASI/Task.c
    src/CMASI/Wedge.c
    src/CMASI/common/conv.c
    LIBS
    hexdump
    queue
)
