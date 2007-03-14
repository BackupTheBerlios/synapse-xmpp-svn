#include "mediastream.h"
#include "sdp.h"
#include "udp.h"

#include <qapplication.h>
#include <qwidget.h>
#include <qtimer.h>



int main( int argc, char **argv )
{
    if ( argc != 5 ) {
        qDebug("usage: ./testmstream localHost localPort remoteHost remotePort");
        return 1;
    }
    
    QApplication a( argc, argv );
    QWidget *mw = new QWidget();
    
    a.setMainWidget( mw );
    mw->show();

    initNetwork();
    
    MediaStream *stream = new MediaStream();
    SDP localSDP;
    localSDP.host = argv[1];
    localSDP.port = QString(argv[2]).toInt();
    localSDP.formats.append(0);
    SDP remoteSDP;
    remoteSDP.host = argv[3];
    remoteSDP.port = QString(argv[4]).toInt();
    remoteSDP.formats.append(0);

    qDebug("local SDP:\n%s\n\n",localSDP.toString().local8Bit().data() );
    qDebug("remote SDP:\n%s\n\n",remoteSDP.toString().local8Bit().data() );
    

    stream->start( localSDP.toString(), remoteSDP.toString() );
    
    int result = a.exec();
    delete mw;
    return result;
}

