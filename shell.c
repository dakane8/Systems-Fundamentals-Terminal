#include "shell.h"

command* create_command(int argc) {
    command* rv = (command*)malloc(sizeof(command));
    if (!rv) {
        fprintf(stderr, "malloc");
        exit(EXIT_FAILURE);
    }
    rv->argc = argc;
    rv->argv = (char**)malloc((argc + 1) * sizeof(char*));
    if (!rv->argv) {
        fprintf(stderr, "malloc");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < argc; i++) {
        rv->argv[i] = (char*)malloc(MAX_ARG_LEN);
        if (!rv->argv[i]) {
            fprintf(stderr, "malloc");
            exit(EXIT_FAILURE);
        }
    }
    rv->argv[argc] = NULL;
    return rv;
}

command* parse(char* line) {
    char* clone = strdup(line);
    if (!clone) {
        fprintf(stderr, "strdup");
        exit(EXIT_FAILURE);
    }
    int argc = 0;
    char* token = strtok(clone, " \t\n");
    while (token) {
        argc++;
        token = strtok(NULL, " \t\n");
    }
    free(clone);

    clone = strdup(line);
    if (!clone) {
        fprintf(stderr, "strdup");
        exit(EXIT_FAILURE);
    }

    command* cmd = create_command(argc);
    int index = 0;
    token = strtok(clone, " \t\n");
    while (token) {
        strncpy(cmd->argv[index++], token, MAX_ARG_LEN);
        token = strtok(NULL, " \t\n");
    }
    free(clone);
    return cmd;
}

bool find_full_path(command* cmd) {
    char* path = getenv("PATH");
    if (!path) return false;

    char* clone = strdup(path);
    if (!clone) {
        fprintf(stderr, "strdup");
        exit(EXIT_FAILURE);
    }

    char* token = strtok(clone, ":");
    struct stat sb;
    while (token) {
        char full_path[MAX_ARG_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", token, cmd->argv[0]);
        if (stat(full_path, &sb) == 0 && S_ISREG(sb.st_mode)) {
            strncpy(cmd->argv[0], full_path, MAX_ARG_LEN);
            free(clone);
            return true;
        }
        token = strtok(NULL, ":");
    }

    free(clone);
    return false;
}

int execute(command* cmd) {
    if (is_builtin(cmd)) {
        return do_builtin(cmd);
    }

    if (!find_full_path(cmd)) {
        printf("Command %s not found!\n", cmd->argv[0]);
        return ERROR;
    }

    pid_t pid = fork();
    if (pid == 0) {
        execv(cmd->argv[0], cmd->argv);
        fprintf(stderr, "execv");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? SUCCESS : ERROR;
    } else {
        fprintf(stderr, "fork");
        return ERROR;
    }
}

void cleanup(command* cmd) {
    for (int i = 0; i < cmd->argc; i++) {
        free(cmd->argv[i]);
    }
    free(cmd->argv);
    free(cmd);
}

bool is_builtin(command* cmd) {
    // Do not modify
    char* executable = cmd->argv[0];
    if (strcmp(executable, "cd") == 0 || strcmp(executable, "exit") == 0)
        return true;
    return false;
}

int do_builtin(command* cmd) {
    // Do not modify
    if (strcmp(cmd->argv[0], "exit") == 0) exit(SUCCESS);

    // cd
    if (cmd->argc == 1)
        return chdir(getenv("HOME"));  // cd with no arguments
    else if (cmd->argc == 2)
        return chdir(cmd->argv[1]);  // cd with 1 arg
    else {
        fprintf(stderr, "cd: Too many arguments\n");
        return ERROR;
    }
}
