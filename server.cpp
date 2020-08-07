#include "server.h"
#include <exception>
#include <unistd.h>
#include <cstring>
#include <iostream>

Server::Server()
    : fd_(-1)
{ }

Server::~Server()
{
    std::cout << "Close server" << std::endl;
    if(fd_ >=0)
        close(fd_);
}
bool Server::start(int port, std::string &out_error)
{
    const int one = 1;
    int rc;

    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0)
    {
       out_error = "socket() failed";
       return false;
    }
    if ((rc = setsockopt(fd_, SOL_SOCKET,  SO_REUSEADDR, &one, sizeof(one))) < 0)
    {
       out_error = "setsockopt() SO_REUSEADDR failed";
       return false;
    }
    if ((rc = ioctl(fd_, FIONBIO, &one)) < 0)
    {
       out_error = "ioctl() FIONBIO failed";
       return false;
    }

    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family      = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port        = htons(port);
    if ((rc = bind(fd_, (struct sockaddr *)&addr_, sizeof(addr_))) < 0)
    {
       out_error = "bind() failed";
       return false;
    }
    if ((rc = listen(fd_, 32)) < 0)
    {
       out_error = "listen() failed";
       return false;
    }
    std::cout << "server connected ... fd="<<fd_<<std::endl;
    return true;
}

bool Server::accept(fd_set &masterSet, int &maxFd)
{
    int clientFd = -1;
    // muze byt vic prichozich klientu zaroven
    do
    {
        clientFd = ::accept4(fd_, NULL, NULL , SOCK_NONBLOCK);
        if (clientFd < 0)
        {
           if (errno != EWOULDBLOCK && errno != EAGAIN)
           {
              // loguj error
              return clientFd;
           }
           return SERVER_ERROR;
        }

        // pridat klienta do seznamu
        if(clients_.size() < MAX_CLIENTS)
        {
            if(clients_.count(clientFd) > 0) clients_.erase(clientFd);

            Client client(clientFd);
            clients_.emplace(clientFd, std::move(client));

            // aktualizace fdset
            FD_SET(clientFd, &masterSet);
            if (clientFd > maxFd)
               maxFd = clientFd;
        }
        else
        {
            ::send(clientFd, REFUSE_MESSAGE, strlen(REFUSE_MESSAGE), 0);
            ::close(clientFd);
        }
    } while (clientFd != -1);

    return clientFd;
}

bool Server::handleClient(const int clientFd, fd_set &fdset, int &maxFd)
{
    int rc = recv(clientFd, recvBuffer, sizeof(recvBuffer), 0);
    if (rc < 0 && errno != EWOULDBLOCK)
    {
        // loguj error
         return false;
    }

    // prijmuti 0 bajtu by melo znamenat ukonceti spojeni od klienta
    if (rc == 0)
    {
        // najdi klienta podle fd a odeber ho e seznamu
        clients_.erase(clientFd);

        // aktualizace fdset
        FD_CLR(clientFd, &fdset);
        if (clientFd == maxFd)
        {
           while (FD_ISSET(maxFd, &fdset) == false)
           {
              maxFd -= 1;
           }
        }
    }
    // zprava od klienta
    else
    {
        auto it = clients_.find(clientFd);
        if (it != clients_.end())
        {
            it->second.setFilter(recvBuffer, rc); // rc == len
        }
    }
    return true;
}

void Server::update(const Datax &datax) const
{
    for (const auto &client : clients_)
    {
        client.second.update(datax);
    }
}
