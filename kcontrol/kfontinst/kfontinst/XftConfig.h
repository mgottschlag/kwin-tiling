#ifndef __XFT_CONFIG_H__
#define __XFT_CONFIG_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXftConfig
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 05/06/2001
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_XFT

#include <qstring.h>
#include <qcstring.h>
#include <qglobal.h>
#if QT_VERSION >= 300
#include <qptrlist.h>
#else
#include <qlist.h>
#endif
#include <qstringlist.h>
#include <fstream>
#include "xftint.h"

class CXftConfig
{
    public:

    struct TEntry
    {
        TEntry(XftTest *t=NULL, XftEdit *e=NULL) : test(t), edit(e) {          };
        ~TEntry()                                                   { clear(); }

        void clear();
        void output(std::ofstream &of);

        QCString testStr();
        QCString editStr();

        XftTest *test;
        XftEdit *edit;
    };

    public:

    CXftConfig()                                           { init(); }
    virtual ~CXftConfig();

    void init();

    bool read(const QString &f);
    bool save(const QString &f, const QStringList &dirs);

    void newFile()                                         { init(); itsMadeChanges=true; }

    bool getExcludeRange(double &from, double &to);
    bool getUseSubPixelHinting()                           { return getUseSubPixelHintingEntry() ? true : false; }

    void setExcludeRange(double from, double to);
    void removeExcludeRange();
    void setUseSubPixelHinting(bool use);

    bool madeChanges()                                     { return itsMadeChanges; }

#if QT_VERSION >= 300
    QPtrList<TEntry> & getEntries()                        { return itsList; }
#else
    QList<TEntry> & getEntries()                           { return itsList; }
#endif
    QStringList &   getIncludes()                          { return itsIncludes; }
    QStringList &   getIncludeIfs()                        { return itsIncludeIfs; }
    void            setIncludes(const QStringList &list)   { itsIncludes=list; itsMadeChanges=true; }
    void            setIncludeIfs(const QStringList &list) { itsIncludeIfs=list; itsMadeChanges=true; }

    void setEntries(QList<TEntry> &list);

    // For use *only* by parser functions... 
    void addEntry(XftTest *test, XftEdit *edit);
    void addInclude(const char *dir);
    void addIncludeIf(const char *dir);

    private:

    TEntry * getExcludeRangeEntry();
    TEntry * getUseSubPixelHintingEntry();

    private:

#if QT_VERSION >= 300
    QPtrList<TEntry> itsList;
#else
    QList<TEntry> itsList;
#endif
    QStringList   itsIncludes,
                  itsIncludeIfs;
    bool          itsMadeChanges;
};

#endif

#endif
