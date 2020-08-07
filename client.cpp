#include "client.h"
#include <algorithm>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
Client::Client()
    : fd_(-1)
{}
Client::Client(int fd)
    : fd_(fd)
{

}

Client::~Client()
{
    if(fd_>=0)
    {
        std::cout << "Client close" << std::endl;
        ::close(fd_);
    }
}

void Client::setFilter(const uint8_t *buffer, int size)
{
    if(buffer == nullptr || size <= 0)
        return;

    filter_.clear();
    for (const uint8_t *it = buffer; it != buffer + size; ++it)
    {
        filter_.push_back(*it);
    }
}

void Client::update(const Datax &datax) const
{
    const char *message = "ahoj";
    auto written = ::write(fd_, message, 4);
    return;
    // update z datadistrib update
    if(std::find(filter_.begin(), filter_.end(), datax.messageId()) != filter_.end())
    {
        auto written = ::write(fd_, datax.data(), datax.size());
        if(written < 0)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                //plny vystupni buffer
            }
            // log error
        }
    }
}
