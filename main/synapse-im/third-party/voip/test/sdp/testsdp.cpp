#include "sdp.h"

int main(int argc, char* argv[])
{
    if ( argc == 2 ) {
        SDP sdp( argv[1] );
        
        qDebug( "sdp:\n%s\n", argv[1] );
        qDebug( "result:\n"+sdp.toString() );
    } else 
        qDebug("usage:\n./testsdp <sdpdata>");
}
