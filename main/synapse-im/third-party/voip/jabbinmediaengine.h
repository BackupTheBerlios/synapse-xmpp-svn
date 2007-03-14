#ifndef JABBIN_MEDIAENGINE_H_
#define JABBIN_MEDIAENGINE_H_

#include "talk/session/phone/mediaengine.h"

#include <Qt/q3cstring.h>
#include <Qt/q3ptrqueue.h>

class MediaStream;

namespace cricket {

class JabbinMediaChannel : public MediaChannel {
 public:
  JabbinMediaChannel();
  virtual ~JabbinMediaChannel();
  virtual void SetCodec(const char *codec);
  virtual void OnPacketReceived(const void *data, int len);

  virtual void SetPlayout(bool playout);
  virtual void SetSend(bool send);

  virtual float GetCurrentQuality();
  virtual int GetOutputLevel();
  //int fd() {return fd_;}
  bool mute() {return mute_;}
  bool dying() {return dying_;}
 private:
  MediaStream *mediaStream;
  Q3PtrQueue<QByteArray> incomingPackets;

  //pthread_t thread_;
  //int fd_;
  int pt_;
  bool dying_;
  bool mute_;
  bool play_;
};

class JabbinMediaEngine : public MediaEngine {
 public:
  JabbinMediaEngine();
  virtual ~JabbinMediaEngine();
  virtual bool Init();
  virtual void Terminate();
  
  virtual MediaChannel *CreateChannel();

  virtual int SetAudioOptions(int options);
  virtual int SetSoundDevices(int wave_in_device, int wave_out_device);

  virtual float GetCurrentQuality();
  virtual int GetInputLevel();
};

}  // namespace cricket

#endif //JABBIN_MEDIAENGINE_H_

