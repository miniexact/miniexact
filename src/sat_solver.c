/*
    miniexact - Toolset to solve exact cover problems and extensions
    Copyright (C) 2021-2023  Maximilian Heisinger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <miniexact/log.h>
#include <miniexact/sat_solver.h>
#include <miniexact/util.h>

// Deal with strlcpy.
#if HAVE_STRLCPY
// Many systems already have this function defined.
#else
static inline size_t
strlcpy(char* dst, const char* src, size_t size) {
  (void)size;
  return strcpy(dst, src) - src;
}
#endif

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif

extern char** environ;

#define BUF_SIZE 4096

// File existence and find_executable taken and adapted from MIT-licensed
// Kissat.
static bool
file_exists(const char* path) {
  if(!path)
    return false;
  if(access(path, F_OK))
    return false;
  return true;
}

static bool
file_readable(const char* path) {
  if(!path)
    return false;
  if(access(path, R_OK))
    return false;
  return true;
}

static bool
find_executable(const char* name, char** overwrite) {
  const size_t name_len = strlen(name);
  const char* environment = getenv("PATH");
  if(!environment)
    return false;
  const size_t dirs_len = strlen(environment);
  char* dirs = malloc(dirs_len + 1);
  if(!dirs)
    return false;
  strlcpy(dirs, environment, dirs_len + 1);
  bool res = false;
  const char* end = dirs + dirs_len + 1;
  for(char *dir = dirs, *q; !res && dir != end; dir = q) {
    for(q = dir; *q && *q != ':'; q++)
      assert(q + 1 < end);
    *q++ = 0;
    const size_t path_len = (q - dir) + name_len;
    char* path = malloc(path_len + 1);
    if(!path) {
      free(dirs);
      return false;
    }
    snprintf(path, path_len + 1, "%s/%s", dir, name);
    assert(strlen(path) == path_len);
    res = file_readable(path);
    miniexact_trc("Trying %s", path);
    if(overwrite)
      *overwrite = strdup(path);
    free(path);
  }
  free(dirs);
  return res;
}

static char* known_sat_solvers[] = { "kissat", "cadical", "lingeling", "picosat", NULL };

static char* known_sat_solver_args[][2] = { { "-q", NULL },
                                            { "-q", NULL },
                                            { "-q", NULL },
					    { NULL },
                                            NULL };

static size_t
find_solver_id() {
  static bool id_found = false;
  static ssize_t id = 0;

  if(id_found)
    return id;

  for(size_t i = 0; known_sat_solvers[i]; ++i) {
    const char* solver = known_sat_solvers[i];
    char* path;
    if(find_executable(solver, &known_sat_solvers[i])) {
      id_found = true;
      id = i;
      return i;
    }
  }

  miniexact_err(
    "No SAT solver found! Please install one of the supported solvers.\n"
    "Tried the following in $PATH:\n");
  for(size_t i = 0; known_sat_solvers[i]; ++i) {
    miniexact_err("  %s\n", known_sat_solvers[i]);
  }
  exit(-1);
}

void
miniexact_sat_solver_find_and_init(miniexact_sat_solver* solver,
                                   unsigned int variables,
                                   unsigned int clauses) {
  assert(solver);

  size_t solver_id = find_solver_id();
  miniexact_sat_solver_init(solver,
                            variables,
                            clauses,
                            known_sat_solvers[solver_id],
                            known_sat_solver_args[solver_id],
                            environ);
}

void
miniexact_sat_solver_init(miniexact_sat_solver* solver,
                          unsigned int variables,
                          unsigned int clauses,
                          char* binary,
                          char* const argv[],
                          char* envp[]) {
  assert(solver);

  pipe(solver->infd);
  pipe(solver->outfd);
  solver->variables = variables;
  solver->clauses = clauses;

  solver->pid = fork();
  if(solver->pid) {
    // Parent
    solver->infd_handle = fdopen(solver->infd[1], "w");
    assert(solver->infd_handle);
    solver->outfd_handle = fdopen(solver->outfd[0], "r");
    assert(solver->outfd_handle);

    solver->assignments =
      realloc(solver->assignments, sizeof(char) * (variables + 1));

    fprintf(solver->infd_handle, "p cnf %u %u\n", variables, clauses);
  } else {
    // Child
    dup2(solver->infd[0], STDIN_FILENO);
    close(solver->infd[0]);
    close(solver->infd[1]);

    dup2(solver->outfd[1], STDOUT_FILENO);
    close(solver->outfd[0]);
    close(solver->outfd[1]);

    char* argv_null[1] = { NULL };
    if(argv == NULL)
      argv = argv_null;

    char* envp_null[1] = { NULL };
    if(argv == NULL)
      argv = argv_null;

    if(envp == NULL) {
      envp = environ;
    }

    size_t arg_count = 0;
    for(arg_count = 0; argv[arg_count] != NULL; ++arg_count) {
    }

    char* real_argv[arg_count + 2];

    real_argv[0] = binary;
    for(size_t i = 0; i < arg_count; ++i) {
      real_argv[i + 1] = argv[i];
    }
    real_argv[arg_count + 1] = NULL;

    int status = execve(binary, real_argv, envp);
    miniexact_err(
      "Executing child \"%s\" failed! Error: %s\n", binary, strerror(errno));
    exit(-1);
  }
}

void
miniexact_sat_solver_destroy(miniexact_sat_solver* solver) {
  assert(solver);
  if(solver->assignments) {
    free(solver->assignments);
    solver->assignments = NULL;
  }
}

void
miniexact_sat_solver_add(miniexact_sat_solver* solver, int l) {
  assert(solver);
  assert(solver->infd_handle);
  miniexact_trc("[SAT] Lit: %d", l);
  if(l == 0) {
    fprintf(solver->infd_handle, "0\n");
  } else {
    fprintf(solver->infd_handle, "%d ", l);
  }
}

void
miniexact_sat_solver_unit(miniexact_sat_solver* solver, int l) {
  assert(solver);
  assert(solver->infd_handle);
  miniexact_trc("[SAT] %d 0", l);
  fprintf(solver->infd_handle, "%d 0\n", l);
}
void
miniexact_sat_solver_binary(miniexact_sat_solver* solver, int a, int b) {
  assert(solver);
  assert(solver->infd_handle);
  miniexact_trc("[SAT] %d %d 0", a, b);
  fprintf(solver->infd_handle, "%d %d 0\n", a, b);
}
void
miniexact_sat_solver_ternary(miniexact_sat_solver* solver,
                             int a,
                             int b,
                             int c) {
  assert(solver);
  assert(solver->infd_handle);
  miniexact_trc("[SAT] %d %d %d 0", a, b, c);
  fprintf(solver->infd_handle, "%d %d %d 0\n", a, b, c);
}

static void
parse_solver_output(miniexact_sat_solver* solver) {
  assert(solver);
  assert(solver->assignments);
  char stack_buf[BUF_SIZE];
  char* buf = stack_buf;
  size_t buf_size = BUF_SIZE;
  size_t len;
  while((len = getline(&buf, &buf_size, solver->outfd_handle)) != -1) {
    if(len > 0 && (buf[0] == 'c' || buf[0] == 'r')) {
      continue;// Comment line or result line
    }
    if(len > 0 && (buf[0] == 'v')) {
      char* numbers = buf + 2;
      int ret = 0;
      do {
        int v = 0;
        int pos;
        ret = sscanf(numbers, "%d%n", &v, &pos);
        numbers += pos + 1;
        if(v == 0)
          break;

        uint32_t v_ = abs(v);

        assert(v_ < solver->variables + 1);
        solver->assignments[v_] = miniexact_sign(v);

        if(v_ == solver->variables)
          return;
      } while(ret == 1);
    }
  }
}

int
miniexact_sat_solver_solve(miniexact_sat_solver* solver) {
  fclose(solver->infd_handle);
  solver->infd_handle = NULL;
  close(solver->infd[1]);

  // Wait for pid to finish.

  int status;
  waitpid(solver->pid, &status, 0);

  if(WIFEXITED(status)) {
    int exit_code = WEXITSTATUS(status);
    switch(exit_code) {
      case 10:
        parse_solver_output(solver);
        fclose(solver->outfd_handle);
        return 10;
      case 20:
        fclose(solver->outfd_handle);
        return 20;
      default:
        miniexact_err("Child SAT solver process had unexpected exit code %d!",
                      exit_code);
        fclose(solver->outfd_handle);
        return exit_code;
    }
  } else {
    miniexact_err("Child SAT solver process had unexpected exit!");
  }
  return 0;
}
