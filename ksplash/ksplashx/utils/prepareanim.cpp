// This tool creates png image with animation (see README)

#include <qfile.h>
#include <qimage.h>
#include <stdlib.h>
#include <kdebug.h>
#include <stdio.h>

const int ANIM_IMAGES_ROW = 10; // must match splash

int main( int argc, char* argv[] )
    {
    // <list of images in order>
    if( argc < 2 )
        return 1;
    const int ARGC_DIFF = 1;
    int frames = argc - ARGC_DIFF;
    QImage result;
    for( int frame = 0;
         frame < frames;
         ++frame )
        {
        QImage fr( argv[ ARGC_DIFF + frame ] );
        if( fr.isNull())
            return 2;
        int w = fr.width();
        int h = fr.height();
        if( result.isNull())
            { // initialize
            if( frames < ANIM_IMAGES_ROW )
                result = QImage( frames * w, h, 32 );
            else
                result = QImage( ANIM_IMAGES_ROW * w, ( frames + ANIM_IMAGES_ROW - 1 ) / ANIM_IMAGES_ROW * h, 32 );
            result.setAlphaBuffer( true );
            }
        int basex = ( frame % ANIM_IMAGES_ROW ) * w;
        int basey = frame / ANIM_IMAGES_ROW * h;
        for( int y = 0;
             y < h;
             ++y )
            for( int x = 0;
                 x < w;
                 ++x )
                result.setPixel( basex + x, basey + y, fr.pixel( x, y ));
        }
    result.save( "result.png", "PNG" );
    }
