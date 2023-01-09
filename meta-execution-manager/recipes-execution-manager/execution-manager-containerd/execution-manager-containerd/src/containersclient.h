/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#ifndef CONTAINERS_CLIENT_H
#define CONTAINERS_CLIENT_H

#include "logging.h"

#include "github.com/containerd/containerd/api/services/containers/v1/containers.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using containerd::services::containers::v1::ListContainersRequest;
using containerd::services::containers::v1::ListContainersResponse;
using containerd::services::containers::v1::CreateContainerRequest;
using containerd::services::containers::v1::CreateContainerResponse;
using containerd::services::containers::v1::DeleteContainerRequest;
using containerd::services::containers::v1::Container;
using containerd::services::containers::v1::Containers;
using containerd::services::containers::v1::Container_Runtime;

class ContainersClient {
    public:
        explicit ContainersClient(std::shared_ptr<Channel> channel) : containersStub_(Containers::NewStub(channel)){};
        
        ListContainersResponse listContainers(const std::string& ctrdNamespace);
        CreateContainerResponse createContainer(const std::string& ctrdNamespace, Container& ctr);
        void deleteContainer(const std::string& ctrdNamespace, const std::string& id);

        json loadOciSpec(const std::string& path);
        google::protobuf::Any modifyAndSerializeOciSpec(nlohmann::json& spec);

    private:
        std::unique_ptr<Containers::Stub> containersStub_;
};

#endif /* CONTAINERS_CLIENT_H */