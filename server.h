#ifndef SERVER_H
#define SERVER_H

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <string>
#include <map>

#include "iobservable.h"
#include "client.h"

class Server : public IObservable
{
    static constexpr const int MAX_CLIENTS = 2;
    static constexpr const int BUFFER_SIZE = 256;
    static constexpr const char *REFUSE_MESSAGE = "Sorry, maximum clients count reached.";
    int fd_;
    struct sockaddr_in addr_;

    uint8_t recvBuffer[BUFFER_SIZE];
    std::map<int, Client> clients_;


public:
//    enum E_SERVER {SERVER_ERROR, SERVER_NEW_CLIENT}
    static constexpr const int SERVER_ERROR = -2;
    Server();
    ~Server();

    bool start(int port, std::string &out_error);

    int serverFd() const { return fd_; }
    bool accept(fd_set &masterSet, int &maxFd);
    bool handleClient(const int clientFd, fd_set &fdset, int &maxFd);

    virtual void update(const Datax &datax) const override;
};

#endif // SERVER_H
