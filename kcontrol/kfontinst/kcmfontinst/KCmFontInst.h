#ifndef __KCM_FONT_INST_H__
#define __KCM_FONT_INST_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CKCmFontInst
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 26/04/2003
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QStringList>
//Added by qt3to4:
#include <QLabel>
#include <QDropEvent>
#include <kcmodule.h>
#include <kurl.h>
#include <kconfig.h>
#include <kio/job.h>
#ifdef HAVE_XFT
#include <kparts/part.h>
#endif

class KDirOperator;
class KAction;
class KToggleAction;
class KActionMenu;
class KFileItem;
class QLabel;
class QSplitter;
class QDropEvent;
class KFileItem;

namespace KFI
{

class CKCmFontInst : public KCModule
{
    Q_OBJECT

    public:

    CKCmFontInst(QWidget *parent, const QStringList &list=QStringList());
    virtual ~CKCmFontInst();

    void    setMimeTypes(bool showBitmap);

    public Q_SLOTS:

    void    filterFonts();
    QString quickHelp() const;
    void    listView();
    void    iconView();
    void    setupMenu();
    void    setupViewMenu();
    void    fileHighlighted(const KFileItem *item);
    void    loadingFinished();
    void    addFonts();
    void    removeFonts();
    void    configure();
    void    print();
    void    dropped(const KFileItem *i, QDropEvent *e, const KUrl::List &urls);
    void    infoMessage(const QString &msg);
    void    updateInformation(int dirs, int fonts);
    void    jobResult(KJob *job);

    private:

    void    addFonts(const KUrl::List &src, const KUrl &dest);

    private:

    KDirOperator         *itsDirOp;
    KUrl                 itsTop;
    KToggleAction        *itsShowBitmapAct;
    KAction              *itsSepDirsAct,
                         *itsShowHiddenAct,
                         *itsDeleteAct;
    KAction              *itsListAct,
                         *itsIconAct;
    KActionMenu          *itsViewMenuAct;
#ifdef HAVE_XFT
    KParts::ReadOnlyPart *itsPreview;
#endif
    QSplitter            *itsSplitter;
    KConfig              itsConfig;
    bool                 itsEmbeddedAdmin;
    QLabel               *itsStatusLabel;
};

}

#endif
