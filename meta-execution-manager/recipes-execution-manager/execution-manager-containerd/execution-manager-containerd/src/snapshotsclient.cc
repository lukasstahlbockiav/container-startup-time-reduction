/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/**
**  Updated by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**  Initial code from https://github.com/grpc/grpc/blob/v1.51.1/examples/cpp/helloworld/greeter_async_client.cc
**/
#include <iostream>
#include <memory>

#include "snapshotsclient.h"

// Example function to list containers
PrepareSnapshotResponse SnapshotsClient::prepareSnapshot(const std::string& _ctrdNamespace, 
                                                            const std::string& _snapshotter,
                                                            const std::string& _key,
                                                            const std::string& _parent){
    PrepareSnapshotRequest request;
    request.set_snapshotter(_snapshotter);
    request.set_key(_key);
    request.set_parent(_parent);
    
    PrepareSnapshotResponse response;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;
    context.AddMetadata("containerd-namespace",_ctrdNamespace);

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    CompletionQueue cq;

    // Storage for the status of the RPC upon completion.
    Status status;

    // stub_->PrepareAsyncList() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    std::unique_ptr<ClientAsyncResponseReader<PrepareSnapshotResponse> > rpc(
        snapshotsStub_->PrepareAsyncPrepare(&context, request, &cq));
    
    // StartCall initiates the RPC call
    createLogMessage(info, "/snapshot/prepare request for " + request.key() + " started");
    rpc->StartCall();
    
    // Request that, upon completion of the RPC, "response" be updated with the
    // server's response; "status" with the indication of whether the operation
    // was successful. Tag the request with the integer 1.
    rpc->Finish(&response, &status, (void*)1);
    void* got_tag;
    bool ok = false;
    // Block until the next result is available in the completion queue "cq".
    // The return value of Next should always be checked. This return value
    // tells us whether there is any kind of event or the cq_ is shutting down.
    GPR_ASSERT(cq.Next(&got_tag, &ok));

    // Verify that the result from "cq" corresponds, by its tag, our previous
    // request.
    GPR_ASSERT(got_tag == (void*)1);
    // ... and that the request was completed successfully. Note that "ok"
    // corresponds solely to the request for updates introduced by Finish().
    GPR_ASSERT(ok);

    // Act upon the status of the actual RPC.
    if (status.ok()) {
        createLogMessage(info, "/snapshot/prepare request for " + request.key() + " finished");
        return response;
    } else {
        std::cout << status.error_message() << std::endl;
    }
};