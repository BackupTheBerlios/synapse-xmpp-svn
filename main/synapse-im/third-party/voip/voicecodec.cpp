#include "voicecodec.h"

//#include <q3valuelist.h>
//#include <q3dict.h>
//#include <q3intdict.h>

#include "codec.h"
#include "codecs/pcmucodec.h"
#include "codecs/speexcodec.h"
//#include "codecs/ilbccodec.h"

VoiceEncoder::VoiceEncoder(){}
VoiceEncoder::~VoiceEncoder(){}
VoiceDecoder::VoiceDecoder(){}
VoiceDecoder::~VoiceDecoder(){}
    


CodecsManager * codecsManagerInstance = 0;

class CodecsManager::Private {
public:
//     Q3IntDict<VoiceCodecFactory> codecs; // payload->VoiceCodecFactory dictionary
//     Q3Dict<VoiceCodecFactory> rtmapDicts; // payload->VoiceCodecFactory dictionary
    std::vector<int> payloads;
    std::vector<long> factories_;
    std::vector<Codec, std::allocator<Codec> > codecs_;
};

CodecsManager::CodecsManager()
{
    d = new Private();
    codecsManagerInstance = this;

    installCodecFactory( new PCMUCodecFactory() );
    installCodecFactory( new SpeexCodecFactory() );
    //installCodecFactory( new ILBCCodecFactory() );
    printf("installed %d codecs.\n",d->codecs_.size());
    printf("installed %d factories.\n",d->factories_.size());
}

CodecsManager::~CodecsManager()
{
    delete d;
}


CodecsManager * CodecsManager::instance()
{
    if ( !codecsManagerInstance )
        new CodecsManager();

    return codecsManagerInstance;
}

void CodecsManager::installCodecFactory( VoiceCodecFactory* factory )
{
//     d->codecs.insert( factory->payload(), factory );
//     d->rtmapDicts.insert( factory->rtmap(), factory );
    d->payloads.push_back( factory->payload() );
    d->factories_.push_back( (long)factory );
    d->codecs_.push_back( factory->codec() );
}

VoiceCodecFactory* CodecsManager::codecFactory( int payload )
{
    std::vector<long>::iterator i;
    for ( i=d->factories_.begin(); i < d->factories_.end(); i++ )
    {
      if ( ((VoiceCodecFactory*)(*i))->payload() == payload)
        return (VoiceCodecFactory*)*i;
    }
//     return d->codecs[payload];
}

VoiceCodecFactory* CodecsManager::codecFactory( std::string rtmap )
{
    std::vector<long>::iterator i;
    for ( i=d->factories_.begin(); i < d->factories_.end(); i++ )
    {
      if (((VoiceCodecFactory*)(*i))->rtmap() == rtmap)
        return (VoiceCodecFactory*)*i;
    }
//     return d->rtmapDicts[rtmap];
}


std::vector<int> CodecsManager::payloads()
{
    return d->payloads;
}

std::vector<Codec, std::allocator<Codec> > CodecsManager::codecs()
{
    return d->codecs_;
}