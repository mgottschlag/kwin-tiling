#ifndef __KFI_CM_MODULE_H__
#define __KFI_CM_MODULE_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CKfiCmModule
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 07/05/2001
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

#include <kcmodule.h>

class CKfiMainWidget;

class CKfiCmModule : public KCModule
{
    Q_OBJECT

    public:

    CKfiCmModule(QWidget *parent=NULL, const char *name=NULL);
    virtual ~CKfiCmModule();

    const KAboutData * aboutData() const;

    static void madeChanges(bool m);
    static CKfiCmModule * instance() { return theirInstance; }

    private:

    void emitChanged(bool m);

    public slots:

    void    show();
    void    scanFonts();
    void    save();
    QString quickHelp() const;

    private:

    CKfiMainWidget *itsMainWidget;
    KAboutData     *itsAboutData;

    static CKfiCmModule *theirInstance;
};

#endif
