##
##  Created by Lukas Stahlbock in 2022
##  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
##

DESCRIPTION = "Execution-Manager for crun or native applications"
DEPENDS = "yajl (>= 2.0.0)"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${THISDIR}/../../../LICENSE;md5=7ff2784717282fe96cd0290c5d9570e0"

FILESEXTRAPATHS_prepend := "${THISDIR}:"

SRC_URI = "file://${PN}/ \
           file://${PN}/src/"

S = "${WORKDIR}/${PN}"
inherit autotools pkgconfig