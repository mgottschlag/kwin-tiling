////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CFontThumbnail
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include "FontThumbnail.h"
#include "KfiConstants.h"
#include <qimage.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kurl.h>

extern "C"
{
    KDE_EXPORT ThumbCreator *new_creator()
    {
        return new KFI::CFontThumbnail;
    }
}

namespace KFI
{

CFontThumbnail::CFontThumbnail()
{
    KGlobal::locale()->insertCatalog(KFI_CATALOGUE);
}

bool CFontThumbnail::create(const QString &path, int width, int height, QImage &img)
{
    QPixmap pix;

    if(itsEngine.draw(KURL(path), width, height, pix, 0, true))
    {
        img=pix.convertToImage();
        return true;
    }

    return false;
}

ThumbCreator::Flags CFontThumbnail::flags() const
{
    return DrawFrame;
}

}
