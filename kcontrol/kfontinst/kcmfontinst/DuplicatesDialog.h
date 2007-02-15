#ifndef __DUPLICATES_DIALOG_H__
#define __DUPLICATES_DIALOG_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2007 Craig Drummond <craig@kde.org>
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

#include <QPixmap>
#include <QThread>
#include <QTreeWidget>
#include "ActionDialog.h"
#include "Misc.h"

class QTimer;
class QLabel;
class QMenu;
class QAction;

namespace KFI
{

class CJobRunner;
class CFontList;
class CDuplicatesDialog;

class CFontFileList : public QThread
{
    Q_OBJECT

    public:

    typedef QHash<Misc::TFont, QStringList> TFontMap;

    //
    // TFile store link from filename to FontMap item.
    // This is used when looking for duplicate filenames (a.ttf/a.TTF).
    struct TFile
    {
        TFile(const QString &n, CFontFileList::TFontMap::Iterator i) : name(n), it(i), userLower(false) { }
        TFile(const QString &n, bool l=false)                        : name(n), userLower(l)            { }

        bool operator==(const TFile &f) const
        {
            return userLower||f.userLower
                    ? name.toLower()==f.name.toLower()
                    : name==f.name;
        }

        QString                           name;
        CFontFileList::TFontMap::Iterator it;
        bool                              userLower;
    };

    public:

    CFontFileList(CDuplicatesDialog *parent);

    void start();
    void terminate();
    void getDuplicateFonts(TFontMap &map);
    bool wasTerminated() const { return itsTerminated; }

    Q_SIGNALS:

    void finished();

    private:

    void run();
    void fileDuplicates(const QString &folder, const QSet<TFile> &files);

    private:

    bool     itsTerminated;
    TFontMap itsMap;
};

class CFontFileListView : public QTreeWidget
{
    Q_OBJECT

    public:

    CFontFileListView(QWidget *parent);
    virtual ~CFontFileListView() { }

    QSet<QString> getMarkedFiles();
    void          removeFiles(const QSet<QString> &files);

    Q_SIGNALS:

    void haveDeletions(bool have);

    private Q_SLOTS:

    void openViewer();
    void properties();
    void mark();
    void unmark();
    void selectionChanged();
    void clicked(QTreeWidgetItem *item, int col);
    void contextMenuEvent(QContextMenuEvent *ev);

    private:

    void checkFiles();

    private:

    QMenu   *itsMenu;
    QAction *itsMarkAct,
            *itsUnMarkAct;
};

class CDuplicatesDialog : public CActionDialog
{
    Q_OBJECT

    public:

    CDuplicatesDialog(QWidget *parent, CJobRunner *jr, CFontList *fl);

    int   exec();
    bool  modifiedSys() const  { return itsModifiedSys; }
    bool  modifiedUser() const { return itsModifiedUser; }

    const CFontList * fontList() const { return itsFontList; }

    private Q_SLOTS:

    void scanFinished();
    void slotButtonClicked(int button);

    private:

    int           deleteFiles();
    QSet<QString> deleteFiles(const QSet<QString> &files);
    QSet<QString> deleteSysFiles(const QStringList &files);

    private:

    bool              itsModifiedSys,
                      itsModifiedUser;
    CFontFileList     *itsFontFileList;
    QLabel            *itsLabel;
    CFontFileListView *itsView;
    CJobRunner        *itsRunner;
    CFontList         *itsFontList;
};

}

#endif
