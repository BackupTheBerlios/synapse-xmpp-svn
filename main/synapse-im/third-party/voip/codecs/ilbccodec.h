// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>


#ifndef __ILBC_CODEC__H__
#define __ILBC_CODEC__H__

#error iLBC is not compatible with GPL and should not be used

#include "voicecodec.h"

class ILBCEncoder: public VoiceEncoder {
public:
	ILBCEncoder();
	virtual ~ILBCEncoder();

    // encode chunk of audio data
    // return value: the size of *res
    virtual int encode( const short *data, int size, char **res, int *samplesProcessed );

private:
	class Private;
	Private *d;

};

class ILBCDecoder: public VoiceDecoder {
public:
	ILBCDecoder();
	virtual ~ILBCDecoder();

    // decode chunk of audio data
    // return value: the size of *res
    virtual int decode( const char *data, int size, short **res );

private:
	class Private;
	Private *d;

};

class ILBCCodecFactory : public VoiceCodecFactory {
public:
	ILBCCodecFactory(); 
	virtual ~ILBCCodecFactory();
    
    virtual QString name() { return "ILBC"; }
    virtual double bandwidth() { return 13.33; }
    virtual QString description() { return ""; }

    virtual int payload() { return 102; };

    virtual QString rtmap() { return QString("iLBC/8000/1"); } ;

    virtual VoiceEncoder *encoder() { return new ILBCEncoder(); }
    virtual VoiceDecoder *decoder() { return new ILBCDecoder(); }
};




#endif // __SPEEX_CODEC__H__

