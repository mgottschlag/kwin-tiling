#ifndef __STAR_OFFICE_CONFIG_H__
#define __STAR_OFFICE_CONFIG_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CStarOfficeConfig
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 04/05/2001
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
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include <qobject.h>

class QString;
class CBufferedFile;

class CStarOfficeConfig : public QObject
{
    Q_OBJECT

    public:

    enum EStatus
    {
        SUCCESS,
        COULD_NOT_OPEN_X11_DIR,
        COULD_NOT_OPEN_PSSTD_FONTS,
        COULD_NOT_OPEN_PPD_FILE,
        COULD_NOT_OPEN_FONTS_DOT_SCALE,
        COULD_NOT_MODIFY_XPRINTER_DOT_PROLOG,
        COULD_NOT_LINK_XPRINTER_DOT_PROLOG,
        COULD_NOT_RESTORE_XPRINTER_DOT_PROLOG
    };

    public:

    CStarOfficeConfig() : QObject(NULL, NULL) {}

    EStatus go(const QString &path);

    static void    removeAfm(const QString &fname);
    static QString statusToStr(EStatus st);

    signals:

    void           step(const QString &);

    private:

    static QString getAfmName(const QString &file);
    static EStatus outputToPsStdFonts(const QString &xDir, CBufferedFile &out, const QString &fileName, const QString &afm);
};

#endif
