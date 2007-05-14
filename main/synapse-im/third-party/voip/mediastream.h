// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>
#ifndef MEDIASTREAM_H
#define MEDIASTREAM_H

#include <qobject.h>
#include <q3cstring.h>
#include <q3ptrqueue.h>

/*namespace cricket {
    class MediaChannel;
    class PacketQueue;
};*/

typedef unsigned int uint32_t;

/*!
 \class MediaStream mediastream.h
 \brief stub
 */

//class cricket::MediaChannel::NetworkInterface;

class MediaStream : public QObject {
    Q_OBJECT
public:
	MediaStream();
	virtual ~MediaStream();

    bool isRunning();
    

public slots:    
    void start( uint32_t ip, int port, int codecPayload);
    void stop();
    
    void timerClick();

    //send packets to network or not
    void setSend(bool send);

signals:
    void finished();

    //show middle and maximum level in range 0..100
    void micLevel( int mid, int max );
    void dspLevel( int mid, int max );


public:    
	class Private;
private:
	Private *d;

    void processMicData(short* data, int size);
    void processDspData(short* data, int size);
}; 

#endif // MEDIASTREAM_H


