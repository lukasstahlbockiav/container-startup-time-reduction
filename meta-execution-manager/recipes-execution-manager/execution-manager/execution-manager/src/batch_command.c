/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include "log.h"
#include "configuration.h"

void command_container(int n_proc, struct config_containers_s* (*p)[]) 
{
  int ret;
      
  execution_manager_warning("%s container %s", (*p)[n_proc]->command, (*p)[n_proc]->name);
  if (strcmp((*p)[n_proc]->command, "mount") == 0){
      ret = mount(NULL, (*p)[n_proc]->mount_target, "overlay", 0, (*p)[n_proc]->mount_options);
      execution_manager_warning("%s container %s finished", (*p)[n_proc]->command, (*p)[n_proc]->name);

  } else if (strcmp((*p)[n_proc]->command, "umount") == 0) {
      ret = umount((*p)[n_proc]->mount_target);

  } else {
      //char* args[] = { (*p)[n_proc]->command, (*p)[n_proc]->name, (*p)[n_proc]->command_options, NULL};
      char* args[9];
      char log[50];
      strcpy(log,"--log=file:/tmp/logs/crun_");
      strcat(log,(*p)[n_proc]->name);
      strcat(log, ".log");
      args[0] = "crun";
      args[1] = "--debug";
      args[2] = log;
      args[3] = (*p)[n_proc]->command;

      if (strcmp(args[3], "create") == 0){
        args[4] = "-b";
        args[5] = (*p)[n_proc]->bundle;
        args[6] = (*p)[n_proc]->name;
        args[7] = NULL;

      } else if (strcmp(args[3], "start") == 0){
        args[4] = (*p)[n_proc]->name;
        args[5] = NULL;

      } else if (strcmp(args[3], "run") == 0){
        args[4] = "-b";
        args[5] = (*p)[n_proc]->bundle;
        args[6] = "-d";
        args[7] = (*p)[n_proc]->name;
        args[8] = NULL;
        execution_manager_warning("mount container %s", (*p)[n_proc]->name);
        ret = mount(NULL, (*p)[n_proc]->mount_target, "overlay", 0, (*p)[n_proc]->mount_options);
        execution_manager_warning("mount container %s finished", (*p)[n_proc]->name);

      } else if (strcmp(args[3], "kill") == 0){
        args[4] = (*p)[n_proc]->name;
        args[5] = "SIGTERM";
        args[6] = NULL;

      } else if (strcmp(args[3], "delete") == 0){
        args[4] = (*p)[n_proc]->name;
        args[5] = NULL;

      }

      ret = execvp("/usr/bin/crun", args);
  }

  if (UNLIKELY (ret))
    execution_manager_fail_with_error(0, "could not execute command %s", (*p)[n_proc]->command);
}

void command_native(int n_proc, struct config_containers_s* (*p)[])
{
  int ret;
  char* cmd_env_arr[15];

  execution_manager_warning("run application %s", (*p)[n_proc]->name);
  parse_environment_config((*p)[n_proc]->config, cmd_env_arr);
  ret = execle((*p)[n_proc]->command, "vsomeip-generic", (char*) NULL, cmd_env_arr);

  if (UNLIKELY (ret))
    execution_manager_fail_with_error(0, "could not execute command %s", (*p)[n_proc]->command);
}

/* fork the process recursively */
void fork_rec(pid_t *pids, int n_proc, struct config_containers_s* (*p)[], bool is_native)
{
  if (n_proc >= 0)
  {
    pids[n_proc] = fork();
    if (pids[n_proc] < 0)
    {
      perror("fork");
    }
    else if (pids[n_proc] == 0)
    {
      //execution_manager_warning("child %d with pid %d", n_proc, pids[n_proc]);
      if (!is_native){
        command_container(n_proc, p);
      } else {
        command_native(n_proc, p);
      }
      
      exit(0);
    }
    else if (pids[n_proc] > 0)
    {
      fork_rec(pids, n_proc - 1, p, is_native);
    }
  }
}