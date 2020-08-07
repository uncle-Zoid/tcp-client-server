#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <cstring>
#include <iostream>
#include <signal.h>

#include "server.h"

void catchSignals(const std::vector<int>& quitSignals);
bool stop = false;

// https://www.ibm.com/support/knowledgecenter/en/ssw_i5_54/rzab6/xnonblock.htm
int main (int /*argc*/, char */*argv*/[])
{
    catchSignals({SIGINT, SIGTERM, SIGQUIT, SIGTERM, SIGHUP, SIGTSTP});

    Server server;
    std::string error;
    if(!server.start(33001, error))
    {
        std::cout << error << std::endl;
        exit(1);
    }


    int maxFd = server.serverFd();
    fd_set masterSet, fdsetTmp;
    FD_ZERO(&masterSet);
    FD_SET(maxFd, &masterSet);

    struct timeval   timeout;

    int rc;
    while(!stop)
    {
        timeout.tv_sec  = 1;
        timeout.tv_usec = 0;//10000;
        // zkopiruj fdset do docasneho (select meni)
        memcpy(&fdsetTmp, &masterSet, sizeof(masterSet));
//        fdsetTmp = masterSet;
        if ((rc = select(maxFd + 1, &fdsetTmp, NULL, NULL, &timeout)) < 0)
        {
            perror("select() failed!");
            printf("select errno = %d\n", errno);
            break;
        }

        // zadne prichozi deni na socketech ... odesli zpravy, pokud jsou k dispozici
        if (rc == 0)
        {
            std::cout << "nic"<<std::endl;
            // cuspomq.pop
            // update all observers
            Datax datax;
            server.update(datax);
        }
        else
        {
            std::cout << "some" << std::endl;

            // vyhledej socket, ktery vyvolal udalost
            for (int fd = 0; fd <= maxFd; ++fd)
            {
                if (FD_ISSET(fd, &fdsetTmp))
                {
                    // server
                    if (fd == server.serverFd())
                    {
                        if(!server.accept(masterSet, maxFd))
                        {
                            stop = true;
                            break;
                        }
                    }
                    // klient
                    else
                    {
                        server.handleClient(fd, masterSet, maxFd);
                    }
                    break;
               }
            }
        }
    }

    std::cout << "KONEC" << std::endl;
    return 0;
}

void catchSignals(const std::vector<int>& quitSignals)
{
    auto handler = [](int /*sig*/) ->void
    {
        stop = true;
    };

    for (int sig : quitSignals)
    {
        signal(sig, handler);
    }
}
