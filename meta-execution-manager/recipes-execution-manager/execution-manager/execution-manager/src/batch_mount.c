/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#include <config.h>
#include "log.h"
#include "batch_mount.h"
#include <sys/mount.h>
#include <pthread.h>

void *mount_thread(void *args)
{
    int ret;
    struct config_containers_s *config_container = (struct config_containers_s*)args;
    execution_manager_warning("mount container %s", config_container->name);
    ret = mount(NULL, config_container->mount_target, "overlay", 0, config_container->mount_options);
    execution_manager_warning("mount container %s finished with state %d", config_container->name, ret);
    if (UNLIKELY (ret <0))
        execution_manager_fail_with_error(0, "Failed to mount: %s", config_container->name);
}

void *umount_thread(void *args)
{
    int ret;
    struct config_containers_s *config_container = (struct config_containers_s*)args;
    execution_manager_warning("umount container %s", config_container->name);
    ret = umount(config_container->mount_target);
    if (UNLIKELY (ret <0))
        execution_manager_fail_with_error(0, "Failed to umount: %s", config_container->name);
}

void 
batch_mount(struct config_containers_s *(*p_config_containers)[], size_t *num)
{
    int ret;
    pthread_t mount_threads[MAX_APPLICATIONS];
    for (int i = 0; i < (*num); ++i)
    {
        ret = pthread_create(&mount_threads[i], NULL, mount_thread, (void *) (*p_config_containers)[i]);
        if (UNLIKELY(ret))
            execution_manager_fail_with_error(0, "Unable to create thread, %d", ret);
    }
};

void
batch_umount(struct config_containers_s *(*p_config_containers)[], size_t *num)
{
    int ret;
    pthread_t umount_threads[MAX_APPLICATIONS];
    for (int i = 0; i < (*num); ++i)
        {
            ret = pthread_create(&umount_threads[i], NULL, umount_thread, (void *) (*p_config_containers)[i]);
            if (UNLIKELY(ret))
                execution_manager_fail_with_error(0, "Unable to create thread, %d", ret);
            
        }
};