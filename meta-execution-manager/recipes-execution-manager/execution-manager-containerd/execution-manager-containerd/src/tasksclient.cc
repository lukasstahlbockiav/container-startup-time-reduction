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

#include "tasksclient.h"

// Example function to list containers
ListTasksResponse TasksClient::listTasks(const std::string& ctrdNamespace){
    ListTasksRequest request;
    
    ListTasksResponse response;

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
    std::unique_ptr<ClientAsyncResponseReader<ListTasksResponse> > rpc(
        tasksStub_->PrepareAsyncList(&context, request, &cq));
    
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

CreateTaskResponse TasksClient::createTask(const std::string& ctrdNamespace, CreateTaskRequest& request){
    CreateTaskResponse response;

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
    std::unique_ptr<ClientAsyncResponseReader<CreateTaskResponse> > rpc(
        tasksStub_->PrepareAsyncCreate(&context, request, &cq));
    
    // StartCall initiates the RPC call
    createLogMessage(info, "/tasks/create request for " + request.container_id() + " started");
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
        createLogMessage(info, "/tasks/create request for " + request.container_id() + " finished");
        return response;
    } else {
        createLogMessage(error, "/tasks/create request for " + request.container_id() + " failed with error: " + status.error_message());
        return response;
    }
};

StartResponse TasksClient::startTask(const std::string& ctrdNamespace, const std::string& containerId){
    StartRequest request;
    request.set_container_id(containerId);
    
    StartResponse response;

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
    std::unique_ptr<ClientAsyncResponseReader<StartResponse> > rpc(
        tasksStub_->PrepareAsyncStart(&context, request, &cq));
    
    // StartCall initiates the RPC call
    createLogMessage(info, "/tasks/start request for " + containerId + " started");
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
    //request.release_container();

    // Act upon the status of the actual RPC.
    if (status.ok()) {
        createLogMessage(info, "/tasks/start request for " + containerId + " finished");
        return response;
    } else {
        createLogMessage(error, "/tasks/start request for " + containerId + " failed with error: " + status.error_message());
    }
};

void TasksClient::killTask(const std::string& ctrdNamespace, const std::string& containerId){
    KillRequest request;
    request.set_container_id(containerId);
    request.set_signal(15); // 15 --> SIGTERM
    request.set_all(true); // send signal to all tasks
    
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
        tasksStub_->PrepareAsyncKill(&context, request, &cq));
    
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
        std::cout << "Killed task " << containerId << std::endl;
    } else {
        std::cout << status.error_message() << std::endl;
    }
};

DeleteResponse TasksClient::deleteTask(const std::string& ctrdNamespace, const std::string& containerId){
    DeleteTaskRequest request;
    request.set_container_id(containerId);
    
    DeleteResponse response;
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
    std::unique_ptr<ClientAsyncResponseReader<DeleteResponse> > rpc(
        tasksStub_->PrepareAsyncDelete(&context, request, &cq));
    
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