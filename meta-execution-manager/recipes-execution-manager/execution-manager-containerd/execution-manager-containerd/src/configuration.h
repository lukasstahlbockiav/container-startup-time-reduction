/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <cstddef>
#include <string>

struct config_options_s {
    bool debug;
};

struct config_containers_s {
    std::string name;
    std::string command;
    std::string digest;
    std::string image;
    std::string oci_spec;
    std::string runtime;
    std::string mount_type;
    std::string mount_source;
    std::string mount_target;
    std::string mount_options[10];
    //char name[20];
    //char command[10];
    //char digest[100];
    //char image[100];
    //char oci_spec[200];
};

void parse_execution_config_containers(const std::string& file_path, struct config_containers_s config_containers[], int& num);

#endif