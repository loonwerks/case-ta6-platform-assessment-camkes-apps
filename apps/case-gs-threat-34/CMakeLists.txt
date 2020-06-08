#
# Copyright 2020, Collins Aerospace
#
# This software may be distributed and modified according to the terms of
# the BSD 3-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD3.txt" for details.
#

cmake_minimum_required(VERSION 3.7.2)

project(case-gs-step2 C)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vm)

DeclareCAmkESRootserver(case-gs-step2.camkes)
