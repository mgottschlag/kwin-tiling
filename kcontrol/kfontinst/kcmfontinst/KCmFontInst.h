#ifndef __KCM_FONT_INST_H__
#define __KCM_FONT_INST_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CKCmFontInst
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003
////////////////////////////////////////////////////////////////////////////////

#include <kcmodule.h>
#include <qstringlist.h>
#include <kurl.h>
#include <kconfig.h>

class KDirOperator;
class KAction;
class KRadioAction;
class KActionMenu;
class KFileItem;
class CFontPreview;
class QLabel;
class QSplitter;

class CKCmFontInst : public KCModule
{
    Q_OBJECT

    public:

    CKCmFontInst(QWidget *parent=NULL, const char *name=NULL, const QStringList &list=QStringList());
    virtual ~CKCmFontInst();

    const KAboutData * aboutData() const;

    public slots:

    QString quickHelp() const;
    void    gotoTop();
    void    goUp();
    void    goBack();
    void    goForward();
    void    listView();
    void    iconView();
    void    setupViewMenu();
    void    urlEntered(const KURL &url);
    void    fileHighlighted(const KFileItem *item);
    void    fileSelected(const KFileItem *item);
    void    loadingFinished();
    void    addFonts();
    void    removeFonts();
    void    enable();
    void    disable();

    private:

    void    setUpAct();
    void    enableItems(bool enable);

    private:

    KAboutData   *itsAboutData;
    KDirOperator *itsDirOp;
    KURL         itsTop;
    KAction      *itsUpAct,
                 *itsSepDirsAct,
                 *itsShowHiddenAct,
                 *itsDeleteAct,
                 *itsEnableAct,
                 *itsDisableAct;
    KRadioAction *itsListAct,
                 *itsIconAct;
    KActionMenu  *itsViewMenuAct;
    CFontPreview *itsPreview;
    QLabel       *itsLabel;
    QSplitter    *itsSplitter;
    KConfig      itsConfig;
    bool         itsAutoSync;
};

#endif
