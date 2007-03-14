// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005  Dmitry Poplavsky <dima@thekompany.com>

#ifndef SDP_H
#define SDP_H

#include <qstring.h>
#include <q3valuelist.h>
#include <qmap.h>

// simple sdp,
// only one m=audio field assumed

class SDP
{
public:
    SDP();
    SDP(QString message);

    QString toString();
    void parse(QString);

    //SDP data
    QString sessionName; // s field
    QString owner; // o field
    QString host; // from c field
    int port;   // from the m=audio field
    Q3ValueList<int> formats; // from the m=audio field
    QMap<int,QString> rtmaps; // dict of values a=rtmap:<format>
    QMap<int,QString> fmtps; // dict of values a=fmtp:<format>
    //QStringList aFields; // a fields, except a=rtmap and a=fmtp
};

#endif // SDP_H

