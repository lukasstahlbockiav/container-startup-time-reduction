/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#ifndef BATCH_MOUNT_H
#define BATCH_MOUNT_H

#include "configuration.h"
#include "main.h"

void batch_mount(struct config_containers_s *(*p_config_containers)[], size_t *num);

void batch_umount(struct config_containers_s *(*p_config_containers)[], size_t *num);

#endif