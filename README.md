# case-ta6-platform-assessment-camkes-apps

This repository contains CAmkES applications for the DARPA CASE program Platform Assessment #1.  The applications emplace
the AFRL-RQ [OpenUxAS](https://github.com/afrl-rq/OpenUxAS) software with CASE program cyber-security enhancements onto
ODROID-XU4 target hardware.

## Instructions

Building the applications is easier using the [Docker files](https://docs.sel4.systems/projects/dockerfiles/).
These may be installed by

~~~
git clone https://github.com/SEL4PROJ/seL4-CAmkES-L4v-dockerfiles.git
~~~

OpenUxAS uses the [meson build system](https://mesonbuild.com/) and this must be accessible when running the build
described below.  But, it isn't installed by default in the docker containers for the CAmkES environment.  This may be
easily remedied by adding it in the extras.dockerfile.  Edit seL4-CAmkES-L4v-dockerfiles/extras.dockerfile (using vi or
your favorite editor) and adding the lines at the end of the file.

~~~
RUN apt-get update -q \
    && apt-get install -y meson \
    && apt-get clean autoclean \
    && apt-get autoremove --yes \
    && rm -rf /var/lib/{apt,dpkg,cache,log}/
~~~

Building the platform assessment applications requires use of [CAmkES](https://docs.sel4.systems/projects/camkes/)
environment and the [CAmkES ARM VM](https://docs.sel4.systems/projects/camkes-arm-vm/) which may be obtained by
following their instructions, summarized below.

~~~
mkdir case-ta6-platform-assessment-camkes
cd case-ta6-platform-assessment-camkes
repo init -u https://github.com/SEL4PROJ/camkes-arm-vm-manifest.git
repo sync
~~~

The applications in this repository can then be added to the resulting directory tree by the following commands.

~~~
cd projects
git clone https://github.com/loonwerks/case-ta6-platform-assessment-camkes-apps.git case-ta6-platform-assessment
cd ..
ln -sf projects/case-ta6-platform-assessment/easy-settings.cmake
~~~

Enter the seL4/CAmkES build environment by

~~~
cd <location of seL4-CAmkES-L4v-dockerfiles>
make user HOST_DIR=<location of case-ta6-platform-assessment-camkes>
~~~

Build the application via the following commands

~~~
mkdir build
cd build
../init-build.sh -DPLATFORM=exynos5422 -DCAMKES_APP=case-uav-step1 -DARM_HYP=ON -DARM=ON
ninja
~~~

The resulting seL4 image can be found in the build/images directory.

## Included Applications

### case-gs-step1

The step 1 ground station application builds a seL4 image containing a single virtual machine.  The hardware ethernet
device is mapped through to the VM and the VM contains OpenUxAS and configuration files to run as the ground station
from the waterway search example.

### case-uav-step1

The step 1 unmanned air vehicle application builds a seL4 image containing a single virtual machine.  The hardware ethernet
device is mapped through to the VM and the VM contains OpenUxAS and configuration files to run as the unmanned air vehicle
from the waterway search example.

## Installing on ODROID-XU4

The ODROID-XU4 must be provisioned either with an MMC card or a MicroSD card containing the first and second stage bootloaders, the trust zone software and uBoot.  Instructions may be found at the
[seL4 on ODROID-XU4](https://docs.sel4.systems/Hardware/OdroidXU.html) page.

It is important to note that in addition, seL4 depends on uBoot to initialize the USB devices.  The `usb start`
command should be made during the boot process from uBoot.

Alternatively to fastboot, the seL4 capdl image may also be placed in file in a FAT formatted partition on the MMC or MicroSD
card and the loaded from that partion.  The boot then may be made as follows:

~~~
mmc init; fatload mmc 0 ${loadaddr} capdl-loader-image-arm-exynos5; usb start; bootelf ${loadaddr}
~~~
