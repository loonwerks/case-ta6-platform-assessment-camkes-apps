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
HOST_DIR=`pwd` make user
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

### case-uav-step1

## Installing on ODROID-XU4
