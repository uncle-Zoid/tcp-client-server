#ifndef IOBSERVABLE_H
#define IOBSERVABLE_H

#include <inttypes.h>
struct Datax
{
int messageId() const {return 0;}
uint8_t *data() const {return 0;}
int size()      const {return 0;}
};

class IObservable
{
public:
    IObservable() = default;
    ~IObservable() = default;

    virtual void update(const Datax &datax) const = 0;
};

#endif // IOBSERVABLE_H
