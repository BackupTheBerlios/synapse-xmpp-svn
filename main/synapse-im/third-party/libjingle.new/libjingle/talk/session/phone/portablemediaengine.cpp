//
// C++ Implementation: portablemediaengine
//
// Based on linphonemediaengine.cpp
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
/*
extern "C" {
#include "talk/third_party/mediastreamer/mediastream.h"
#ifdef HAVE_ILBC
#include "talk/third_party/mediastreamer/msilbcdec.h"
#endif
#ifdef HAVE_SPEEX
#include "talk/third_party/mediastreamer/msspeexdec.h"
#endif
}
*/
//#include <ortp/ortp.h>
//#include <ortp/telephonyevents.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include "talk/base/logging.h"
#include "talk/base/thread.h"
#include "talk/session/phone/codec.h"
#include "talk/session/phone/portablemediaengine.h"
#include "voicecodec.h"
using namespace cricket;

PortableMediaChannel::PortableMediaChannel(PortableMediaEngine*eng) :
  pt_(-1),
  engine_(eng),
  first_time_(true),
  dying_(false) {
  mediaStream = new MediaStream();
}

PortableMediaChannel::~PortableMediaChannel() {
  dying_ = true;
  mediaStream->stop();
  delete mediaStream;
}

void PortableMediaChannel::SetCodecs(const std::vector<Codec> &codecs) {
 bool first = true;
 std::vector<Codec>::const_iterator i;
 printf("SetCodecs()..\n");
  for (i = codecs.begin(); i < codecs.end(); i++) {

    if (!engine_->FindCodec(*i))
      continue;

    if (first) {
      LOG(LS_INFO) << "Using " << i->name << "/" << i->clockrate;
      pt_ = engine_->ResolvePayload(*i);
      mediaStream->stop();
      mediaStream->start(&packet_queue_, this, pt_);
      first = false;
      return;
    }
  }
  
  if (first) {
    // We're being asked to set an empty list of codecs. This will only happen when
    // working with a buggy client; let's try PCMU.
     LOG(LS_WARNING) << "Received empty list of codces; using PCMU/8000";
     mediaStream->stop();
     mediaStream->start(&packet_queue_, this, 0);
    pt_ = 0;
  }
  printf("..done\n");
}

int PortableMediaEngine::ResolvePayload(const Codec &c) {
  std::vector<Codec>::const_iterator i;
  for (i = CodecsManager::instance()->codecs().begin(); i < CodecsManager::instance()->codecs().end(); i++) {
      if ((c.name == (*i).name) && (c.clockrate == (*i).clockrate)) {
          return (*i).id;
        }
  }
  return 0;
}

bool PortableMediaEngine::FindCodec(const Codec &c) {
  std::vector<Codec>::const_iterator i;
  printf("FindCodec()..\n");
  if (c.id == 0)
    return true;
  printf("..seaching in %d codecs..\n",CodecsManager::instance()->codecs().size());
  for (i = CodecsManager::instance()->codecs().begin(); i < CodecsManager::instance()->codecs().end(); i++) {
      printf("id= %d %d\n", c.id, (*i).id);
      if ((c.id == (*i).id) || ((c.name == (*i).name) && (c.clockrate == (*i).clockrate))) {
          printf("..founded.\n");
          return true;
        }
  }
return false;
}

void PortableMediaChannel::OnPacketReceived(const void *data, int len) {
  uint8 buf[2048];
  memcpy(buf, data, len);
  printf("Packet recieved -> id=%d\n", buf[1]);
  printf("Putting on queue\n");
  int payloadtype = buf[1] & 0x7f;
  if (play_ && payloadtype != 13)
  {
    packet_queue_.push(new PacketQueue::Data(&buf,len));
  }
}

void PortableMediaChannel::SetPlayout(bool playout) {
  printf("SetPlayout()..\n");
  play_ = playout;
}

void PortableMediaChannel::SetSend(bool send) {
  printf("SetSend()..\n");
  mute_ = !send;
  mediaStream->setSend(send);
}

int PortableMediaChannel::GetOutputLevel() { return 1; }

PortableMediaEngine::PortableMediaEngine() {}
PortableMediaEngine::~PortableMediaEngine() {}

bool PortableMediaEngine::Init() {
  codecs_ = CodecsManager::instance()->codecs();
  printf("Init with %d codecs.\n",codecs_.size());
  return true;
}

void PortableMediaEngine::Terminate() {
 
}
  
MediaChannel *PortableMediaEngine::CreateChannel() {
  return new PortableMediaChannel(this);
}

int PortableMediaEngine::SetAudioOptions(int options) { return 0; }
int PortableMediaEngine::SetSoundDevices(int wave_in_device, int wave_out_device) { return 0; }

float PortableMediaEngine::GetCurrentQuality() { return 1.0; }
int PortableMediaEngine::GetInputLevel() { return 1; }
