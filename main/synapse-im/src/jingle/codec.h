//
// C++ Interface: codec
//
// Description: 
//
//
// Author: Andrzej Wójcik <andrzej@hi-low.eu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _CODEC_H_
#define _CODEC_H_

struct Codec {
  int id;
  QString name;
  int clockrate;
  int bitrate;
  int channels;

  int preference;

 // Creates a codec with the given parameters.
  Codec(int pt, const QString& nm, int cr, int br, int cs, int pr) : 
    id(pt), name(nm), clockrate(cr), preference(pr), bitrate(br), channels(cs) {}
  // Ranks codecs by their preferences.
};

#endif // CODEC_H_
