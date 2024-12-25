#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    char buffer[50];
    char *argv[10];
    while (1)
    {
        printf("mini-shell>");
        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            buffer[strcspn(buffer, "\n")] = 0;
            if (strcmp(buffer, "exit") == 0)
            {
                printf("exited\n");
                break;
            }
            else
            {
                int argc = 0;
                char *dup = strdup(buffer); // Duplicate input buffer
                char *token;
                char *saveptr; // Used to keep track of the rest of the string

                token = strtok_r(dup, " \t\n", &saveptr); // First call to strtok_r
                while (token != NULL && argc < 10)
                {
                    argv[argc++] = token;                      // Add token to argv array
                    token = strtok_r(NULL, " \t\n", &saveptr); // Get the next token
                }
                argv[argc] = NULL; // Null-terminate the argv array

                if (argv[0] == NULL)
                {
                    // user typed an empty line
                    free(dup);
                    continue;
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
                    if (execvp(argv[0], argv) == -1)
                    {
                        perror("execvp failed");
                        free(dup);
                        exit(1);
                    }
                }
                else
                {
                    int status;
                    waitpid(pid, &status, 0);
                    /*if (WIFEXITED(status))
                    {
                        printf("Child exited with status %d\n", WEXITSTATUS(status));
                    }
                    else if (WIFSIGNALED(status))
                    {
                        printf("Child killed by signal %d\n", WTERMSIG(status));
                    }*/
                }
                free(dup);
            }
        }
    }
    return 0;
}
