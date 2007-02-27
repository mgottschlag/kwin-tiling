#include <config.h>

#ifdef HAVE_XSHM
#define QT_MITSHM_CONVERSIONS
#endif

#include "qimage.h"
#include "qnamespace.h"
#include "qcolor.h"

#include <stdlib.h>
#include <string.h>

#include "pixmap.h"

#include "x11_defs.h"

#ifdef QT_MITSHM_CONVERSIONS
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

// Returns position of highest bit set or -1 if none
static int highest_bit( uint v )
{
    int i;
    uint b = (uint)1 << 31;
    for ( i=31; ((b & v) == 0) && i>=0;	 i-- )
	b >>= 1;
    return i;
}

// Returns position of lowest set bit in 'v' as an integer (0-31), or -1
static int lowest_bit( uint v )
{
    int i;
    ulong lb;
    lb = 1;
    for (i=0; ((v & lb) == 0) && i<32;  i++, lb<<=1);
    return i==32 ? -1 : i;
}

// Counts the number of bits set in 'v'
static uint n_bits( uint v )
{
    int i = 0;
    while ( v ) {
	v = v & (v - 1);
	i++;
    }
    return i;
}

inline static void qSafeXDestroyImage( XImage *x )
{
    if ( x->data ) {
	free( x->data );
	x->data = 0;
    }
    XDestroyImage( x );
}

#ifdef QT_MITSHM_CONVERSIONS

static bool qt_mitshm_error = false;
static int qt_mitshm_errorhandler( Display*, XErrorEvent* )
{
    qt_mitshm_error = true;
    return 0;
}

static XImage* qt_XShmCreateImage( Display* dpy, Visual* visual, unsigned int depth,
    int format, int /*offset*/, char* /*data*/, unsigned int width, unsigned int height,
    int /*bitmap_pad*/, int /*bytes_per_line*/, XShmSegmentInfo* shminfo )
{
    if( width * height * depth < 100*100*32 )
        return NULL;
    static int shm_inited = -1;
    if( shm_inited == -1 ) {
        if( XShmQueryExtension( dpy ))
            shm_inited = 1;
        else
            shm_inited = 0;
    }
    if( shm_inited == 0 )
        return NULL;
    XImage* xi = XShmCreateImage( dpy, visual, depth, format, NULL, shminfo, width,
        height );
    if( xi == NULL )
        return NULL;
    shminfo->shmid = shmget( IPC_PRIVATE, xi->bytes_per_line * xi->height,
        IPC_CREAT|0600);
    if( shminfo->shmid < 0 ) {
        XDestroyImage( xi );
        return NULL;
    }
    shminfo->readOnly = False;
    shminfo->shmaddr = (char*)shmat( shminfo->shmid, 0, 0 );
    if( shminfo->shmaddr == (char*)-1 ) {
        XDestroyImage( xi );
        shmctl( shminfo->shmid, IPC_RMID, 0 );
        return NULL;
    }
    xi->data = shminfo->shmaddr;
#ifndef QT_MITSHM_RMID_IGNORES_REFCOUNT
    // mark as deleted to automatically free the memory in case
    // of a crash (but this doesn't work e.g. on Solaris)
    shmctl( shminfo->shmid, IPC_RMID, 0 );
#endif
    if( shm_inited == 1 ) { // first time
        XErrorHandler old_h = XSetErrorHandler( qt_mitshm_errorhandler );
        XShmAttach( dpy, shminfo );
        shm_inited = 2;
        XSync( dpy, False );
        XSetErrorHandler( old_h );
        if( qt_mitshm_error ) { // oops ... perhaps we are remote?
            shm_inited = 0;
            XDestroyImage( xi );
            shmdt( shminfo->shmaddr );
#ifdef QT_MITSHM_RMID_IGNORES_REFCOUNT
            shmctl( shminfo->shmid, IPC_RMID, 0 );
#endif    
            return NULL;
        }
    } else
        XShmAttach( dpy, shminfo );
    return xi;
}

static void qt_XShmDestroyImage( XImage* xi, XShmSegmentInfo* shminfo )
{
    XShmDetach( QPaintDevice::x11AppDisplay(), shminfo );
    XDestroyImage( xi );
    shmdt( shminfo->shmaddr );
#ifdef QT_MITSHM_RMID_IGNORES_REFCOUNT
    shmctl( shminfo->shmid, IPC_RMID, 0 );
#endif    
}

#endif // QT_MITSHM_CONVERSIONS


