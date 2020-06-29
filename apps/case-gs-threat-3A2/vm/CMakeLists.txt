
cmake_minimum_required(VERSION 3.8.2)

project(arm-vm C)

includeGlobalComponents()
find_package(camkes-vm REQUIRED)
find_package(camkes-vm-images REQUIRED)
find_package(camkes-arm-vm REQUIRED)
find_package(camkes-vm-linux REQUIRED)
camkes_arm_vm_import_project()

include(${CAMKES_VM_LINUX_HELPERS_PATH})
include(ExternalProject)
include(external-project-helpers)

message("The value of CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message("The value of CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")

# Taken and modified from tools/seL4/cmake-tool/helpers/external-project-helpers.cmake
# Function to add a given file to an overlay directory.
# filename: The preferred name of the file in the overlay
# file_location: Absolute path to the file we are adding the overlay
# root_location: The location that the file we are adding will exist in the VM's root filesystem
# overlay_name: Overlay target name
function(MyAddFileToOverlayDir filename file_location root_location overlay_name)
    # Get any existing dependencies when adding the file into the overlay
    cmake_parse_arguments(PARSE_ARGV 4 ROOTFS_FILE_OVERLAY
        ""
        ""
        "DEPENDS"
    )
    if (NOT "${ROOTFS_FILE_OVERLAY_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "Unknown arguments to MyAddFileToOverlay")
    endif()
    set(rootfs_output_dir "${CMAKE_CURRENT_BINARY_DIR}/${overlay_name}")
    if(TARGET ${overlay_name})
        get_target_property(rootfs_output_dir ${overlay_name} ROOTFS_OUTPUT_DIR)
    else()
        add_custom_target(${overlay_name})
        set_property(TARGET ${overlay_name} APPEND PROPERTY ROOTFS_OUTPUT_DIR "${rootfs_output_dir}")
    endif()
    # Copy the file into the rootfs output directory at 'root_location'
    add_custom_command(OUTPUT ${rootfs_output_dir}/${root_location}/${filename}
        COMMAND ${CMAKE_COMMAND} -E copy "${file_location}" "${rootfs_output_dir}/${root_location}/${filename}"
        VERBATIM
        DEPENDS ${file_location} ${ROOTFS_FILE_OVERLAY_DEPENDS}
    )
    # Create a semi-unique target name based on the files location
    set(copy_target_name "copy_${rootfs_output_dir}/${root_location}/${filename}")
    string(REGEX REPLACE "/" "_" copy_target_name ${copy_target_name})
    string(MD5 copy_target_name ${copy_target_name})
    # Create a custom target for the copy command
    add_custom_target(${copy_target_name} DEPENDS "${rootfs_output_dir}/${root_location}/${filename}")
    # Add the copy command as a dependency for the overlay directory
    set_property(TARGET ${overlay_name} APPEND PROPERTY DEPENDS "${copy_target_name}")
    set_property(TARGET ${overlay_name} APPEND PROPERTY DEPENDS "${file_location}")
endfunction(MyAddFileToOverlayDir)

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
        MyAddFileToOverlayDir("${file_name}" ${external_install_dir}/${file} "${overlay_root_location}" ${overlay_target}
            DEPENDS ${external_target})
    endforeach()
endfunction(MyAddExternalProjFilesToOverlay)

ExternalProject_Add(
    useram
    GIT_REPOSITORY
    https://github.com/gaj7/am-bin-gs.git
    GIT_TAG
    meas
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
    overlay
    "usr/bin"
    FILES
    groundstation
)

ExternalProject_Add(
    UxASGS
    GIT_REPOSITORY
    https://github.com/loonwerks/case-ta6-experimental-platform-OpenUxAS.git
    GIT_TAG
    develop-case-ta6-ph2-example01-threat-3A2
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

ExternalProject_Get_property(UxASGS DOWNLOAD_DIR)
message("Download dir of UxASGS = ${DOWNLOAD_DIR}")
MyAddExternalProjFilesToOverlay(
    UxASGS
    ${DOWNLOAD_DIR}/UxASGS-build
    overlay
    "home/uxas/build"
    FILES
    uxas
)
MyAddExternalProjFilesToOverlay(
    UxASGS
    ${DOWNLOAD_DIR}/UxASGS/examples/CASE-TA6-Challenge-Problems/ph2_01_WaterwaySearch
    overlay
    "home/uxas/ex/p2/01_Waterway"
    FILES
    cfg_WaterwaySearch_GS.xml
    runUxAS_WaterwaySearch_GS.sh
)
MyAddExternalProjFilesToOverlay(
    UxASGS
    ${DOWNLOAD_DIR}/UxASGS/examples/CASE-TA6-Challenge-Problems/ph2_01_WaterwaySearch/Messages
    overlay
    "home/uxas/ex/p2/01_Waterway/Messages"
    FILES
    OperatingRegion_336.xml
)
MyAddExternalProjFilesToOverlay(
    UxASGS
    ${DOWNLOAD_DIR}/UxASGS/examples/CASE-TA6-Challenge-Problems/ph2_01_WaterwaySearch/Messages/tasks
    overlay
    "home/uxas/ex/p2/01_Waterway/Messages/tasks"
    FILES
    1000_LineSearch_LINE_Waterway_Deschutes_bad.xml
    1000_LineSearch_LINE_Waterway_Deschutes_good.xml
    1001_AutomationRequest_LINE_Waterway_Deschutes.xml
)

# Overwrite inittab file for using the virtio console hvc0.
AddFileToOverlayDir(
    "inittab"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/inittab_hvc0
    "etc"
    overlay
)

# Add init.d file for starting UxAS as service.
AddFileToOverlayDir(
    "S98uxas"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/init_scripts/init.d/S98uxas
    "etc/init.d"
    overlay
)

# Overwrite /etc/network/interfaces file.
AddFileToOverlayDir(
    "interfaces"
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay_files/network/interfaces
    "etc/network"
    overlay
)

# Use initrd with crossvm kernel module and setup already included.
set(rootfs_file "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/rootfs_crossvm.cpio.gz")
# Construct new rootfs
AddOverlayDirToRootfs(
    overlay
    ${rootfs_file}
    "buildroot"
    "rootfs_install"
    output_overlayed_rootfs_location
    rootfs_target
    GZIP
)
AddToFileServer("linux-initrd-vm-client" ${output_overlayed_rootfs_location} DEPENDS rootfs_target)
# Add linux kernel image to file server
AddToFileServer("linux" "${CAMKES_VM_IMAGES_DIR}/${KernelARMPlatform}/linux")

DefineCAmkESVMFileServer()

CAmkESAddImportPath(${CMAKE_CURRENT_SOURCE_DIR}/${KernelARMPlatform}/)

# Define our VM Component with out cross vm dataports glue code
# ExtendCAmkESComponentInstance(VM vmReceiver SOURCES src/cross_vm_connections_receiver.c)
# ExtendCAmkESComponentInstance(VM vmSender SOURCES src/cross_vm_connections_sender.c)

# Link the vm component against the queue library.
# DeclareCAmkESComponent(VM LIBS queue)

CAmkESAddCPPInclude(${CAMKES_ARM_VM_DIR}/components/VM)
