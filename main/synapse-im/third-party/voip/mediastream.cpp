// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>

#include "mediastream.h"

// jrtplib includes
#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpsessionparams.h"
#include "rtpipv4address.h"

// portaudio
#include "portaudio.h"

#include "voicecodec.h"
#include "ringbuffer.h"

//#include <qtimer.h>
//#include <qdatetime.h>
//#include <qthread.h>
//#ifdef POSIX
#include <qthread.h>
#define uint32 unsigned int
//#include <sys/time.h>
//#endif

#ifndef WIN32
	#include <netinet/in.h>
	#include <arpa/inet.h>
#else
	#include "talk/base/win32.h"
	#include <winsock2.h>
#endif // WIN32

#include <stdio.h>
class MediaThread : public QThread {
public:
    MediaThread( MediaStream *mediaStream ):
        QThread() 
    {
        this->mediaStream = mediaStream;
    }

    virtual void run() {
        while ( mediaStream->isRunning() ) {
            mediaStream->timerClick();
            msleep(1);
        }
    }

    MediaStream *mediaStream;
};

class MediaStream::Private
{
public:
    RTPSession session;

    // output network packets buffer
    char outBuffer[ 160*2*512 ];
    int outBufferPos, outBufferTime;

    PortAudioStream *audioStream;

    RingBuffer *micBuffer, *dspBuffer;

    VoiceEncoder *encoder;
    VoiceDecoder *decoder;

    int codecPayload;


    RTPUDPv4TransmissionParams transparams;

    bool sendPacketsFlag;

//   QTimer timer;
    bool isRunning;
    //QMutex mutex;
    MediaThread *processThread;

};

MediaStream::MediaStream()
{
    d = new Private;
    d->micBuffer = new RingBuffer(4*2*8000);
    d->dspBuffer = new RingBuffer(4*2*8000);
    d->audioStream = 0;
    d->codecPayload = -1;
    d->isRunning = false;
    d->processThread = new MediaThread(this);

}

MediaStream::~MediaStream()
{
    if ( isRunning() )
        stop();

    delete d->micBuffer;
    delete d->dspBuffer;
    delete d->processThread;
    delete d;
    d = 0;
}


static int audioCallback( void *inputBuffer, void *outputBuffer, // {{{
                       unsigned long framesPerBuffer, PaTimestamp outTime, void *userData )
{
    MediaStream::Private *d = (MediaStream::Private*)userData;

//     Q_UNUSED( outTime );

    d->micBuffer->lock();
    d->micBuffer->put( (char*)inputBuffer, framesPerBuffer*2 );
    d->micBuffer->unlock();

    memset(outputBuffer, 0 , framesPerBuffer*2 );
    
    d->dspBuffer->lock();
    unsigned int dataSize = d->dspBuffer->size();
    
    if ( dataSize >= framesPerBuffer*2 ) {
        memcpy( outputBuffer, d->dspBuffer->data(), framesPerBuffer*2 );
        d->dspBuffer->fetch( framesPerBuffer*2 );
    } else {
        if ( dataSize > 0 ) {
            memcpy( outputBuffer, d->dspBuffer->data(), (dataSize/2)*2 );
            d->dspBuffer->fetch( dataSize );
            // Zero out remainder of buffer if we run out of data.
            // it's already 0 filled with prev memset
            //for( unsigned int i=dataSize/2; i<framesPerBuffer; i++ ) {
            //    out[i] = 0;
            //}
        }
    }
    
    d->dspBuffer->unlock();
        
    return 0;
}


