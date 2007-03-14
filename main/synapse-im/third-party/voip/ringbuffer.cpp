// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>

#include "ringbuffer.h"

#include <stdlib.h> 
#include <string.h>

//#include <qmutex.h>
#ifdef POSIX
#include <pthread.h>
#endif

class RingBuffer::Private {
public:
    char* data;
    int ringSize;
    
    int curIndex; // current buffer index
    int size;     // current data size

#ifdef POSIX
    pthread_mutex_t mutex;
#endif
};

RingBuffer::RingBuffer( int size )
{
    d = new Private;
#ifdef POSIX
    pthread_mutex_init( &d->mutex, NULL);
#endif
    d->data = (char*)malloc(size);
    d->ringSize = size;

    d->curIndex = 0;
    d->size = 0;
}

RingBuffer::~RingBuffer()
{
    free(d->data);
    delete d;
}


int RingBuffer::size()
{
    return d->size;
}

char* RingBuffer::data()
{
    return d->data + d->curIndex;
}

void RingBuffer::fetch(int size)
{
    if ( size>d->size ) {
//        qDebug( "Fetched more then buffer size!" );
        d->curIndex = 0;
        d->size = 0;
        
    } else {
        d->size -= size;
        if ( d->size == 0 )
            d->curIndex = 0;
        else
            d->curIndex += size;
    }
}

void RingBuffer::put( char* newData, int size )
{
	if (!newData) return;
    while ( d->size+size > d->ringSize ) {
        d->data = (char*)realloc( d->data, d->ringSize*2 );
        d->ringSize *= 2;
    }

    if ( d->size + d->curIndex + size > d->ringSize ) {
        memmove( d->data, d->data + d->curIndex, d->size );
        d->curIndex = 0;
    }

    memcpy( d->data + d->curIndex + d->size, newData, size );

    d->size += size;
}

void RingBuffer::clear()
{
    d->size = 0;
    d->curIndex = 0;
}

void RingBuffer::lock()
{
#ifdef POSIX
    pthread_mutex_lock(&d->mutex);
#endif
}

void RingBuffer::unlock()
{
#ifdef POSIX
    pthread_mutex_unlock(&d->mutex);
#endif
}


