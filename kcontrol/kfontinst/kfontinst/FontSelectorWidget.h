#ifndef __FONT_SELECTOR_WIDGET_H__
#define __FONT_SELECTOR_WIDGET_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontSelectorWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 18/06/2002
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
// (C) Craig Drummond, 2002
////////////////////////////////////////////////////////////////////////////////
 
#include <klistview.h>
#include <kurl.h>

class QPopupMenu;

class CFontSelectorWidget : public KListView
{
    Q_OBJECT

    public:

    class CListViewItem : public QListViewItem
    {
        public:

        CListViewItem(CFontSelectorWidget *listWidget, const QString &name,
                      const QString &icon, const QString &base);
        CListViewItem(CFontSelectorWidget *listWidget, QListViewItem *parent, const QString &name);
        virtual ~CListViewItem() {}

        void    initIcon(const QString &icn);
        QString key(int column, bool ascending) const;
        QString fullName() const;
        void    open();
        void    setup();
        void    setOpen(bool open);

        private:

        QString             itsBase;
        CFontSelectorWidget *itsListWidget;
    };

    public:

    CFontSelectorWidget(QWidget *parent);
    virtual ~CFontSelectorWidget() {}

    void        storeSettings();
    void        progressInit(const QString &title, int numSteps);
    void        progressShow(const QString &step);
    void        progressStop();
    void        showContents();
    KURL::List  getSelectedFonts();

    public slots:

    void        popupMenu(QListViewItem *item, const QPoint &point, int column);
    void        selectionChanged();
    void        install();
    void        showMeta();

    signals:

    void        fontSelected(const QString &file);
    void        initProgress(const QString &title, int numSteps);
    void        progress(const QString &step);
    void        stopProgress();
    void        installSelected();
    void        showMetaData(QStringList files);

    private:

    bool       itsShowingProgress,
               itsSetup;
    QPopupMenu *itsPopup;
};

#endif
