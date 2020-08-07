#ifndef CLIENT_H
#define CLIENT_H

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>
#include <vector>
#include <inttypes.h>

#include "iobservable.h"

class Client : public IObservable
{
    int fd_;
    std::vector<uint8_t> filter_;

public:
    Client();
    Client(int fd);
    Client(Client &cl) = delete;
    Client(const Client &cl) = delete;
    void operator =(Client &d) = delete;
    void operator =(const Client &d) = delete;

    Client(Client &&cl)
    {
        swap(cl);
    }

    void operator =(Client &&d)
    {
        swap(d);
    }
    void swap(Client &d)
    {
        std::swap(fd_, d.fd_);
        std::swap(filter_, d.filter_);
        d.fd_ = -1;
    }

    ~Client();

    void setFilter(const uint8_t* buffer, int size);

    virtual void update(const Datax &datax) const override;
};


#endif // CLIENT_H
