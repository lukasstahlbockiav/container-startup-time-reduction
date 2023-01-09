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
#include <fstream>

#include "containersclient.h"

// Example function to list containers
ListContainersResponse ContainersClient::listContainers(const std::string& ctrdNamespace){
    ListContainersRequest request;
    
    ListContainersResponse response;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;
    context.AddMetadata("containerd-namespace",ctrdNamespace);

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    CompletionQueue cq;

    // Storage for the status of the RPC upon completion.
    Status status;

    // stub_->PrepareAsyncList() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    std::unique_ptr<ClientAsyncResponseReader<ListContainersResponse> > rpc(
        containersStub_->PrepareAsyncList(&context, request, &cq));
    
    // StartCall initiates the RPC call
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
        return response;
    } else {
        std::cout << status.error_message() << std::endl;
    }
};

CreateContainerResponse ContainersClient::createContainer(const std::string& ctrdNamespace, Container& ctr){
    //createLogMessage(info, "/containers/create request function for " + ctr.id() + " entered");
    CreateContainerRequest request;
    request.set_allocated_container(&ctr);
    
    CreateContainerResponse response;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;
    context.AddMetadata("containerd-namespace", ctrdNamespace);

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    CompletionQueue cq;

    // Storage for the status of the RPC upon completion.
    Status status;

    // stub_->PrepareAsyncCreate() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    std::unique_ptr<ClientAsyncResponseReader<CreateContainerResponse> > rpc(
        containersStub_->PrepareAsyncCreate(&context, request, &cq));
    
    // StartCall initiates the RPC call
    createLogMessage(info, "/containers/create request for " + ctr.id() + " started");
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


    // release nested container object. Otherwise protobuf will delete the objects causing a double free error
    // https://stackoverflow.com/questions/33960999/protobuf-will-set-allocated-delete-the-allocated-object
    // https://developers.google.com/protocol-buffers/docs/reference/cpp-generated
    request.release_container();

    // Act upon the status of the actual RPC.
    if (status.ok()) {
        createLogMessage(info, "/containers/create request for " + ctr.id() + " finished");
        return response;
    } else {
        std::cout << status.error_message() << std::endl;
    }
};

void ContainersClient::deleteContainer(const std::string& ctrdNamespace, const std::string& id){
    DeleteContainerRequest request;
    request.set_id(id);
    
    google::protobuf::Empty response;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;
    context.AddMetadata("containerd-namespace", ctrdNamespace);

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    CompletionQueue cq;

    // Storage for the status of the RPC upon completion.
    Status status;

    // stub_->PrepareAsyncCreate() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    std::unique_ptr<ClientAsyncResponseReader<google::protobuf::Empty> > rpc(
        containersStub_->PrepareAsyncDelete(&context, request, &cq));
    
    // StartCall initiates the RPC call
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
        std::cout << "Deleted container " << id << std::endl;
    } else {
        std::cout << status.error_message() << std::endl;
    }
};

json ContainersClient::loadOciSpec(const std::string& _path){
    std::ifstream file_in(_path);
    json j;
    file_in >> j;

    return j;
};

google::protobuf::Any ContainersClient::modifyAndSerializeOciSpec(json& _spec){
    google::protobuf::Any spec_;
    //std::cout << _spec.dump() << std::endl;
    spec_.set_value(_spec.dump());
    return spec_;
};