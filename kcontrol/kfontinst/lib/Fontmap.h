#ifndef __FONTMAP_H__
#define __FONTMAP_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontmap
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 06/06/2003
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

#include <qstring.h>
#include <qstringlist.h>

class CFontmap
{
    private:

    class CFile
    {
        private:

        struct TEntry
        {
            TEntry(const QString &fname) : filename(fname) {}

            QString     filename,
                        psName;
            QStringList entries;
        };

        public:

        CFile(const QString &dir);

        const QStringList * getEntries(const QString &fname);

        private:

        TEntry * findEntry(const QString &fname, bool isAlias=false);
        TEntry * getEntry(TEntry **current, const QString &fname, bool isAlias=false);

        private:

        QPtrList<TEntry> itsEntries;
    };

    public:

    //
    // Create per-folder fontmap file
    static void createLocal(const QString &dir);
    //
    // Create top-level fontmap file
    static void createTopLevel();

    private:

    CFontmap();
    ~CFontmap();
};

#endif
