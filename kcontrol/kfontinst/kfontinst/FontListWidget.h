#ifndef __FONT_LIST_WIDGET_H__
#define __FONT_LIST_WIDGET_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontListWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 20/04/2001
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
 
#include "FontListWidgetData.h"
#include "Config.h"
#include <qlistview.h>

class QPainter;
class QColorGroup;

class CFontListWidget : public CFontListWidgetData
{
    Q_OBJECT

    private:

    struct TAdvanced
    {
        TAdvanced(const QString &d1, const QString &d1n, const QString &d1i, const QString &d2, const QString &d2n, const QString &d2i, bool b2)
            : dir1(d1), dir1Name(d1n), dir1Icon(d1i), dir2(d2), dir2Name(d2n), dir2Icon(d2i), button2(b2) {}

        QString dir1,
                dir1Name,
                dir1Icon,
                dir2,
                dir2Name,
                dir2Icon;
        bool    button2;
    };

    struct TBasic
    {
        TBasic (const QString &d, bool u) : dir(d), useSubDirs(u) {}

        QString dir;
        bool    useSubDirs;
    };

    public:

    class CListViewItem : public QListViewItem
    {
        public:
 
        enum EType
        {
            FONT,
            DIR
        };
 
        public:
 
        CListViewItem(QListView *parent, const QString &name, EType type) : QListViewItem(parent, name), itsType(type) {}
        CListViewItem(QListViewItem *parent, const QString &name, EType type) : QListViewItem(parent, name), itsType(type) {}
 
        virtual ~CListViewItem() {}

        QString         key(int column, bool ascending) const;
        void            paintCell(QPainter *painter, const QColorGroup &colourGroup, int column, int width, int align);
 
        EType           getType() const { return itsType; }
        virtual QString fullName() const =0;
        virtual QString dir() const =0;

        private:
 
        EType itsType;
    };

    public:

    CFontListWidget(QWidget *parent, CConfig::EListWidget t, bool useSubDirs, bool showButton2Advanced,
                    const QString &boxLabel, const QString &button1Label, const QString &button2Label,
                    const QString &basicDir,
                    const QString &dir1, const QString &dir1Name, const QString &dir1Icon,
                    const QString &dir2=QString::null, const QString &dir2Name=QString::null, const QString &dir2Icon=QString::null);

    virtual ~CFontListWidget();

    void         setAdvanced(bool on);
    unsigned int getNumSelected(CListViewItem::EType type);
    unsigned int getNumSelectedFonts()  { return getNumSelected(CListViewItem::FONT); }
    unsigned int getNumSelectedDirs()   { return getNumSelected(CListViewItem::DIR); }
    unsigned int getNumSelected()       { return getNumSelected(CListViewItem::FONT)+getNumSelected(CListViewItem::DIR); }
    void         getNumSelected(int &numTT, int &numT1);
    void         progressInit(const QString &title, int numSteps);
    void         progressShow(const QString &step);
    void         progressStop();
    void         scan();

    static const QString & getDir(const QListViewItem *item);

    public slots:

    virtual void addFont(const QString &path, const QString &file);
    virtual void addSubDir(const QString &top, const QString &sub);

    signals:

    void fontSelected(const QString &dir, const QString &file);
    void directorySelected(const QString &dir);
    void initProgress(const QString &title, int numSteps);
    void progress(const QString &step);
    void stopProgress();

    protected:

    void addDir(const QString &dir, const QString &name, const QString &icon);
    void scanDir(const QString &dir, int sub=0);

    virtual void selectionChanged();

    CListViewItem * getFirstSelectedItem();

    protected:

    bool                 itsAdvancedMode,
                         itsShowingProgress;
    TAdvanced            itsAdvancedData;
    TBasic               itsBasicData;
    QString              itsBoxTitle;
    CConfig::EListWidget itsType;
};

#endif
