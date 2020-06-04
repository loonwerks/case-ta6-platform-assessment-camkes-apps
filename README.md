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
described below.  But, it isn't installed by default in the docker containers for the CAmkES environment.  Likewise,
building the CakeML filter components requires the CakeML compiler targeting the ARM7L architecture.  This items may be
easily installed by adding to the extras.dockerfile.  Edit seL4-CAmkES-L4v-dockerfiles/dockerfiles/extras.dockerfile (using vi or
your favorite editor) and adding the lines at the end of the file.

~~~
RUN apt-get update -q \
    && apt-get install -y meson \
    && apt-get clean autoclean \
    && apt-get autoremove --yes \
    && rm -rf /var/lib/{apt,dpkg,cache,log}/

RUN curl http://www.cse.chalmers.se/~myreen/cake-1239-x64-32.tar.gz > cake-x64-32.tar.gz \
    && tar -xvzf cake-x64-32.tar.gz && cd cake-x64-32 && make cake \
    && mv cake /usr/bin/cake32
~~~

Building the platform assessment applications requires use of [CAmkES](https://docs.sel4.systems/projects/camkes/)
environment and the [CAmkES ARM VM](https://docs.sel4.systems/projects/camkes-arm-vm/) which may be obtained by
following their instructions, summarized below.

~~~
mkdir case-ta6-platform-assessment-camkes
cd case-ta6-platform-assessment-camkes
repo init -u https://github.com/kent-mcleod/camkes.git -b kent/aadl --depth=1 -m master.xml
repo sync
~~~

The applications in this repository can then be added to the resulting directory tree by the following commands.
Eventually, the necessary elements in the CAmkES repository will be merged into the main line of the canonical
CAmkES repository.  Accordingly, the `repo init` command above is expected to change to
`repo init -u https://github.com/SEL4PROJ/camkes-arm-vm-manifest.git -b kent/aadl --depth=1`.

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
../init-build.sh -DPLATFORM=exynos5422 -DCAMKES_APP=<application_name> -DARM_HYP=ON -DARM=ON
ninja
~~~

where the `<application_name>` is replaced with one of the applications listed in the section describing the included
applications.

The resulting seL4 image can be found in the build/images directory.

### Serial I/O Cooking

By default the seL4 platform support library inserts a carriage return after each linefeed.  The code that does this is
located in the portion of the code that fills the FIFO.  For the Exynos5422 processor, this is located in
projects/util_libs/libplatsupport/src/mach/exynos/serial.c at lines 152..161.  Since the applications in this repository
transport binary data from the ODROID-XU4 to a host computer via serial port.  These lines should be commented out.
Otherwise, checksum errors will result at the receiving side of the serial bus.

## Included Applications

### case-gs-step1

The step 1 ground station application builds a seL4 image containing a single virtual machine.  The hardware ethernet
device is mapped through to the VM and the VM contains OpenUxAS and configuration files to run as the ground station
from the waterway search example.

### case-gs-step2

The step 2 ground station application builds a seL4 image containing a single virtual machine.  The hardware ethernet
device is mapped through to the VM and the VM contains OpenUxAS and configuration files to run as the ground station
from the waterway search example.  The VM also contains the User Attestation Manager.

### case-uav-step1

The step 1 unmanned air vehicle application builds a seL4 image containing a single virtual machine.  The hardware ethernet
device is mapped through to the VM and the VM contains OpenUxAS and configuration files to run as the unmanned air vehicle
from the waterway search example.

### case-uav-step2

The step 2 unmanned air vehicle application builds a seL4 image containing a single virtual machine with the UxAS Waypoint
Plan Manager Service and the autopilot (modeled by AMASE) via a serial port moved into native CAmkES components.  The hardware
ethernet device is mapped through to the VM and the VM contains OpenUxAS and configuration files to run as the unmanned air
vehicle from the waterway search example.

### case-uav-step3

The step 3 unmanned air vehicle application extends the case-uav-step2 application by separating the UxAS virtual machine into
two machines, one modeling the Radio and the other containing the UxAS services.  The hardware ethernet device is mapped
through to the VM modeling the radio.  Thus, the VM containing OpenUxAS has no direct communication with the outside world;
all communications are through other components and via seL4 connections where monitors filters and other guards may be
installed.

### case-uav-step4

The step 4 unmanned air vehicle application extends the case-uav-step3 application by including the following high-assurance 
components: Attestation Gate, Operating Region Filter, Line Search Task Filter, Automation Request Filter, Response Monitor, 
and Geofence Monitor.  The current behavior of the high-assurance components is to pass messages straight through with no 
processing.

### case-uav-step5

The step 5 unmanned air vehicle application extends the case-uav-step3 application by including the Attestation Manager in the 
Radio VM.

### case-uav-threat-3A1-unmitigated

The unmanned air vehicle threat 3A1 unmitigated application extends the case-uav-step4 application by intruducing the threat
trojan code to render a sensitivity to a buffer overrun attack in the form of an overly-long line search task waypoint list.
This application is intended to demonstrate that such an attack succeeds if high-assurance mitigations are not in place.  A
corresponding line search task message definition XML file containing an overly-long waypoint list is then used to demonstrate
the vulnerability.

### case-uav-threat-3A1

The unmanned air vehicle threat 3A1 application extends the case-uav-threat-3A1-unmitigated application by introducing
high-assurance componenets including filters, gates and monitors to mitigate some cyber threats.  The OpenUxAS application
contains the same injected trojan code as the case-uav-threat-3A1-unmitigated scenario.  Once the overly-lengthy waypoint list
reaches the OpenUxAS application, the vulnerability is triggered and the OpenUxAS application fails to generate an automation
response.  The response monitor responds by warning the user that an automation response was expected but none was generated.

### case-uav-threat-3A2-unmitigated

The unmanned air vehicle threat 3A2 unmitigated application extends the case-uav-step4 application by intruducing the threat
trojan code to render a sensitivity to invalid input data in the form of waypoints with out-of-range longitude values.
This application is intended to demonstrate that such an attack succeeds if high-assurance mitigations are not in place.  A
corresponding line search task message definition XML file containing an waypoints with out-of-rage longitude values is then
used to demonstrate the vulnerability.

### case-uav-threat-3A2

The unmanned air vehicle threat 3A2 application extends the case-uav-threat-3A2-unmitigated application by introducing
high-assurance componenets including filters, gates and monitors to mitigate some cyber threats.  The OpenUxAS application
contains the same injected trojan code as the case-uav-threat-3A2-unmitigated scenario, but the line search task filter catches
the faulty waypoint longitudes and prevents the message from reaching the OpenUxAS application.  This results in the response
monitor warning the user that an automation response was expected but none was generated.

### case-uav-threat-10A-unmitigated

The unmanned air vehicle threat 10A unmitigated application extends the case-uav-step4 application by intruducing the threat
trojan code to inject undesired behavior in the form of commanding the UAV to fly through a Keep-Out-Zone airspace. This
application is intended to demonstrate that such an attack succeeds if high-assurance mitigations are not in place.

### case-uav-threat-10A

The unmanned air vehicle threat 10A application extends the case-uav-threat-10A-unmitigated application by introducing
high-assurance componenets including filters, gates and monitors to mitigate some cyber threats.  The OpenUxAS application
contains the same injected trojan code as the case-uav-threat-10A-unmitigated scenario, but the line search task filter catches
the waypoints located in the keep-out zone and triggers an alert to send the UAV back to home base.

### case-uav-threat-10B-unmitigated

The unmanned air vehicle threat 10B unmitigated application extends the case-uav-step4 application by intruducing the threat
trojan code to inject undesired behavior in the form of commanding the UAV to fly into a (hypothetical) tethered balloon. This
application is intended to demonstrate that such an attack succeeds if high-assurance mitigations are not in place.

### case-uav-threat-10B

The unmanned air vehicle threat 10B application extends the case-uav-threat-10B-unmitigated application by introducing
high-assurance componenets including filters, gates and monitors to mitigate some cyber threats.  The OpenUxAS application
contains the same injected trojan code as the case-uav-threat-10B-unmitigated scenario, but the line search task filter catches
the suspicious waypoints and triggers an alert to send the UAV back to home base.

## Installing on ODROID-XU4

The ODROID-XU4 must be provisioned either with an MMC card or a MicroSD card containing the first and second stage bootloaders,
the trust zone software and uBoot.  Instructions may be found at the
[seL4 on ODROID-XU4](https://docs.sel4.systems/Hardware/OdroidXU.html) page.

It is important to note that in addition, seL4 depends on uBoot to initialize the USB devices.  The `usb start`
command should be made during the boot process from uBoot.

Alternatively to fastboot, the seL4 capdl image may also be placed in file in a FAT formatted partition on the MMC or MicroSD
card and the loaded from that partion.  The boot then may be made as follows:

~~~
mmc init; fatload mmc 0 ${loadaddr} capdl-loader-image-arm-exynos5; usb start; bootelf ${loadaddr}
~~~
