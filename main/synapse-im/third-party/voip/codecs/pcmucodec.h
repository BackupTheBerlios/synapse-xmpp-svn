#ifndef PCMUCODEC_H
#define PCMUCODEC_H

#include "voicecodec.h"
#include "codec.h"

class PCMUEncoder: public VoiceEncoder {
public:
	PCMUEncoder();
	virtual ~PCMUEncoder();

    // encode chunk of audio data
    // return value: the size of *res
    virtual int encode( const short *data, int size, char **res, int *samplesProcessed  );

};

class PCMUDecoder: public VoiceDecoder {
public:
	PCMUDecoder();
	virtual ~PCMUDecoder();

    // decode chunk of audio data
    // return value: the size of *res
    virtual int decode( const char *data, int size, short **res );

};

class PCMUCodecFactory : public VoiceCodecFactory {
public:
	PCMUCodecFactory();
	virtual ~PCMUCodecFactory();
    
    virtual std::string name() { return "PCMU"; }
    virtual double bandwidth() { return 64.0; }
    virtual std::string description() { return ""; }

    virtual int payload() { return 0; };
    virtual Codec codec();

    virtual std::string rtmap() { return std::string("PCMU/8000/1"); } ;

    virtual VoiceEncoder *encoder() { return new PCMUEncoder(); }
    virtual VoiceDecoder *decoder() { return new PCMUDecoder(); }
};


#endif

