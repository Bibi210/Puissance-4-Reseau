#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wait.h>
#include "../tcp.h"
#include "../tlv.hpp"
#include "../server_func.h"
#include "../util_func.h"

int serverCore(int sockfd)
{
    int rc;
    int i;

    int fds[CONNEXIONS_LIMIT];

    while (1)
    {
        for (i = 0; i < CONNEXIONS_LIMIT; i++)
        {
            struct sockaddr_in6 addr = {0};
            socklen_t addrlen = sizeof(addr);
            fds[i] = accept(sockfd, (struct sockaddr *)&addr, &addrlen);

            if (fds[i] < 0)
            {
                ERROR_HANDLER("accept()", fds[i]);
                continue;
            }

            printf("Connection from %s\n",
                   str_of_sockaddr((struct sockaddr *)&addr));
        }

        pid_t pid;

        pid = fork();
        if (pid < 0)
        {
            ERROR_HANDLER("fork()", pid);

            rc = closeFds(fds);
            {
                ERROR_HANDLER("closeFds(fds)", rc);
            }
        }
        else if (pid == 0)
        {
            childWork(fds); // Manage connexions

            exit(0);
        }
        
        rc = closeFds(fds);
        {
            ERROR_HANDLER("closeFds(fds)", rc);
        }

        while (1)
        {
            // Handle connexion
            pid = waitpid(-1, NULL, WNOHANG);
            if (pid < 0)
            {
                ERROR_HANDLER("waitpid(-1, NULL, WNOHANG)", pid);
            }
        }
    }

    return sockfd;
}

int childWork(int *fds)
{
    // Lis tlv
    // do something
    // send tlv back ?

    int rc;

    int color = ROUGE;

    while (1)
    {
        Generic_tlv_t *tlv;
        rc = read_tlv(tlv, fds[color]);
        if (rc < 0)        
        {
            ERROR_SHUTDOWN("read_tlv(tlv, fds[color])", rc);
        }
        // PSEUDO pas complet

        // Decrypte tlv
        
        // Renvoie tlv approprié
        color = (color + 1) % 2; // switch player

    }

    exit(0);
}

int closeFds(int *fds)
{
    int rc;
    int failure = 0;

    for (int i = 0; i < CONNEXIONS_LIMIT; i++)
    {
        rc = close(fds[i]);
        if (rc < 0)
        {
            failure = 1;
            ERROR_HANDLER("close(fd)", rc);
            continue;
        }
    }

    if (failure)
    {
        return -1;
    }

    return 0;
}