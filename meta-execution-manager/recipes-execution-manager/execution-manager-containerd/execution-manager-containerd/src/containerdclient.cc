/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#include <iostream>

#include "containerdclient.h"

void ContainerdClient::killAndDeleteContainer(const std::string& ctrdNamespace, const std::string& containerId){
    usleep(1000000); // sleep 1 second before killing the task
    this->killTask(ctrdNamespace, containerId);
    usleep(1000000); // sleep 0.5 second before deleting the task
    DeleteResponse respTaskDelete = this->deleteTask(ctrdNamespace, containerId);
    usleep(5000000); // sleep 0.5 second before deleting the container
    this->deleteContainer(ctrdNamespace, containerId);
}

void ContainerdClient::killAndDeleteTask(const std::string& ctrdNamespace, const std::string& containerId){
    usleep(1000000); // sleep 1 second before killing the task
    this->killTask(ctrdNamespace, containerId);
    usleep(1000000); // sleep 0.5 second before deleting the task
    DeleteResponse respTaskDelete = this->deleteTask(ctrdNamespace, containerId);
}