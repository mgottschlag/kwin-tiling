/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "FontThumbnail.h"
#include "KfiConstants.h"
#include "FcEngine.h"
#include <QImage>
#include <QBitmap>
#include <QPainter>
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

    CFcEngine::setBgndCol(Qt::white);
    CFcEngine::setTextCol(Qt::black);
    if(CFcEngine::instance()->draw(KUrl(path), width, height, pix, 0, true))
    {
        img=pix.toImage().convertToFormat(QImage::Format_ARGB32);

        int pixelsPerLine=img.bytesPerLine()/4;

        for(int l=0; l<img.height(); ++l)
        {
            QRgb *scanLine=(QRgb *)img.scanLine(l);

            for(int pixel=0; pixel<pixelsPerLine; ++pixel)
                scanLine[pixel]=qRgba(qRed(scanLine[pixel]), qGreen(scanLine[pixel]),
                                      qBlue(scanLine[pixel]),
                                      0xFF-qRed(scanLine[pixel]));
        }

        return true;
    }

    return false;
}

ThumbCreator::Flags CFontThumbnail::flags() const
{
    return None;
}

}
