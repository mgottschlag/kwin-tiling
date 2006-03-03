/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __taskbarextension_h__
#define __taskbarextension_h__

#include <qimage.h>
//Added by qt3to4:
#include <QPixmap>
#include <QResizeEvent>

#include <dcopobject.h>
#include <kpanelextension.h>

class KRootPixmap;
class TaskBar;

class TaskBarExtension : public KPanelExtension, virtual public DCOPObject
{
    Q_OBJECT
    K_DCOP

k_dcop:
    int panelSize() { return sizeInPixels(); }
    int panelOrientation() { return static_cast<int>(orientation()); }
    int panelPosition() { return static_cast<int>(position()); }
    void setPanelSize(int size) { setSize(static_cast<Plasma::Size>(size),true);}///slotSetSize(size); }

public:
    TaskBarExtension(const QString& configFile, int actions = 0, QWidget *parent = 0);
    ~TaskBarExtension();

    QSize sizeHint( Plasma::Position, QSize maxSize ) const;
    Plasma::Position preferedPosition() const { return Plasma::Bottom; }

protected:
    void positionChange( Plasma::Position );
    void preferences();

private:
    TaskBar* m_container;
};

#endif
