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

project(ComponentAttestationGate C)

DeclareCAmkESComponent(AttestationGate
    SOURCES
    src/attestation_gate.c
    src/attestation_gate_ffi.c
    src/attestation_gate.S
    LIBS
    hexdump
    am_queue
    queue
)
