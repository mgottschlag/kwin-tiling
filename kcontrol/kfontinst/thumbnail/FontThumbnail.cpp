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
#include <QFile>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kurl.h>
#include <kzip.h>
#include <ktempdir.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#define KFI_DBUG kDebug(7115)

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
    QPixmap  pix;
    QString  realPath(path);
    KTempDir *tempDir(NULL);

    CFcEngine::setBgndCol(Qt::white);
    CFcEngine::setTextCol(Qt::black);

    KFI_DBUG << "Create font thumbnail for:" << path << endl;

    // Is this a fonts/package file? If so, extract 1 scalable font...
    if(Misc::checkExt(path, &KFI_FONTS_PACKAGE[1]))
    {
        KZip zip(path);

        if(zip.open(QIODevice::ReadOnly))
        {
            const KArchiveDirectory *zipDir=zip.directory();

            if(zipDir)
            {
                QStringList fonts(zipDir->entries());

                if(fonts.count())
                {
                    QStringList::ConstIterator it(fonts.begin()),
                                               end(fonts.end());

                    for(; it!=end; ++it)
                    {
                        const KArchiveEntry *entry=zipDir->entry(*it);

                        if(entry && entry->isFile())
                        {
                            tempDir=new KTempDir(KStandardDirs::locateLocal("tmp", KFI_TMP_DIR_PREFIX));
                            tempDir->setAutoRemove(true);

                            ((KArchiveFile *)entry)->copyTo(tempDir->name());

                            QString mime(KMimeType::findByPath(tempDir->name()+entry->name())->name());

                            if(mime=="application/x-font-ttf" || mime=="application/x-font-otf" ||
                               mime=="application/x-font-ttc" || mime=="application/x-font-type1")
                            {
                                realPath=tempDir->name()+entry->name();
                                break;
                            }
                            else
                                ::unlink(QFile::encodeName(tempDir->name()+entry->name()).data());
                        }
                    }
                }
            }
        }
    }

    if(CFcEngine::instance()->draw(KUrl(realPath), width, height, pix, 0, true))
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

        delete tempDir;
        return true;
    }

    delete tempDir;
    return false;
}

ThumbCreator::Flags CFontThumbnail::flags() const
{
    return None;
}

}
