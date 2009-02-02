/****************************************************************************

 KHotKeys

 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef WINDOWSELECTOR_H
#define WINDOWSELECTOR_H

#include <QWidget>

namespace KHotKeys
{

class WindowSelector : public QWidget
    {
    Q_OBJECT

    public:
        WindowSelector( QObject* receiver, const char* slot );
        void select();
    protected:
        virtual bool x11Event( XEvent* e );
    Q_SIGNALS:
        void selected_signal( WId w );
    private:
        WId findRealWindow( WId w, int depth = 0 );
    };

} // namespace KHotKeys

#endif
