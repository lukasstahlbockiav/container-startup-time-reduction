/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#include <config.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <error.h>
#include <sys/wait.h>


#include "configuration.h"
#include "log.h"
#include "batch_mount.h"
#include "batch_command.h"
#include "main.h"

#define arg_unused __attribute__ ((unused))


static struct execution_manager_global_arguments arguments;

execution_manager_output_handler output_handler;
void *output_handler_arg;
int 
init_execution_manager(const char *id, struct execution_manager_global_arguments *glob, log_error_t *err)
{
    int ret;
    ret = execution_manager_init_logging(&output_handler, &output_handler_arg, id, glob->log, err);
    if (UNLIKELY (ret < 0))
        return ret;

    if (glob->log_format)
        {
            ret = execution_manager_set_log_format (glob->log_format, err);
            if (UNLIKELY (ret < 0))
                return ret;
        }
};

static char args_doc[] = "CONFIG_FILE_PATH";

enum
{
    OPTION_DEBUG = 1000,
    OPTION_LOG,
    OPTION_LOG_FORMAT
};

const char *argp_program_version = PACKAGE_STRING;
const char *argp_program_bug_address = "lukas.stahlbock@iav.de";

static struct argp_option options[] = { { "debug", OPTION_DEBUG, 0, 0, "produce verbose output", 0 },
                                        { "log", OPTION_LOG, "FILE", 0, NULL, 0 },
                                        { "log-format", OPTION_LOG_FORMAT, "FORMAT", 0, NULL, 0 },
                                        { "native", 'n', 0, 0, "specify if config file targets native applications. Default: false (containers)", 0 },
                                        { 0 } };

static void print_version(FILE *stream, struct argp_state *state arg_unused) {
    fprintf (stream, "%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
    fprintf (stream, "+YAJL\n");
}

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    const char *tmp;

    switch (key) {
        case OPTION_DEBUG:
            arguments.debug = true;
            break;

        case OPTION_LOG:
            arguments.log = argp_mandatory_argument (arg, state);
            break;

        case OPTION_LOG_FORMAT:
            arguments.log_format = argp_mandatory_argument (arg, state);
            break;

        case 'n':
            arguments.native = true;
            break;

        case ARGP_KEY_ARG:
            if (state->arg_num >= 1)
                /* Too many arguments. */
                argp_usage (state);

            arguments.config = arg;

            break;

        case ARGP_KEY_END:
            if (state->arg_num < 1)
                /* Not enough arguments. */
                argp_usage (state);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
        }

    return 0;
}

char *
argp_mandatory_argument (char *arg, struct argp_state *state)
{
  if (arg)
    return arg;
  return state->argv[state->next++];
}

static struct argp argp = { options, parse_opt, args_doc, NULL, NULL, NULL, NULL };


int main(int argc, char **argv) {
    log_error_t err = NULL;
    int ret, first_argument;
    size_t num;
    
    arguments.native = false;

    argp_program_version_hook = print_version;
    argp_parse (&argp, argc, argv, ARGP_IN_ORDER, &first_argument, &arguments);

    if (arguments.debug)
        execution_manager_set_verbosity(LOG_VERBOSITY_WARNING);

    execution_manager_warning("Starting " PACKAGE_STRING);


    struct config_options_s config_options;
    struct config_containers_s* config_containers[MAX_APPLICATIONS];
    struct config_containers_s* (*p_config_containers)[] = &config_containers;
    pid_t pids[MAX_APPLICATIONS], wpid;
    int status = 0;

    for ( int i = 0; i < MAX_APPLICATIONS; i++){
        config_containers[i] = malloc(sizeof(struct config_containers_s));
    }

    parse_execution_config(arguments.config, &config_options, p_config_containers, &num);
    execution_manager_warning("Parsed config and found %d containers", num);

    //batch_mount(config_containers, &num);
    fork_rec(pids, num - 1, p_config_containers, arguments.native);

    if (!arguments.native) {
        while ((wpid = wait(&status)) > 0);
    };

    return EXIT_SUCCESS;
}