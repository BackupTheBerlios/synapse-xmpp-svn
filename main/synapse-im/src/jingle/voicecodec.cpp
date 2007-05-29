#include "voicecodec.h"

#include <q3valuelist.h>
#include <q3dict.h>
#include <q3intdict.h>


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
    Q3IntDict<VoiceCodecFactory> codecs; // payload->VoiceCodecFactory dictionary
    Q3Dict<VoiceCodecFactory> rtmapDicts; // payload->VoiceCodecFactory dictionary
    Q3ValueList<int> payloads;
};

CodecsManager::CodecsManager()
{
    d = new Private();
    codecsManagerInstance = this;

    installCodecFactory( new PCMUCodecFactory() );
    installCodecFactory( new SpeexCodecFactory() );
    //installCodecFactory( new ILBCCodecFactory() );
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
    d->codecs.insert( factory->payload(), factory );
    d->rtmapDicts.insert( factory->rtmap(), factory );
    d->payloads.append( factory->payload() );
}

VoiceCodecFactory* CodecsManager::codecFactory( int payload )
{
    return d->codecs[payload];
}

VoiceCodecFactory* CodecsManager::codecFactory( QString rtmap )
{
    return d->rtmapDicts[rtmap];
}


Q3ValueList<int> CodecsManager::payloads()
{
    return d->payloads;
}
