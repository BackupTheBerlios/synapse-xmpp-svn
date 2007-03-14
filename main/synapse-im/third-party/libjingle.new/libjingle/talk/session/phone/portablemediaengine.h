//
// C++ Interface: portablemediaengine
//
// Based on linphonemediaengine.h
//
// Description: 
//
//
// Author: root <root@mainframe>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
/*
 * Jingle call example
 * Copyright 2004--2005, Google Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// LinphoneMediaEngine is a Linphone implementation of MediaEngine

#ifndef TALK_SESSION_PHONE_PORTABLEMEDIAENGINE_H_
#define TALK_SESSION_PHONE_PORTABLEMEDIAENGINE_H_

//extern "C" {
#include "mediastream.h"
//}
//#include "talk/base/asyncsocket.h"
//#include "talk/base/scoped_ptr.h"
#include "talk/session/phone/mediaengine.h"

class MediaStream;

namespace cricket {

class PortableMediaEngine;

class PacketQueue {
  public:
    class Data {
      public:
	Data(void *data,int len) : next(NULL) {
          memcpy(data_, data, len);
          size_ = len;
        }
	~Data() {
          delete data_;
        }
	void *data() {
          return &data_;
        }
        int size() {
          return size_;
        }
        Data *next;
      private:
        uint8 data_[2048];
        int size_;
    };

    PacketQueue() {
      begin_ = NULL;
      end_ = NULL;
    }
    ~PacketQueue() {
      while(begin_ != NULL)
      {
        Data *tmp = pop();
	delete tmp;
      }
    }

    void push(Data *data) {
      if(end_ != NULL) {
        end_->next = data;
      } else {
        begin_ = data;
      }
      end_ = data;
    }
    Data *pop() {
      Data *data = begin_;
      if(begin_ != NULL) {
        begin_ = data->next;
	if(begin_ == NULL) 
	  end_ = NULL;
      } else {
        end_ = NULL;
      }
      return data;
    }

    bool isEmpty() {
       return (begin_ == NULL) ? true : false;
    }

  private:
    Data *begin_;
    Data *end_;
};

class PortableMediaChannel : public MediaChannel {
 public:
  PortableMediaChannel(PortableMediaEngine *eng);
  virtual ~PortableMediaChannel();

  virtual void SetCodecs(const std::vector<Codec> &codecs);
  virtual void OnPacketReceived(const void *data, int len);

  virtual void SetPlayout(bool playout);
  virtual void SetSend(bool send);


  virtual int GetOutputLevel();
  bool mute() {return mute_;}

  virtual void StartMediaMonitor(VoiceChannel * voice_channel, uint32 cms) {}
  virtual void StopMediaMonitor() {}
   
 private:
  PortableMediaEngine *engine_;
  MediaStream *mediaStream;
//  talk_base::scoped_ptr<talk_base::AsyncSocket> socket_;
  PacketQueue packet_queue_;
//   void OnIncomingData(talk_base::AsyncSocket *s);
  
  int pt_;
  bool mute_;
  bool dying_;
  bool play_;
  bool first_time_;
};

class PortableMediaEngine : public MediaEngine {
 public:
  PortableMediaEngine();
  ~PortableMediaEngine();
  virtual bool Init();
  virtual void Terminate();
  
  virtual MediaChannel *CreateChannel();

  virtual int SetAudioOptions(int options);
  virtual int SetSoundDevices(int wave_in_device, int wave_out_device);

  virtual float GetCurrentQuality();
  virtual int GetInputLevel();
  
  virtual std::vector<Codec, std::allocator<Codec> > codecs() {return codecs_;}
  virtual bool FindCodec(const Codec&);
  virtual int ResolvePayload(const Codec&);

 private:
  std::vector<Codec, std::allocator<Codec> > codecs_;
};

}  // namespace cricket

#endif  // TALK_SESSION_PHONE_PORTABLEMEDIAENGINE_H_
