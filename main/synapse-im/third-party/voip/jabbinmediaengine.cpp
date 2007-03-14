#include "jabbinmediaengine.h"

#include <sys/types.h>
#ifndef WIN32
    #include <unistd.h>
#endif
#include <fcntl.h>


#ifndef WIN32
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
#else
	#include <winsock2.h>
#endif // WIN32

#include "mediastream.h"

using namespace cricket;

JabbinMediaChannel::JabbinMediaChannel() {
  pt_ = -1;
  dying_ = false;

  mediaStream = new MediaStream(0);

}

JabbinMediaChannel::~JabbinMediaChannel() {
  dying_ = true;
  
  mediaStream->stop();
  delete mediaStream;
}

void JabbinMediaChannel::SetCodec(const char *codec) {

  if (!strcmp(codec, "iLBC"))
    pt_ = 102;
  else if (!strcmp(codec, "speex"))
    pt_ = 110; 
  else
    pt_ = 0;


  qDebug("set codec: %s, pauload type: %d", codec, pt_);

  mediaStream->stop();
  mediaStream->start( &incomingPackets, this, pt_ );
}

void JabbinMediaChannel::OnPacketReceived(const void *data, int len) {
  
  char buf[2048];
  memcpy(buf, data, len);
  
  if (buf[1] == pt_) {
  } else {
      if (buf[1] == 13) {
      } else if (buf[1] == 102) {
        SetCodec("iLBC");
      } else if (buf[1] == 110) {
        SetCodec("speex");
      } else if (buf[1] == 0) {
        SetCodec("PCMU");
      }
  }
  
  if (play_ && buf[1] != 13) { // 13 == comfort noise
      QByteArray *ba = new QByteArray(len);
      memcpy( ba->data(), data, len );
      incomingPackets.enqueue( ba );
  }
}

void JabbinMediaChannel::SetPlayout(bool playout) {
  play_ = playout;
}

void JabbinMediaChannel::SetSend(bool send) {
  mute_ = !send;
  mediaStream->setSend(send);
}

float JabbinMediaChannel::GetCurrentQuality() { return 0.0; }
int JabbinMediaChannel::GetOutputLevel() { return 0; }

JabbinMediaEngine::JabbinMediaEngine() {}
JabbinMediaEngine::~JabbinMediaEngine() {}

bool JabbinMediaEngine::Init() {
  //codecs_.push_back(Codec(102, "iLBC", 4));
  codecs_.push_back(Codec(110, "speex", 4));
  codecs_.push_back(Codec(0, "PCMU", 2));
  
return true;
}

void JabbinMediaEngine::Terminate() {
}
  
MediaChannel *JabbinMediaEngine::CreateChannel() {
  return new JabbinMediaChannel();
}

int JabbinMediaEngine::SetAudioOptions(int options) {return 0;}
int JabbinMediaEngine::SetSoundDevices(int wave_in_device, int wave_out_device) {return 0;}

float JabbinMediaEngine::GetCurrentQuality() {return 0.0;}
int JabbinMediaEngine::GetInputLevel() {return 0;}

