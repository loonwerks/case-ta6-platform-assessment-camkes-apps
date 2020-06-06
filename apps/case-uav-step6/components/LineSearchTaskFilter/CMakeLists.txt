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

project(ComponentLineSearchTaskFilter C)

DeclareCAmkESComponent(LineSearchTaskFilter
    SOURCES
    src/line_search_task_filter.c
    src/line_search_task_filter_ffi.c
    src/line_search_task_filter.S
    LIBS
    CMASI
    hexdump
    queue
)
