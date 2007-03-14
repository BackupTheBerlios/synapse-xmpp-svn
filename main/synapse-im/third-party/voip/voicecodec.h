#ifndef VOICECODEC_H
#define VOICECODEC_H

//#include <qstring.h>
//#include <q3valuelist.h>
#include <string>
#include <vector>
#include "talk/session/phone/codec.h"

class VoiceEncoder {
public:
	VoiceEncoder();
	virtual ~VoiceEncoder();

    // encode chunk of audio data
    // return value: the size of *res, 0 is ok, becouse of prebuffering
    // samplesProcessed: number of samples, coded used to encode res,
    // this is needed for RTP timing
    virtual int encode( const short *data, int size, char **res,int *samplesPocessed ) = 0;

};

class VoiceDecoder {
public:
	VoiceDecoder();
	virtual ~VoiceDecoder();

    // decode chunk of audio data
    // return value: the size of *res, 0 is ok, becouse of prebuffering
    virtual int decode( const char *data, int size, short **res ) = 0;

};


class VoiceCodecFactory {
public:
    //VoiceCodecFactory();
    
    // codec's description 
    virtual std::string name() = 0;
    virtual std::string description() { return ""; }
    virtual double bandwidth() = 0;

    // RTP payload type, rfc1890
    virtual int payload() = 0;
    virtual Codec codec() = 0;

    // a:rtmap SDP argument, like "SPEEX/8000" 
    virtual std::string rtmap() = 0;

    virtual VoiceEncoder *encoder() = 0;
    virtual VoiceDecoder *decoder() = 0;
};


class CodecsManager {
public:
    CodecsManager();
    ~CodecsManager();

    static CodecsManager * instance();

    void installCodecFactory( VoiceCodecFactory* );

    VoiceCodecFactory* codecFactory( int payload );
    VoiceCodecFactory* codecFactory( std::string rtmap );

    std::vector<int> payloads();

    std::vector<Codec, std::allocator<Codec> > codecs();
private:    
    class Private;
    Private *d;
};

#endif // VOICECODEC_H

