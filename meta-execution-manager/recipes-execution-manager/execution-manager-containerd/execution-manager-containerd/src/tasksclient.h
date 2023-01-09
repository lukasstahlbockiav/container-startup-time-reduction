/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#ifndef TASKS_CLIENT_H
#define TASKS_CLIENT_H

#include "logging.h"

#include "github.com/containerd/containerd/api/services/tasks/v1/tasks.grpc.pb.h"

#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using containerd::services::tasks::v1::ListTasksRequest;
using containerd::services::tasks::v1::ListTasksResponse;
using containerd::services::tasks::v1::CreateTaskRequest;
using containerd::services::tasks::v1::CreateTaskResponse;
using containerd::services::tasks::v1::StartRequest;
using containerd::services::tasks::v1::StartResponse;
using containerd::services::tasks::v1::KillRequest;
using containerd::services::tasks::v1::DeleteTaskRequest;
using containerd::services::tasks::v1::DeleteResponse;
using containerd::services::tasks::v1::Tasks;

class TasksClient {
    public:
        explicit TasksClient(std::shared_ptr<Channel> channel) : tasksStub_(Tasks::NewStub(channel)){};
        
        ListTasksResponse listTasks(const std::string& ctrdNamespace);
        CreateTaskResponse createTask(const std::string& ctrdNamespace, CreateTaskRequest& request);
        StartResponse startTask(const std::string& ctrdNamespace, const std::string& containerId);
        void killTask(const std::string& ctrdNamespace, const std::string& containerId);
        DeleteResponse deleteTask(const std::string& ctrdNamespace, const std::string& containerId);

    private:
        std::unique_ptr<Tasks::Stub> tasksStub_;
};

#endif /* TASKS_CLIENT_H */