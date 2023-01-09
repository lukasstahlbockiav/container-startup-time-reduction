/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include <stdbool.h>

#define OCI_CONFIG_BUFFER_SIZE 5000

struct config_options_s {
    bool debug;
};

struct config_containers_s {
    char name[20];
    char bundle[100];
    char command[50];
    char command_options[10];
    char mount_options[1000];
    char mount_target[100];
    char config[100];
};

void parse_execution_config(unsigned char* file_path, struct config_options_s* config_options, struct config_containers_s* (*p_config_containers)[], size_t *num);
void parse_environment_config(unsigned char* file_path, char* env[]);

void parse_config(unsigned char* file_path, struct config_options_s* config_options, struct config_containers_s* (*p_config_containers)[]);

#endif