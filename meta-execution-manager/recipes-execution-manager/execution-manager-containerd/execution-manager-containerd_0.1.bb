##
##  Created by Lukas Stahlbock in 2022
##  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
##

DESCRIPTION = "Execution-Manager for containerd"
DEPENDS += "nlohmann-json (>= 3.7.0) protobuf-c-native protobuf-c grpc grpc-native"
RDEPENDS_${PN} = " grpc"
#TOOLCHAIN_HOST_TASK_append = " nativesdk-protobuf-compiler nativesdk-protobuf nativesdk-grpc"
#TOOLCHAIN_TARGET_TASK_append = " grpc"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${THISDIR}/../../../LICENSE;md5=7ff2784717282fe96cd0290c5d9570e0"

FILESEXTRAPATHS_prepend := "${THISDIR}/:"

SRCREV_protobuf = "df2bce345d4bc8cdc3eba2a866e11e79e1fff4df"
SRCREV_containerd = "d12516713c315ea9e651eb1df89cf32ff7c8137c"

PATH_PROTOBUF = "${WORKDIR}/${PN}/src/proto/google/protobuf"
PATH_CONTAINERD = "${WORKDIR}/${PN}/src/proto/github.com/containerd/containerd"

SRC_URI = "file://${PN}/src/ \
            git://git@github.com/google/protobuf.git;protocol=ssh;branch=3.11.x;name=protobuf;destsuffix=${PATH_PROTOBUF} \
            git://git@github.com/containerd/containerd.git;protocol=ssh;branch=release/1.6;name=containerd;destsuffix=${PATH_CONTAINERD}/api;subpath=api"

SRC_URI += "file://${PN}/containerd/remove_gogoproto_services_containers.patch;patchdir=${PATH_CONTAINERD} \
            file://${PN}/containerd/remove_gogoproto_services_content.patch;patchdir=${PATH_CONTAINERD}  \
            file://${PN}/containerd/remove_gogoproto_services_snapshots.patch;patchdir=${PATH_CONTAINERD}  \
            file://${PN}/containerd/remove_gogoproto_services_tasks.patch;patchdir=${PATH_CONTAINERD}  \
            file://${PN}/containerd/remove_gogoproto_types.patch;patchdir=${PATH_CONTAINERD} "

S = "${WORKDIR}/${PN}/src/"

inherit cmake pkgconfig

EXTRA_OECMAKE = "-DCMAKE_CROSSCOMPILING=ON"

FILES_${PN} += "${bindir}/*"