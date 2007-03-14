// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005  Dmitry Poplavsky <dima@thekompany.com>

#include "sdp.h"
#include <q3stringlist.h>
#include <qregexp.h>


SDP::SDP()
{
    sessionName = "Tins session";
}

SDP::SDP(QString message)
{
    parse(message);
}

QString SDP::toString()
{
    QString res;
    res =  "v=0\n";
    res += QString("o=%1\n").arg(owner);
    res += QString("s=%1\n").arg(sessionName);
    res += QString("c=IN IP4 %1\n").arg(host);
    res += QString("m=audio %1 RTP/AVP").arg(port);

    QString aFields = "";

    QValueList<int>::Iterator it;
    for ( it=formats.begin(); it!=formats.end(); ++it ) {
        int format = *it;
        res += QString(" %1").arg(format);
        if ( rtmaps[format] )
            aFields += QString("a=rtpmap:%1 %2\n").arg(format).arg( *rtmaps[format] );
        if ( fmtps[format] )
            aFields += QString("a=fmtps:%1 %2\n").arg(format).arg( *fmtps[format] );
    }

    if ( aFields.length() )
        res += QString("\n")+aFields;
    
    return res;
}


void SDP::parse(QString data)
{
    formats.clear();
    rtmaps.clear();
    fmtps.clear();
    
    QStringList items = QStringList::split('\n',data);
    for ( QStringList::Iterator it = items.begin(); it != items.end(); ++it ) {
        QString item = (*it).stripWhiteSpace();
        if ( item.startsWith("o=") ) {
            owner = item.mid(2);
        } else
        if ( item.startsWith("s=") ) {
            sessionName = item.mid(2);
        } else
        if ( item.startsWith("c=IN IP4") ) {
            QRegExp rx( "c=IN IP4 ([^ /]+)" );
            if ( rx.search(item) >= 0 ) {
                host = rx.cap(1);
            }
            //host = item.mid(9).stripWhiteSpace();
        } else
        if ( item.startsWith("m=audio") ) {
            QRegExp rx( "m=audio (\\d+) RTP/AVP ([\\s\\d]*)" );
            if ( rx.search(item) >= 0 ) {
                port = rx.cap(1).toInt();
                QStringList sformats = QStringList::split(' ',rx.cap(2));
                //qDebug("sformats:"+rx.cap(2));
                for ( QStringList::Iterator it = sformats.begin(); it != sformats.end(); ++it ) {
                    int format = (*it).toInt();
                    formats.append( format );
                    //qDebug("add format %d",format);
                }
            }
        } else
        if ( item.startsWith("a=rtmap:") ) {
            QRegExp rx( "a=rtmap:(\\d+) (.*)" ); // TODO I don't know why, but cap(2) fetches only the first symbol
            if ( rx.search(item) >= 0 ) {
                rtmaps.insert( rx.cap(1).toInt(), rx.cap(2) );
            }
        } else
        if ( item.startsWith("a=fmtp:") ) {
            QRegExp rx( "a=fmtp:(\\d+) (.*)" ); // TODO the same problem as with a=rtmap:
            if ( rx.search(item) >= 0 ) {
                fmtps.insert( rx.cap(1).toInt(), rx.cap(2) );
            }
        }
    }
}

