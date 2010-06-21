/****************************************************************************
**
** This file is based on sources of the Qt GUI Toolkit, used under the terms
** of the GNU General Public License version 2 (see the original copyright
** notice below).
** All further contributions to this file are (and are required to be)
** licensed under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** The original Qt license header follows:
** 
**
** Implementation of JPEG QImage IOHandler
**
** Created : 990521
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be used under the terms of the GNU General
** Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the files LICENSE.GPL2
** and LICENSE.GPL3 included in the packaging of this file.
** Alternatively you may (at your option) use any later version
** of the GNU General Public License if such license has been
** publicly approved by Trolltech ASA (or its successors, if any)
** and the KDE Free Qt Foundation.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/.
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** This file may be used under the terms of the Q Public License as
** defined by Trolltech ASA and appearing in the file LICENSE.QPL
** included in the packaging of this file.  Licensees holding valid Qt
** Commercial licenses may use this file in accordance with the Qt
** Commercial License Agreement provided with the Software.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not granted
** herein.
**
**********************************************************************/

#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include "qimage.h"
#include "qcolor.h"

#ifndef QT_NO_IMAGEIO_JPEG

#include <stdio.h>      // jpeglib needs this to be pre-included
#include <setjmp.h>


// including jpeglib.h seems to be a little messy
extern "C" {
#define XMD_H           // shut JPEGlib up
#if defined(Q_OS_UNIXWARE)
#  define HAVE_BOOLEAN  // libjpeg under Unixware seems to need this
#endif
#include <jpeglib.h>
#ifdef const
#  undef const          // remove crazy C hackery in jconfig.h
#endif
}


struct my_error_mgr : public jpeg_error_mgr {
    jmp_buf setjmp_buffer;
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static
void my_error_exit (j_common_ptr cinfo)
{
    my_error_mgr* myerr = (my_error_mgr*) cinfo->err;
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);
    qWarning(buffer);
    longjmp(myerr->setjmp_buffer, 1);
}

#if defined(Q_C_CALLBACKS)
}
#endif


static const int max_buf = 4096;

struct my_jpeg_source_mgr : public jpeg_source_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    FILE* f;
    JOCTET buffer[max_buf];

public:
    my_jpeg_source_mgr(FILE* f);
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static
void qt_init_source(j_decompress_ptr)
{
}

static
boolean qt_fill_input_buffer(j_decompress_ptr cinfo)
{
    int num_read;
    my_jpeg_source_mgr* src = (my_jpeg_source_mgr*)cinfo->src;
    src->next_input_byte = src->buffer;
    num_read = fread( (char*)src->buffer, 1, max_buf, src->f );

    if ( num_read <= 0 ) {
	// Insert a fake EOI marker - as per jpeglib recommendation
	src->buffer[0] = (JOCTET) 0xFF;
	src->buffer[1] = (JOCTET) JPEG_EOI;
	src->bytes_in_buffer = 2;
    } else {
	src->bytes_in_buffer = num_read;
    }
#if defined(Q_OS_UNIXWARE)
    return B_TRUE;
#else
    return TRUE;
#endif
}

static
void qt_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    my_jpeg_source_mgr* src = (my_jpeg_source_mgr*)cinfo->src;

    // `dumb' implementation from jpeglib

    /* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
     * any trouble anyway --- large skips are infrequent.
     */
    if (num_bytes > 0) {
	while (num_bytes > (long) src->bytes_in_buffer) {
	    num_bytes -= (long) src->bytes_in_buffer;
	    (void) qt_fill_input_buffer(cinfo);
	    /* note we assume that qt_fill_input_buffer will never return FALSE,
	    * so suspension need not be handled.
	    */
	}
	src->next_input_byte += (size_t) num_bytes;
	src->bytes_in_buffer -= (size_t) num_bytes;
    }
}

static
void qt_term_source(j_decompress_ptr)
{
}

#if defined(Q_C_CALLBACKS)
}
#endif


inline my_jpeg_source_mgr::my_jpeg_source_mgr(FILE* fptr)
{
    jpeg_source_mgr::init_source = qt_init_source;
    jpeg_source_mgr::fill_input_buffer = qt_fill_input_buffer;
    jpeg_source_mgr::skip_input_data = qt_skip_input_data;
    jpeg_source_mgr::resync_to_restart = jpeg_resync_to_restart;
    jpeg_source_mgr::term_source = qt_term_source;
    f = fptr;
    bytes_in_buffer = 0;
    next_input_byte = buffer;
}


QImage splash_read_jpeg_image(FILE* f)
{
    QImage image;

    struct jpeg_decompress_struct cinfo;

    struct my_jpeg_source_mgr *iod_src = new my_jpeg_source_mgr(f);
    struct my_error_mgr jerr;

    jpeg_create_decompress(&cinfo);

    cinfo.src = iod_src;

    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = my_error_exit;

    if (!setjmp(jerr.setjmp_buffer)) {
#if defined(Q_OS_UNIXWARE)
	(void) jpeg_read_header(&cinfo, B_TRUE);
#else
	(void) jpeg_read_header(&cinfo, TRUE);
#endif

	(void) jpeg_start_decompress(&cinfo);

       {
	    bool created = FALSE;
	    if ( cinfo.output_components == 3 || cinfo.output_components == 4) {
		created = image.create( cinfo.output_width, cinfo.output_height, 32 );
	    } else if ( cinfo.output_components == 1 ) {
		created = image.create( cinfo.output_width, cinfo.output_height, 8, 256 );
		for (int i=0; i<256; i++)
		    image.setColor(i, qRgb(i,i,i));
	    } else {
		// Unsupported format
	    }
	    if (!created)
		image = QImage();

	    if (!image.isNull()) {
		uchar** lines = image.jumpTable();
		while (cinfo.output_scanline < cinfo.output_height)
		    (void) jpeg_read_scanlines(&cinfo,
		                               lines + cinfo.output_scanline,
		                               cinfo.output_height);
		(void) jpeg_finish_decompress(&cinfo);

		if ( cinfo.output_components == 3 ) {
		    // Expand 24->32 bpp.
		    for (uint j=0; j<cinfo.output_height; j++) {
			uchar *in = image.scanLine(j) + cinfo.output_width * 3;
			QRgb *out = (QRgb*)image.scanLine(j);

			for (uint i=cinfo.output_width; i--; ) {
			    in-=3;
			    out[i] = qRgb(in[0], in[1], in[2]);
			}
		    }
		}
	    }
        }

	if (!image.isNull()) {
	    if ( cinfo.density_unit == 1 ) {
	        image.setDotsPerMeterX( int(100. * cinfo.X_density / 2.54) );
	        image.setDotsPerMeterY( int(100. * cinfo.Y_density / 2.54) );
	    } else if ( cinfo.density_unit == 2 ) {
		image.setDotsPerMeterX( int(100. * cinfo.X_density) );
		image.setDotsPerMeterY( int(100. * cinfo.Y_density) );
	    }
	}

    }

    jpeg_destroy_decompress(&cinfo);
    delete iod_src;
    return image;
}

#endif
