#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_INP 100
#define MAX_ARGS  100
void parse_args(char *input, char **args);
/* ---------- Builtin helpers ---------- */

bool is_builtin(const char *cmd) {
  return strcmp(cmd, "echo") == 0 || strcmp(cmd, "exit") == 0 || strcmp(cmd, "type") == 0 || strcmp(cmd, "pwd")==0 || (strcmp(cmd, "cd") == 0) || 
  (strcmp(cmd,"history") == 0);
}

/* ---------- Input ---------- */

bool read_input(char *input) {
    printf("$ ");
    if (fgets(input, MAX_INP, stdin) == NULL) {
      printf("\n");
      return false;
    }
    input[strcspn(input, "\n")] = '\0';
    return true;
}

/* ---------- echo ---------- */

bool handle_echo(char *input) {
  char input_copy[MAX_INP];
  strcpy(input_copy, input);

  char *args[MAX_ARGS];
  parse_args(input_copy, args);

  if (args[0] == NULL || strcmp(args[0], "echo") != 0)
    return false;

  for (int i = 1; args[i]; i++) {
    printf("%s", args[i]);
    if (args[i + 1])
      printf(" ");
  }
  printf("\n");
  return true;
}


/* ---------- type ---------- */

bool find_in_path(const char *cmd, char *resolved_path) {
    char *path_env = getenv("PATH");
    if (!path_env) return false;

    char *path_copy = strdup(path_env);
    char *dir = strtok(path_copy, ":");

    while (dir) {
        snprintf(resolved_path, 1024, "%s/%s", dir, cmd);

        if (access(resolved_path, X_OK) == 0) {
          struct stat st;
          if (stat(resolved_path, &st) == 0 && S_ISREG(st.st_mode)) {
            free(path_copy);
            return true;
          }
        }
        dir = strtok(NULL, ":");
    }

    free(path_copy);
    return false;
}

bool handle_type(const char *input) {
    if (strncmp(input, "type ", 5) != 0)
      return false;

    const char *cmd = input + 5;

    if (is_builtin(cmd)) {
      printf("%s is a shell builtin\n", cmd);
      return true;
    }

    char path[1024];
    if (find_in_path(cmd, path)) {
      printf("%s is %s\n", cmd, path);
    } else {
      printf("%s: not found\n", cmd);
    }

    return true;
}

/* ---------- Argument parsing ---------- */

void parse_args(char *input, char **args) {
    int argc = 0;
    char *p = input;

    while (*p) {
        /* Skip leading spaces/tabs */
        while (*p == ' ' || *p == '\t')
            p++;

        if (*p == '\0')
            break;

        args[argc++] = p;

        bool in_single_quotes = false;
        bool in_double_quotes = false;

        while (*p) {

            /* Backslash handling */
            if (*p == '\\') {

                /* Outside quotes: escape anything */
                if (!in_single_quotes && !in_double_quotes) {
                    memmove(p, p + 1, strlen(p));
                    if (*p != '\0')
                        p++;
                    continue;
                }

                /* Inside double quotes: escape only certain chars */
                if (in_double_quotes) {
                    char next = *(p + 1);

                    if (next == '"' || next == '\\' ||
                        next == '$' || next == '`') {
                        memmove(p, p + 1, strlen(p));
                        p++;   /* skip escaped char */
                        continue;
                    }

                    /* Escaped newline */
                    if (next == '\n') {
                        memmove(p, p + 2, strlen(p + 1));
                        continue;
                    }

                    /* Otherwise: backslash is literal */
                    p++;
                    continue;
                }

                /* Inside single quotes: literal */
                p++;
                continue;
            }

            /* Single quotes (ignored inside double quotes) */
            if (*p == '\'' && !in_double_quotes) {
                in_single_quotes = !in_single_quotes;
                memmove(p, p + 1, strlen(p));
                continue;
            }

            /* Double quotes (ignored inside single quotes) */
            if (*p == '"' && !in_single_quotes) {
                in_double_quotes = !in_double_quotes;
                memmove(p, p + 1, strlen(p));
                continue;
            }

            /* Argument delimiter */
            if ((*p == ' ' || *p == '\t') &&
                !in_single_quotes &&
                !in_double_quotes) {
                *p = '\0';
                p++;
                break;
            }

            p++;
        }

        if (argc >= MAX_ARGS - 1)
            break;
    }

    args[argc] = NULL;
}

/* ---------- External execution ---------- */

bool execute_external(char *input) {
    char input_copy[MAX_INP];
    strcpy(input_copy, input);

    char *args[MAX_ARGS];
    parse_args(input_copy, args);

    char exec_path[1024];
    if (!find_in_path(args[0], exec_path))
      return false;

    pid_t pid = fork();

    if (pid == 0) {
      execv(exec_path, args);
      perror("execv");
      exit(1);
    } else if (pid > 0) {
      waitpid(pid, NULL, 0);
    }

    return true;
}

/*------------Changing Directory------------*/
bool handle_cd(char* input){
  if (strncmp(input, "cd ", 3) == 0) {
    const char *path = input + 3;
    /* handle cd ~ */
    if (strcmp(path, "~") == 0) {
      char *home = getenv("HOME");
      if (home == NULL || chdir(home) != 0) {
        printf("cd: ~: No such file or directory\n");
      }
      return true;
    }
    /* handle absolute & relative paths */
    if (chdir(path) != 0) {
      printf("cd: %s: No such file or directory\n", path);
    }
    return true;
  }
  else{
    return false;
  }
}

/* ---------- main ---------- */

int main(void) {
    char input[MAX_INP];
    setbuf(stdout, NULL);

    while (1) {
        if (!read_input(input))
          break;

        if (strlen(input) == 0)
          continue;

        if (strcmp(input, "exit") == 0)
          break;

        if (handle_echo(input))
          continue;

        if (handle_type(input))
          continue;

        if(strcmp(input,"pwd") == 0){
          char cwd[1024];
          if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
          } else {
            perror("pwd");
          }
          continue;
        }

        if (handle_cd(input))
          continue;

        if (execute_external(input))
          continue;

        printf("%s: command not found\n", input);
    }

    return 0;
}
