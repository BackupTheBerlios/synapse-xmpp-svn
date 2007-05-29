// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>

class RingBuffer
{
public:
    RingBuffer( int defaultSize = 1024 );
    ~RingBuffer();

    int size();
    char* data();

    void fetch(int size);
    void put( char* newData, int size );
    void clear();

    void lock();
    void unlock();

private:
    class Private;
    Private *d;
};

