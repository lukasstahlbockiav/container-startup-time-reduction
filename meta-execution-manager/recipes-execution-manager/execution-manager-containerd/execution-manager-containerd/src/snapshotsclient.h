/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#ifndef SNAPSHOT_CLIENT_H
#define SNAPSHOT_CLIENT_H

#include "logging.h"

#include "github.com/containerd/containerd/api/services/snapshots/v1/snapshots.grpc.pb.h"

#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using containerd::services::snapshots::v1::PrepareSnapshotRequest;
using containerd::services::snapshots::v1::PrepareSnapshotResponse;
using containerd::services::snapshots::v1::RemoveSnapshotRequest;
using containerd::services::snapshots::v1::Snapshots;

class SnapshotsClient {
    public:
        explicit SnapshotsClient(std::shared_ptr<Channel> channel) : snapshotsStub_(Snapshots::NewStub(channel)){};
        
        PrepareSnapshotResponse prepareSnapshot(const std::string& ctrdNamespace, 
                                                const std::string& snapshotter,
                                                const std::string& key,
                                                const std::string& parent);
        void removeSnapshot();

    private:
        std::unique_ptr<Snapshots::Stub> snapshotsStub_;
};

#endif /* SNAPSHOT_CLIENT_H */