////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontThumbnail
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 02/05/2002
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2002
////////////////////////////////////////////////////////////////////////////////

#include "FontThumbnail.h"
#include "FontEngine.cpp"
#include "Misc.cpp"
#include "CompressedFile.cpp"
#include <qpixmap.h>
#include <qimage.h>
#include <qfont.h>
#include <qpainter.h>
#include <kiconloader.h>

extern "C"
{
    ThumbCreator *new_creator()
    {
        return new CFontThumbnail;
    }

    static FT_Error face_requester(FTC_FaceID faceId, FT_Library lib, FT_Pointer ptr, FT_Face *face)
    {
        ptr=ptr;
        return FT_New_Face(lib, ((QString *)faceId)->local8Bit(), 0, face);
    }
}

CFontThumbnail::CFontThumbnail()
           : itsBuffer(NULL),
             itsBufferSize(0)
{
    itsFiles.setAutoDelete(true);
    FTC_Manager_New(itsEngine.itsFt.library, 0, 0, 0, face_requester, 0, &itsCacheManager);
    FTC_SBit_Cache_New(itsCacheManager, &itsSBitCache);
    FTC_Image_Cache_New(itsCacheManager, &itsImageCache);
}

CFontThumbnail::~CFontThumbnail()
{
    FTC_Manager_Done(itsCacheManager);
    if(itsBuffer)
        delete [] itsBuffer;
}

FTC_FaceID CFontThumbnail::getId(const QString &f)
{
    QString *p=NULL;

    for(p=itsFiles.first(); p; p=itsFiles.next())
        if (*p==f)
            break;
   
    if(!p)
    {
        p=new QString(f);
        itsFiles.append(p);
    }

    return (FTC_FaceID)p;
}

bool CFontThumbnail::getGlyphBitmap(FTC_Image_Desc &font, FT_ULong index, Bitmap &target, int &left, int &top,
                                    int &xAdvance, FT_Pointer *ptr)
{
    bool ok=false;

    *ptr=NULL;

    //
    // Cache small glyphs, else render on demand...
    if(font.font.pix_width<48 && font.font.pix_height<48)
    {
        FTC_SBit sbit;

        if(!FTC_SBit_Cache_Lookup(itsSBitCache, &font, index, &sbit))
        {
            target.greys=ft_pixel_mode_mono==sbit->format ? 1 : 256;
            target.height=sbit->height;
            target.width=sbit->width;
            target.buffer=sbit->buffer;
            left=sbit->left;
            top=sbit->top;
            xAdvance=sbit->xadvance;
            ok=true;
        }
    }
    else
    {
        FT_Glyph glyph;

        if(!FTC_Image_Cache_Lookup(itsImageCache, &font, index, &glyph))
        {
            ok=true;

            if(ft_glyph_format_outline==glyph->format)
                if(ok=!FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, NULL, 0))
                    *ptr=glyph;

            if(ok)
                if(ft_glyph_format_bitmap==glyph->format)
                {
                    FT_BitmapGlyph bitmap=(FT_BitmapGlyph)glyph;
                    FT_Bitmap      *source=&(bitmap->bitmap);

                    target.greys= (ft_pixel_mode_mono==(FT_Pixel_Mode_) source->pixel_mode) ? 1 : source->num_grays;
                    target.height=source->rows;
                    target.width=source->width;
                    target.buffer=source->buffer;
                    left=bitmap->left;
                    top=bitmap->top;
                    xAdvance=(glyph->advance.x+0x8000)>>16;
                }
                else
                    ok=false;
        }
    }

    return ok;
}

void CFontThumbnail::align32(Bitmap &bmp)
{
    int mod=bmp.width%4;

    if(mod)
    {
        bmp.mod=4-mod;

        int width=bmp.width+bmp.mod,
            size=(bmp.width+bmp.mod)*bmp.height,
            row;

        if(size>itsBufferSize)
        {
            static const int constBufferBlock=512;

            if(itsBuffer)
                delete [] itsBuffer;
            itsBufferSize=(((int)(size/constBufferBlock))+(size%constBufferBlock ? 1 : 0))*constBufferBlock;
            itsBuffer=new unsigned char [itsBufferSize];
        }

        memset(itsBuffer, 0, itsBufferSize);
        for(row=0; row<bmp.height; ++row)
            memcpy(&itsBuffer[row*width], &bmp.buffer[row*bmp.width], bmp.width);
 
        bmp.buffer=itsBuffer;
        bmp.width+=bmp.mod;
    }
    else
        bmp.mod=0;
}

