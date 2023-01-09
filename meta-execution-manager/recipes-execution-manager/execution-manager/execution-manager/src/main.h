/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#ifndef MAIN_H
#define MAIN_H

#include <argp.h>
#include "log.h"

#define MAX_APPLICATIONS 50

struct execution_manager_global_arguments {
    bool debug;
    char *log;
    char *log_format;
    char *config;
    bool native;
};

char * argp_mandatory_argument (char *arg, struct argp_state *state);
int init_execution_manager(const char *id, struct execution_manager_global_arguments *glob, log_error_t *err);

#endif