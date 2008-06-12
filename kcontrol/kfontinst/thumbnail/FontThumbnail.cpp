/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QApplication>
#include <QtGui/QPalette>
#include <QtCore/QFile>
#include <KDE/KGlobalSettings>
#include <KDE/KLocale>
#include <KDE/KUrl>
#include <KDE/KZip>
#include <KDE/KTempDir>
#include <KDE/KMimeType>
#include <KDE/KStandardDirs>
#include <KDE/KDebug>

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
    KTempDir *tempDir = 0;

    CFcEngine::setTextCol(QApplication::palette().color(QPalette::Text));

    KFI_DBUG << "Create font thumbnail for:" << path << endl;

    // Is this a fonts/package file? If so, extract 1 scalable font...
    if(Misc::isPackage(path) || "application/zip"==KMimeType::findByFileContent(path)->name())
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
                            delete tempDir;
                            tempDir=new KTempDir(KStandardDirs::locateLocal("tmp", KFI_TMP_DIR_PREFIX));
                            tempDir->setAutoRemove(true);

                            ((KArchiveFile *)entry)->copyTo(tempDir->name());

                            QString mime(KMimeType::findByPath(tempDir->name()+entry->name())->name());

                            if(mime=="application/x-font-ttf" || mime=="application/x-font-otf" ||
                               mime=="application/x-font-type1")
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
