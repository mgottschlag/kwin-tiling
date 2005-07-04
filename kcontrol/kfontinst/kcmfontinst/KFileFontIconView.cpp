////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CKFileFontIconView
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 31/05/2003
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
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include <qevent.h>
#include <kfileitem.h>
#include <kurldrag.h>
#include "KFileFontIconView.h"

namespace KFI
{

// CPD: KDE4 should make acceptDrag() virtual there fore can get rid of all these ::contentsX functions...
void CKFileFontIconView::contentsDragEnterEvent(QDragEnterEvent *e)
{
    if(acceptDrag(e))
        KFileIconView::contentsDragEnterEvent(e);
    else
        e->ignore();
}

void CKFileFontIconView::contentsDragMoveEvent(QDragMoveEvent *e)
{
    if(acceptDrag(e))
        KFileIconView::contentsDragMoveEvent(e);
    else
        e->ignore();
}

void CKFileFontIconView::contentsDropEvent(QDropEvent *e)
{
    contentsDragLeaveEvent(NULL);

    if(acceptDrag(e))
        KFileIconView::contentsDropEvent(e);
    else
        e->ignore();
}

bool CKFileFontIconView::acceptDrag(QDropEvent *e) const
{
#if 0 // Crashes - seems to be called to quick???
    bool       ok=false;
    KURL::List urls;

    if(KURLDrag::canDecode(e) && (e->source()!=const_cast<CKFileFontIconView *>(this)) &&
       (QDropEvent::Copy==e->action() || QDropEvent::Move==e->action()) &&
       KURLDrag::decode(e, urls) && !urls.isEmpty())
    {
        KURL::List::Iterator it;

        ok=true;
        for(it=urls.begin(); ok && it!=urls.end(); ++it)
            if(!CFontEngine::isAFontOrAfm(QFile::encodeName((*it).path())))
                ok=false;
    }

    return ok;
#endif
    return KURLDrag::canDecode(e) && (e->source()!= const_cast<CKFileFontIconView*>(this)) &&
           (QDropEvent::Copy==e->action() || QDropEvent::Move==e->action());
}

}