PixmapData PP::convertFromImage( const QImage &img, int conversion_flags )
{
    if ( img.isNull() ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QPixmap::convertFromImage: Cannot convert a null image" );
#endif
	return PixmapData();
    }
//#    detach();					// detach other references
    QImage  image = img;
    const uint	 w   = image.width();
    const uint	 h   = image.height();
    int	 d   = image.depth();
    const int	 dd  = x11Depth();
    bool force_mono = (dd == 1 || //#*isQBitmap() ||
		       (conversion_flags & Qt::ColorMode_Mask)==MonoOnly );

#if 0
    // get rid of the mask
    delete data->mask;
    data->mask = 0;

    // get rid of alpha pixmap
    delete data->alphapm;
    data->alphapm = 0;
#endif

    // must be monochrome
    if ( force_mono ) {
	if ( d != 1 ) {
	    // dither
	    image = image.convertDepth( 1, conversion_flags );
	    d = 1;
	}
    } else {					// can be both
	bool conv8 = FALSE;
	if ( d > 8 && dd <= 8 ) {		// convert to 8 bit
	    if ( (conversion_flags & DitherMode_Mask) == AutoDither )
		conversion_flags = (conversion_flags & ~DitherMode_Mask)
				   | PreferDither;
	    conv8 = TRUE;
	} else if ( (conversion_flags & ColorMode_Mask) == ColorOnly ) {
	    conv8 = d == 1;			// native depth wanted
	} else if ( d == 1 ) {
	    if ( image.numColors() == 2 ) {
		QRgb c0 = image.color(0);	// Auto: convert to best
		QRgb c1 = image.color(1);
		conv8 = QMIN(c0,c1) != qRgb(0,0,0) || QMAX(c0,c1) != qRgb(255,255,255);
	    } else {
		// eg. 1-color monochrome images (they do exist).
		conv8 = TRUE;
	    }
	}
	if ( conv8 ) {
	    image = image.convertDepth( 8, conversion_flags );
	    d = 8;
	}
    }

    if ( d == 1 ) {				// 1 bit pixmap (bitmap)
        abort();
#if 0
	if ( hd ) {				// delete old X pixmap

#ifndef QT_NO_XFTFREETYPE
	    if (rendhd) {
		XftDrawDestroy( (XftDraw *) rendhd );
		rendhd = 0;
	    }
#endif // QT_NO_XFTFREETYPE

	    XFreePixmap( x11Display(), hd );
	}

	// make sure image.color(0) == color0 (white) and image.color(1) == color1 (black)
	if (image.color(0) == Qt::black.rgb() && image.color(1) == Qt::white.rgb()) {
	    image.invertPixels();
	    image.setColor(0, Qt::white.rgb());
	    image.setColor(1, Qt::black.rgb());
	}

	char  *bits;
	uchar *tmp_bits;
	int    bpl = (w+7)/8;
	int    ibpl = image.bytesPerLine();
	if ( image.bitOrder() == QImage::BigEndian || bpl != ibpl ) {
	    tmp_bits = new uchar[bpl*h];
	    Q_CHECK_PTR( tmp_bits );
	    bits = (char *)tmp_bits;
	    uchar *p, *b, *end;
	    uint y, count;
	    if ( image.bitOrder() == QImage::BigEndian ) {
		const uchar *f = qt_get_bitflip_array();
		b = tmp_bits;
		for ( y=0; y<h; y++ ) {
		    p = image.scanLine( y );
		    end = p + bpl;
		    count = bpl;
		    while ( count > 4 ) {
			*b++ = f[*p++];
			*b++ = f[*p++];
			*b++ = f[*p++];
			*b++ = f[*p++];
			count -= 4;
		    }
		    while ( p < end )
			*b++ = f[*p++];
		}
	    } else {				// just copy
		b = tmp_bits;
		p = image.scanLine( 0 );
		for ( y=0; y<h; y++ ) {
		    memcpy( b, p, bpl );
		    b += bpl;
		    p += ibpl;
		}
	    }
	} else {
	    bits = (char *)image.bits();
	    tmp_bits = 0;
	}
	hd = (HANDLE)XCreateBitmapFromData( x11Display(),
					    RootWindow(x11Display(), x11Screen() ),
					    bits, w, h );

#ifndef QT_NO_XFTFREETYPE
	if ( qt_has_xft )
	    rendhd = (HANDLE) XftDrawCreateBitmap( x11Display(), hd );
#endif // QT_NO_XFTFREETYPE

	if ( tmp_bits )				// Avoid purify complaint
	    delete [] tmp_bits;
	data->w = w;  data->h = h;  data->d = 1;

	if ( image.hasAlphaBuffer() ) {
	    QBitmap m;
	    m = image.createAlphaMask( conversion_flags );
	    setMask( m );
	}
	return TRUE;
#endif
    }

    Display *dpy   = x11Display();
    Visual *visual = (Visual *)x11Visual();
    XImage *xi	   = 0;
    bool    trucol = (visual->c_class == TrueColor);
    int	    nbytes = image.numBytes();
    uchar  *newbits= 0;
    int newbits_size = 0;
#ifdef QT_MITSHM_CONVERSIONS
    bool mitshm_ximage = false;
    XShmSegmentInfo shminfo;
#endif

    if ( trucol ) {				// truecolor display
	QRgb  pix[256];				// pixel translation table
	const bool  d8 = d == 8;
	const uint  red_mask	  = (uint)visual->red_mask;
	const uint  green_mask  = (uint)visual->green_mask;
	const uint  blue_mask	  = (uint)visual->blue_mask;
	const int   red_shift	  = highest_bit( red_mask )   - 7;
	const int   green_shift = highest_bit( green_mask ) - 7;
	const int   blue_shift  = highest_bit( blue_mask )  - 7;
	const uint  rbits = highest_bit(red_mask) - lowest_bit(red_mask) + 1;
	const uint  gbits = highest_bit(green_mask) - lowest_bit(green_mask) + 1;
	const uint  bbits = highest_bit(blue_mask) - lowest_bit(blue_mask) + 1;

	if ( d8 ) {				// setup pixel translation
	    QRgb *ctable = image.colorTable();
	    for ( int i=0; i<image.numColors(); i++ ) {
		int r = qRed  (ctable[i]);
		int g = qGreen(ctable[i]);
		int b = qBlue (ctable[i]);
		r = red_shift	> 0 ? r << red_shift   : r >> -red_shift;
		g = green_shift > 0 ? g << green_shift : g >> -green_shift;
		b = blue_shift	> 0 ? b << blue_shift  : b >> -blue_shift;
		pix[i] = (b & blue_mask) | (g & green_mask) | (r & red_mask)
			 | ~(blue_mask | green_mask | red_mask);
	    }
	}

#ifdef QT_MITSHM_CONVERSIONS
        xi = qt_XShmCreateImage( dpy, visual, dd, ZPixmap, 0, 0, w, h, 32, 0, &shminfo );
        if( xi != NULL ) {
            mitshm_ximage = true;
            newbits = (uchar*)xi->data;
        }
        else
#endif
	    xi = XCreateImage( dpy, visual, dd, ZPixmap, 0, 0, w, h, 32, 0 );
	Q_CHECK_PTR( xi );
        if( newbits == NULL )
    	    newbits = (uchar *)malloc( xi->bytes_per_line*h );
	Q_CHECK_PTR( newbits );
	if ( !newbits )				// no memory
	    return PixmapData();
	int    bppc = xi->bits_per_pixel;

	bool contig_bits = n_bits(red_mask) == rbits &&
                           n_bits(green_mask) == gbits &&
                           n_bits(blue_mask) == bbits;
	bool dither_tc =
	    // Want it?
	    (conversion_flags & Dither_Mask) != ThresholdDither &&
	    (conversion_flags & DitherMode_Mask) != AvoidDither &&
	    // Need it?
	    bppc < 24 && !d8 &&
	    // Can do it? (Contiguous bits?)
	    contig_bits;

	static bool init=FALSE;
	static int D[16][16];
	if ( dither_tc && !init ) {
	    // I also contributed this code to XV - WWA.
	    /*
	      The dither matrix, D, is obtained with this formula:

	      D2 = [ 0 2 ]
	      [ 3 1 ]


	      D2*n = [ 4*Dn       4*Dn+2*Un ]
	      [ 4*Dn+3*Un  4*Dn+1*Un ]
	    */
	    int n,i,j;
	    init=1;

	    /* Set D2 */
	    D[0][0]=0;
	    D[1][0]=2;
	    D[0][1]=3;
	    D[1][1]=1;

	    /* Expand using recursive definition given above */
	    for (n=2; n<16; n*=2) {
		for (i=0; i<n; i++) {
		    for (j=0; j<n; j++) {
			D[i][j]*=4;
			D[i+n][j]=D[i][j]+2;
			D[i][j+n]=D[i][j]+3;
			D[i+n][j+n]=D[i][j]+1;
		    }
		}
	    }
	    init=TRUE;
	}
        
        enum { BPP8, 
               BPP16_8_3_M3, BPP16_7_2_M3, BPP16_MSB, BPP16_LSB,
               BPP24_MSB, BPP24_LSB,
               BPP32_16_8_0, BPP32_MSB, BPP32_LSB
        } mode = BPP8;

	if ( bppc > 8 && xi->byte_order == LSBFirst )
	    bppc++;

        int wordsize;
        bool bigendian;
        qSysInfo( &wordsize, &bigendian );
        bool same_msb_lsb = ( xi->byte_order == MSBFirst ) == ( bigendian );
        
        if( bppc == 8 ) // 8 bit
            mode = BPP8;
        else if( bppc == 16 || bppc == 17 ) { // 16 bit MSB/LSB
            if( red_shift == 8 && green_shift == 3 && blue_shift == -3
                && !d8 && same_msb_lsb )
                mode = BPP16_8_3_M3;
            else if( red_shift == 7 && green_shift == 2 && blue_shift == -3
                && !d8 && same_msb_lsb )
                mode = BPP16_7_2_M3;
            else
                mode = bppc == 17 ? BPP16_LSB : BPP16_MSB;
        } else if( bppc == 24 || bppc == 25 ) { // 24 bit MSB/LSB
            mode = bppc == 25 ? BPP24_LSB : BPP24_MSB;
        } else if( bppc == 32 || bppc == 33 ) { // 32 bit MSB/LSB
            if( red_shift == 16 && green_shift == 8 && blue_shift == 0
                && !d8 && same_msb_lsb )
                mode = BPP32_16_8_0;
            else
                mode = bppc == 33 ? BPP32_LSB : BPP32_MSB;
        } else
	    qFatal("Logic error 3");

#define GET_PIXEL \
                int pixel; \
		if ( d8 ) pixel = pix[*src++]; \
		else { \
		    int r = qRed  ( *p ); \
		    int g = qGreen( *p ); \
		    int b = qBlue ( *p++ ); \
		    r = red_shift   > 0 \
		        ? r << red_shift   : r >> -red_shift; \
		    g = green_shift > 0 \
		        ? g << green_shift : g >> -green_shift; \
		    b = blue_shift  > 0 \
		        ? b << blue_shift  : b >> -blue_shift; \
		    pixel = (r & red_mask)|(g & green_mask) | (b & blue_mask) \
			    | ~(blue_mask | green_mask | red_mask); \
		}

// optimized case - no d8 case, shift only once instead of twice, mask only once instead of twice,
// use direct values instead of variables, and use only one statement
// (*p >> 16), (*p >> 8 ) and (*p) are qRed(),qGreen() and qBlue() without masking
// shifts have to be passed including the shift operator (e.g. '>>3'), because of the direction
#define GET_PIXEL_OPT(red_shift,green_shift,blue_shift,red_mask,green_mask,blue_mask) \
                int pixel = ((( *p >> 16 ) red_shift ) & red_mask ) \
                    | ((( *p >> 8 ) green_shift ) & green_mask ) \
                    | ((( *p ) blue_shift ) & blue_mask ); \
                ++p;

#define GET_PIXEL_DITHER_TC \
		int r = qRed  ( *p ); \
		int g = qGreen( *p ); \
		int b = qBlue ( *p++ ); \
		const int thres = D[x%16][y%16]; \
		if ( r <= (255-(1<<(8-rbits))) && ((r<<rbits) & 255) \
			> thres) \
		    r += (1<<(8-rbits)); \
		if ( g <= (255-(1<<(8-gbits))) && ((g<<gbits) & 255) \
			> thres) \
		    g += (1<<(8-gbits)); \
		if ( b <= (255-(1<<(8-bbits))) && ((b<<bbits) & 255) \
			> thres) \
		    b += (1<<(8-bbits)); \
		r = red_shift   > 0 \
		    ? r << red_shift   : r >> -red_shift; \
		g = green_shift > 0 \
		    ? g << green_shift : g >> -green_shift; \
		b = blue_shift  > 0 \
		    ? b << blue_shift  : b >> -blue_shift; \
		int pixel = (r & red_mask)|(g & green_mask) | (b & blue_mask);

// again, optimized case
// can't be optimized that much :(
#define GET_PIXEL_DITHER_TC_OPT(red_shift,green_shift,blue_shift,red_mask,green_mask,blue_mask, \
                                rbits,gbits,bbits) \
		const int thres = D[x%16][y%16]; \
		int r = qRed  ( *p ); \
		if ( r <= (255-(1<<(8-rbits))) && ((r<<rbits) & 255) \
			> thres) \
		    r += (1<<(8-rbits)); \
		int g = qGreen( *p ); \
		if ( g <= (255-(1<<(8-gbits))) && ((g<<gbits) & 255) \
			> thres) \
		    g += (1<<(8-gbits)); \
		int b = qBlue ( *p++ ); \
		if ( b <= (255-(1<<(8-bbits))) && ((b<<bbits) & 255) \
			> thres) \
		    b += (1<<(8-bbits)); \
                int pixel = (( r red_shift ) & red_mask ) \
                    | (( g green_shift ) & green_mask ) \
                    | (( b blue_shift ) & blue_mask );

#define CYCLE(body) \
	for ( uint y=0; y<h; y++ ) { \
	    uchar* src = image.scanLine( y ); \
	    uchar* dst = newbits + xi->bytes_per_line*y; \
	    QRgb* p = (QRgb *)src; \
            body \
        }

        if ( dither_tc ) {
	    switch ( mode ) {
                case BPP16_8_3_M3:
                    CYCLE(
                        Q_INT16* dst16 = (Q_INT16*)dst;
		        for ( uint x=0; x<w; x++ ) {
			    GET_PIXEL_DITHER_TC_OPT(<<8,<<3,>>3,0xf800,0x7e0,0x1f,5,6,5)
                            *dst16++ = pixel;
		        }
                    )
		    break;
                case BPP16_7_2_M3:
                    CYCLE(
                        Q_INT16* dst16 = (Q_INT16*)dst;
		        for ( uint x=0; x<w; x++ ) {
			    GET_PIXEL_DITHER_TC_OPT(<<7,<<2,>>3,0x7c00,0x3e0,0x1f,5,5,5)
                            *dst16++ = pixel;
		        }
                    )
		    break;
		case BPP16_MSB:			// 16 bit MSB
                    CYCLE(
		        for ( uint x=0; x<w; x++ ) {
			    GET_PIXEL_DITHER_TC
			    *dst++ = (pixel >> 8);
			    *dst++ = pixel;
		        }
                    )
		    break;
		case BPP16_LSB:			// 16 bit LSB
                    CYCLE(
    		        for ( uint x=0; x<w; x++ ) {
			    GET_PIXEL_DITHER_TC
			    *dst++ = pixel;
			    *dst++ = pixel >> 8;
		        }
                    )
		    break;
		default:
		    qFatal("Logic error");
		}
	} else {
	    switch ( mode ) {
		case BPP8:			// 8 bit
                    CYCLE(
                    Q_UNUSED(p);
		        for ( uint x=0; x<w; x++ ) {
			    int pixel = pix[*src++];
			    *dst++ = pixel;
		        }
                    )
		    break;
                case BPP16_8_3_M3:
                    CYCLE(
                        Q_INT16* dst16 = (Q_INT16*)dst;
		        for ( uint x=0; x<w; x++ ) {
			    GET_PIXEL_OPT(<<8,<<3,>>3,0xf800,0x7e0,0x1f)
                            *dst16++ = pixel;
		        }
                    )
		    break;
                case BPP16_7_2_M3:
                    CYCLE(
                        Q_INT16* dst16 = (Q_INT16*)dst;
		        for ( uint x=0; x<w; x++ ) {
			    GET_PIXEL_OPT(<<7,<<2,>>3,0x7c00,0x3e0,0x1f)
                            *dst16++ = pixel;
		        }
                    )
		    break;
		case BPP16_MSB:			// 16 bit MSB
                    CYCLE(
		        for ( uint x=0; x<w; x++ ) {
			    GET_PIXEL
			    *dst++ = (pixel >> 8);
			    *dst++ = pixel;
		        }
                    )
		    break;
		case BPP16_LSB:			// 16 bit LSB
                    CYCLE(
		        for ( uint x=0; x<w; x++ ) {
			    GET_PIXEL
			    *dst++ = pixel;
			    *dst++ = pixel >> 8;
		        }
                    )
		    break;
		case BPP24_MSB:			// 24 bit MSB
                    CYCLE(
		        for ( uint x=0; x<w; x++ ) {
			    GET_PIXEL
			    *dst++ = pixel >> 16;
			    *dst++ = pixel >> 8;
			    *dst++ = pixel;
		        }
                    )
		    break;
		case BPP24_LSB:			// 24 bit LSB
                    CYCLE(
		        for ( uint x=0; x<w; x++ ) {
			    GET_PIXEL
			    *dst++ = pixel;
			    *dst++ = pixel >> 8;
			    *dst++ = pixel >> 16;
		        }
                    )
		    break;
                case BPP32_16_8_0:
                    CYCLE(
                        memcpy( dst, p, w * 4 );
                    )
                    break;
		case BPP32_MSB:			// 32 bit MSB
                    CYCLE(
		        for ( uint x=0; x<w; x++ ) {
			    GET_PIXEL
			    *dst++ = pixel >> 24;
			    *dst++ = pixel >> 16;
			    *dst++ = pixel >> 8;
			    *dst++ = pixel;
		        }
                    )
		    break;
		case BPP32_LSB:			// 32 bit LSB
                    CYCLE(
		        for ( uint x=0; x<w; x++ ) {
			    GET_PIXEL
			    *dst++ = pixel;
			    *dst++ = pixel >> 8;
			    *dst++ = pixel >> 16;
			    *dst++ = pixel >> 24;
		        }
                    )
  		    break;
  		default:
  		    qFatal("Logic error 2");
  	    }
  	}
  	xi->data = (char *)newbits;
    }

    if ( d == 8 && !trucol ) {			// 8 bit pixmap
	int  pop[256];				// pixel popularity

	if ( image.numColors() == 0 )
	    image.setNumColors( 1 );

	memset( pop, 0, sizeof(int)*256 );	// reset popularity array
	uint i;
	for ( i=0; i<h; i++ ) {			// for each scanline...
	    uchar* p = image.scanLine( i );
	    uchar *end = p + w;
	    while ( p < end )			// compute popularity
		pop[*p++]++;
	}

	newbits = (uchar *)malloc( nbytes );	// copy image into newbits
        newbits_size = nbytes;
	Q_CHECK_PTR( newbits );
	if ( !newbits )				// no memory
	    return PixmapData();
	uchar* p = newbits;
	memcpy( p, image.bits(), nbytes );	// copy image data into newbits

	/*
	 * The code below picks the most important colors. It is based on the
	 * diversity algorithm, implemented in XV 3.10. XV is (C) by John Bradley.
	 */

	struct PIX {				// pixel sort element
	    uchar r,g,b,n;			// color + pad
	    int	  use;				// popularity
	    int	  index;			// index in colormap
	    int	  mindist;
	};
	int ncols = 0;
	for ( i=0; i< (uint) image.numColors(); i++ ) { // compute number of colors
	    if ( pop[i] > 0 )
		ncols++;
	}
	for ( i=image.numColors(); i<256; i++ ) // ignore out-of-range pixels
	    pop[i] = 0;

	// works since we make sure above to have at least
	// one color in the image
	if ( ncols == 0 )
	    ncols = 1;

	PIX pixarr[256];			// pixel array
	PIX pixarr_sorted[256];			// pixel array (sorted)
	memset( pixarr, 0, ncols*sizeof(PIX) );
	PIX *px		   = &pixarr[0];
	int  maxpop = 0;
	int  maxpix = 0;
	Q_CHECK_PTR( pixarr );
	uint j = 0;
	QRgb* ctable = image.colorTable();
	for ( i=0; i<256; i++ ) {		// init pixel array
	    if ( pop[i] > 0 ) {
		px->r = qRed  ( ctable[i] );
		px->g = qGreen( ctable[i] );
		px->b = qBlue ( ctable[i] );
		px->n = 0;
		px->use = pop[i];
		if ( pop[i] > maxpop ) {	// select most popular entry
		    maxpop = pop[i];
		    maxpix = j;
		}
		px->index = i;
		px->mindist = 1000000;
		px++;
		j++;
	    }
	}
	pixarr_sorted[0] = pixarr[maxpix];
	pixarr[maxpix].use = 0;

	for ( i=1; i< (uint) ncols; i++ ) {		// sort pixels
	    int minpix = -1, mindist = -1;
	    px = &pixarr_sorted[i-1];
	    int r = px->r;
	    int g = px->g;
	    int b = px->b;
	    int dist;
	    if ( (i & 1) || i<10 ) {		// sort on max distance
		for ( int j=0; j<ncols; j++ ) {
		    px = &pixarr[j];
		    if ( px->use ) {
			dist = (px->r - r)*(px->r - r) +
			       (px->g - g)*(px->g - g) +
			       (px->b - b)*(px->b - b);
			if ( px->mindist > dist )
			    px->mindist = dist;
			if ( px->mindist > mindist ) {
			    mindist = px->mindist;
			    minpix = j;
			}
		    }
		}
	    } else {				// sort on max popularity
		for ( int j=0; j<ncols; j++ ) {
		    px = &pixarr[j];
		    if ( px->use ) {
			dist = (px->r - r)*(px->r - r) +
			       (px->g - g)*(px->g - g) +
			       (px->b - b)*(px->b - b);
			if ( px->mindist > dist )
			    px->mindist = dist;
			if ( px->use > mindist ) {
			    mindist = px->use;
			    minpix = j;
			}
		    }
		}
	    }
	    pixarr_sorted[i] = pixarr[minpix];
	    pixarr[minpix].use = 0;
	}

	uint pix[256];				// pixel translation table
	px = &pixarr_sorted[0];
	for ( i=0; i< (uint) ncols; i++ ) {		// allocate colors
	    QColor c( px->r, px->g, px->b );
	    pix[px->index] = c.pixel(x11Screen());
	    px++;
	}

	p = newbits;
	for ( i=0; i< (uint) nbytes; i++ ) {		// translate pixels
	    *p = pix[*p];
	    p++;
	}
    }

    if ( !xi ) {				// X image not created
#ifdef QT_MITSHM_CONVERSIONS
        xi = qt_XShmCreateImage( dpy, visual, dd, ZPixmap, 0, 0, w, h, 32, 0, &shminfo );
        if( xi != NULL )
            mitshm_ximage = true;
        else
#endif
	    xi = XCreateImage( dpy, visual, dd, ZPixmap, 0, 0, w, h, 32, 0 );
	if ( xi->bits_per_pixel == 16 ) {	// convert 8 bpp ==> 16 bpp
	    ushort *p2;
	    int	    p2inc = xi->bytes_per_line/sizeof(ushort);
	    ushort *newerbits = (ushort *)malloc( xi->bytes_per_line * h );
            newbits_size = xi->bytes_per_line * h;
	    Q_CHECK_PTR( newerbits );
	    if ( !newerbits )				// no memory
		return PixmapData();
	    uchar* p = newbits;
	    for ( uint y=0; y<h; y++ ) {		// OOPS: Do right byte order!!
		p2 = newerbits + p2inc*y;
		for ( uint x=0; x<w; x++ )
		    *p2++ = *p++;
	    }
	    free( newbits );
	    newbits = (uchar *)newerbits;
	} else if ( xi->bits_per_pixel != 8 ) {
#if defined(QT_CHECK_RANGE)
	    qWarning( "QPixmap::convertFromImage: Display not supported "
		      "(bpp=%d)", xi->bits_per_pixel );
#endif
	}
#ifdef QT_MITSHM_CONVERSIONS
        if( newbits_size > 0 && mitshm_ximage ) { // need to copy to shared memory
            memcpy( xi->data, newbits, newbits_size );
            free( newbits );
            newbits = (uchar*)xi->data;
        }
        else
#endif
            xi->data = (char *)newbits;
    }

