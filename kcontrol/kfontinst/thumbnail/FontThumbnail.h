#ifndef __FONT_THUMBNAIL_H__
#define __FONT_THUMBNAIL_H__

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

//
// Want to use some classes from main KFontinst code, but don't want/need all functionaility...
#define KFI_THUMBNAIL

//
// NOTE: The FreeType headers are in quotes and not brackets, as the path is supplied in the compile line
//       and some systems have both FreeType1 and FreeType2 installed - if we used <freetype/freetype.h> then it
//       is likely that the FreeType1 header would be included!
//
 
#include "freetype/freetype.h"
#include "freetype/cache/ftcimage.h"
#include "freetype/cache/ftcsbits.h"
#include "freetype/ftcache.h"
#include "FontEngine.h"
#include <kio/thumbcreator.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qpaintdevice.h>

class CFontThumbnail : public ThumbCreator
{
    private:

    struct Bitmap
    {
        int           width,
                      height,
                      greys,
                      mod;
        unsigned char *buffer;
    };

    public:

    enum
    {
        SMALL  = 12,
        MEDIUM = 18,
        LARGE  = 24
    };

    public:

    CFontThumbnail();
    virtual ~CFontThumbnail();

    virtual bool  create(const QString &path, int width, int height, QImage &img);
    virtual Flags flags() const;

    static int point2Pixel(int point)
    {
        return (point* /*QPaintDevice::x11AppDpiX()*/ 75 +36)/72;
    }

    private:

    FTC_FaceID getId(const QString &f);
    bool       getGlyphBitmap(FTC_Image_Desc &font, FT_ULong index, Bitmap &target, int &left, int &top,
                             int &xAdvance, FT_Pointer *ptr);
    void       align32(Bitmap &bmp);

    private:

    CFontEngine       itsEngine;
    FTC_Manager       itsCacheManager;
    FTC_Image_Cache   itsImageCache;
    FTC_SBit_Cache    itsSBitCache;
    QPtrList<QString> itsFiles;
    unsigned char     *itsBuffer;
    int               itsBufferSize;
};

#endif
