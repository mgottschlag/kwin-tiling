////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontThumbnail
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 02/08/2003
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
// (C) Craig Drummond, 2003
////////////////////////////////////////////////////////////////////////////////

#include "FontThumbnail.h"
#include "FontEngine.h"
#include "Misc.h"
#include "CompressedFile.h"
#include "Global.h"
#include "KfiConfig.h"
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
}

CFontThumbnail::CFontThumbnail()
{
    KGlobal::locale()->insertCatalogue("kfontinst");
}

CFontThumbnail::~CFontThumbnail()
{
    CGlobal::destroy();
}

bool CFontThumbnail::create(const QString &path, int width, int height, QImage &img)
{
    if(CGlobal::fe().openKioFont(path, CFontEngine::NAME, true, 0))
    {
        QPixmap pix;
        CGlobal::fe().createPreview(width, height, pix, 0);

        img=pix.convertToImage();

        CGlobal::fe().closeFont();
        return true;
    }

    return false;
}

ThumbCreator::Flags CFontThumbnail::flags() const
{
    return DrawFrame;
}
