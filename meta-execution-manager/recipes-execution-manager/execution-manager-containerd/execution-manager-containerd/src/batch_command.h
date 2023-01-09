/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#ifndef BATCH_COMMAND_H
#define BATCH_COMMAND_H

#include <vector>
#include <future>
#include <thread>

#include "configuration.h"
#include "containerdclient.h"

void fork_rec(pid_t *pids, int n_proc, ContainerdClient& client, struct config_containers_s config_containers[]);

#endif