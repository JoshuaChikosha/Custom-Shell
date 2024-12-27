#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define REDIRECT_STO ">"
#define REDIRECT_STI "<"
#define REDIRECT_STE "2>"
int main()
{
    char buffer[50];
    char *argv[10];
    while (1)
    {
        printf("mini-shell> ");
        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            buffer[strcspn(buffer, "\n")] = 0;

            // Used to check if argv[0] was NULL later on to check for empty input
            // But it can get checked earlier with this
            if (buffer[0] == '\0')
            {
                continue;
            }

            if (strcmp(buffer, "exit") == 0)
            {
                printf("exited\n");
                break;
            }

            int argc = 0;
            char *token;
            char *saveptr;              // Used to keep track of the rest of the string
            char *dup = strdup(buffer); // Duplicate input buffer
            if (!dup)
            {
                perror("strdup failed");
                continue;
            }

            token = strtok_r(dup, " \t\n", &saveptr); // First call to strtok_r
            while (token != NULL && argc < 10)
            {
                argv[argc++] = token;                      // Add token to argv array
                token = strtok_r(NULL, " \t\n", &saveptr); // Get the next token
            }
            argv[argc] = NULL; // Null-terminate the argv array

            char *cmd = argv[0];
            char *out_file = NULL;
            char *in_file = NULL;
            char *err_file = NULL;

            for (int i = 0; i < argc; i++)
            {
                if (strcmp(argv[i], REDIRECT_STO) == 0 && i + 1 < argc)
                {
                    out_file = argv[i + 1];
                    argv[i] = NULL;
                    i++;
                }
                if (strcmp(argv[i], REDIRECT_STI) == 0 && i + 1 < argc)
                {
                    in_file = argv[i + 1];
                    argv[i] = NULL;
                    i++;
                }
                if (strcmp(argv[i], REDIRECT_STE) == 0 && i + 1 < argc)
                {
                    err_file = argv[i + 1];
                    argv[i] = NULL;
                    i++;
                }
            }

            pid_t pid = fork();
            if (pid < 0)
            {
                perror("fork failed");
                free(dup);
                continue;
            }

            if (pid == 0)
            {
                // 1) Redirect stderr first
                if (err_file != NULL)
                {
                    int fd_err = open(err_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd_err < 0)
                    {
                        perror("error with error redirection");
                        exit(1);
                    }
                    dup2(fd_err, STDERR_FILENO);
                    close(fd_err);
                }

                // 2) Redirect stdin
                if (in_file != NULL)
                {
                    int fd_in = open(in_file, O_RDONLY);
                    if (fd_in < 0)
                    {
                        perror("error with input redirection");
                        exit(1);
                    }
                    dup2(fd_in, STDIN_FILENO);
                    close(fd_in);
                }

                // 3) Redirect stdout
                if (out_file != NULL)
                {
                    int fd_out = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd_out < 0)
                    {
                        perror("error with output redirection");
                        exit(1);
                    }
                    dup2(fd_out, STDOUT_FILENO);
                    close(fd_out);
                }

                // 4) Finally, exec
                if (execvp(argv[0], argv) == -1)
                {
                    perror("execvp failed");
                    exit(1);
                }
            }
            // parent
            else
            {
                int status;
                if (waitpid(pid, &status, 0) < 0)
                {
                    perror("waitpid failed");
                    continue; // Parent shell should continue even if waitpid fails
                }
            }
            free(dup);
        }
        else
        {
            printf("\nEnd of input (Ctrl+D), exiting.\n");
            break;
        }
    }
    return 0;
}
