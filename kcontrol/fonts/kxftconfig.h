#ifndef __KXFTCONFIG_H__
#define __KXFTCONFIG_H__

/*
   Copyright (c) 2002 Craig Drummond <craig@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QStringList>
#include <QString>
#include <q3ptrlist.h>
#include <stdio.h>
#include <fstream>
#include <time.h>

#ifdef HAVE_FONTCONFIG
#include <qdom.h>
#endif

class KXftConfig
{
    public:

    enum RequiredData
    {
        Dirs           = 0x01,
        SubPixelType   = 0x02,
        ExcludeRange   = 0x04,
        AntiAlias      = 0x08,
#ifdef HAVE_FONTCONFIG
        HintStyle      = 0x10
#else
        SymbolFamilies = 0x10
#endif
    };

#ifdef HAVE_FONTCONFIG
    static const int constStyleSettings=SubPixelType|ExcludeRange|AntiAlias|HintStyle;
#else
    static const int constStyleSettings=SubPixelType|ExcludeRange|AntiAlias;
#endif

    struct Item
    {
#ifdef HAVE_FONTCONFIG
        Item(QDomNode &n) : node(n), toBeRemoved(false) {}
        Item()            : toBeRemoved(false)          {}
        virtual void reset()                            { node.clear(); toBeRemoved=false; }
        bool         added()                            { return node.isNull(); }

        QDomNode node;
#else
        Item(char *s, char *e) : start(s), end(e), toBeRemoved(false) {}
        virtual void reset()                                          { start=end=NULL;
                                                                        toBeRemoved=false; }
        bool         added()                                          { return NULL==start; }

        char *start,
             *end;
#endif
        virtual ~Item() {}
        bool toBeRemoved;
    };

    struct ListItem : public Item
    {
#ifdef HAVE_FONTCONFIG
        ListItem(const QString &st, QDomNode &n) : Item(n), str(st) {}
        ListItem(const QString &st)              : str(st)          {}
#else
        ListItem(const QString &st, char *s=NULL, char *e=NULL) : Item(s, e), str(st) {}
#endif

        QString str;
    };

    struct SubPixel : public Item
    {
        enum Type
        {
            None,
            Rgb,
            Bgr,
            Vrgb,
            Vbgr
        };

#ifdef HAVE_FONTCONFIG
        SubPixel(Type t, QDomNode &n) : Item(n), type(t) {}
        SubPixel(Type t=None)         : type(t)          {}
#else
        SubPixel(Type t=None, char *s=NULL, char *e=NULL) : Item(s, e), type(t) {}
#endif
        void reset() { Item::reset(); type=None; }

        Type type;
    };

    struct Exclude : public Item
    {
#ifdef HAVE_FONTCONFIG
        Exclude(double f, double t, QDomNode &n) : Item(n), from(f), to(t) {}
        Exclude(double f=0, double t=0)          : from(f), to(t)          {}
#else
        Exclude(double f=0, double t=0, char *s=NULL, char *e=NULL) : Item(s, e), from(f), to(t) {}
#endif
        void reset() { Item::reset(); from=to=0; }

        double from,
               to;
    };

#ifdef HAVE_FONTCONFIG
    struct Hint : public Item
    {
        enum Style
        {
            NotSet,
            None,
            Slight,
            Medium,
            Full
        };

        Hint(Style s, QDomNode &n) : Item(n), style(s) {}
        Hint(Style s=NotSet)       : style(s)          {}

        void reset() { Item::reset(); style=NotSet; }

        Style style;
    };

    struct Hinting : public Item
    {
        Hinting(bool s, QDomNode &n) : Item(n), set(s) {}
        Hinting(bool s=true)         : set(s)          {}

        void reset() { Item::reset(); set=true; }

        bool set;
    };

    struct AntiAliasing : public Item
    {
        AntiAliasing(bool s, QDomNode &n) : Item(n), set(s) {}
        AntiAliasing(bool s=true)         : set(s)          {}

        void reset() { Item::reset(); set=true; }

        bool set;
    };
#endif

    public:

    static QString contractHome(QString path);
    static QString expandHome(QString path);

    //
    // Constructor
    //    required - This should be a bitmask of 'RequiredData', and indicates the data to be
    //               read/written to the config file. It is intended that the 'fonts' KControl
    //               module will use KXftConfig::SubPixelType|KXftConfig::ExcludeRange, and the
    //               font installer will use KXftConfig::Dirs|KXftConfig::SymbolFamilies.
    //
    //    system   - Indicates if the system-wide config file, or the users ~/.xftconfig file
    //               should be used. Only the font-installer should access the system file (and then
    //               only if run as root.
    KXftConfig(int required, bool system=false);

    virtual ~KXftConfig();

    bool        reset();
    bool        apply();
    bool        getSubPixelType(SubPixel::Type &type);
    void        setSubPixelType(SubPixel::Type type);  // SubPixel::None => turn off sub-pixel rendering
    bool        getExcludeRange(double &from, double &to);
    void        setExcludeRange(double from, double to); // from:0, to:0 => turn off exclude range
    void        addDir(const QString &d);
    void        removeDir(const QString &d);
    void        clearDirs()                          { clearList(m_dirs); }
    QStringList getDirs()                            { return getList(m_dirs); }
#ifdef HAVE_FONTCONFIG
    bool        getHintStyle(Hint::Style &style);
    void        setHintStyle(Hint::Style style);
#else
    void        addSymbolFamily(const QString &f)    { addItem(m_symbolFamilies, f); }
    void        removeSymbolFamily(const QString &f) { removeItem(m_symbolFamilies, f); }
    void        clearSymbolFamilies()                { clearList(m_symbolFamilies); }
    QStringList getSymbolFamilies()                  { return getList(m_symbolFamilies); }
#endif
    void        setAntiAliasing(bool set);
    bool        getAntiAliasing() const;
    bool        changed()                            { return m_madeChanges; }
    static QString description(SubPixel::Type t);
    static const char * toStr(SubPixel::Type t);
#ifdef HAVE_FONTCONFIG
    static QString description(Hint::Style s);
    static const char * toStr(Hint::Style s);
#endif
    bool        hasDir(const QString &d);

    private:

    ListItem *  findItem(Q3PtrList<ListItem> &list, const QString &i);
    void        clearList(Q3PtrList<ListItem> &list);
    static QStringList getList(Q3PtrList<ListItem> &list);
    void        addItem(Q3PtrList<ListItem> &list, const QString &i);
    void        removeItem(Q3PtrList<ListItem> &list, ListItem *item);
    void        removeItem(Q3PtrList<ListItem> &list, const QString &i)
                    { removeItem(list, findItem(list, i)); }
    void        readContents();
#ifdef HAVE_FONTCONFIG
    void        applyDirs();
    void        applySubPixelType();
    void        applyHintStyle();
    void        applyAntiAliasing();
    void        setHinting(bool set);
    void        applyHinting();
    void        applyExcludeRange(bool pixel);
    void        removeItems(Q3PtrList<ListItem> &list);
#else
    void        outputDir(std::ofstream &f, const QString &str);
    void        outputNewDirs(std::ofstream &f);
    void        outputSymbolFamily(std::ofstream &f, const QString &str);
    void        outputNewSymbolFamilies(std::ofstream &f);
    void        outputSubPixelType(std::ofstream &f, bool ifNew);
    void        outputExcludeRange(std::ofstream &f, bool ifNew, bool pixel);
#endif

    private:

    SubPixel           m_subPixel;
    Exclude            m_excludeRange,
                       m_excludePixelRange;
#ifdef HAVE_FONTCONFIG
    Hint               m_hint;
    Hinting            m_hinting;
    AntiAliasing       m_antiAliasing;
    bool               aliasingEnabled();
#else
    Q3PtrList<ListItem> m_symbolFamilies;
#endif
    Q3PtrList<ListItem> m_dirs;
    QString            m_file;
    int                m_required;
#ifdef HAVE_FONTCONFIG
    QDomDocument       m_doc;
#else
    int                m_size;
    char               *m_data;
#endif
    bool               m_madeChanges,
                       m_system;
    time_t             m_time;
};

#endif
