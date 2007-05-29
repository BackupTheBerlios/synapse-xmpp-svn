// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>

#include "ilbccodec.h"
#include "ringbuffer.h"

#include <string.h>
#include <math.h>


#include "../3party/iLBC/iLBC_define.h" 
#include "../3party/iLBC/iLBC_encode.h" 
#include "../3party/iLBC/iLBC_decode.h" 


ILBCCodecFactory::ILBCCodecFactory() {}
ILBCCodecFactory::~ILBCCodecFactory() {}

class ILBCEncoder::Private {
public:
    iLBC_Enc_Inst_t iLBCenc_inst;

    RingBuffer *inputBuffer;
    RingBuffer *outputBuffer;

};


class ILBCDecoder::Private {
public:
    iLBC_Dec_Inst_t iLBCdec_inst;


    RingBuffer *inputBuffer;
    RingBuffer *outputBuffer;
};


ILBCEncoder::ILBCEncoder()
{
    d = new Private;
    int mode = 30;

    initEncode(&d->iLBCenc_inst, mode);
    
    d->inputBuffer = new RingBuffer(1024);
    d->outputBuffer = new RingBuffer(1024);
    
}

ILBCDecoder::ILBCDecoder()
{
    d = new Private;
    int mode = 30;
    initDecode(&d->iLBCdec_inst, mode, 1); 

    d->inputBuffer = new RingBuffer(1024);
    d->outputBuffer = new RingBuffer(1024);

}


ILBCEncoder::~ILBCEncoder()
{
    delete d->inputBuffer;
    delete d->outputBuffer;
    delete d;
}

ILBCDecoder::~ILBCDecoder()
{
    delete d->inputBuffer;
    delete d->outputBuffer;
    delete d;
}


int ILBCEncoder::encode( const short *data, int size, char **res, int *samplesProcessed )
{

    d->inputBuffer->put( (char*)data, size*2 );
    *samplesProcessed = 0;

    //qDebug("get %d samples to encode", size);

    int frameSize = d->iLBCenc_inst.blockl;

    if ( d->inputBuffer->size() >= frameSize*2 )
    {
        float * input_frame = new float[frameSize];
        char *  output_frame = new char[frameSize*2];
        short * tmp = (short*)d->inputBuffer->data(); 
        
        /*Copy the 16 bits values to float so ILBC can work on them*/
        for(int i=0;i<frameSize;i++)
            input_frame[i] = tmp[i];

        iLBC_encode( (unsigned char *)output_frame, input_frame, &d->iLBCenc_inst );
        d->inputBuffer->fetch( frameSize*2 );
        *samplesProcessed += frameSize;

        
        int bytes = d->iLBCenc_inst.no_of_bytes;
        
        d->outputBuffer->put(output_frame, bytes);

	delete[] output_frame;
	delete[] input_frame;
    }

    int resSize = d->outputBuffer->size();
    if ( resSize>0 )
    {
        *res = new char[resSize];
        memcpy(*res, d->outputBuffer->data(), resSize );
        d->outputBuffer->clear();
        
        return resSize;
    } else {
        *res = 0;
    }

    return 0;
}


int ILBCDecoder::decode( const char *data, int size, short **res )
{
    //qDebug("iLPC decode %d bytes", size);
   
    d->inputBuffer->put( (char*)data, size );

    int packetSize = d->iLBCdec_inst.no_of_bytes;
    int frameSize = d->iLBCdec_inst.blockl;

    // may be it will use for VBR later
    while ( d->inputBuffer->size() >= packetSize ) {
        
        float * output_frame  = new float[BLOCKL_MAX];
        short * output  = new short[BLOCKL_MAX];

        iLBC_decode(output_frame, (unsigned char *)d->inputBuffer->data(),
            &d->iLBCdec_inst, 1 );

        d->inputBuffer->fetch(packetSize);
        
        // decoded ok
        for(int i=0;i<frameSize;i++)
        {
            if(output_frame[i] > 32000.)
                output_frame[i] = 32000.;
            if(output_frame[i] < -32000.)
                output_frame[i] = - 32000.;
            
            output[i] = (short)output_frame[i];
        }
        //for(int i=0;i<frame_size;i++)
        //    output[i] = (short)floor(0.5 + output_frame[i]);
                
        
        d->outputBuffer->put((char*)output, frameSize*2);
        delete[] output_frame;
        delete[] output;
    }

    d->inputBuffer->clear();

    int resSize = d->outputBuffer->size();

    if (  resSize > 0 ) {  
        *res = new short[resSize/2];
        memcpy(*res, d->outputBuffer->data(), (resSize/2)*2 );
        d->outputBuffer->clear();
        qDebug("iLPC: decoded %d samples", resSize/2 );
        return resSize/2;
    }

    return 0;
}


