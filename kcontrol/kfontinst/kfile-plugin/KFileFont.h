#ifndef __KFILE_FONT_H__
#define __KFILE_FONT_H__

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

#include <kfilemetainfo.h>

namespace KFI
{

class KFileFontPlugin : public KFilePlugin
{
    public:

    KFileFontPlugin(QObject *parent, const QStringList& args);
    virtual ~KFileFontPlugin() {}

    bool readInfo(KFileMetaInfo& info, uint what = KFileMetaInfo::Fastest);

    private:

    void addMimeType(const char *mime);
};

}

#endif
