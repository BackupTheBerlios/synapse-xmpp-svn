// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>


#ifndef __SPEEX_CODEC__H__
#define __SPEEX_CODEC__H__

#include "voicecodec.h"
#include "codec.h"

class SpeexEncoder: public VoiceEncoder {
public:
	SpeexEncoder();
	virtual ~SpeexEncoder();

    // encode chunk of audio data
    // return value: the size of *res
    virtual int encode( const short *data, int size, char **res, int *samplesProcessed );

private:
	class Private;
	Private *d;

};

class SpeexDecoder: public VoiceDecoder {
public:
	SpeexDecoder();
	virtual ~SpeexDecoder();

    // decode chunk of audio data
    // return value: the size of *res
    virtual int decode( const char *data, int size, short **res );

private:
	class Private;
	Private *d;

};

class SpeexCodecFactory : public VoiceCodecFactory {
public:
	SpeexCodecFactory(); 
	virtual ~SpeexCodecFactory();
    
    virtual QString name() { return "Speex"; }
    virtual double bandwidth() { return 8.0; }
    virtual QString description() { return ""; }

    virtual int payload() { return 99; };

    virtual QString rtmap() { return QString("speex/8000/1"); } ;

    virtual VoiceEncoder *encoder() { return new SpeexEncoder(); }
    virtual VoiceDecoder *decoder() { return new SpeexDecoder(); }
};




#endif // __SPEEX_CODEC__H__