void MediaStream::start(uint32_t ip, int port, int codecPayload )
{
    if ( isRunning() )
        stop();

    if(d->processThread == NULL)
	d->processThread = new MediaThread(this);
    d->outBufferPos = 0;
    d->outBufferTime = 0;

    int localPort = 3000;
printf("getFactory(%d)\n", codecPayload);
    VoiceCodecFactory *factory = CodecsManager::instance()->codecFactory(codecPayload);
    if ( !factory ) {
        printf("VoiceCodecFactory not found!\n");
        return;
    }

    d->codecPayload = codecPayload;
    d->decoder =  factory->decoder();
    d->encoder =  factory->encoder();

printf("transparams\n");
    // Now, we'll create a RTP session and set the destination
//     d->transparams.mediaChannel = mediaChannel;
//     d->transparams.incomingPackets = incomingPackets;
    d->transparams.SetPortbase(localPort);
	
    RTPSessionParams sessparams;

    sessparams.SetOwnTimestampUnit(1.0/8000.0); // 8KHz
	sessparams.SetAcceptOwnPackets(true);

printf("session.Create()\n");
    int status = d->session.Create( sessparams, &d->transparams );

    if ( status<0 ) {
//        qDebug("can't create RTP session, %s", RTPGetErrorString(status).c_str() );
        d->session.Destroy(); 
        return;
    }

printf("session.AddDestination()\n");
    RTPIPv4Address addr(ip,port);
	status = d->session.AddDestination(addr);
    
    if ( status<0 ) {
//         qDebug("can't add rtp destination, %s", RTPGetErrorString(status).c_str() );
        d->session.Destroy(); 
        return;
    }

    //initialise audio

    status = Pa_Initialize();
    if( status != paNoError ) {
//        qDebug( "PortAudio error: %s", Pa_GetErrorText(status) );
        stop();
        return;
    }

    status = Pa_OpenDefaultStream(
        &d->audioStream,/* passes back stream pointer */
        1,              /* 1 input channel */
        1,              /* mono output */
        paInt16,        /* 16 bit fixed point output */
        8000,           /* sample rate */
        240,            /* frames per buffer */
        16,             /* number of buffers, if zero then use default minimum */
        audioCallback,  /* specify our custom callback */
        d );            /* pass our data through to callback */

    status = Pa_StartStream( d->audioStream );
    if( status != paNoError ) {
//         qDebug( "PortAudio error: %s", Pa_GetErrorText(status) );
        stop();
        return;
    }

    

    // put something to dsp buffer
    /*
    char emptyData[160*8];
    memset(  emptyData, 1, sizeof(emptyData) );
    d->dspBuffer->lock();
    d->dspBuffer->put( emptyData, sizeof(emptyData) );
    d->dspBuffer->unlock();
    */



    //d->timer.start(1,false);
    d->isRunning = true;
    d->processThread->start();
    
//    qDebug("mediastream started");
    printf("mediastream started\n");
        
} // }}}


void MediaStream::stop()
{
    if(!d->isRunning)
	return;
    printf("stopping...\n");
    d->isRunning = false;

    d->processThread->wait();
    printf("..1..\n");
    //d->processThread = NULL;
    //d->mutex.lock(); //wait for thread to stop
    
    //d->timer.stop();
    if ( d->audioStream ) {
        Pa_CloseStream( d->audioStream );
        d->session.Destroy(); 
    }
    printf("..2..\n");
    d->audioStream = 0;
    
    //d->mutex.unlock();

    printf("mediastream terminated\n");
}
//mic/dsp mixer levels, 0..100
int call_dlg_mic_level = 75;
int call_dlg_dsp_level = 75;


bool MediaStream::isRunning()
{
    return d->isRunning;
}

void MediaStream::setSend(bool send)
{
    d->sendPacketsFlag = send;
}


