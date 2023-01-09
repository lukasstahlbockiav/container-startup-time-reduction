# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ helloworld example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building helloworld.

#
#  Updated by Lukas Stahlbock in 2022
#  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
#  Initial code from https://github.com/grpc/grpc/blob/v1.51.1/examples/cpp/helloworld/CMakeLists.txt
#

cmake_minimum_required(VERSION 3.5.1)

project(execution-manager-containerd C CXX)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

find_package(Threads REQUIRED)

if(GRPC_AS_SUBMODULE)
  # One way to build a projects that uses gRPC is to just include the
  # entire gRPC project tree via "add_subdirectory".
  # This approach is very simple to use, but the are some potential
  # disadvantages:
  # * it includes gRPC's CMakeLists.txt directly into your build script
  #   without and that can make gRPC's internal setting interfere with your
  #   own build.
  # * depending on what's installed on your system, the contents of submodules
  #   in gRPC's third_party/* might need to be available (and there might be
  #   additional prerequisites required to build them). Consider using
  #   the gRPC_*_PROVIDER options to fine-tune the expected behavior.
  #
  # A more robust approach to add dependency on gRPC is using
  # cmake's ExternalProject_Add (see cmake_externalproject/CMakeLists.txt).
  
  # Include the gRPC's cmake build (normally grpc source code would live
  # in a git submodule called "third_party/grpc", but this example lives in
  # the same repository as gRPC sources, so we just look a few directories up)
  add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/grpc EXCLUDE_FROM_ALL)
  message(STATUS "Using gRPC via add_subdirectory.")

  # After using add_subdirectory, we can now use the grpc targets directly from
  # this build.
  set(_PROTOBUF_LIBPROTOBUF libprotobuf)
  set(_REFLECTION grpc++_reflection)
  if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
  else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
  endif()
  set(_GRPC_GRPCPP grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
  endif()
elseif(GRPC_FETCHCONTENT)
  # Another way is to use CMake's FetchContent module to clone gRPC at
  # configure time. This makes gRPC's source code available to your project,
  # similar to a git submodule.
  message(STATUS "Using gRPC via add_subdirectory (FetchContent).")
  include(FetchContent)
  FetchContent_Declare(
    grpc
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    # when using gRPC, you will actually set this to an existing tag, such as
    # v1.25.0, v1.26.0 etc..
    # For the purpose of testing, we override the tag used to the commit
    # that's currently under test.
    GIT_TAG        vGRPC_TAG_VERSION_OF_YOUR_CHOICE)
  FetchContent_MakeAvailable(grpc)

  # Since FetchContent uses add_subdirectory under the hood, we can use
  # the grpc targets directly from this build.
  set(_PROTOBUF_LIBPROTOBUF libprotobuf)
  set(_REFLECTION grpc++_reflection)
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
  set(_GRPC_GRPCPP grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
  endif()
else()
  # This branch assumes that gRPC and all its dependencies are already installed
  # on this system, so they can be located by find_package().

  # Find Protobuf installation
  # Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
  #set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
  set(protobuf_MODULE_COMPATIBLE TRUE)
  find_package(Protobuf REQUIRED protobuf)
  #include(FindPkgConfig)
  #find_package(PkgConfig REQUIRED)
  #pkg_check_modules(Protobuf REQUIRED protobuf)
  message(STATUS "Using protobuf ${Protobuf_VERSION}")

  set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
  set(_REFLECTION gRPC::grpc++_reflection)
  if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
  else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
  endif()

  # Find gRPC installation
  # Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
  find_package(gRPC CONFIG REQUIRED)
  message(STATUS "Using gRPC ${gRPC_VERSION}")

  set(_GRPC_GRPCPP gRPC::grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
  endif()
endif()

# Add external json library
#set(JSON_BuildTests OFF CACHE "Internal" "")
#add_subdirectory(third_party/json)
#set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
find_package(nlohmann_json)

# Proto files
add_subdirectory(proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
set_source_files_properties(${PROTO_SRC} ${PROTO_HEADER} PROPERTIES GENERATED TRUE)
#include_directories(${PROTOBUF_INCLUDE_PATH})
#add_executable( ${EXEC} ${SRCS} ${PROTO_SRC} ${PROTO_HEADER} )
#target_link_libraries( ${EXEC} ${PROTOBUF_LIBRARIES} )

set(proto_path "${CMAKE_SOURCE_DIR}/proto/")
#get_filename_component(proto_path_helper "${CMAKE_SOURCE_DIR}/proto/CMakeLists.txt" ABSOLUTE)
#get_filename_component(proto_path "${proto_path_helper}" PATH)
## services/containers
get_filename_component(services_containers_proto "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/services/containers/v1/containers.proto" ABSOLUTE)
get_filename_component(services_containers_proto_path "${services_containers_proto}" PATH)

set(services_containers_gen_path "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/services/containers/v1/")
set(services_containers_proto_srcs "${services_containers_gen_path}/containers.pb.cc")
set(services_containers_proto_hdrs "${services_containers_gen_path}/containers.pb.h")
set(services_containers_grpc_srcs "${services_containers_gen_path}/containers.grpc.pb.cc")
set(services_containers_grpc_hdrs "${services_containers_gen_path}/containers.grpc.pb.h")

set(list_services_containers "${services_containers_proto_srcs}" "${services_containers_proto_hdrs}" "${services_containers_grpc_srcs}" "${services_containers_grpc_hdrs}" "${services_containers_proto}")

## services/snapshots
get_filename_component(services_snapshots_proto "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/services/snapshots/v1/snapshots.proto" ABSOLUTE)
get_filename_component(services_snapshots_proto_path "${services_snapshots_proto}" PATH)

set(services_snapshots_gen_path "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/services/snapshots/v1/")
set(services_snapshots_proto_srcs "${services_snapshots_gen_path}/snapshots.pb.cc")
set(services_snapshots_proto_hdrs "${services_snapshots_gen_path}/snapshots.pb.h")
set(services_snapshots_grpc_srcs "${services_snapshots_gen_path}/snapshots.grpc.pb.cc")
set(services_snapshots_grpc_hdrs "${services_snapshots_gen_path}/snapshots.grpc.pb.h")

set(list_services_snapshots "${services_snapshots_proto_srcs}" "${services_snapshots_proto_hdrs}" "${services_snapshots_grpc_srcs}" "${services_snapshots_grpc_hdrs}" "${services_snapshots_proto}")

## services/tasks
get_filename_component(services_tasks_proto "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/services/tasks/v1/tasks.proto" ABSOLUTE)
get_filename_component(services_tasks_proto_path "${services_tasks_proto}" PATH)

set(services_tasks_gen_path "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/services/tasks/v1/")
set(services_tasks_proto_srcs "${services_tasks_gen_path}/tasks.pb.cc")
set(services_tasks_proto_hdrs "${services_tasks_gen_path}/tasks.pb.h")
set(services_tasks_grpc_srcs "${services_tasks_gen_path}/tasks.grpc.pb.cc")
set(services_tasks_grpc_hdrs "${services_tasks_gen_path}/tasks.grpc.pb.h")

set(list_services_tasks "${services_tasks_proto_srcs}" "${services_tasks_proto_hdrs}" "${services_tasks_grpc_srcs}" "${services_tasks_grpc_hdrs}" "${services_tasks_proto}")

## types/task
get_filename_component(types_task_proto "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/types/task/task.proto" ABSOLUTE)
get_filename_component(types_task_proto_path "${types_task_proto}" PATH)

set(types_task_gen_path "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/types/task/")
set(types_task_proto_srcs "${types_task_gen_path}/task.pb.cc")
set(types_task_proto_hdrs "${types_task_gen_path}/task.pb.h")
set(types_task_grpc_srcs "${types_task_gen_path}/task.grpc.pb.cc")
set(types_task_grpc_hdrs "${types_task_gen_path}/task.grpc.pb.h")

set(list_types_task "${types_task_proto_srcs}" "${types_task_proto_hdrs}" "${types_task_grpc_srcs}" "${types_task_grpc_hdrs}" "${types_task_proto}")

## types/descriptor
get_filename_component(types_descriptor_proto "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/types/descriptor.proto" ABSOLUTE)
get_filename_component(types_descriptor_proto_path "${types_descriptor_proto}" PATH)

set(types_descriptor_gen_path "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/types/")
set(types_descriptor_proto_srcs "${types_descriptor_gen_path}/descriptor.pb.cc")
set(types_descriptor_proto_hdrs "${types_descriptor_gen_path}/descriptor.pb.h")
set(types_descriptor_grpc_srcs "${types_descriptor_gen_path}/descriptor.grpc.pb.cc")
set(types_descriptor_grpc_hdrs "${types_descriptor_gen_path}/descriptor.grpc.pb.h")

set(list_types_descriptor "${types_descriptor_proto_srcs}" "${types_descriptor_proto_hdrs}" "${types_descriptor_grpc_srcs}" "${types_descriptor_grpc_hdrs}" "${types_descriptor_proto}")

## types/metrics
get_filename_component(types_metrics_proto "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/types/metrics.proto" ABSOLUTE)
get_filename_component(types_metrics_proto_path "${types_metrics_proto}" PATH)

set(types_metrics_gen_path "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/types/")
set(types_metrics_proto_srcs "${types_metrics_gen_path}/metrics.pb.cc")
set(types_metrics_proto_hdrs "${types_metrics_gen_path}/metrics.pb.h")
set(types_metrics_grpc_srcs "${types_metrics_gen_path}/metrics.grpc.pb.cc")
set(types_metrics_grpc_hdrs "${types_metrics_gen_path}/metrics.grpc.pb.h")

set(list_types_metrics "${types_metrics_proto_srcs}" "${types_metrics_proto_hdrs}" "${types_metrics_grpc_srcs}" "${types_metrics_grpc_hdrs}" "${types_metrics_proto}")

## types/mount
get_filename_component(types_mount_proto "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/types/mount.proto" ABSOLUTE)
get_filename_component(types_mount_proto_path "${types_mount_proto}" PATH)

set(types_mount_gen_path "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/types/")
set(types_mount_proto_srcs "${types_mount_gen_path}/mount.pb.cc")
set(types_mount_proto_hdrs "${types_mount_gen_path}/mount.pb.h")
set(types_mount_grpc_srcs "${types_mount_gen_path}/mount.grpc.pb.cc")
set(types_mount_grpc_hdrs "${types_mount_gen_path}/mount.grpc.pb.h")

set(list_types_mount "${types_mount_proto_srcs}" "${types_mount_proto_hdrs}" "${types_mount_grpc_srcs}" "${types_mount_grpc_hdrs}" "${types_mount_proto}")

## types/platform
get_filename_component(types_platform_proto "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/types/platform.proto" ABSOLUTE)
get_filename_component(types_platform_proto_path "${types_platform_proto}" PATH)

set(types_platform_gen_path "${CMAKE_SOURCE_DIR}/proto/github.com/containerd/containerd/api/types/")
set(types_platform_proto_srcs "${types_platform_gen_path}/platform.pb.cc")
set(types_platform_proto_hdrs "${types_platform_gen_path}/platform.pb.h")
set(types_platform_grpc_srcs "${types_platform_gen_path}/platform.grpc.pb.cc")
set(types_platform_grpc_hdrs "${types_platform_gen_path}/platform.grpc.pb.h")

set(list_types_platform "${types_platform_proto_srcs}" "${types_platform_proto_hdrs}" "${types_platform_grpc_srcs}" "${types_platform_grpc_hdrs}" "${types_platform_proto}")

# Protoc generation
set(list_group list_services_containers 
                list_services_tasks 
                list_services_snapshots 
                list_types_task
                list_types_descriptor
                list_types_metrics
                list_types_mount
                list_types_platform)

foreach(target_list IN LISTS list_group)
  #message("Target list is: ${target_list}")
  list(GET ${target_list} 0 proto_srcs)
  list(GET ${target_list} 1 proto_hdrs)
  list(GET ${target_list} 2 grpc_srcs)
  list(GET ${target_list} 3 grpc_hdrs)
  list(GET ${target_list} 4 proto)
  message("${_PROTOBUF_PROTOC} --grpc_out ${gen_path} --cpp_out ${gen_path} -I ${proto_path} --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE} ${proto}")
  add_custom_command(
    OUTPUT "${proto_srcs}" "${proto_hdrs}" "${grpc_srcs}" "${grpc_hdrs}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${proto_path}"
      --cpp_out "${proto_path}"
      -I "${proto_path}"
      --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
      "${proto}"
    DEPENDS "${proto}")
endforeach()


# Include generated *.pb.h files
include_directories("${CMAKE_SOURCE_DIR}/proto/")

add_executable(execution-manager-containerd 
              main.cc logging.cc containerdclient.cc containersclient.cc 
              snapshotsclient.cc tasksclient.cc configuration.cc batch_command.cc 
              ${types_task_proto_srcs} ${types_task_grpc_srcs}
              ${types_descriptor_proto_srcs} ${types_descriptor_grpc_srcs}
              ${types_metrics_proto_srcs} ${types_metrics_grpc_srcs}
              ${types_mount_proto_srcs} ${types_mount_grpc_srcs}
              ${types_platform_proto_srcs} ${types_platform_grpc_srcs}
              ${services_containers_proto_srcs} ${services_containers_grpc_srcs} 
              ${services_tasks_proto_srcs} ${services_tasks_grpc_srcs}
              ${services_snapshots_proto_srcs} ${services_snapshots_grpc_srcs})

target_link_libraries(execution-manager-containerd ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF} ${PROTOBUF_LIBRARY} nlohmann_json::nlohmann_json)

install(TARGETS execution-manager-containerd DESTINATION "/usr/bin")
# Targets greeter_[async_](client|server)
#foreach(_target
#  main)
#  add_executable(${_target} "${_target}.cc" ${cc_proto_srcs} ${cc_grpc_srcs})
#  target_link_libraries(${_target} ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})
#endforeach()

