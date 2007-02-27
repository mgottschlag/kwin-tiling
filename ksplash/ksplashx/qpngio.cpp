/****************************************************************************
** 
**
** Implementation of PNG QImage IOHandler
**
** Created : 970521
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QT_NO_IMAGEIO_PNG

#include "qimage.h"
#include "qcolor.h"

#include <png.h>


#ifdef Q_OS_TEMP
#define CALLBACK_CALL_TYPE	__cdecl
#else
#define CALLBACK_CALL_TYPE
#endif


/*
  All PNG files load to the minimal QImage equivalent.

  All QImage formats output to reasonably efficient PNG equivalents.
  Never to grayscale.
*/

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static
void CALLBACK_CALL_TYPE iod_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    FILE* in = (FILE*)png_get_io_ptr(png_ptr);

    while (length) {
	int nr = fread( (char*)data, 1, length, in );
	if (nr <= 0) {
	    png_error(png_ptr, "Read Error");
	    return;
	}
	length -= nr;
    }
}


#if defined(Q_C_CALLBACKS)
}
#endif

static
void setup_qt( QImage& image, png_structp png_ptr, png_infop info_ptr, float screen_gamma=0.0 )
{
    if ( 0.0 == screen_gamma )
	// PNG docs say this is a good guess for a PC monitor
        // in a dark room
	screen_gamma = 2.2;
    if ( png_get_valid(png_ptr, info_ptr, PNG_INFO_gAMA) ) {
	// the file has a gAMA attribute
	double file_gamma;
	if ( png_get_gAMA(png_ptr, info_ptr, &file_gamma))
	    png_set_gamma( png_ptr, screen_gamma, file_gamma );
    } else {
	// no file gamma, use a reasonable default
	png_set_gamma( png_ptr, screen_gamma, 0.45455 ); 
    }

    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
	0, 0, 0);

    if ( color_type == PNG_COLOR_TYPE_GRAY ) {
	// Black & White or 8-bit grayscale
	if ( bit_depth == 1 && info_ptr->channels == 1 ) {
	    png_set_invert_mono( png_ptr );
	    png_read_update_info( png_ptr, info_ptr );
	    if (!image.create( width, height, 1, 2, QImage::BigEndian ))
		return;
	    image.setColor( 1, qRgb(0,0,0) );
	    image.setColor( 0, qRgb(255,255,255) );
	} else if (bit_depth == 16 && png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
	    png_set_expand(png_ptr);
	    png_set_strip_16(png_ptr);
	    png_set_gray_to_rgb(png_ptr);

	    if (!image.create(width, height, 32))
		return;
	    image.setAlphaBuffer(TRUE);

	    if (QImage::systemByteOrder() == QImage::BigEndian)
		png_set_swap_alpha(png_ptr);

	    png_read_update_info(png_ptr, info_ptr);
	} else {
	    if ( bit_depth == 16 )
		png_set_strip_16(png_ptr);
	    else if ( bit_depth < 8 )
		png_set_packing(png_ptr);
	    int ncols = bit_depth < 8 ? 1 << bit_depth : 256;
	    png_read_update_info(png_ptr, info_ptr);
	    if (!image.create(width, height, 8, ncols))
		return;
	    for (int i=0; i<ncols; i++) {
		int c = i*255/(ncols-1);
		image.setColor( i, qRgba(c,c,c,0xff) );
	    }
	    if ( png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) ) {
		const int g = info_ptr->trans_values.gray;
		if (g < ncols) {
		    image.setAlphaBuffer(TRUE);
		    image.setColor(g, image.color(g) & RGB_MASK);
		}
	    }
	}
    } else if ( color_type == PNG_COLOR_TYPE_PALETTE
     && png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE)
     && info_ptr->num_palette <= 256 )
    {
	// 1-bit and 8-bit color
	if ( bit_depth != 1 )
	    png_set_packing( png_ptr );
	png_read_update_info( png_ptr, info_ptr );
	png_get_IHDR(png_ptr, info_ptr,
	    &width, &height, &bit_depth, &color_type, 0, 0, 0);
	if (!image.create(width, height, bit_depth, info_ptr->num_palette,
	    QImage::BigEndian))
	    return;
	int i = 0;
	if ( png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) ) {
	    image.setAlphaBuffer( TRUE );
	    while ( i < info_ptr->num_trans ) {
		image.setColor(i, qRgba(
		    info_ptr->palette[i].red,
		    info_ptr->palette[i].green,
		    info_ptr->palette[i].blue,
		    info_ptr->trans[i]
		    )
		);
		i++;
	    }
	}
	while ( i < info_ptr->num_palette ) {
	    image.setColor(i, qRgba(
		info_ptr->palette[i].red,
		info_ptr->palette[i].green,
		info_ptr->palette[i].blue,
		0xff
		)
	    );
	    i++;
	}
    } else {
	// 32-bit
	if ( bit_depth == 16 )
	    png_set_strip_16(png_ptr);

	png_set_expand(png_ptr);

	if ( color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
	    png_set_gray_to_rgb(png_ptr);

	if (!image.create(width, height, 32))
	    return;

	// Only add filler if no alpha, or we can get 5 channel data.
	if (!(color_type & PNG_COLOR_MASK_ALPHA)
	   && !png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
	    png_set_filler(png_ptr, 0xff,
		QImage::systemByteOrder() == QImage::BigEndian ?
		    PNG_FILLER_BEFORE : PNG_FILLER_AFTER);
	    // We want 4 bytes, but it isn't an alpha channel
	} else {
	    image.setAlphaBuffer(TRUE);
	}

	if ( QImage::systemByteOrder() == QImage::BigEndian ) {
	    png_set_swap_alpha(png_ptr);
	}

	png_read_update_info(png_ptr, info_ptr);
    }

    // Qt==ARGB==Big(ARGB)==Little(BGRA)
    if ( QImage::systemByteOrder() == QImage::LittleEndian ) {
	png_set_bgr(png_ptr);
    }
}


