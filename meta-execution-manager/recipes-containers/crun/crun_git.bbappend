LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263"


SRCREV_crun = "3b3061afdeb49d691cfbbeaca09c0a0f45694cb2"
SRCREV_libocispec = "ace546f0a9c82c1878b93a43dab414d41da20821"
SRCREV_ispec = "693428a734f5bab1a84bd2f990d92ef1111cd60c"
SRCREV_rspec = "a3c33d663ebc56c4d35dbceaa447c7bf37f6fab3"
SRCREV_yajl = "f344d21280c3e4094919fd318bc5ce75da91fc06"

#FILESEXTRAPATHS_prepend := "${THISDIR}/patches:"
SRCREV_FORMAT = "crun_rspec"
SRC_URI = "git://github.com/containers/crun.git;branch=main;name=crun;protocol=https \
           git://github.com/containers/libocispec.git;branch=main;name=libocispec;destsuffix=git/libocispec;protocol=https \
           git://github.com/opencontainers/runtime-spec.git;branch=main;name=rspec;destsuffix=git/libocispec/runtime-spec;protocol=https \
           git://github.com/opencontainers/image-spec.git;branch=main;name=ispec;destsuffix=git/libocispec/image-spec;protocol=https \
           git://github.com/containers/yajl.git;branch=main;name=yajl;destsuffix=git/libocispec/yajl;protocol=https \
          "

PATCHTOOL = "git"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI += "file://crun_c.patch \
            file://libcrun_error_c.patch \
            file://batch_c.patch \
            file://batch_h.patch \
            file://batch_command_c.patch \
            file://batch_command_h.patch \
            file://configuration_c.patch \
            file://configuration_h.patch \
            file://configure_ac.patch \
            file://Makefile_am.patch"

PV = "1.4.3+git${SRCREV_crun}"

REQUIRED_DISTRO_FEATURES ?= "systemd"
DEPENDS += "systemd"

inherit autotools-brokensep pkgconfig

inherit features_check
REQUIRED_DISTRO_FEATURES ?= "seccomp"

DEPENDS = "yajl libcap go-md2man-native m4-native"
# TODO: is there a packageconfig to turn this off ?
DEPENDS += "libseccomp"
DEPENDS += "systemd"
DEPENDS += "oci-image-spec oci-runtime-spec"

do_configure_prepend () {
    # extracted from autogen.sh in crun source. This avoids
    # git submodule fetching.
    mkdir -p m4
    autoreconf -fi
}
