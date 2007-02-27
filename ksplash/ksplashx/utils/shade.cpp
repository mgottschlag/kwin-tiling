// This tool makes the image grayscale and transparent.

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <qimage.h>

int main( int argc, char* argv[] )
    {
    if( argc != 2 )
        return 1;
    QImage im( argv[ 1 ] );
    if( im.isNull())
        return 2;
    for( int x = 0;
         x < im.width();
         ++x )
        for( int y = 0;
             y < im.height();
             ++y )
            {
            QRgb c = im.pixel( x, y );
            QRgb c2 = qRgba( qGray( c ), qGray( c ), qGray( c ), qAlpha( c ) / 2 );
            im.setPixel( x, y, c2 );
            }
    im.save( "result.png", "PNG" );
    }
