#include <unistd.h>
#include <qimage.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <utime.h>
#include <stdlib.h>
#include <stdio.h>
#include <qfile.h>

int main( int argc, char* argv[] )
    {
    if( argc != 9 )
        return 1;
    QString theme = argv[ 1 ];
    QString file = argv[ 2 ];
    QString real_file = argv[ 3 ];
    int width = atoi( argv[ 4 ] );
    int height = atoi( argv[ 5 ] );
    int res_w = atoi( argv[ 6 ] );
    int res_h = atoi( argv[ 7 ] );
    time_t timestamp = atol( argv[ 8 ] );
    KComponentData k( "ksplashx_scale" );
    QString outfile = QString( "ksplashx/%1-%2x%3-%4" ).arg( theme ).arg( res_w ).arg( res_h ).arg( file );
    outfile = KStandardDirs::locateLocal( "cache", outfile );
    QImage img( real_file );
    if( img.isNull())
        return 2;
    QImage img2 = img.smoothScale( width, height );
    if( img2.save( outfile + ".tmp", "PNG" ))
        {
        rename( QFile::encodeName( outfile + ".tmp" ), QFile::encodeName( outfile ));
        utimbuf tm;
        tm.actime = tm.modtime = timestamp;
        utime( QFile::encodeName( outfile ), &tm );
        }
    else
        {
        unlink( QFile::encodeName( outfile ));
        unlink( QFile::encodeName( outfile + ".tmp" ));
        return 3;
        }
    return 0;
    }
