/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#include <fstream>
#include <unistd.h>
#include <sstream> 
#include <sys/wait.h>

#include "containerdclient.h"
#include "configuration.h"
#include "batch_command.h"

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " <option(s)>"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-c,--config \tSpecify json config file path"
              << std::endl;
} 

int main(int argc, char** argv) {
    int num;
    struct config_containers_s config_containers[MAX_CONTAINERS];
    pid_t pids[MAX_CONTAINERS], wpid;
    int status = 0;
    std::string configPath;

    // check arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        } else if ((arg == "-c") || (arg == "--config")) {
            if (i+1 < argc){
                //nc = std::stoi(argv[i++]);
                configPath = argv[i+1];
                createLogMessage(info, "Using config: " + configPath);
            } else {
                std::cerr << "--config option requires one argument." << std::endl;
                return 1;
            }
        } 
    }

    // Instantiate the client.
    createLogMessage(info, "Instantiating client");
    ContainerdClient client(grpc::CreateChannel("unix:///run/containerd/containerd.sock", grpc::InsecureChannelCredentials()));

    parse_execution_config_containers(configPath, config_containers, num);
    createLogMessage(info, "Found " + std::to_string(num) + " containers");
    
    createLogMessage(info, "Starting batch command");

    fork_rec(pids, num - 1, client, config_containers);
    while ((wpid = wait(&status)) > 0);

    createLogMessage(info, "Finished batch command");

    return 0;
}