#ifndef __AFM_CREATOR_H__
#define __AFM_CREATOR_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CAfmCreator
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 01/05/2001
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
#include <qstring.h>
#include <qglobal.h>
#if QT_VERSION >= 300
#include <qptrlist.h>
#else
#include <qlist.h>
#endif
#include <qstringlist.h>
#include "Encodings.h"
 
class CAfmCreator : public QObject
{
    Q_OBJECT

    public:

    enum EStatus
    {
        SUCCESS,
        COULD_NOT_FIND_ENCODING,
        COULD_NOT_OPEN_FONT,
        COLD_NOT_CREATE_FILE
    };

    struct TKerning
    {
        TKerning(const QString &l, const QString &r, short v) : left(l), right(r), value(v) {}

        QString left,
                right;
        short   value;
    };

    public:

    CAfmCreator() : QObject(NULL, NULL) {}
 
    EStatus go(const QString &dir);

    static QString statusToStr(EStatus val);

    signals:

    void           step(const QString &);

    private:

    static EStatus create(const QString &fName); 
    static EStatus create(const QString &fname, const QString &encoding, bool symEnc);
    static QString getEncoding(const QString &afm);
#if QT_VERSION >= 300
    static void    readKerningAndComposite(const QString &afm, QPtrList<TKerning> &kern, QStringList &comp, CEncodings::T8Bit *enc);
#else
    static void    readKerningAndComposite(const QString &afm, QList<TKerning> &kern, QStringList &comp, CEncodings::T8Bit *enc);
#endif
};

#endif
