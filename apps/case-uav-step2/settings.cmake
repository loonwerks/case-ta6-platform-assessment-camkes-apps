#
# Copyright 2020, Collins Aerospace
#
# This software may be distributed and modified according to the terms of
# the BSD 3-Clause license. Note that NO WARRANTY is provided.
# See "LICENSE_BSD3.txt" for details.
#
# Copied from an example by Data61

# Add virtual PCI device to VMM for registering cross component connectors as
# devices on the PCI bus.
set(VmPCISupport ON CACHE BOOL "" FORCE)

# Disable libusb from being compiled.
set(LibUSB OFF CACHE BOOL "" FORCE)

# Enables the option for the VM to open and load a seperate initrd file
set(VmInitRdFile ON CACHE BOOL "" FORCE)

# Enable virtio console vmm module
set(VmVirtioConsole ON CACHE BOOL "" FORCE)

# Make VTimers see absolute time rather than virtual time.
set(KernelArmVtimerUpdateVOffset OFF CACHE BOOL "" FORCE)

# Don't trap WFI or WFE instructions in a VM.
set(KernelArmDisableWFIWFETraps ON CACHE BOOL "" FORCE)

set(KernelNumDomains 2 CACHE STRING "" FORCE)
set(KernelDomainSchedule "${CMAKE_CURRENT_LIST_DIR}/vm/domain_schedule.c" CACHE INTERNAL "")
