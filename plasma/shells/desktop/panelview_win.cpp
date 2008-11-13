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
//        LONG_PTR lp = GetWindowLongPtr(hwndAccessBar, GWL_EXSTYLE);
/* the following line should remove the taskbar entry of the panel
  * but as this doesn't work correctly (it somehow moves the panel) comment it out */
//        SetWindowLongPtr(hwndAccessBar, GWL_EXSTYLE, lp | WS_EX_TOOLWINDOW);
        SetWindowPos(winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

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
            SetWindowPos(winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            break;

        case ABN_FULLSCREENAPP:
            // A full-screen application has started, or the last full-
            // screen application has closed. Set the appbar's
            // z-order appropriately.
            if (result) {
                SetWindowPos(winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            } else {
                uState = SHAppBarMessage(ABM_GETSTATE, &abd);

                if (uState & ABS_ALWAYSONTOP) {
                    SetWindowPos(winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                }
            }

        case ABN_POSCHANGED:
            // The taskbar or another appbar has changed its
            // size or position.
//            appBarPosChanged(&abd);
            break;
    }
}

void PanelView::appBarPosChanged(PAPPBARDATA pabd)
{
}

bool PanelView::winEvent(MSG *message, long *result)
{
    if (message->message == APPBAR_CALLBACK) {
        appBarCallback(message, result);
        return true;
    }

    return false;
}

