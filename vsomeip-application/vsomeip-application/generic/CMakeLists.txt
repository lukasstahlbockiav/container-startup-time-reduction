# Copyright (C) 2015-2017 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Updated by Lukas Stahlbock in 2022
# Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
# Initial code from https://github.com/COVESA/vsomeip/blob/3.1.16.1/examples/hello_world/CMakeLists.txt
#

cmake_minimum_required (VERSION 2.8.7)
project (vsomeip-generic)
set (VSOMEIP_NAME vsomeip3)

# This will get us acces to
#   VSOMEIP_INCLUDE_DIRS - include directories for vSomeIP
#   VSOMEIP_LIBRARIES    - libraries to link against

find_package(${VSOMEIP_NAME})
if (NOT ${VSOMEIP_NAME}_FOUND)
    message("${VSOMEIP_NAME} was not found. Please specify vsomeip_DIR")
endif()

find_package(Threads REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${VSOMEIP_INCLUDE_DIRS})

add_executable (vsomeip-generic main.cpp)
target_link_libraries(vsomeip-generic ${VSOMEIP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS vsomeip-generic DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