#if defined(Q_C_CALLBACKS)
extern "C" {
#endif
static void CALLBACK_CALL_TYPE qt_png_warning(png_structp /*png_ptr*/, png_const_charp message)
{
    qWarning("libpng warning: %s", message);
}

#if defined(Q_C_CALLBACKS)
}
#endif


QImage splash_read_png_image(FILE* f)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;
    png_bytep* row_pointers;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
    if (!png_ptr) {
	return QImage();
    }

    png_set_error_fn(png_ptr, 0, 0, qt_png_warning);

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
	png_destroy_read_struct(&png_ptr, 0, 0);
	return QImage();
    }

    end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	return QImage();
    }

    if (setjmp(png_ptr->jmpbuf)) {
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	return QImage();
    }

    png_set_read_fn(png_ptr, (void*)f, iod_read_fn);
    png_read_info(png_ptr, info_ptr);

    QImage image;
    setup_qt(image, png_ptr, info_ptr, 0);
    if (image.isNull()) {
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	return QImage();
    }

    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
	0, 0, 0);

    uchar** jt = image.jumpTable();
    row_pointers=new png_bytep[height];

    for (uint y=0; y<height; y++) {
	row_pointers[y]=jt[y];
    }

    png_read_image(png_ptr, row_pointers);

#if 0 // libpng takes care of this.
png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)
    if (image.depth()==32 && png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
	QRgb trans = 0xFF000000 | qRgb(
	      (info_ptr->trans_values.red << 8 >> bit_depth)&0xff,
	      (info_ptr->trans_values.green << 8 >> bit_depth)&0xff,
	      (info_ptr->trans_values.blue << 8 >> bit_depth)&0xff);
	for (uint y=0; y<height; y++) {
	    for (uint x=0; x<info_ptr->width; x++) {
		if (((uint**)jt)[y][x] == trans) {
		    ((uint**)jt)[y][x] &= 0x00FFFFFF;
		} else {
		}
	    }
	}
    }
#endif

    image.setDotsPerMeterX(png_get_x_pixels_per_meter(png_ptr,info_ptr));
    image.setDotsPerMeterY(png_get_y_pixels_per_meter(png_ptr,info_ptr));

#ifndef QT_NO_IMAGE_TEXT
    png_textp text_ptr;
    int num_text=0;
    png_get_text(png_ptr,info_ptr,&text_ptr,&num_text);
    while (num_text--) {
	image.setText(text_ptr->key,0,text_ptr->text);
	text_ptr++;
    }
#endif

    delete [] row_pointers;

    if ( image.hasAlphaBuffer() ) {
	// Many PNG files lie (eg. from PhotoShop). Fortunately this loop will
	// usually be quick to find those that tell the truth.
	QRgb* c;
	int n;
	if (image.depth()==32) {
	    c = (QRgb*)image.bits();
	    n = image.bytesPerLine() * image.height() / 4;
	} else {
	    c = image.colorTable();
	    n = image.numColors();
	}
	while ( n-- && qAlpha(*c++)==0xff )
	    ;
	if ( n<0 ) // LIAR!
	    image.setAlphaBuffer(FALSE);
    }

    png_read_end(png_ptr, end_info);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    return image;
}
#endif // QT_NO_IMAGEIO_PNG
