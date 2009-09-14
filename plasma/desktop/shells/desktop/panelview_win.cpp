/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "panelview.h"

#include <KDebug>

#include "panelcontroller.h"

#include <windows.h>

#define WM_APPBAR_CALLBACK ( WM_USER + 1010 )


bool PanelView::registerAccessBar(bool fRegister)
{
    if(fRegister) {
        setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = (HWND)winId();
        abd.uCallbackMessage = WM_APPBAR_CALLBACK;

        if(!SHAppBarMessage(ABM_NEW, &abd)) {
            kWarning() << "SHAppBarMessage( ABM_NEW ) failed";
            return false;
        }

        switch (location()) {
            case Plasma::TopEdge:
                abd.uEdge = ABE_TOP;
                break;

            case Plasma::BottomEdge:
                abd.uEdge = ABE_BOTTOM;
                break;

            case Plasma::LeftEdge:
                abd.uEdge = ABE_LEFT;
                break;

            case Plasma::RightEdge:
                abd.uEdge = ABE_RIGHT;
                break;
        }
        
        //appBarPosChanged();

    } else {
        SHAppBarMessage(ABM_REMOVE, &abd);
    }
    return true;
}

bool PanelView::winEvent( MSG* msg, long* result )
{
	*result = 0;

	switch( msg->message )
	{
    	case WM_APPBAR_CALLBACK:
    		appBarCallback( msg->wParam, msg->lParam );
            return true;

    	case WM_ACTIVATE:
    		SHAppBarMessage( ABM_ACTIVATE, &abd );
    		break;

    	case WM_WINDOWPOSCHANGED:
            SHAppBarMessage( ABM_WINDOWPOSCHANGED, &abd );
    		break;

    	default:
    		return false;
	}
	
	return false;
}

void PanelView::appBarCallback( WPARAM msg, LPARAM lParam )
{
    uint uState;
    switch (msg) {
        case ABN_STATECHANGE:
            break;

        case ABN_FULLSCREENAPP:
            uState = SHAppBarMessage(ABM_GETSTATE, &abd);
            if (lParam) {
                SetWindowPos(winId(), (ABS_ALWAYSONTOP & uState) ? HWND_TOPMOST : HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            } else if (uState & ABS_ALWAYSONTOP) {
                SetWindowPos(winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
            break;

        case ABN_POSCHANGED:
            appBarPosChanged();
            break;
    }
}

void PanelView::appBarPosChanged()
{
    RECT rc; 

	// Get the area of the entire desktop
	rc.left = rc.top = 0;
	rc.right = GetSystemMetrics( SM_CXSCREEN );
	rc.bottom = GetSystemMetrics( SM_CYSCREEN );

    switch (location()) {
        case Plasma::TopEdge:
            abd.uEdge = ABE_TOP;
            rc.bottom = rc.top + height();
            break;

        case Plasma::BottomEdge:
            abd.uEdge = ABE_BOTTOM;
            rc.top = rc.bottom - height();
            break;

        case Plasma::LeftEdge:
            abd.uEdge = ABE_LEFT;
            rc.right = rc.left + width();
            break;

        case Plasma::RightEdge:
            abd.uEdge = ABE_RIGHT;
            rc.left = rc.right - width();
            break;
    }
    
    appBarQuerySetPos(&rc);
}

void PanelView::appBarQuerySetPos(LPRECT lprc)
{
    int iHeight = 0; 
    int iWidth = 0; 

    abd.rc = *lprc;

    if ((abd.uEdge == ABE_LEFT) || (abd.uEdge == ABE_RIGHT)) {
        iWidth = abd.rc.right - abd.rc.left;
        abd.rc.top = y();
        abd.rc.bottom = y()+height();
    } else {
        iHeight = abd.rc.bottom - abd.rc.top; 
        abd.rc.left = x(); 
        abd.rc.right = x()+width(); 
    }

    // Query the system for an approved size and position.
    SHAppBarMessage(ABM_QUERYPOS, &abd);

    // Adjust the rectangle, depending on the edge to which the 
    // appbar is anchored. 
    switch (abd.uEdge) {
        case ABE_LEFT:
            abd.rc.right = abd.rc.left + iWidth;
            break;

        case ABE_RIGHT:
            abd.rc.left = abd.rc.right - iWidth;
            break;

        case ABE_TOP:
            abd.rc.bottom = abd.rc.top + iHeight;
            break;

        case ABE_BOTTOM:
            abd.rc.top = abd.rc.bottom - iHeight;
            break;
    }

    // Pass the final bounding rectangle to the system. 
    SHAppBarMessage(ABM_SETPOS, &abd);
    
    // Move and size the appbar so that it conforms to the 
    // bounding rectangle passed to the system. 
    //MoveWindow(winId(), abd.rc.left, abd.rc.top, abd.rc.right - abd.rc.left, abd.rc.bottom - abd.rc.top, true); 
}
