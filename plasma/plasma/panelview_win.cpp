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

#define APPBAR_CALLBACK         WM_USER + 1010

bool PanelView::registerAccessBar(HWND hwndAccessBar, bool fRegister)
{
    APPBARDATA abd;

    // Specify the structure size and handle to the appbar.
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hwndAccessBar;

    if (fRegister) {

        LONG_PTR lp = GetWindowLongPtr(hwndAccessBar, GWL_EXSTYLE);
        SetWindowLongPtr(hwndAccessBar, GWL_EXSTYLE, lp | WS_EX_TOOLWINDOW);
        SetWindowLongPtr(hwndAccessBar, GWL_STYLE, WS_TILED );

        // Provide an identifier for notification messages.
        abd.uCallbackMessage = APPBAR_CALLBACK;

        // Register the appbar.
        if (!SHAppBarMessage(ABM_NEW, &abd)) {
            m_barRegistered = false;
            return false;
        }

        m_barRegistered = true;

    } else {

        // Unregister the appbar.
        SHAppBarMessage(ABM_REMOVE, &abd);
        m_barRegistered = false;
    }

    return true;

}

void PanelView::appBarQuerySetPos(UINT uEdge, LPRECT lprc, PAPPBARDATA pabd)
{
    int iHeight = 0;
    int iWidth = 0;

    pabd->rc = *lprc;
    pabd->uEdge = uEdge;

    // Copy the screen coordinates of the appbar's bounding
    // rectangle into the APPBARDATA structure.
    if ((uEdge == ABE_LEFT) ||
            (uEdge == ABE_RIGHT)) {

        iWidth = pabd->rc.right - pabd->rc.left;
        pabd->rc.top = 0;
        pabd->rc.bottom = GetSystemMetrics(SM_CYSCREEN);

    } else {

        iHeight = pabd->rc.bottom - pabd->rc.top;
        pabd->rc.left = 0;
        pabd->rc.right = GetSystemMetrics(SM_CXSCREEN);

    }

    // Query the system for an approved size and position.
    SHAppBarMessage(ABM_QUERYPOS, pabd);

    // Adjust the rectangle, depending on the edge to which the
    // appbar is anchored.
    switch (uEdge) {

        case ABE_LEFT:
            pabd->rc.right = pabd->rc.left + iWidth;
            break;

        case ABE_RIGHT:
            pabd->rc.left = pabd->rc.right - iWidth;
            break;

        case ABE_TOP:
            pabd->rc.bottom = pabd->rc.top + iHeight;
            break;

        case ABE_BOTTOM:
            pabd->rc.top = pabd->rc.bottom - iHeight;
            break;

    }

    // Pass the final bounding rectangle to the system.
    SHAppBarMessage(ABM_SETPOS, pabd);

    // Move and size the appbar so that it conforms to the
    // bounding rectangle passed to the system.
    MoveWindow(pabd->hWnd, pabd->rc.left, pabd->rc.top,
        pabd->rc.right - pabd->rc.left,
        pabd->rc.bottom - pabd->rc.top, true);

}

void PanelView::appBarCallback(MSG *message, long *result)
{
    APPBARDATA abd;
    UINT uState;

    abd.cbSize = sizeof(abd);
    abd.hWnd = winId();

    switch (message->wParam) {
        case ABN_STATECHANGE:

            // Check to see if the taskbar's always-on-top state has
            // changed and, if it has, change the appbar's state
            // accordingly.
            uState = SHAppBarMessage(ABM_GETSTATE, &abd);

            SetWindowPos(winId(),
                (ABS_ALWAYSONTOP & uState) ? HWND_TOPMOST : HWND_BOTTOM,
                0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

            break;

        case ABN_FULLSCREENAPP:

            // A full-screen application has started, or the last full-
            // screen application has closed. Set the appbar's
            // z-order appropriately.
            if (result) {

                SetWindowPos(winId(),
                    (ABS_ALWAYSONTOP & uState) ? HWND_TOPMOST : HWND_BOTTOM,
                    0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

            }
			
			else {

                uState = SHAppBarMessage(ABM_GETSTATE, &abd);

                if (uState & ABS_ALWAYSONTOP)
                    SetWindowPos(winId(), HWND_TOPMOST,
                        0, 0, 0, 0,
                        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	        }

        case ABN_POSCHANGED:

            // The taskbar or another appbar has changed its
            // size or position.
            appBarPosChanged(&abd);
            break;

    }
}

void PanelView::appBarPosChanged(PAPPBARDATA pabd)
{
    RECT rc;
    RECT rcWindow;
    int iHeight;
    int iWidth;
    uint edge;

    rc.top = 0;
    rc.left = 0;
    rc.right = GetSystemMetrics(SM_CXSCREEN);
    rc.bottom = GetSystemMetrics(SM_CYSCREEN);

    GetWindowRect(pabd->hWnd, &rcWindow);
    iHeight = rcWindow.bottom - rcWindow.top;
    iWidth = rcWindow.right - rcWindow.left;
    if(!m_panelController)
        return;
    switch (m_panelController->location()) {

        case Plasma::TopEdge:
            rc.bottom = rc.top + iHeight;
            edge = ABE_TOP;
            break;

        case Plasma::BottomEdge:
            rc.top = rc.bottom - iHeight;
            edge = ABE_BOTTOM;
            break;

        case Plasma::LeftEdge:
            rc.right = rc.left + iWidth;
            edge = ABE_LEFT;
            break;

        case Plasma::RightEdge:
            rc.left = rc.right - iWidth;
            edge = ABE_RIGHT;
            break;
        }

        appBarQuerySetPos(edge, &rc, pabd);
}

bool PanelView::winEvent(MSG *message, long *result)
{
    if(message->message == APPBAR_CALLBACK) {
        appBarCallback(message, result);
        return true;
    }

    return false;
}