//called on timer
//all data is processed here
void MediaStream::timerClick()
{
//d->mutex.lock();

#ifndef TEST_AUDIO    
    int status = d->session.Poll();
    if ( status<0 ) {
//        qDebug("Poll: %s", RTPGetErrorString(status).c_str() );
    }

    //checkRtpError( status );
    // check incoming packets
    if ( d->session.GotoFirstSourceWithData() ) {

        //qDebug("have rtp data");
        do {
            //RTPSourceData *sourceData = d->session.GetCurrentSourceInfo();

            RTPPacket *pack;
            if ((pack = d->session.GetNextPacket()) != NULL) {
                //qDebug("Get packet N %ld", pack->GetExtendedSequenceNumber());


                // debug("Got  packet with payload type %d, size %d", pack->GetPayloadType(), pack->GetPayloadLength() );

                // TODO initialise decoder here using pack payload type, maybe use QIntDict of decoders
                if ( d->decoder ) {
                    short* decodedData = 0;

                    int size = d->decoder->decode((char*)pack->GetPayloadData(), pack->GetPayloadLength(), &decodedData );

                    if ( size > 0 ) {

                        // adjust the volume
                        for ( int i=0; i<size; i++ ) {
                            double val = double(decodedData[i]) * call_dlg_dsp_level / 50.0;
                            if ( val > 32700.0 )
                                val = 32700.0;
                            if ( val < -32700.0 )
                                val = -32700.0;

                            decodedData[i] = short(val);
                        }
                        
                        // write to DSP buffer
                        d->dspBuffer->lock();
                        d->dspBuffer->put( (char*)decodedData, size*2 );
                        d->dspBuffer->unlock();
                        
                        processDspData(decodedData,size);

                        delete[] decodedData;
                    }
//                    qDebug("decoded data (%d byes) with payload type %d",  size*2, pack->GetPayloadType() );


                } else {
//                    qDebug("can't decode data with payload type %d", pack->GetPayloadType() );
                }

                // we don't longer need the packet, so
                // we'll delete it
                delete pack;
            }
        } while ( d->session.GotoNextSourceWithData());
    }

    // send the packet
    // check for in data


    short *data = 0;
    int micDataSize = 0; // size of readed mic data in samples

    d->micBuffer->lock();
    micDataSize = d->micBuffer->size()/2;
    if ( micDataSize ) {
        data = new short[micDataSize];
        memcpy( data, d->micBuffer->data(), micDataSize*2 );
        d->micBuffer->fetch( micDataSize*2 );
    }
    d->micBuffer->unlock();

    // adjust mic volume
    for ( int i=0; i<micDataSize; i++ ) {
        double val = double(data[i]) * call_dlg_mic_level / 50.0;
        if ( val > 32700.0 )
            val = 32700.0;
        if ( val < -32700.0 )
            val = -32700.0;

        data[i] = short(val);
    }



    // examine the data here, to calculate levels
    processMicData(data, micDataSize);


    if ( data ) {
        char * encodedData = 0;
        //int readed = micDataSize;
        int size = 0;

        //qDebug("have mic data %d", micDataSize );
        
        
        do {
            int readed = 0;
            size = d->encoder->encode( data, micDataSize, &encodedData, &readed );

            int localPayload = d->codecPayload; // TODO get local payload here

  //          qDebug("readed %d  encoded %d", readed, size );

            delete[] data;
            data = 0;
            micDataSize = 0;

            // TODO: for pcmu packet (payload==0) send packets of certain size
            if ( size > 0 ) {
                memcpy( d->outBuffer+d->outBufferPos, encodedData, size );
                d->outBufferPos += size;
                d->outBufferTime += readed;
                if ( d->outBufferPos ) {
                    //checkRtpError( 

                    if ( d->session.IsActive() && d->sendPacketsFlag ) {
                        int status = d->session.SendPacket( (void *)d->outBuffer, (int)d->outBufferPos, (unsigned char)localPayload , false, (long)d->outBufferTime );
                        if ( status<0 ) {
//                             qDebug("can't SendPacket, %s", RTPGetErrorString(status).c_str() );
                        }
                    }
                    //qDebug("sent packet");
                }

                    

                d->outBufferPos = 0;
                d->outBufferTime = 0;
            }

            if ( encodedData ) {
                delete[] encodedData;
                encodedData = 0;
            }

        } while (size > 0);
    }
    
    status = d->session.Poll();
    if ( status<0 ) {
//         qDebug("Poll: %s", RTPGetErrorString(status).c_str() );
    }
#else // TEST_AUDIO

    short *data = 0;
    int micDataSize = 0; // size of readed mic data in samples

    d->micBuffer->lock();
    micDataSize = d->micBuffer->size()/2;
    if ( micDataSize ) {
        data = new short[micDataSize];
        memcpy( data, d->micBuffer->data(), micDataSize*2 );
        d->micBuffer->fetch( micDataSize*2 );
    }
    d->micBuffer->unlock();

    if (data) {
        // write to DSP buffer
        d->dspBuffer->lock();
        d->dspBuffer->put( (char*)data,micDataSize*2 );
        d->dspBuffer->unlock();

    }

    static int totalSamples = 0;
    totalSamples += micDataSize;


    if ( micDataSize )
        printf("total audio samples: %d  %d   \r", micDataSize, totalSamples);
    

#endif // TEST_AUDIO
    
//    d->mutex->unlock();
}



void MediaStream::processMicData(short* data, int size)
{
    int max = 0;
    long sum = 0;
    if (!size) return;
    for (int i=0;i<size;i++) {
        sum += abs(data[i]);
        if (abs(data[i]) > max)
            max = abs(data[i]);
    }
//    emit micLevel(sum/size, max);
    //qDebug("mic --- %i", int(sum/size));
}

void MediaStream::processDspData(short* data, int size)
{
    int max =0;
    long sum = 0;
    if (!size) return;
    for (int i=0;i<size;i++) {
        sum += abs(data[i]);
        if (abs(data[i]) > max)
            max = abs(data[i]);
    }
//    emit dspLevel(sum/size, max);
    //qDebug("dsp --- %i", int(sum/size));
}


