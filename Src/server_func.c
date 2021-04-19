#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wait.h>
#include "tcp.h"
#include "../Header/server_func.h"
#include "../Header/util_func.h"

int serverCore(int sockfd)
{
    int rc;
    int i;
    int p;
    int ps;

    pid_t pids[CONNEXIONS_LIMIT];
    int fds[CONNEXIONS_LIMIT];
    int pipesfd[CONNEXIONS_LIMIT][2];

    while (1)
    {
        for (i = 0; i < CONNEXIONS_LIMIT; i++)
        {

            struct sockaddr_in6 addr = {0};
            socklen_t addrlen = sizeof(addr);
            fds[i] = accept(sockfd, (struct sockaddr *)&addr, &addrlen);

            if (fds[i] < 0)
            {
                log_error(fds[i], "accept()");
                continue;
            }

            printf("Connection from %s\n",
                   str_of_sockaddr((struct sockaddr *)&addr));

            rc = pipe(pipesfd[i]);
            if (rc < 0)
            {
                log_error(rc, "pipe(pipesfd[i])");
                continue;
            }

            pids[i] = fork();
            if (pids[i] < 0)
            {
                log_error(pids[i], "fork()");
                rc = close(fds[i]);
                if (rc < 0)
                {
                    log_error(rc, "close(fd)");
                    continue;
                }
            }
            else if (pids[i] == 0)
            {
                childWork(fds[i], pipesfd[i]); // Manage connexions

                rc = closePipes(pipesfd[i]);
                if (rc < 0)
                {
                    log_error(rc, "closePipes()");
                    continue;
                }

                exit(0);
            }
        }

        pid_t pid;
        int quit = 0;

        int j = 0;

        while (1)
        {

            // Handle connexion
            if (quit)
            {
                pid = waitpid(pids[j], NULL, 0);
                if (pid < 0)
                {
                    log_error(pid, "waitpid(pids[j], NULL, NULL)");
                    continue;
                }
            }
            else{
                pid = waitpid(pids[j], NULL, WNOHANG);
                if (pid < 0)
                {
                    log_error(pid, "waitpid(pids[j], NULL, WNOHANG)");
                    continue;
                }
            }
            if (pid == pids[j])
            {
                printf("Child %ld is dead\n", (long)pids[j]);
                rc = close(fds[j]);
                if (rc < 0)
                {
                    log_error(rc, "close(fds[j])");
                    continue;
                }

                rc = closePipes(pipesfd[j]);
                if (rc < 0)
                {
                    log_error(rc, "closePipes(pipesfd[j])");
                    continue;
                }

                quit++;
            }

            if (quit == 2)
            {
                break;
            }
            // exchange tlv
            // swtich between 1 and 0 to send tlv

            j =(j + 1) % 2;
        }

        // temporaire
        break;
    }

    return sockfd;
}

int childWork(int fd, int *pipefd)
{
    // Lis tlv
    // do something
    // send tlv back ?
    int br;
    int bw;

    char buff[BUFSIZE];

    while (1)
    {
        br = read_all(fd, buff, sizeof(char));
        if (br < 0)
        {
            handle_error(br, "read_all(fd, buff, sizeof(char))");
        }

        if (!strcmp(buff, "s"))
        {
            break;
        }
    }

    exit(0);
}

int closePipes(int *pipefd)
{
    int rc;

    for (int p = 0; p < 2; p++)
    {
        if ((rc = close(pipefd[p])) < 0)
        {
            log_error(rc, "close(fd)");
            return -1;
        }
    }

    return 0;
}