bool CFontThumbnail::create(const QString &path, int width, int height, QImage &img)
{
    enum
    {
        KONQ_SMALL  = 48,
        KONQ_MEDIUM = 60,
        KONQ_LARGE  = 90
    };

    bool status=false,
         konqPreview=height==width && (KONQ_SMALL==height || KONQ_MEDIUM==height || KONQ_LARGE==height);
    int  pointSize=height <= KONQ_SMALL && width <= KONQ_SMALL ? SMALL :   // Konqueror's small icon preview
                   height <= KONQ_MEDIUM && width <=KONQ_MEDIUM ? MEDIUM :  // "           medium "   "
                   height < 30 ? MEDIUM : LARGE;
    bool showName=!konqPreview && (pointSize==LARGE) && (height > (point2Pixel(pointSize)*3));

    if(!showName || itsEngine.openFont(path))
    {
        FT_Face        face;
        FT_Size        size;
        FTC_Image_Desc font;

        font.font.face_id=getId(path);
        font.font.pix_width=font.font.pix_height=point2Pixel(pointSize);
        font.image_type=ftc_image_grays;
   
        static const int constOffset=4;

        FT_F26Dot6 startX=constOffset,
                   startY=constOffset+font.font.pix_height,
                   x=startX,
                   y=startY;
        QPixmap    pix(width, height);

        pix.fill(Qt::black);

        if(showName)
        {
            QFont    font("times", 12);
            QPainter painter(&pix);
            QString  name(itsEngine.getFullName()),
                     info;
            bool     bmp=CFontEngine::isABitmap(path.local8Bit());

            if(bmp)
            {
                int pos=name.findRev('(');

                info=name.mid(pos);
                name=name.left(pos);
            }

            painter.setFont(font);
            painter.setPen(Qt::white);
            y=constOffset+painter.fontMetrics().height();
            painter.drawText(x, y, name);

            if(bmp)
            {
                y+=2+painter.fontMetrics().height();
                painter.drawText(x, y, info);
            }

            y+=4;
            painter.drawLine(constOffset, y, width-(constOffset*2), y);
            y+=2;
            y+=startY;
            status=true;
        }
 
        if(!FTC_Manager_Lookup_Size(itsCacheManager, &(font.font), &face, &size))
        {
            int        i;
            FT_F26Dot6 stepY=(size->metrics.height>>6) + constOffset;

            for(i=1; i<face->num_glyphs; ++i)  // Glyph 0 is the NULL glyph
            {
                int        left,
                           top,
                           xAdvance;
                FT_Pointer glyph;
                Bitmap     bmp;

                if(getGlyphBitmap(font, i, bmp, left, top, xAdvance, &glyph) && bmp.width>0 && bmp.height>0)
                {
                    static QRgb clut [256];
                    static bool clutSetup=false;

                    if(!clutSetup)
                    {
                        int j;
                        for(j=0; j<256; j++)
                            clut[j]=qRgb(j, j, j);
                        clutSetup=true;
                    }

                    status=true;
                    align32(bmp);

                    QPixmap glyphPix(QImage(bmp.buffer, bmp.width, bmp.height, bmp.greys>1 ? 8 : 1, clut, bmp.greys, QImage::IgnoreEndian));

                    bitBlt(&pix, x+left, y-top, &glyphPix, 0, 0, bmp.width, bmp.height, Qt::XorROP);

                    if(glyph)
                        FT_Done_Glyph((FT_Glyph)glyph);

                    x+=xAdvance+1;

                    if(x+size->metrics.x_ppem>width)
                    {
                        x=startX;
                        y+=stepY;

                        if(y>height)
                            break;
                    }
                }
            }
        }

        if(status)
        {
            img=pix.convertToImage();
            img.invertPixels();
        }

        if(showName)
            itsEngine.closeFont();
    }

    if(!status)
    {
        int size= konqPreview ? (height==KONQ_SMALL  ? KIcon::SizeSmall :
                                 height==KONQ_MEDIUM ? KIcon::SizeMedium :
                                                       KIcon::SizeLarge)
                              : (pointSize==SMALL    ? KIcon::SizeSmall :
                                 pointSize==MEDIUM   ? KIcon::SizeMedium :
                                                       KIcon::SizeLarge);

        status=true;

        switch(CFontEngine::getType(QFile::encodeName(path)))
        {
            case CFontEngine::TRUE_TYPE:
                img=KGlobal::iconLoader()->loadIcon("font_truetype", KIcon::Desktop, size).convertToImage();
                break;
            case CFontEngine::TYPE_1:
                img=KGlobal::iconLoader()->loadIcon("font_type1", KIcon::Desktop, size).convertToImage();
                break;
            case CFontEngine::SPEEDO:
                img=KGlobal::iconLoader()->loadIcon("font_speedo", KIcon::Desktop, size).convertToImage();
                break;
            case CFontEngine::BITMAP:
                img=KGlobal::iconLoader()->loadIcon("font_bitmap", KIcon::Desktop, size).convertToImage();
                break;
            default:
                status=false;
        }
    }

    return status;
}

ThumbCreator::Flags CFontThumbnail::flags() const
{
    return None;
}
