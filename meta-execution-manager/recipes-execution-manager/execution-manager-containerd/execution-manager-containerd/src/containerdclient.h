/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#ifndef CONTAINERD_CLIENT_H
#define CONTAINERD_CLIENT_H

#include "logging.h"
#include <unistd.h>

#include "containersclient.h"
#include "tasksclient.h"
#include "snapshotsclient.h"

#define MAX_CONTAINERS 50

class ContainerdClient : public ContainersClient, public SnapshotsClient, public TasksClient{
    public:
        explicit ContainerdClient(std::shared_ptr<Channel> channel) : ContainersClient(channel),
                                                                        SnapshotsClient(channel),
                                                                        TasksClient(channel){};
        void killAndDeleteContainer(const std::string& ctrdNamespace, const std::string& containerId);
        void killAndDeleteTask(const std::string& ctrdNamespace, const std::string& containerId);

};

#endif /* CONTAINERD_CLIENT_H */