#if 0
    if ( hd && (width() != (int)w || height() != (int)h || this->depth() != dd) ) {

#ifndef QT_NO_XFTFREETYPE
	if (rendhd) {
	    XftDrawDestroy( (XftDraw *) rendhd );
	    rendhd = 0;
	}
#endif // QT_NO_XFTFREETYPE

	XFreePixmap( dpy, hd );			// don't reuse old pixmap
	hd = 0;
    }
#endif
    PixmapData dat;
    PixmapData* data = &dat;
    data->hd = None;
    Pixmap& hd = data->hd;

    if ( !hd ) {					// create new pixmap
	hd = (HANDLE)XCreatePixmap( x11Display(),
				    RootWindow(x11Display(), x11Screen() ),
				    w, h, dd );

#ifndef QT_NO_XFTFREETYPE
	if ( qt_has_xft ) {
	    if ( data->d == 1 ) {
		rendhd = (HANDLE) XftDrawCreateBitmap( x11Display (), hd );
	    } else {
		rendhd = (HANDLE) XftDrawCreate( x11Display (), hd,
						 (Visual *) x11Visual(), x11Colormap() );
	    }
	}
#endif // QT_NO_XFTFREETYPE

    }

#ifdef QT_MITSHM_CONVERSIONS
    if( mitshm_ximage )
        XShmPutImage( dpy, hd, qt_xget_readonly_gc( x11Screen(), FALSE ),
                      xi, 0, 0, 0, 0, w, h, False );
    else
