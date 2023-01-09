/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#ifndef BATCH_COMMAND_H
#define BATCH_COMMAND_H

#include <sys/types.h>

void fork_rec(pid_t *pids, int n_proc, struct config_containers_s* (*p)[], bool is_native);

#endif