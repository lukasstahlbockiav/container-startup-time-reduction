/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#include <string>
#include <vector>
#include <future>
#include <unistd.h>

#include "logging.h"
#include "configuration.h"
#include "containerdclient.h"
#include "batch_command.h"

using containerd::types::Mount;

void command(ContainerdClient& client, struct config_containers_s config_containers[], int n_c) 
{ 
  std::future<PrepareSnapshotResponse> futureSnapshotPrepare;
  PrepareSnapshotResponse responseSnapshotPrepare;

  google::protobuf::Any serializedOciSpec;
  std::future<CreateContainerResponse> futureContainersCreate;
  CreateContainerResponse responseContainersCreate;
  Container ctr;
  Container_Runtime runtime;
  
  CreateTaskRequest requestTask;
  int i, j;
  Mount *mnt_ptr;
  std::future<CreateTaskResponse> futureTasksCreate;
  CreateTaskResponse responseTasksCreate;

  std::future<StartResponse> futureTasksStart;
  StartResponse responseTasksStart;

  std::future<void> futureContainersDelete;
  std::future<void> futureTaskDelete;

  if (config_containers[n_c].command == "prepare"){
    // /snapshots/prepare
    futureSnapshotPrepare = std::async(std::launch::async, &ContainerdClient::prepareSnapshot, &client, "default", "overlayfs", config_containers[n_c].name, config_containers[n_c].digest);    
    responseSnapshotPrepare = futureSnapshotPrepare.get();

    // /containers/create
    json ociSpec = client.loadOciSpec(config_containers[n_c].oci_spec);
    serializedOciSpec = client.modifyAndSerializeOciSpec(ociSpec);

    runtime.set_name(config_containers[n_c].runtime);

    ctr.set_id(config_containers[n_c].name);
    ctr.set_image(config_containers[n_c].image);
    ctr.set_allocated_runtime(&runtime);
    ctr.set_snapshotter("overlayfs");
    ctr.set_snapshot_key(config_containers[n_c].name);
    ctr.set_allocated_spec(&serializedOciSpec);

    futureContainersCreate = std::async(std::launch::async, &ContainerdClient::createContainer, &client, "default", std::ref(ctr));
    responseContainersCreate = futureContainersCreate.get();
    ctr.release_runtime();
    ctr.release_spec();
  }

  if (config_containers[n_c].command == "create" || config_containers[n_c].command == "run"){
    // /tasks/create
    requestTask.set_container_id(config_containers[n_c].name);

    /*
    // OBSOLETE SETTING OF MOUNT DATA - SHOULD BE PROVIDED VIA JSON CONFIG
    i = 0;
    while (i < responseSnapshotPrepare.mounts_size()){
        //std::cout << "Mount: " << i << std::endl;
        mnt_ptr = requestTask.add_rootfs();
        mnt_ptr->set_type(responseSnapshotPrepare.mounts().at(i).type());
        mnt_ptr->set_source(responseSnapshotPrepare.mounts().at(i).source());
        mnt_ptr->set_target(responseSnapshotPrepare.mounts().at(i).target());
        j = 0;
        while (j < responseSnapshotPrepare.mounts().at(i).options_size()){
            mnt_ptr->add_options(responseSnapshotPrepare.mounts().at(i).options().at(j));
            j++;
        }
        i++;
    }
    */
    mnt_ptr = requestTask.add_rootfs();
    mnt_ptr->set_type(config_containers[n_c].mount_type);
    mnt_ptr->set_source(config_containers[n_c].mount_source);
    mnt_ptr->set_target(config_containers[n_c].mount_target);
    i = 0;
    while (i < config_containers[n_c].mount_options->size()){
        mnt_ptr->add_options(config_containers[n_c].mount_options[i]);
        i++;
    }
    requestTask.set_terminal(false);
    //helper.reqCreateTasks.push_back(requestTask);
    futureTasksCreate = std::async(std::launch::async, &ContainerdClient::createTask, &client, "default", std::ref(requestTask));
    responseTasksCreate = futureTasksCreate.get();
    
  } 
  
  if (config_containers[n_c].command == "start" || config_containers[n_c].command == "run") {
    futureTasksStart = std::async(std::launch::async, &ContainerdClient::startTask, &client, "default", config_containers[n_c].name);
    responseTasksStart = futureTasksStart.get();
  } else if (config_containers[n_c].command == "kill"){
    client.killTask("default",config_containers[n_c].name);
  } else if (config_containers[n_c].command == "killAndDeleteTask") {
    futureTaskDelete = std::async(std::launch::async, &ContainerdClient::killAndDeleteTask, &client, "default", config_containers[n_c].name);
    futureTaskDelete.wait();
  } else if (config_containers[n_c].command == "killAndDeleteContainer") {
    futureContainersDelete = std::async(std::launch::async, &ContainerdClient::killAndDeleteContainer, &client, "default", config_containers[n_c].name);
    futureContainersDelete.wait();
  } else if (config_containers[n_c].command != "create") {
    createLogMessage(error, "Unknown command: " + config_containers[n_c].command);
  }
  
}

void fork_rec(pid_t *pids, int n_proc, ContainerdClient& client, struct config_containers_s config_containers[])
{

  if (n_proc >= 0)
  {
    pids[n_proc] = fork();
    if (pids[n_proc] < 0)
    {
      perror("fork");
    }
    else if (pids[n_proc] == 0)
    {
      command(client, config_containers, n_proc);
      exit(0);
    }
    else if (pids[n_proc] > 0)
    {
      fork_rec(pids, n_proc - 1, client, config_containers);
    }
  }
}