
cmake_minimum_required(VERSION 3.8.2)

project(arm-vm-radio C)

include(${CAMKES_VM_LINUX_HELPERS_PATH})
include(${CAMKES_VM_LINUX_MODULE_HELPERS_PATH})
include(${CAMKES_VM_LINUX_SOURCE_HELPERS_PATH})
include(ExternalProject)
include(external-project-helpers)

message("The value of CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message("The value of CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")

if("${KernelARMPlatform}" STREQUAL "qemu-arm-virt")
    set(cpp_flags "-DKERNELARMPLATFORM_QEMU-ARM-VIRT")
    set(linux_repo "https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git")
    set(linux_tag "v4.9.189")
    set(linux_arch "arm64")
    set(linux_cross_compile "aarch64-linux-gnu-")
    set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/rootfs.cpio.gz")
    # Checkout and configure linux to build crossvm module
    ExternalProject_Add(
        checkout_linux
        GIT_REPOSITORY
        ${linux_repo}
        GIT_TAG
        ${linux_tag}
        GIT_SHALLOW
        1
        GIT_PROGRESS
        1
        BUILD_COMMAND
        ""
        INSTALL_COMMAND
        ""
        CONFIGURE_COMMAND
        ""
        SOURCE_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/linux_out
    )
    # Linux config and symvers are to be copied to unpacked archive
    set(linux_config "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/linux_configs/config")
    set(linux_symvers "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/linux_configs/Module.symvers")
    # Configure unpacked archive with config and symvers
    ConfigureLinux(
        ${CMAKE_CURRENT_BINARY_DIR}/linux_out
        ${linux_config}
        ${linux_symvers}
        configure_vm_linux
        ARCH
        ${linux_arch}
        CROSS_COMPILE
        ${linux_cross_compile}
        DEPENDS
        checkout_linux
    )

    # Compile CrossVM Dataport Module
    DefineLinuxModule(
        ${CAMKES_VM_LINUX_DIR}/camkes-linux-artifacts/camkes-linux-modules/camkes-connector-modules/connection
        output_module
        output_module_target
        KERNEL_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/linux_out
        ARCH
        ${linux_arch}
        CROSS_COMPILE
        ${linux_cross_compile}
        DEPENDS
        checkout_linux
        configure_vm_linux
    )
    AddFileToOverlayDir(
        "connection.ko"
        ${output_module}
        "lib/modules/4.14.87/kernel/drivers/vmm"
        overlay_radio
        DEPENDS
        output_module_target
    )

    # Add script to initialise dataport module
    AddFileToOverlayDir(
        "S90crossvm_module_init"
        ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/cross_vm_module_init
        "etc/init.d"
        overlay_radio
    )
else()
    # User pre-configured rootfs file with crossvm modules and apps installed
    set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/rootfs_crossvm.cpio.gz")
endif()

# Taken and modified from tools/seL4/cmake-tool/helpers/external-project-helpers.cmake
# Function for declaring target object files that are produced by an external project. This adds a custom_command
# that forces a stale check on the object file after the External projects install step.
# external_proj_target: Target name of the external project
# external_prog_output_dir: Location of binary or install directory for external project
# FILES: List of object files that exist in the external project
function(MyDeclareExternalProjObjectFiles external_proj_target external_proj_output_dir)
    # Parse the given files object files
    cmake_parse_arguments(PARSE_ARGV 2 EXT_OBJECT "" "" "FILES")
    if(NOT "${EXT_OBJECT_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "Unknown arguments to DeclareExternalProjObjectFiles")
    endif()
    # Necessary for FILES to be passed
    if(NOT EXT_OBJECT_FILES)
        message(FATAL_ERROR "NO FILES declared for ${external_proj_target}")
    endif()
    # Get external project binary and stamp dir properties
    ExternalProject_Get_property(${external_proj_target} STAMP_DIR)
    foreach(obj_file IN LISTS EXT_OBJECT_FILES)
        # Generate a unique name based on the object files location
        set(file_path ${external_proj_output_dir}/${obj_file})
        list(APPEND objfiles ${file_path})
    endforeach()
    if(NOT (TARGET ${external_proj_target}-install))
        ExternalProject_Add_StepTargets(${external_proj_target} install)
    endif()
    add_custom_command(
        OUTPUT ${objfiles}
        COMMAND true
        DEPENDS ${external_proj_target}-install ${STAMP_DIR}/${external_proj_target}-install
    )
endfunction(MyDeclareExternalProjObjectFiles)

# Taken and modified from projects/camkes-vm-linux/vm-linux-helpers.cmake
# Wrapper function to declare object files in an external project and add them to the targeted overlay
# external_target: Target of external project
# external_install_dir: The install/binary directory where the files are located
# overlay_target: The overlay target we are adding files to
# overlay_root_location: The location in the overlay the file is to be installed. Note all files passed in a given
# call of this helper will install in the same root location.
# FILES: A list of files to declare from an external project and add to the given overlay
function(MyAddExternalProjFilesToOverlay external_target external_install_dir overlay_target overlay_root_location)
    # Get the external project files to add the the overlay
    cmake_parse_arguments(PARSE_ARGV 4 EXTERNAL_PROJ_OVERLAY
        ""
        ""
        "FILES"
    )
    # Error checking
    if (NOT "${EXTERNAL_PROJ_OVERLAY_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "Unknown arguments to AddExternalProjFilesToOverlay")
    endif()
    # Necessary to provide a least one file
    if(NOT EXTERNAL_PROJ_OVERLAY_FILES)
        message(FATAL_ERROR "NO FILES declared in AddExternalProjFilesToOverlay")
    endif()
    # Declare the project files
    MyDeclareExternalProjObjectFiles(${external_target} ${external_install_dir}
        FILES ${EXTERNAL_PROJ_OVERLAY_FILES})
    # Iterate adding each file to the overlay
    foreach(file IN LISTS EXTERNAL_PROJ_OVERLAY_FILES)
        get_filename_component(file_name ${file} NAME)
        AddFileToOverlayDir("${file_name}" ${external_install_dir}/${file} "${overlay_root_location}" ${overlay_target}
            DEPENDS ${external_target})
    endforeach()
endfunction(MyAddExternalProjFilesToOverlay)

ExternalProject_Add(
    useram
    GIT_REPOSITORY
    https://github.com/gaj7/am-bin-uav.git
    GIT_TAG
    master
    INSTALL_COMMAND
    ""
    UPDATE_COMMAND
    ""
    CONFIGURE_COMMAND
    ""
    BUILD_COMMAND
    ""
)
MyAddExternalProjFilesToOverlay(
    useram
    ${CMAKE_CURRENT_BINARY_DIR}/useram-prefix/src/useram
    overlay_radio
    "usr/bin"
    FILES
    uav
)

ExternalProject_Add(
    camkes_log_relay
    SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/camkes_log_relay
    BINARY_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/camkes_log_relay
    INSTALL_COMMAND
    ""
    BUILD_ALWAYS
    ON
    EXCLUDE_FROM_ALL
    CMAKE_ARGS
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCAMKES_LOG_QUEUE_LIB=${CMAKE_CURRENT_SOURCE_DIR}/../camkes_log_queue -DCMAKE_C_FLAGS=${BASE_C_FLAGS}
)

MyAddExternalProjFilesToOverlay(
    camkes_log_relay
    ${CMAKE_CURRENT_BINARY_DIR}/camkes_log_relay
    overlay_radio
    "usr/bin"
    FILES
    camkes_log_relay
)

ExternalProject_Add(
    Radio_UxAS
    GIT_REPOSITORY
    https://github.com/loonwerks/case-ta6-experimental-platform-OpenUxAS.git
    GIT_TAG
    develop-case-ta6-ph2-example01
    GIT_SHALLOW
    true
    INSTALL_COMMAND
    ""
    UPDATE_COMMAND
    ""
    CONFIGURE_COMMAND
    cd <SOURCE_DIR> && ./prepare && cd <BINARY_DIR> && meson setup <BINARY_DIR> <SOURCE_DIR> --cross-file=<SOURCE_DIR>/camkes-vm-arm-linux-gnueabi-cross-file.txt --buildtype=release
    BUILD_COMMAND
    ninja -C <BINARY_DIR> all
)

ExternalProject_Get_property(Radio_UxAS DOWNLOAD_DIR)
message("Download dir of UxAS = ${DOWNLOAD_DIR}")
MyAddExternalProjFilesToOverlay(
    Radio_UxAS
    ${DOWNLOAD_DIR}/Radio_UxAS-build
    overlay_radio
    "home/uxas/build"
    FILES
    uxas
)
MyAddExternalProjFilesToOverlay(
    Radio_UxAS
    ${DOWNLOAD_DIR}/Radio_UxAS/examples/CASE-TA6-Challenge-Problems/ph2_01_WaterwaySearch
    overlay_radio
    "home/uxas/ex/p2/01_Waterway"
    FILES
    runUxAS_WaterwaySearch_UAV.sh
)

# UxAS config
AddFileToOverlayDir(
    "cfg_WaterwaySearch_UAV.xml"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/ex/p2/01_Waterway/cfg_WaterwaySearch_UAV.xml
    "/home/uxas/ex/p2/01_Waterway"
    overlay_radio
)

# Overwrite inittab file for using the virtio console hvc0.
AddFileToOverlayDir(
    "inittab"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/inittab_hvc0
    "etc"
    overlay_radio
)
AddFileToOverlayDir(
    "wait_for_eth0.sh"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/wait_for_eth0.sh
    "etc"
    overlay_radio
)

# Overwrite /etc/network/interfaces file.
AddFileToOverlayDir(
    "interfaces"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/network/interfaces
    "etc/network"
    overlay_radio
)

# Use initrd with crossvm kernel module and setup already included.
set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/rootfs_crossvm.cpio.gz")
# Construct new rootfs
AddOverlayDirToRootfs(
    overlay_radio
    ${rootfs_file}
    "buildroot"
    "rootfs_install"
    output_overlayed_rootfs_location
    rootfs_target_radio
    GZIP
)
AddToFileServer("linux-initrd-vm-client-radio" ${output_overlayed_rootfs_location} DEPENDS rootfs_target_radio)

CAmkESAddImportPath(${CMAKE_CURRENT_SOURCE_DIR}/${KernelARMPlatform}/)

# Define our VM Component with out cross vm dataports glue code
ExtendCAmkESComponentInstance(Radio_VM vmRadio SOURCES src/cross_vm_connections_vmRadio.c)

# Link the vm component against the queue library.
DeclareCAmkESComponent(Radio_VM LIBS queue)

CAmkESAddCPPInclude(${CAMKES_ARM_VM_DIR}/components/VM)

DeclareCAmkESARMVM(Radio_VM)
