#ifndef __KFILE_FONT_H__
#define __KFILE_FONT_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFileFont
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 20/03/2003
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
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include <kfilemetainfo.h>
#include "FcEngine.h"

namespace KFI
{

class KFileFontPlugin : public KFilePlugin
{
    public:

    KFileFontPlugin(QObject *parent, const char *name, const QStringList& args);
    virtual ~KFileFontPlugin() {}

    bool readInfo(KFileMetaInfo& info, uint what = KFileMetaInfo::Fastest);

    private:

    void addMimeType(const char *mime);

    private:

    CFcEngine itsEngine;
};

}

#endif
