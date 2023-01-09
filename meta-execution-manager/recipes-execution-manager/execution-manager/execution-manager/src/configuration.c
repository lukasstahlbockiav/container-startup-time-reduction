/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#include <config.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "yajl/yajl_tree.h"

#include "log.h"
#include "configuration.h"

void parse_execution_config(unsigned char* file_path, struct config_options_s* config_options, struct config_containers_s* (*p_config_containers)[], size_t *num)
{
    unsigned char file_data[65535];
    size_t rd;
    yajl_val node;
    char errbuf[1024];

    /* null plug buffers */
    file_data[0] = errbuf[0] = 0;

    /* read the entire config file */
    FILE* fp = fopen(file_path, "r");
    rd = fread((void *) file_data, 1, sizeof(file_data) - 1, fp);

    /* file read error handling */
    if (rd == 0 && !feof(fp)) {
        puts("error encountered on file read\n");
    } else if (rd >= sizeof(file_data) - 1) {
        puts("config file too big\n");
    }

    /* we have the whole config file in memory.  let's parse it ... */
    node = yajl_tree_parse((const char *) file_data, errbuf, sizeof(errbuf));

    /* parse error handling */
    if (node == NULL) {
        puts("parse config error");
    }

    /* ... and extract a nested value from the config file */
    {
        size_t array_size;
        size_t i;
        const char * path[] = { "containers", (const char *) 0 };
        yajl_val v = yajl_tree_get(node, path, yajl_t_array);
        if (v && YAJL_IS_ARRAY(v)) {
            array_size = v->u.array.len;
            (*num) = array_size;
            for ( i = 0; i < array_size; ++i){
                yajl_val obj = v->u.array.values[i];
                size_t n_elem = obj->u.object.len;
                int i_n;
                for (i_n = 0; i_n < n_elem; ++i_n){
                    const char* key = obj->u.object.keys[i_n];
                    yajl_val val = obj->u.object.values[i_n];
                    //execution_manager_warning("%s:%s", key, val->u.string);
                    if (strcmp(key, "name") == 0 && YAJL_IS_STRING(val)){
                        strcpy((*p_config_containers)[i]->name, val->u.string);
                    } else if (strcmp(key, "bundle") == 0 && YAJL_IS_STRING(val)) {
                        strcpy((*p_config_containers)[i]->bundle, val->u.string);
                    } else if (strcmp(key, "command") == 0 && YAJL_IS_STRING(val)) {
                        strcpy((*p_config_containers)[i]->command, val->u.string);
                    } else if (strcmp(key, "command_options") == 0 && YAJL_IS_STRING(val)) {
                        strcpy((*p_config_containers)[i]->command_options, val->u.string);
                    } else if (strcmp(key, "mount_target") == 0 && YAJL_IS_STRING(val)) {
                        strcpy((*p_config_containers)[i]->mount_target, val->u.string);
                    } else if (strcmp(key, "mount_options") == 0 && YAJL_IS_STRING(val)) {
                        strcpy((*p_config_containers)[i]->mount_options, val->u.string);
                    } else if (strcmp(key, "config") == 0 && YAJL_IS_STRING(val)) {
                        strcpy((*p_config_containers)[i]->config, val->u.string);
                    }
                }
                //strcpy((*data)[i],v->u.array.values[i]->u.string);
                //puts((*data)[i]);
            }
            //(*data)[i] = (char *) NULL;
        }
    }

    yajl_tree_free(node);
    fclose(fp);
};

void parse_environment_config(unsigned char* file_path, char* env[])
{
  unsigned char file_data[OCI_CONFIG_BUFFER_SIZE];
  size_t rd;
  yajl_val node;
  char errbuf[1024];

  /* null plug buffers */
  file_data[0] = errbuf[0] = 0;

  /* read the entire config file */
  FILE* fp = fopen(file_path, "r");
  rd = fread((void *) file_data, 1, sizeof(file_data) - 1, fp);

  /* file read error handling */
  if (rd == 0 && !feof(fp)) {
      puts("error encountered on file read\n");
  } else if (rd >= sizeof(file_data) - 1) {
      puts("config file too big\n");
  }

  /* we have the whole config file in memory.  let's parse it ... */
  node = yajl_tree_parse((const char *) file_data, errbuf, sizeof(errbuf));

  /* parse error handling */
  if (node == NULL) {
    execution_manager_warning("parse config error: %s", errbuf);
  }

  /* ... and extract a nested value from the config file */
  {
      size_t array_size;
      size_t i;
      const char * path[] = { "process", "env", (const char *) 0 };
      yajl_val v = yajl_tree_get(node, path, yajl_t_array);
      if (v) {
        array_size = v->u.array.len;
        for ( i = 0; i < array_size; ++i){
          //execution_manager_warning("env var: %s", v->u.array.values[i]->u.string);
          //(*env)[i] = malloc(sizeof(v->u.array.values[i]->u.string));
          env[i] = malloc(sizeof(char) * strlen(v->u.array.values[i]->u.string));
          if (env[i])
          {
            //execution_manager_warning("allocated %d bytes for %d", sizeof(v->u.array.values[i]->u.string), i);
            strcpy(env[i],v->u.array.values[i]->u.string);
            //env[i] = v->u.array.values[i]->u.string;
            //execution_manager_warning("copied env var: %s", env[i]);
          }
          //execution_manager_warning("allocated memory");
          //strcpy((*env)[i],v->u.array.values[i]->u.string);
        }
        //execution_manager_warning("add null termination for %d", i);
        env[i] = malloc(sizeof(char));
        env[i] = (char *) NULL;
      }
  }

  yajl_tree_free(node);
  fclose(fp);
};