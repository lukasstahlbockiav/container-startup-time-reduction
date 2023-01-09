/*
 * crun - OCI runtime written in C
 *
 * Copyright (C) 2017, 2018, 2019 Giuseppe Scrivano <giuseppe@scrivano.org>
 * crun is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * crun is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with crun.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
**  Adapted by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**  This work was derived from https://github.com/containers/crun/blob/1.4.3/src/libcrun/error.h and
**  https://github.com/containers/crun/blob/1.4.3/src/libcrun/utils.h
**/
#ifndef LOG_H
#define LOG_H
#include <config.h>

#ifdef HAVE_ERROR_H
#  include <error.h>
#else
#  define error(status, errno, fmt, ...)                      \
    do                                                        \
      {                                                       \
        if (errno == 0)                                       \
          fprintf (stderr, "execution_manager: " fmt "\n", ##__VA_ARGS__); \
        else                                                  \
          {                                                   \
            fprintf (stderr, "execution_manager: " fmt, ##__VA_ARGS__);    \
            fprintf (stderr, ": %s\n", strerror (errno));     \
          }                                                   \
        if (status)                                           \
          exit (status);                                      \
      }                                                       \
    while (0)
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <syslog.h>
#include <unistd.h>

struct log_error_s
{
  int status;
  char *msg;
};
typedef struct log_error_s *log_error_t;

#define OOM()                            \
  do                                     \
    {                                    \
      fprintf (stderr, "out of memory"); \
      _exit (EXIT_FAILURE);              \
    }                                    \
  while (0)

/* copied from utils */
#define cleanup_free __attribute__ ((cleanup (cleanup_freep)))
#define arg_unused __attribute__ ((unused))
#define LIKELY(x) __builtin_expect ((x), 1)
#define UNLIKELY(x) __builtin_expect ((x), 0)

static inline void *
xmalloc (size_t size)
{
  void *res = malloc (size);
  if (UNLIKELY (res == NULL))
    OOM ();
  return res;
}

static inline void
cleanup_freep (void *p)
{
  void **pp = (void **) p;
  free (*pp);
}

int xasprintf (char **str, const char *fmt, ...);

int has_prefix (const char *str, const char *prefix);

/* end of copied from utils */

typedef void (*execution_manager_output_handler) (int errno_, const char *msg, bool warning, void *arg);

void execution_manager_set_output_handler (execution_manager_output_handler handler, void *arg, bool log_to_stderr);

void log_write_to_journald (int errno_, const char *msg, bool warning, void *arg);

void log_write_to_syslog (int errno_, const char *msg, bool warning, void *arg);

void log_write_to_stream (int errno_, const char *msg, bool warning, void *arg);

void log_write_to_stderr (int errno_, const char *msg, bool warning, void *arg);

int execution_manager_make_error (log_error_t *err, int status, const char *msg, ...);

int execution_manager_error_wrap (log_error_t *err, const char *fmt, ...);

int execution_manager_error_get_errno (log_error_t *err);

int execution_manager_error_release (log_error_t *err);

void execution_manager_error_write_warning_and_release (FILE *out, log_error_t **err);

void execution_manager_warning (const char *msg, ...);

void log_error (int errno_, const char *msg, ...);
int execution_manager_make_error (log_error_t *err, int status, const char *msg, ...);

void log_error_write_warning_and_release (FILE *out, log_error_t **err);

void execution_manager_fail_with_error (int errno_, const char *msg, ...) __attribute__ ((noreturn));

int execution_manager_set_log_format (const char *format, log_error_t *err);

int execution_manager_init_logging (execution_manager_output_handler *output_handler, void **output_handler_arg, const char *id,
                                         const char *log, log_error_t *err);

int log_error_release (log_error_t *err);

enum
{
  LOG_VERBOSITY_ERROR,
  LOG_VERBOSITY_WARNING
};

void execution_manager_set_verbosity (int verbosity);
int execution_manager_get_verbosity ();

#endif
