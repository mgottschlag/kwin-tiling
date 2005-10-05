#ifndef __KFILE_FONT_ICON_VIEW_H__
#define __KFILE_FONT_ICON_VIEW_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CKFileFontIconView
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 01/08/2003
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

#include <kfileiconview.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

namespace KFI
{

class CKFileFontIconView : public KFileIconView
{
    public:

    CKFileFontIconView(QWidget *parent, const char *name) : KFileIconView(parent, name) {}
    virtual ~CKFileFontIconView()                                                       {}

    protected:

    // DND support
    void contentsDragEnterEvent(QDragEnterEvent *e);
    void contentsDragMoveEvent(QDragMoveEvent *e);
    void contentsDropEvent(QDropEvent *e);
    bool acceptDrag(QDropEvent *e) const;
};

}

#endif
