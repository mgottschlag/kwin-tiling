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
 
#include "Config.h"
#include "Misc.h"

#include <klistview.h>
#include <qptrlist.h>

class QPainter;
class QColorGroup;
class QPopupMenu;

class CFontListWidget : public KListView
{
    Q_OBJECT

    public:

    enum EStatus
    {
        SUCCESS,
        PERMISSION_DENIED,
        ALREADY_INSTALLED,
        HAS_SUB_DIRS,
        COULD_NOT_CREATE_DIR,
        COULD_NOT_DELETE_DIR,
        INVALID_FONT,
        NOT_EMPTY
    };

    class CListViewItem : public QListViewItem
    {
        public:

        enum EParameter
        {
            AVAILABLE,
            ENABLE
        };
 
        enum EType
        {
            FONT,
            DIR
        };

        public:
 
        CListViewItem(QListView *parent, const QString &name, EType type, bool isNew, bool enabled);
        CListViewItem(QListViewItem *parent, const QString &name, EType type, bool isNew, bool enabled);
        virtual ~CListViewItem() {}

        QString         key(int column, bool ascending) const;
        void            paintCell(QPainter *painter, const QColorGroup &colourGroup, int column, int width, int align);
        EType           getType() const { return itsType; }
        virtual QString fullName() const =0;
        virtual QString dir() const =0;
        bool            changed() const  { return (itsAvailable != itsAvailableOrig) || (itsEnabled != itsEnabledOrig); }
        void            reset();
        void            saved(bool toggleState);
        QString         extraData() const { return itsData; }
        void            setExtraData(const QString &data) { itsData = CMisc::dirSyntax(data); setupDisplay(); }
        virtual void    setAvailable(bool available);
        virtual bool    available() const { return itsAvailable; }
        bool            deleted() const { return !itsAvailable && itsAvailableOrig; }
        bool            added() const { return itsAvailable && !itsAvailableOrig; }
        virtual void    setEnabled(bool enabled);
        bool            enabled() const { return itsEnabled; }
        void            changeStatus(EParameter what, bool b);
        virtual void    setupDisplay()=0;

        protected:
 
        EType   itsType;
        bool    itsAvailable,
                itsAvailableOrig,
                itsEnabled,
                itsEnabledOrig;
        QString itsData;
    };

    struct TItem
    {
        TItem(const QString &s, const QString &d, const QString &f) : source(s), dest(d), file(f) {}

        QString source,
                dest, 
                file;
    };

    public:

    CFontListWidget(QWidget *parent);
    virtual ~CFontListWidget()          { }

    void          reset();
    void          clearLists();
    void          restore(QListViewItem *item, bool checkOpen=true);
    QString       currentDir();
    unsigned int  getNumSelected(CListViewItem::EType type);
    unsigned int  getNumSelectedFonts() { return getNumSelected(CListViewItem::FONT); }
    unsigned int  getNumSelectedDirs()  { return getNumSelected(CListViewItem::DIR); }
    unsigned int  getNumSelected()      { return getNumSelected(CListViewItem::FONT)+getNumSelected(CListViewItem::DIR); }
    void          getNumSelected(int &numTT, int &numT1);
    QStringList & getAdvancedOpenDirs() { return itsAdvancedOpenDirs; }
    QPtrList<TItem> & getAddItems()     { return itsAddItems; }
    QStringList & getDelItems()         { return itsDelItems; }
    QStringList & getEnabledItems()     { return itsEnabledItems; }
    QStringList & getDisabledItems()    { return itsDisabledItems; }
    void          progressInit(const QString &title, int numSteps);
    void          progressShow(const QString &step);
    void          progressStop();
    void          scan();
    void          addFont(const QString &from, const QString &path, const QString &file, bool checkOpen=true);
    void          addSubDir(const QString &top, const QString &sub);
    void          changeStatus(bool status);

    public slots:

    void          setAdvanced(bool on);
    void          updateConfig();
    void          applyChanges();
    void          install();
    void          uninstall();
    void          disable();
    void          enable();
    void          popupMenu(QListViewItem *item, const QPoint &point, int column);
    void          listClicked(QListViewItem *item, const QPoint &point, int column);
    void          fixTtfPsNames();
    void          createDir();
    void          toggleUnscaled();
    void          selectionChanged();

    signals:

    void          fontSelected(const QString &file);
    void          initProgress(const QString &title, int numSteps);
    void          progress(const QString &step);
    void          stopProgress();
    void          configureSystem();
    void          fontMoved(const QString &font, const QString &from, const QString &to);
    void          dirMoved(const QString &top, const QString &sub);
    void          madeChanges();

    private:

    void          addDir(const QString &dir, const QString &name, const QString &icon);
    void          scanDir(const QString &dir, int sub=0);

    CListViewItem * getFirstSelectedItem();

    EStatus       uninstall(const QString &path, bool deleteAfm);
    EStatus       install(const QString &sourceDir, const QString &destDir, const QString &fname);
    EStatus       move(const QString &sourceDir, const QString &destDir, const QString &fname);
    QString       statusToStr(EStatus status);
    EStatus       doDeleteDir(const QString &dir);
    void          startDrag();
    void          movableDropEvent(QListViewItem *parent, QListViewItem *afterme);

    private:

    bool            itsAdvancedMode,
                    itsShowingProgress;
    QPopupMenu      *itsFontsPopup,
                    *itsDirsPopup;
    int             itsFixTtfPsNamesME,
                    itsCreateDirME,
                    itsSetScaledME,
                    itsSetUnscaledME;
    QStringList     itsAdvancedOpenDirs,
                    itsDelItems,
                    itsDisabledItems,
                    itsEnabledItems;
    QPtrList<TItem> itsAddItems;
};

#endif
