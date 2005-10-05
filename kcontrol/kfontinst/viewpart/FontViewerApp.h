#ifndef __FONT_VIEWER_APP_H__
#define __FONT_VIEWER_APP_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CFontViewerApp, KFI::CFontViewAppMainWindow
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 30/04/2004
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
// (C) Craig Drummond, 2004
////////////////////////////////////////////////////////////////////////////////

#include <kapplication.h>
#include <kparts/part.h>
#include <kparts/mainwindow.h>

namespace KFI
{

class CFontViewerAppMainWindow : public KParts::MainWindow
{
    Q_OBJECT

    public:

    CFontViewerAppMainWindow();
    virtual ~CFontViewerAppMainWindow();

    public slots:

    void fileOpen();

    private:

    KParts::ReadOnlyPart *itsPreview;

};

class CFontViewerApp : public KApplication
{
    public:

    CFontViewerApp();
    virtual ~CFontViewerApp() {}
};

}

#endif