#endif
        XPutImage( dpy, hd, qt_xget_readonly_gc( x11Screen(), FALSE  ),
                   xi, 0, 0, 0, 0, w, h );

    data->w = w;
    data->h = h;
    data->d = dd;

    XImage* axi = NULL;
#ifdef QT_MITSHM_CONVERSIONS
    bool mitshm_aximage = false;
    XShmSegmentInfo ashminfo;
#endif
    if ( image.hasAlphaBuffer() ) {
        abort();
#if 0
	QBitmap m;
	m = image.createAlphaMask( conversion_flags );
	setMask( m );

#ifndef QT_NO_XFTFREETYPE
	// does this image have an alphamap (and not just a 1bpp mask)?
	bool alphamap = image.depth() == 32;
	if (image.depth() == 8) {
	    const QRgb * const rgb = image.colorTable();
	    for (int i = 0, count = image.numColors(); i < count; ++i) {
		const int alpha = qAlpha(rgb[i]);
		if (alpha != 0 && alpha != 0xff) {
		    alphamap = true;
		    break;
		}
	    }
	}

	if (qt_use_xrender && qt_has_xft && alphamap) {
	    data->alphapm = new QPixmap; // create a null pixmap

	    // setup pixmap data
	    data->alphapm->data->w = w;
	    data->alphapm->data->h = h;
	    data->alphapm->data->d = 8;

	    // create 8bpp pixmap and render picture
	    data->alphapm->hd =
		XCreatePixmap(x11Display(), RootWindow(x11Display(), x11Screen()),
			      w, h, 8);

	    data->alphapm->rendhd =
		(HANDLE) XftDrawCreateAlpha( x11Display(), data->alphapm->hd, 8 );

#ifdef QT_MITSHM_CONVERSIONS
            axi = qt_XShmCreateImage( x11Display(), (Visual*)x11Visual(),
                                      8, ZPixmap, 0, 0, w, h, 8, 0, &ashminfo );
            if( axi != NULL )
                mitshm_aximage = true;
            else
#endif
	        axi = XCreateImage(x11Display(), (Visual *) x11Visual(),
				   8, ZPixmap, 0, 0, w, h, 8, 0);

	    if (axi) {
                if( axi->data==NULL ) {
		    // the data is deleted by qSafeXDestroyImage
		    axi->data = (char *) malloc(h * axi->bytes_per_line);
		    Q_CHECK_PTR( axi->data );
                }
		char *aptr = axi->data;

		if (image.depth() == 32) {
		    const int *iptr = (const int *) image.bits();
                    if( axi->bytes_per_line == (int)w ) {
		        int max = w * h;
		        while (max--)
			    *aptr++ = *iptr++ >> 24; // squirt
                    } else {
                        for (uint i = 0; i < h; ++i ) {
                            for (uint j = 0; j < w; ++j )
                                *aptr++ = *iptr++ >> 24; // squirt
                            aptr += ( axi->bytes_per_line - w );
                        }
                    }
		} else if (image.depth() == 8) {
		    const QRgb * const rgb = image.colorTable();
		    for (uint y = 0; y < h; ++y) {
			const uchar *iptr = image.scanLine(y);
			for (uint x = 0; x < w; ++x)
			    *aptr++ = qAlpha(rgb[*iptr++]);
                        aptr += ( axi->bytes_per_line - w );
		    }
		}

		GC gc = XCreateGC(x11Display(), data->alphapm->hd, 0, 0);
#ifdef QT_MITSHM_CONVERSIONS
                if( mitshm_aximage )
                    XShmPutImage( dpy, data->alphapm->hd, gc, axi, 0, 0, 0, 0, w, h, False );
                else
#endif
		    XPutImage(dpy, data->alphapm->hd, gc, axi, 0, 0, 0, 0, w, h);
		XFreeGC(x11Display(), gc);
	    }
	}
#endif // QT_NO_XFTFREETYPE
#endif
    }

#ifdef QT_MITSHM_CONVERSIONS
    if( mitshm_ximage || mitshm_aximage )
        XSync( x11Display(), False ); // wait until processed
#endif

    if ( data->optim != BestOptim ) {		// throw away image
#ifdef QT_MITSHM_CONVERSIONS
        if( mitshm_ximage )
            qt_XShmDestroyImage( xi, &shminfo );
        else
#endif
	qSafeXDestroyImage( xi );
	data->ximage = 0;
    } else {					// keep ximage that we created
#ifdef QT_MITSHM_CONVERSIONS
        if( mitshm_ximage ) { // copy the XImage?
            qt_XShmDestroyImage( xi, &shminfo );
            xi = 0;
        }
#endif
	data->ximage = xi;
    }
    if( axi ) {
#ifdef QT_MITSHM_CONVERSIONS
        if( mitshm_aximage )
            qt_XShmDestroyImage( axi, &ashminfo );
        else
#endif
        qSafeXDestroyImage(axi);
    }
    return *data;
}
