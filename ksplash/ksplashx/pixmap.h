#ifndef _PIXMAP_H
#define _PIXMAP_H

#include "qnamespace.h"

#include "x11_defs.h"

class QImage;

enum Optimization { DefaultOptim, NoOptim, MemoryOptim=NoOptim,
			NormalOptim, BestOptim };

struct PixmapData
    {
    PixmapData() : hd( None ), optim( NoOptim ), ximage( NULL ) {}
    Pixmap hd;
    int w;
    int h;
    int d;
    Optimization optim;
    XImage* ximage;
    };

class PP : public Qt // inherit from Qt to reduce needed code changes in the function
    {
    public:
        static PixmapData convertFromImage( const QImage& img, int conversion_flags );
    };

inline
PixmapData imageToPixmap( const QImage& im )
    {
    return PP::convertFromImage( im, 0 );
    }

QImage splash_read_png_image(FILE* f);

#endif
