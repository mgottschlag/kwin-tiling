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

#ifndef __taskbar_h__
#define __taskbar_h__

#include <kpanelextension.h>
#include <taskmanager.h>

#include "taskcontainer.h"
#include "panner.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>

#define WINDOWLISTBUTTON_SIZE 15
#define BUTTON_MAX_WIDTH 200
#define BUTTON_MIN_WIDTH 20

class Startup;
class Task;
class KActionCollection;
class KShadowEngine;
class KWindowListMenu;
class SimpleButton;
class QBoxLayout;

class KDE_EXPORT TaskBar : public QWidget, public DCOPObject
{
    Q_OBJECT
    K_DCOP

public:
    TaskBar( QWidget *parent = 0 );
    ~TaskBar();

    QSize sizeHint() const;
    QSize sizeHint( Plasma::Position, QSize maxSize ) const;

    void setOrientation( Qt::Orientation );
    void setArrowType( Qt::ArrowType at );

    int containerCount() const;
    int taskCount() const;
    int showScreen() const;

    bool showIcon() const { return m_showIcon; }
    bool sortByDesktop() const { return m_sortByDesktop; }
    bool showAllWindows() const { return m_showAllWindows; }

    void drawShadowText(QPainter  &p, QRect tr, int tf, const QString & str,
                        int len = -1, QRect* brect = 0);
    void orientationChange( Qt::Orientation );
    void popupDirectionChange( Plasma::Position );
    void preferences();

k_dcop:
    void configChanged();

public Q_SLOTS:
    void configure();
    void showWindowListMenu();
    void windowListMenuAboutToHide();
    void reconnectWindowListButton();

Q_SIGNALS:
    void containerCountChanged();

protected Q_SLOTS:
    void add(Task::TaskPtr);
    void add(Startup::StartupPtr);
    void showTaskContainer(TaskContainer*);
    void remove(Task::TaskPtr task, TaskContainer *container = 0);
    void remove(Startup::StartupPtr startup, TaskContainer *container = 0);

    void desktopChanged( int );
    void windowChanged(Task::TaskPtr);
    void windowChangedGeometry(Task::TaskPtr);

    void publishIconGeometry();

    void activateNextTask( bool forward );
    void slotActivateNextTask();
    void slotActivatePreviousTask();
    void slotSettingsChanged(int);
    void reLayout();

protected:
    void wheelEvent(QWheelEvent*);
    void resizeEvent( QResizeEvent* );
    bool idMatch( const QString& id1, const QString& id2 );
    TaskContainer::List filteredContainers();

private:
    void sortContainersByDesktop(TaskContainer::List& list);

    bool			blocklayout;
    bool			m_showAllWindows;
    // The screen to show, -1 for all screens
    int 			m_currentScreen;
    bool			m_showOnlyCurrentScreen;
    bool			m_sortByDesktop;
    bool			m_showIcon;
    Qt::ArrowType 		arrowType;
    TaskContainer::List		containers;
    TaskContainer::List		m_hiddenContainers;
    PixmapList			frames;
    int                         maximumButtonsWithoutShrinking() const;
    bool                        shouldGroup() const;
    bool                        isGrouping;
    void                        reGroup();
    KActionCollection*          keys;
    KShadowEngine* m_textShadowEngine;
    Plasma::Position            m_direction;
    bool                        m_showWindowListButton;
    QBoxLayout *                m_layout;
    SimpleButton *              m_windowListButton;
    KWindowListMenu *           m_windowListMenu;
    Panner*			m_container;
};

#endif
