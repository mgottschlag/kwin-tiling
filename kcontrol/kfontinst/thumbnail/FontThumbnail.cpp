////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontThumbnail
// Author        : Craig Drummond
// Project       : K Font Installer
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2002, 2003
////////////////////////////////////////////////////////////////////////////////

#include "FontThumbnail.h"
#include "FontEngine.h"
#include "Misc.h"
#include "CompressedFile.h"
#include "Global.h"
#include "Config.h"
#include <qimage.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <klocale.h>

extern "C"
{
    ThumbCreator *new_creator()
    {
        return new CFontThumbnail;
    }

    static FT_Error face_requester(FTC_FaceID faceId, FT_Library lib, FT_Pointer ptr, FT_Face *face)
    {
        ptr=ptr;

        FT_Error err=FT_New_Face(lib, QFile::encodeName(*((QString *)faceId)), 0, face);

        if(err)
            err=FT_New_Face(lib, QFile::encodeName(
                                   CGlobal::cfg().getRealTopDir(*((QString *)faceId))+CMisc::getSub(*((QString *)faceId))),
                            0, face);
        return err;
    }
}

CFontThumbnail::CFontThumbnail()
           : itsBuffer(NULL),
             itsBufferSize(0)
{
    itsFiles.setAutoDelete(true);
    FTC_Manager_New(CGlobal::fe().itsFt.library, 0, 0, 0, face_requester, 0, &itsCacheManager);
    FTC_SBit_Cache_New(itsCacheManager, &itsSBitCache);
    FTC_Image_Cache_New(itsCacheManager, &itsImageCache);
}

