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

#include <qlayout.h>
#include <qtimer.h>
#include <qmatrix.h>
//Added by qt3to4:
#include <QPixmap>
#include <QFrame>
#include <QHBoxLayout>
#include <QResizeEvent>

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <krootpixmap.h>
#include <kstandarddirs.h>

#include "utils.h"
#include "kickerSettings.h"
#include "taskbar.h"

#include "taskbarextension.h"
#include "taskbarextension.moc"

extern "C"
{
    KDE_EXPORT KPanelExtension* init( QWidget *parent, const QString& configFile )
    {
        KGlobal::locale()->insertCatalog( "taskbarextension" );
        return new TaskBarExtension(configFile, Plasma::Preferences, parent);
    }
}

TaskBarExtension::TaskBarExtension(const QString& configFile,
                                   int actions, QWidget *parent)
    : KPanelExtension(configFile, actions, parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    m_container = new TaskBar(this);
    positionChange(position());
    layout->addWidget(m_container);

    connect(m_container, SIGNAL(containerCountChanged()),
            SIGNAL(updateLayout()));
}

TaskBarExtension::~TaskBarExtension()
{
    KGlobal::locale()->removeCatalog( "taskbarextension" );
}

void TaskBarExtension::positionChange( Plasma::Position p )
{

    m_container->orientationChange(orientation());

    switch (p)
    {
    case Plasma::Bottom:
        m_container->popupDirectionChange(Plasma::Up);
        break;
    case Plasma::Top:
        m_container->popupDirectionChange(Plasma::Down);
        break;
    case Plasma::Right:
        m_container->popupDirectionChange(Plasma::Left);
        break;
    case Plasma::Left:
        m_container->popupDirectionChange(Plasma::Right);
        break;
    case Plasma::Floating:
        if (orientation() == Qt::Horizontal)
        {
            m_container->popupDirectionChange(Plasma::Down);
        }
        else if (QApplication::isRightToLeft())
        {
            m_container->popupDirectionChange(Plasma::Left);
        }
        else
        {
            m_container->popupDirectionChange(Plasma::Right);
        }
        break;
    }
}

void TaskBarExtension::preferences()
{
    m_container->preferences();
}

QSize TaskBarExtension::sizeHint(Plasma::Position p, QSize maxSize) const
{
    if (p == Plasma::Left || p == Plasma::Right)
        maxSize.setWidth(sizeInPixels());
    else
        maxSize.setHeight(sizeInPixels());

//    kDebug(1210) << "TaskBarExtension::sizeHint( Position, QSize )" << endl;
//    kDebug(1210) << " width: " << size.width() << endl;
//    kDebug(1210) << "height: " << size.height() << endl;
    return m_container->sizeHint(p, maxSize);
}