CFontThumbnail::~CFontThumbnail()
{
    FTC_Manager_Done(itsCacheManager);
    if(itsBuffer)
        delete [] itsBuffer;
    CGlobal::destroy();
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
            target.greys=ft_pixel_mode_mono==sbit->format ? 2 : 256;
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

bool CFontThumbnail::drawGlyph(QPixmap &pix, FTC_Image_Desc &font, FT_Size &size, int glyphNum,
                               FT_F26Dot6 &x, FT_F26Dot6 &y, FT_F26Dot6 width, FT_F26Dot6 height,
                               FT_F26Dot6 startX, FT_F26Dot6 stepY, int space)
{
    int        left,
               top,
               xAdvance;
    FT_Pointer glyph;
    Bitmap     bmp;

    if(getGlyphBitmap(font, glyphNum, bmp, left, top, xAdvance, &glyph) && bmp.width>0 && bmp.height>0)
    {
        if(2==bmp.greys)
        {
            QPixmap glyphPix(QBitmap(bmp.width, bmp.height, bmp.buffer));

            bitBlt(&pix, x+left, y-top, &glyphPix, 0, 0, bmp.width, bmp.height, Qt::AndROP);
        }
        else
        {
            static QRgb clut[256];
            static bool clutSetup=false;

            if(!clutSetup)
            {
                int j;
                for(j=0; j<256; j++)
                    clut[j]=qRgb(255-j, 255-j, 255-j);
                clutSetup=true;
            }

            align32(bmp);

            QPixmap glyphPix(QImage(bmp.buffer, bmp.width, bmp.height, 8, clut , bmp.greys, QImage::IgnoreEndian));

            bitBlt(&pix, x+left, y-top, &glyphPix, 0, 0, bmp.width, bmp.height, Qt::AndROP);
        }

        if(glyph)
            FT_Done_Glyph((FT_Glyph)glyph);

        x+=xAdvance+1;

        if(x+size->metrics.x_ppem>width)
        {
            x=startX;
            y+=stepY;

            if(y>height)
                return true;
        }
    }
    else if(x!=startX)
        x+=space;

    return false;
}

static bool getCharMap(FT_Face &face, const QString &str)
{
    int cm;

    for(cm=0; cm<face->num_charmaps; cm++)
    {
        unsigned int ch;
        bool         found=true;

        FT_Set_Charmap(face, face->charmaps[cm]);

        for(ch=0; ch<str.length() && found; ch++)
            if(FT_Get_Char_Index(face, str[ch].unicode())==0)
                found=false;

        if(found)
            return true;
    }
    return false;
}

bool CFontThumbnail::create(const QString &path, int width, int height, QImage &img)
{
    static const struct
    {
        int height,
            titleFont,
            font, 
            offset,
            space;
    } sizes[] = { { 16, 0,  10, 2, 0 },
                  { 32, 0,  12, 2, 0 },
                  { 48, 10, 10, 1, 3 },
                  { 64, 12, 14, 1, 4 },
                  { 90, 12, 28, 2, 6 },
                  { 0,  12, 28, 4, 8 }
                };

    bool status=false;
    int  s;

    for(s=0; sizes[s].height && height>sizes[s].height; ++s)
        ;

    if(CGlobal::fe().openKioFont(path, CFontEngine::NAME, true))
    {
        FT_Face        face;
        FT_Size        size;
        FTC_Image_Desc font;

        font.font.face_id=getId(path);
        font.font.pix_width=font.font.pix_height=point2Pixel(sizes[s].font);
        font.image_type=ftc_image_grays;

        FT_F26Dot6 startX=sizes[s].offset,
                   startY=sizes[s].offset+font.font.pix_height,
                   x=startX,
                   y=startY;
        QPixmap    pix(width, height);
        QPainter   painter(&pix);

        pix.fill(Qt::white);

        if(sizes[s].titleFont)
        {
            QString name(CGlobal::fe().getFullName()),
                    info;
            bool    bmp=CFontEngine::isABitmap(QFile::encodeName(path));
            QFont   title(KGlobalSettings::generalFont());

            if(bmp)
            {
                int pos=name.findRev('(');

                info=name.mid(pos);
                name=name.left(pos);
            }

            title.setPixelSize(sizes[s].titleFont);
            painter.setFont(title);
            painter.setPen(Qt::black);
            y=painter.fontMetrics().height();
            painter.drawText(x, y, name);

            if(bmp)
            {
                y+=2+painter.fontMetrics().height();
                painter.drawText(x, y, info);
            }

            y+=4;
            painter.drawLine(sizes[s].offset, y, width-(sizes[s].offset*2), y);
            y+=2;
            y+=startY;
        }

        status=true;

        if(!FTC_Manager_Lookup_Size(itsCacheManager, &(font.font), &face, &size))
        {
            int        i;
            FT_F26Dot6 stepY=size->metrics.y_ppem /*(size->metrics.height>>6)*/ + sizes[s].offset;

            if(0==sizes[s].height)
            {
                QString  quote(i18n("The quick brown fox jumps over the lazy dog"));
                bool     foundCmap=getCharMap(face, quote);

                if(foundCmap)
                {
                    unsigned int ch;

                    for(ch=0; ch<quote.length(); ++ch)
                        if(drawGlyph(pix, font, size, FT_Get_Char_Index(face, quote[ch].unicode()),
                           x, y, width, height, startX, stepY, sizes[s].space))
                            break;
                }

                font.font.pix_width=font.font.pix_height=point2Pixel((int)(sizes[s].font*0.75));

                if(y<height && !FTC_Manager_Lookup_Size(itsCacheManager, &(font.font), &face, &size))
                {
                    FT_F26Dot6 stepY=size->metrics.y_ppem /*(size->metrics.height>>6)*/ + sizes[s].offset;

                    if(foundCmap)
                    {
                        if(x!=startX)
                        {
                            y+=stepY;
                            x=startX;
                        }

                        y+=font.font.pix_height;
                    }

                    for(i=1; i<face->num_glyphs; ++i)  // Glyph 0 is the NULL glyph
                        if(drawGlyph(pix, font, size, i, x, y, width, height, startX, stepY))
                            break;
                }
            }
            else
            {
                QString str(i18n("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz"));

                if(getCharMap(face, str))
                {
                    unsigned int ch;

                    for(ch=0; ch<str.length(); ++ch)
                        if(drawGlyph(pix, font, size, FT_Get_Char_Index(face, str[ch].unicode()),
                           x, y, width, height, startX, stepY))
                            break;

                }
                else
                    for(i=1; i<face->num_glyphs; ++i)  // Glyph 0 is the NULL glyph
                        if(drawGlyph(pix, font, size, i, x, y, width, height, startX, stepY))
                            break;
            }

        }

        if(status)
            img=pix.convertToImage();

        CGlobal::fe().closeFont();
    }

    return status;
}

ThumbCreator::Flags CFontThumbnail::flags() const
{
    return DrawFrame;
}
