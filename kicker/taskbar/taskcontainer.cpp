/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>
Copyright (c) 2002 John Firebaugh <jfirebaugh@kde.org>
Copyright (c) 2005 Aaron Seigo <aseigo@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.#

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <assert.h>

#include <qbitmap.h>
#include <QColor>
#include <qcursor.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <QToolTip>
#include <q3tl.h>
#include <QStyleOption>
#include <QDragLeaveEvent>
#include <QPaintEvent>
#include <QEvent>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QTextDocument>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <kimageeffect.h>
#include <kauthorized.h>

#include "utils.h"
#include "kickerSettings.h"
#include "taskbar.h"
#include "taskbarsettings.h"
#include "tasklmbmenu.h"
#include "taskrmbmenu.h"

#include "taskcontainer.h"
#include "taskcontainer.moc"

QImage TaskContainer::blendGradient = QImage();

TaskContainer::TaskContainer(Task::TaskPtr task, TaskBar* bar,
                             QWidget *parent)
    : QToolButton(parent),
      currentFrame(0),
      attentionState(-1),
      lastActivated(0),
      m_menu(0),
      m_startup(0),
      arrowType(Qt::UpArrow),
      taskBar(bar),
      discardNextMouseEvent(false),
      aboutToActivate(false),
      m_mouseOver(false)
{
    init();
    setAcceptDrops(true); // Always enabled to activate task during drag&drop.

    add(task);

    // we abuse this timer once to get shown
    // no point in having another timer just for this, and
    // a single shot won't do because we need to stop the timer
    // in case our task is deleted out from under us
    dragSwitchTimer.setSingleShot(true);
    dragSwitchTimer.start(0);
}

TaskContainer::TaskContainer(Startup::StartupPtr startup, PixmapList& startupFrames,
                             TaskBar* bar, QWidget *parent)
    : QToolButton(parent),
      currentFrame(0),
      frames(startupFrames),
      attentionState(-1),
      lastActivated(0),
      m_menu(0),
      m_startup(startup),
      arrowType(Qt::LeftArrow),
      taskBar(bar),
      discardNextMouseEvent(false),
      aboutToActivate(false),
      m_mouseOver(false)
{
    init();
    setEnabled(false);

    sid = m_startup->bin();

    connect(m_startup.data(), SIGNAL(changed()), SLOT(update()));

    dragSwitchTimer.setSingleShot(true);
    dragSwitchTimer.start(333);
}

void TaskContainer::init()
{
    setAttribute(Qt::WA_NoSystemBackground, true);
    animBg = QPixmap(16, 16);

    installEventFilter(KickerTip::self());

    connect(&animationTimer, SIGNAL(timeout()), SLOT(animationTimerFired()));
    connect(&dragSwitchTimer, SIGNAL(timeout()), SLOT(showMe()));
    connect(&attentionTimer, SIGNAL(timeout()), SLOT(attentionTimerFired()));
}

TaskContainer::~TaskContainer()
{
    stopTimers();
}

void TaskContainer::showMe()
{
    if(!frames.isEmpty() && taskBar->showIcon())
        animationTimer.start(100);

    emit showMe(this);
    disconnect(&dragSwitchTimer, SIGNAL(timeout()), this, SLOT(showMe()));
    connect(&dragSwitchTimer, SIGNAL(timeout()), SLOT(dragSwitch()));
}

void TaskContainer::stopTimers()
{
    animationTimer.stop();
    dragSwitchTimer.stop();
    attentionTimer.stop();
}

void TaskContainer::taskChanged()
{
    const QObject* source = sender();
    Task::TaskPtr task;
    Task::List::const_iterator itEnd = tasks.constEnd();
    for (Task::List::const_iterator it = tasks.constBegin(); it != itEnd; ++it)
    {
        if ((*it).data() == source)
        {
            task = *it;
            break;
        }
    }

    if (task)
    {
        checkAttention(task);
    }

    KickerTip::Client::updateTip();
    update();
}

void TaskContainer::iconChanged()
{
    const QObject* source = sender();
    Task::TaskPtr task;
    Task::List::const_iterator itEnd = tasks.constEnd();
    for (Task::List::const_iterator it = tasks.constBegin(); it != itEnd; ++it)
    {
        if ((*it).data() == source)
        {
            task = *it;
            break;
        }
    }

    if (task && !m_filteredTasks.empty() && task != m_filteredTasks.first())
    {
        if (m_menu)
        {
            m_menu->update();
        }
        return;
    }
    QToolButton::update();
}

void TaskContainer::setLastActivated()
{
    Task::List::const_iterator itEnd = m_filteredTasks.constEnd();
    for (Task::List::const_iterator it = m_filteredTasks.constBegin(); it != itEnd; ++it)
    {
        Task::TaskPtr t = *it;
        if ( t->isActive() )
        {
            lastActivated = t;
            return;
        }
    }
    lastActivated = 0L;
}


void TaskContainer::animationTimerFired()
{
    if (!frames.isEmpty() && taskBar->showIcon() && frames.at(currentFrame) != frames.back())
    {
        QPixmap *pm = frames.at(currentFrame);
        QPainter p(this);
        // draw pixmap
        if ( pm && !pm->isNull() ) {
	    // we only have to redraw the background for frames 0, 8 and 9
	    if ( currentFrame == 0 || currentFrame > 7 ) {
		p.drawPixmap( iconRect.x(), iconRect.y(), animBg );
		p.drawPixmap( iconRect.x(), iconRect.y(), *pm );
    	    }
	    else
		p.drawPixmap( iconRect.x(), iconRect.y(), *pm );
	}

        // increment frame counter
        if ( currentFrame >= 9)
	    currentFrame = 0;
        else
	    currentFrame++;
    }
}

void TaskContainer::checkAttention(const Task::TaskPtr t)
{
    bool attention = t ? t->demandsAttention() : false;
    if (attention && attentionState == -1) // was activated
    {
        attentionState = 0;
        attentionTimer.start(500);
    }
    else if(!attention && attentionState >= 0)
    { // need to check all
        Task::List::iterator itEnd = tasks.end();
        for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
        {
            if ((*it)->demandsAttention())
            {
                attention = true;
                break;
            }
        }

        if (!attention)
        {
            attentionTimer.stop();
            attentionState = -1;
        }
    }
}

void TaskContainer::attentionTimerFired()
{
    assert( attentionState != -1 );
    if( attentionState < ATTENTION_BLINK_TIMEOUT )
        ++attentionState;
    else
        attentionTimer.stop();
    update();
}

QSizePolicy TaskContainer::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

void TaskContainer::resizeEvent( QResizeEvent * )
{
    // calculate the icon rect
    QStyleOption bOpt;
    bOpt.init(this);
    QRect br( style()->subElementRect( QStyle::SE_PushButtonContents, &bOpt, this ) );
    iconRect = QStyle::visualRect( layoutDirection(), rect(), QRect(br.x() + 2, (height() - 16) / 2, 16, 16) );
}

void TaskContainer::add(Task::TaskPtr task)
{
    if (!task)
    {
        return;
    }

    tasks.append(task);

    if (sid.isEmpty())
    {
        sid = task->classClass();
    }

    updateFilteredTaskList();
    checkAttention(task);

    KickerTip::Client::updateTip();
    update();

    connect(task.data(), SIGNAL(changed()), SLOT(taskChanged()));
    connect(task.data(), SIGNAL(iconChanged()), SLOT(iconChanged()));
    connect(task.data(), SIGNAL(activated()), SLOT(setLastActivated()));
}

void TaskContainer::remove(Task::TaskPtr task)
{
    if (!task)
    {
        return;
    }

    task->publishIconGeometry(QRect());
    for (Task::List::Iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        if ((*it) == task)
        {
            tasks.erase(it);
            break;
        }
    }

    updateFilteredTaskList();

    if (isEmpty())
    {
        stopTimers();
        return;
    }

    checkAttention();
    KickerTip::Client::updateTip();
    update();
}

void TaskContainer::remove(Startup::StartupPtr startup)
{
    if (!startup || startup != m_startup)
    {
        return;
    }

    m_startup = 0;
    animationTimer.stop();
    frames.clear();

    if (!tasks.isEmpty())
    {
        setEnabled(true);
    }
}

bool TaskContainer::contains(Task::TaskPtr task)
{
    if (!task)
    {
        return false;
    }

    for (Task::List::Iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        if ((*it) == task)
        {
            return true;
        }
    }

    return false;
}

bool TaskContainer::contains(Startup::StartupPtr startup)
{
    return startup && (m_startup == startup);
}

bool TaskContainer::contains(WId win)
{
    Task::List::iterator itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        if ((*it)->window() == win)
        {
            return true;
        }
    }

    return false;
}

bool TaskContainer::isEmpty()
{
    return (tasks.isEmpty() && !m_startup);
}

QString TaskContainer::id()
{
    return sid;
}

void TaskContainer::paintEvent(QPaintEvent*)
{
    QPixmap* pm = new QPixmap(size());

    if (!TaskBarSettings::drawButtons())
    {
        pm->fill(taskBar->palette().color(taskBar->backgroundRole()));
    }

    QPainter p;
    p.begin(pm);
    drawButton(&p);
    p.end();

    p.begin(this);
    p.drawPixmap(0, 0, *pm);
    p.end();
    delete pm;
}

void TaskContainer::drawButton(QPainter *p)
{
    if (isEmpty())
    {
        return;
    }

    // get a pointer to the pixmap we're drawing on
    QPixmap *pm((QPixmap*)p->device());
    QPixmap pixmap; // icon
    Task::TaskPtr task;
    bool iconified = !TaskBarSettings::showOnlyIconified();
    bool halo = TaskBarSettings::haloText();
    bool alwaysDrawButtons = TaskBarSettings::drawButtons();
    bool drawButton = alwaysDrawButtons ||
                      (m_mouseOver && isEnabled() &&
                       TaskBarSettings::showButtonOnHover());
    QFont font(KGlobalSettings::taskbarFont());

    // draw sunken if we contain the active task
    bool active = false;
    bool demandsAttention = false;
    Task::List::iterator itEnd = m_filteredTasks.end();
    for (Task::List::iterator it = m_filteredTasks.begin(); it != itEnd; ++it)
    {
        task = *it;
        if (iconified && !task->isIconified())
        {
            iconified = false;
        }

        if (task->isActive())
        {
            active = true;
        }

        if (task->demandsAttention())
        {
            demandsAttention = attentionState == ATTENTION_BLINK_TIMEOUT ||
                               attentionState % 2 == 0;
        }
    }

    font.setBold(active);

    QPalette pal = palette();
    if (demandsAttention)
    {
        if (!drawButton)
        {
            halo = true;

            QRect r = rect();
            QColor line = pal.color(QPalette::Highlight);
            r.adjust(2, 2, -2, -2);
            p->fillRect(r, line);
            for (int i = 0; i < 2; ++i)
            {
                line = Plasma::blendColors(line, pal.color(QPalette::Background));
                p->setPen(QPen(line, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                r.adjust(-1, -1, 1, 1);
                p->drawRect(r);
            }
        }

        // blink until blink timeout, then display differently without blinking
        pal.setColor( QPalette::Button,    pal.color(QPalette::Highlight) );
        pal.setColor( QPalette::Background,pal.color(QPalette::Highlight) );
        pal.setColor( QPalette::ButtonText,pal.color(QPalette::HighlightedText) );
        pal.setColor( QPalette::Text,      pal.color(QPalette::HighlightedText) );
    }

    if (active || aboutToActivate)
    {
        pal.setColor(QPalette::Button, pal.color(QPalette::Button).dark(110));
    }

    // get the task icon
    if (task)
    {
        pixmap = task->pixmap();
    }

    bool sunken = isDown() || (alwaysDrawButtons && (active || aboutToActivate));
    bool reverse = QApplication::isRightToLeft();

    QStyleOption bOpt;
    bOpt.init(this);
    QRect br( style()->subElementRect( QStyle::SE_PushButtonContents, &bOpt, this ) );
    QPoint shift = QPoint(style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal),
                          style()->pixelMetric(QStyle::PM_ButtonShiftVertical));

    // draw button background
    if (drawButton)
    {
        QStyleOptionHeader hOpt;
        hOpt.init(this);
        hOpt.palette = pal;
        //### SectionPosition?
        style()->drawControl(QStyle::CE_HeaderSection, &hOpt, p, this);
    }

    // shift button label on sunken buttons
    if (sunken)
    {
        p->translate(shift.x(), shift.y());
    }

    if (taskBar->showIcon())
    {
        if (pixmap.isNull() && m_startup)
        {
            pixmap = SmallIcon(m_startup->icon());
        }

        if ( !pixmap.isNull() )
        {
            // make sure it is no larger than 16x16
            if ( pixmap.width() > 16 || pixmap.height() > 16 )
            {
                pixmap = pixmap.scaled( 16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
            }

            // fade out the icon when minimized
            if (iconified)
            {
                KIconEffect::semiTransparent( pixmap );
            }

            // draw icon
            QRect pmr(0, 0, pixmap.width(), pixmap.height());
            pmr.moveCenter(iconRect.center());
            p->drawPixmap(pmr, pixmap);
        }
    }

    // find text
    QString text = name();

    // modified overlay
    static QString modStr = "[" + i18n( "modified" ) + "]";
    int modStrPos = text.indexOf( modStr );
    int textPos = ( taskBar->showIcon() && !pixmap.isNull() ) ? 2 + 16 + 2 : 0;

    if (modStrPos >= 0)
    {
        // +1 because we include a space after the closing brace.
        text.remove(modStrPos, modStr.length() + 1);
        QPixmap modPixmap = SmallIcon("modified");

        // draw modified overlay
        if (!modPixmap.isNull())
        {
            QRect r = QStyle::visualRect(layoutDirection(), rect(),
                                         QRect(br.x() + textPos,
                                               (height() - 16) / 2, 16, 16));
            if (iconified)
            {
                KIconEffect::semiTransparent(modPixmap);
            }

            p->drawPixmap(r, modPixmap);
            textPos += 16 + 2;
        }
    }

    // draw text
    if (!text.isEmpty())
    {
        QRect tr = QStyle::visualRect(layoutDirection(), rect(),
                                      QRect(br.x() + textPos + 1, 0,
                                            width() - textPos, height()));
        int textFlags = Qt::AlignVCenter | Qt::TextSingleLine;
        textFlags |= reverse ? Qt::AlignRight : Qt::AlignLeft;
        QPen textPen;

        // get the color for the text label
        if (iconified)
        {
            textPen = QPen(Plasma::blendColors(pal.color(QPalette::Button), pal.color(  QPalette::ButtonText)));
        }
        else if (!active)
        {
            textPen = QPen(pal.color(QPalette::ButtonText));
        }
        else // hack for the dotNET style and others
        {
            textPen = p->pen();
        }

        int availableWidth = width() - (br.x() * 2) - textPos;
        if (m_filteredTasks.count() > 1)
        {
            availableWidth -= 8;
        }

        if (QFontMetrics(font).width(text) > availableWidth)
        {
            if (blendGradient.isNull() || blendGradient.size() != size())
            {
                QPixmap bgpm(size());
                QPainter bgp(&bgpm);
                bgpm.fill(Qt::black);

                if (reverse)
                {
                    QImage gradient = KImageEffect::gradient(
                                            QSize(30, height()),
                                            QColor(255,255,255),
                                            QColor(0,0,0),
                                            KImageEffect::HorizontalGradient);
                    bgp.drawImage(0, 0, gradient);
                }
                else
                {
                    QImage gradient = KImageEffect::gradient(
                                            QSize(30, height()),
                                            QColor(0,0,0),
                                            QColor(255,255,255),
                                            KImageEffect::HorizontalGradient);
                    bgp.drawImage(width() - 30, 0, gradient);
                }

                blendGradient = bgpm.toImage();
            }

            // draw text into overlay pixmap
            QPixmap tpm(*pm);
            QPainter tp(&tpm);

            if (sunken)
            {
                tp.translate(shift.x(), shift.y());
            }

            tp.setFont(font);
            tp.setPen(textPen);

            if (halo)
            {
                taskBar->drawShadowText(tp, tr, textFlags, text);
            }
            else
            {
                tp.drawText(tr, textFlags, text);
            }

            // blend text into background image
            QImage img = pm->toImage();
            QImage timg = tpm.toImage();
            KImageEffect::blend(img, timg, blendGradient, KImageEffect::Red);
            p->drawImage(0, 0, img);
        }
        else
        {
            p->setFont(font);
            p->setPen(textPen);

            if (halo)
            {
                taskBar->drawShadowText(*p, tr, textFlags, text);
            }
            else
            {
                p->drawText(tr, textFlags, text);
            }
        }
    }

    if (!frames.isEmpty() && m_startup && frames.at(currentFrame) != frames.back())
    {
        QPixmap *anim = frames.at(currentFrame);

        if (anim && !anim->isNull())
        {
            // save the background for the other frames
            QPainter pAnim(&animBg);
            pAnim.drawPixmap(QPoint(0, 0), *pm, iconRect);
            // draw the animation frame
            p->drawPixmap(iconRect.x(), iconRect.y(), *anim);
        }
    }

    if (sunken)
    {
        // Change the painter back so the arrow, etc gets drawn in the right location
        p->translate(-shift.x(), -shift.y());
    }

    // draw popup arrow
    if (m_filteredTasks.count() > 1)
    {
        QStyle::PrimitiveElement e = QStyle::PE_IndicatorArrowLeft;

        switch (arrowType)
        {
            case Qt::LeftArrow:
                e = QStyle::PE_IndicatorArrowLeft;
            break;
            case Qt::RightArrow:
                e = QStyle::PE_IndicatorArrowRight;
            break;
            case Qt::UpArrow:
                e = QStyle::PE_IndicatorArrowUp;
            break;
            case Qt::DownArrow:
                e = QStyle::PE_IndicatorArrowDown;
            break;
            case Qt::NoArrow:
            default:
                // do nothing to make the compiler be quiet! =)
            break;
        }

        QStyle::State flags = QStyle::State_Enabled;
        QRect ar = QStyle::visualRect(layoutDirection(), rect(),
                                      QRect(br.x() + br.width() - 8 - 2,
                                            br.y(), 8, br.height()));
        if (sunken)
        {
            flags |= QStyle::State_Sunken;
        }

        QStyleOption opt;
        opt.init(this);
        opt.state   = flags;
        opt.rect    = ar;
        opt.palette = pal;
        style()->drawPrimitive(e, &opt, p, this);
    }

    if (aboutToActivate)
    {
        aboutToActivate = false;
    }
}

QString TaskContainer::name()
{
    // default to container id
    QString text;

    // single task -> use mainwindow caption
    if (m_filteredTasks.count() == 1)
    {
        text = m_filteredTasks.first()->visibleName();
    }
    else if (m_filteredTasks.count() > 1)
    {
        // multiple tasks -> use the common part of all captions
        // if it is more descriptive than the class name
        const QString match = m_filteredTasks.first()->visibleName();
        int maxLength = match.length();
        int i = 0;
        bool stop = false;

        // what we do is find the right-most letter than the names do NOT have
        // in common, and then use everything UP TO that as the name in the button
        while (i < maxLength)
        {
            QChar check = match.at(i).toLower();
            Task::List::iterator itEnd = m_filteredTasks.end();
            for (Task::List::iterator it = m_filteredTasks.begin(); it != itEnd; ++it)
            {
                // we're doing a lot of Utf8 -> QString conversions here
                // by repeatedly calling visibleName() =/
                QString matchAgainst = (*it)->visibleName();
                if (i >= matchAgainst.length() ||
                    check != matchAgainst.at(i).toLower())
                {
                    if (i > 0)
                    {
                        --i;
                    }
                    stop = true;
                    break;
                }
            }

            if (stop)
            {
                break;
            }

            ++i;
        }

        // strip trailing crap
        while (i > 0 && !match.at(i).isLetterOrNumber())
        {
            --i;
        }

        // more descriptive than id()?
        if (i > 0 && (i + 1) >= id().length())
        {
            text = match.left(i + 1);
        }
    }
    else if (m_startup && !m_startup->text().isEmpty())
    {
        // fall back to startup name
        text = m_startup->text();
    }

    if (text.isEmpty())
    {
        text = id();

        // Upper case first letter: seems to be the right thing to do for most cases
        text[0] = text[0].toUpper();
    }

    if (m_filteredTasks.count() > 1)
    {
        // this is faster than (" [%1]").arg() or +
        // and it's as fast as using append, but cleaner looking
        text += " [";
        text += QString::number(m_filteredTasks.count());
        text += "]";
    }

    return text;
}

void TaskContainer::mousePressEvent( QMouseEvent* e )
{
    if (discardNextMouseEvent)
    {
        discardNextMouseEvent = false;
        return;
    }

    if (e->button() == Qt::LeftButton)
    {
        m_dragStartPos = e->pos();
    }
    else
    {
        m_dragStartPos = QPoint();
    }

    int buttonAction = 0;

    // On left button, only do actions that invoke a menu.
    // Other actions will be handled in mouseReleaseEvent
    switch (e->button())
    {
        case Qt::LeftButton:
            buttonAction = TaskBarSettings::action(TaskBarSettings::LeftButton);
            break;
        case Qt::MidButton:
            buttonAction = TaskBarSettings::action(TaskBarSettings::MiddleButton);
            break;
        case Qt::RightButton:
        default:
            buttonAction = TaskBarSettings::action(TaskBarSettings::RightButton);
            break;
    }

    if ((buttonAction == TaskBarSettings::ShowTaskList &&
          m_filteredTasks.count() > 1) ||
        buttonAction == TaskBarSettings::ShowOperationsMenu)
    {
        performAction(buttonAction);
    }
}

void TaskContainer::mouseReleaseEvent(QMouseEvent *e)
{
    m_dragStartPos = QPoint();

    // This is to avoid the flicker caused by redrawing the
    // button as unpressed just before it's activated.
    if (!rect().contains(e->pos()))
    {
        QToolButton::mouseReleaseEvent(e);
        return;
    }

    int buttonAction = 0;

    switch (e->button())
    {
        case Qt::LeftButton:
            buttonAction = TaskBarSettings::action(TaskBarSettings::LeftButton);
            break;
        case Qt::MidButton:
            buttonAction = TaskBarSettings::action(TaskBarSettings::MiddleButton);
            break;
        case Qt::RightButton:
        default:
            buttonAction = TaskBarSettings::action(TaskBarSettings::RightButton);
            break;
    }

    if ((buttonAction == TaskBarSettings::ShowTaskList &&
         m_filteredTasks.count() > 1) ||
        buttonAction == TaskBarSettings::ShowOperationsMenu)
    {
        return;
    }

    if (buttonAction == TaskBarSettings::ActivateRaiseOrMinimize ||
        buttonAction == TaskBarSettings::Activate)
    {
        aboutToActivate = true;
    }

    performAction(buttonAction);
    QTimer::singleShot(0, this, SLOT(update()));
}

void TaskContainer::performAction(int action)
{
    if (m_filteredTasks.isEmpty())
    {
        return;
    }

    switch( action ) {
    case TaskBarSettings::ShowTaskList:
	// If there is only one task, the correct behavior is
	// to activate, raise, or iconify it, not show the task menu.
	if( m_filteredTasks.count() > 1 ) {
            popupMenu( TaskBarSettings::ShowTaskList );
	} else {
            performAction( TaskBarSettings::ActivateRaiseOrMinimize );
	}
	break;
    case TaskBarSettings::ShowOperationsMenu:
        popupMenu( TaskBarSettings::ShowOperationsMenu );
	break;
    case TaskBarSettings::ActivateRaiseOrMinimize:
    if (m_filteredTasks.isEmpty())
    {
        break;
    }
    if (m_filteredTasks.count() == 1)
    {
        m_filteredTasks.first()->activateRaiseOrIconify();
    }
    else
    {
        // multiple tasks -> cycle list
        bool hasLastActivated = false;
        Task::List::iterator itEnd = m_filteredTasks.end();
        for (Task::List::iterator it = m_filteredTasks.begin(); it != itEnd; ++it)
        {
            if ((*it) == lastActivated)
            {
                hasLastActivated = true;
            }

            if ((*it)->isActive())
            {
                // activate next
                ++it;
                if (it == itEnd)
                {
                    it = m_filteredTasks.begin();
                }
                (*it)->activateRaiseOrIconify();
                return;
            }
        }

        if (hasLastActivated)
        {
            lastActivated->activateRaiseOrIconify();
        }
        else
        {
            m_filteredTasks[0]->activateRaiseOrIconify();
        }
    }
    break;
    case TaskBarSettings::Activate:
        m_filteredTasks.first()->activate();
    break;
    case TaskBarSettings::Raise:
        m_filteredTasks.first()->raise();
    break;
    case TaskBarSettings::Lower:
        m_filteredTasks.first()->lower();
    break;
    case TaskBarSettings::Minimize:
        m_filteredTasks.first()->toggleIconified();
    break;
    case TaskBarSettings::ToCurrentDesktop:
        m_filteredTasks.first()->toCurrentDesktop();
    break;
    default:
        kWarning(1210) << "Unknown taskbar action!" << endl;
        break;
    }
}

// forcenext == true means the last entry in the previous
// taskcontainer was active -> activate first
bool TaskContainer::activateNextTask(bool forward, bool& forcenext)
{
    if (forcenext)
    {
        if (m_filteredTasks.isEmpty())
        {
            return false;
        }

        if (forward)
        {
            m_filteredTasks.first()->activate();
        }
        else
        {
            m_filteredTasks.last()->activate();
        }

        forcenext = false;
        return true;
    }

    Task::List::iterator itEnd = m_filteredTasks.end();
    for (Task::List::iterator it = m_filteredTasks.begin();
         it != itEnd;
         ++it)
    {
        if ((*it)->isActive())
        {
            if (forward)
            {
                ++it;
                if (it == itEnd)
                {
                    forcenext = true;
                    return false;
                }

                (*it)->activate();
                return true;
            }
            else if (it == m_filteredTasks.begin())
            {
                forcenext = true;
                return false;
            }

            --it;
            (*it)->activate();
            return true;
        }
    }

    return false;
}

void TaskContainer::popupMenu(int action)
{
    if (action == TaskBarSettings::ShowTaskList )
    {
        m_menu = new TaskLMBMenu(m_filteredTasks);
    }
    else if (action == TaskBarSettings::ShowOperationsMenu)
    {
        if (!KAuthorized::authorizeKAction("kwin_rmb"))
        {
            return;
        }

        m_menu = new TaskRMBMenu(m_filteredTasks, taskBar->showAllWindows());
    }
    else
    {
        return;
    }

    // calc popup menu position
    QPoint pos(mapToGlobal(QPoint(0, 0)));

    switch( arrowType )
    {
        case Qt::RightArrow:
            pos.setX(pos.x() + width());
            break;
        case Qt::LeftArrow:
            pos.setX(pos.x() - m_menu->sizeHint().width());
            break;
        case Qt::DownArrow:
            if ( QApplication::isRightToLeft() )
                pos.setX( pos.x() + width() - m_menu->sizeHint().width() );
            pos.setY( pos.y() + height() );
            break;
        case Qt::UpArrow:
            if ( QApplication::isRightToLeft() )
                pos.setX( pos.x() + width() - m_menu->sizeHint().width() );
            pos.setY(pos.y() - m_menu->sizeHint().height());
            break;
        default:
            break;
    }
    m_menu->installEventFilter( this );
    m_menu->exec( pos );

    delete m_menu;
    m_menu = 0;
}

void TaskContainer::mouseMoveEvent( QMouseEvent* e )
{
    kDebug() << "regular move" << endl;
    if (!m_dragStartPos.isNull())
    {
        startDrag(e->pos());
    }

    QToolButton::mouseMoveEvent(e);
}

bool TaskContainer::startDrag(const QPoint& pos)
{
    if (m_filteredTasks.count() != 1)
    {
        return false;
    }

    int delay = KGlobalSettings::dndEventDelay();

    if ((m_dragStartPos - pos).manhattanLength() > delay)
    {
        if (!m_filteredTasks.first()->isActive())
        {
            setDown(false);
        }

        TaskDrag* drag = new TaskDrag(m_filteredTasks, this);

        if (!m_filteredTasks.isEmpty())
        {
            kDebug() << m_filteredTasks.first()->name() << endl;
            drag->setPixmap(m_filteredTasks.first()->pixmap());
        }

        drag->start();
        return true;
    }

    return false;
}

// This is the code that gives us the proper behavior
// when a popup menu is displayed and we are clicked:
// close the menu, and don't reopen it immediately.
// It's copied from QToolButton. Unfortunately Qt is lame
// as usual and makes interesting stuff private or
// non-virtual, so we have to copy code.
bool TaskContainer::eventFilter(QObject *o, QEvent *e)
{
    switch ( e->type() )
    {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonDblClick:
        {
            QMouseEvent *me = (QMouseEvent*)e;
            QPoint p = me->globalPos();
            if ( QApplication::widgetAt( p ) == this )
            {
                if (me->type() == QEvent::MouseButtonPress &&
                    me->button() == Qt::LeftButton)
                {
                    m_dragStartPos = mapFromGlobal(p);
                }

                discardNextMouseEvent = true;
            }
            break;
        }
        case QEvent::MouseButtonRelease:
        {
            m_dragStartPos = QPoint();
            break;
        }
        case QEvent::MouseMove:
        {
            if (!m_dragStartPos.isNull())
            {
                QMouseEvent* me = static_cast<QMouseEvent*>(e);
                QPoint p(me->globalPos());

                if (me->buttons() & Qt::LeftButton &&
                    QApplication::widgetAt(p) == this)
                {
                    kDebug() << "event move" << endl;
                    if (startDrag(mapFromGlobal(p)))
                    {
                        QMenu* menu = dynamic_cast<QMenu*>(o);

                        if (menu)
                        {
                            menu->hide();
                        }
                    }
                }
            }
            break;
        }

        default:
        break;
    }

    return QToolButton::eventFilter( o, e );
}

void TaskContainer::setArrowType( Qt::ArrowType at )
{
    if (arrowType == at)
    {
        return;
    }

    arrowType = at;
    repaint();
}

void TaskContainer::publishIconGeometry( QPoint global )
{
    QPoint p = global + geometry().topLeft();

    Task::List::const_iterator itEnd = tasks.constEnd();
    for (Task::List::const_iterator it = tasks.constBegin(); it != itEnd; ++it)
    {
        Task::TaskPtr t = *it;
        t->publishIconGeometry(QRect(p.x(), p.y(), width(), height()));
    }
}

void TaskContainer::dragEnterEvent( QDragEnterEvent* e )
{
    // ignore task drags
    if (TaskDrag::canDecode(e->mimeData()))
    {
        return;
    }

    // if a dragitem is held for over a taskbutton for two seconds,
    // activate corresponding window
    if (m_filteredTasks.isEmpty())
    {
        return;
    }

    if (!m_filteredTasks.first()->isActive() ||
        m_filteredTasks.count() > 1)
    {
        dragSwitchTimer.setSingleShot(true);
        dragSwitchTimer.start(1000);
    }

    QToolButton::dragEnterEvent( e );
}

void TaskContainer::dragLeaveEvent( QDragLeaveEvent* e )
{
    dragSwitchTimer.stop();

    QToolButton::dragLeaveEvent( e );
}

void TaskContainer::enterEvent(QEvent* e)
{
    this->setToolTip("");
    m_mouseOver = true;
    update();

    if (tasks.isEmpty())
    {
        QToolButton::enterEvent(e);
        return;
    }

    if (!KickerSettings::showMouseOverEffects())
    {
        QString tooltip = "<qt>" + Qt::escape(name()) + "</qt>";
        this->setToolTip( tooltip);
        QToolButton::enterEvent(e);
        return;
    }

    QToolButton::enterEvent(e);
}

void TaskContainer::leaveEvent(QEvent* e)
{
    m_mouseOver = false;
    update();

    QToolButton::leaveEvent(e);
}

void TaskContainer::dragSwitch()
{
    if (m_filteredTasks.isEmpty())
    {
        return;
    }

    if (m_filteredTasks.count() == 1)
    {
        m_filteredTasks.first()->activate();
    }
    else
    {
        popupMenu(TaskBarSettings::ShowTaskList);
    }
}

int TaskContainer::desktop()
{
    if ( tasks.isEmpty() )
        return TaskManager::self()->currentDesktop();

    if ( tasks.count() > 1 )
        return TaskManager::self()->numberOfDesktops();

    return tasks.first()->desktop();
}

bool TaskContainer::onCurrentDesktop()
{
    if (m_startup)
    {
        return true;
    }

    if (m_filteredTasks.isEmpty())
    {
        return false;
    }

    Task::List::const_iterator itEnd = tasks.constEnd();
    for (Task::List::const_iterator it = tasks.constBegin(); it != itEnd; ++it)
    {
        Task::TaskPtr t = *it;
        if (t->isOnCurrentDesktop())
        {
            return true;
        }
    }

    return false;
}

bool TaskContainer::isOnScreen()
{
    if (isEmpty())
    {
        return false;
    }

    int screen = taskBar->showScreen();
    if ((tasks.isEmpty() && m_startup) || screen == -1)
    {
        return true;
    }

    Task::List::iterator itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        if ((*it)->isOnScreen( screen ))
        {
            return true;
        }
    }

    return false;
}

bool TaskContainer::isIconified()
{
    if (isEmpty())
    {
        return false;
    }

    if (tasks.isEmpty() && m_startup)
    {
        return true;
    }

    Task::List::const_iterator itEnd = tasks.constEnd();
    for (Task::List::const_iterator it = tasks.constBegin(); it != itEnd; ++it)
    {
        if ((*it)->isIconified())
        {
            return true;
        }
    }

    return false;
}

void TaskContainer::updateFilteredTaskList()
{
    m_filteredTasks.clear();

    Task::List::const_iterator itEnd = tasks.constEnd();
    for (Task::List::const_iterator it = tasks.constBegin(); it != itEnd; ++it)
    {
        Task::TaskPtr t = *it;
        if ((taskBar->showAllWindows() || t->isOnCurrentDesktop()) &&
            (!TaskBarSettings::showOnlyIconified() || t->isIconified()))
        {
            m_filteredTasks.append(t);
        }
        else
        {
            t->publishIconGeometry( QRect());
        }
    }

    // sort container list by desktop
    if (taskBar->sortByDesktop() && m_filteredTasks.count() > 1)
    {
        QVector<QPair<int, Task::TaskPtr> > sorted;
        sorted.resize(m_filteredTasks.count());
        int i = 0;

        Task::List::const_iterator itEnd = m_filteredTasks.constEnd();
        for (Task::List::const_iterator it = m_filteredTasks.constBegin(); it != itEnd; ++it)
        {
            Task::TaskPtr t = *it;
            sorted[i] = (qMakePair(t->desktop(), t));
            ++i;
        }

        qHeapSort(sorted);

        m_filteredTasks.clear();
        for (QVector<QPair<int, Task::TaskPtr> >::iterator it = sorted.begin();
             it != sorted.end();
             ++it)
        {
            m_filteredTasks.append((*it).second);
        }
    }
}

void TaskContainer::desktopChanged(int)
{
    updateFilteredTaskList();
    update();
}

void TaskContainer::windowChanged(Task::TaskPtr)
{
    updateFilteredTaskList();
    update();
}

void TaskContainer::settingsChanged()
{
    updateFilteredTaskList();
    update();
}

void TaskContainer::updateTipData(KickerTip::Data& data)
{
    if (m_startup)
    {
        data.message = m_startup->text();
        data.subtext = i18n("Loading application ...");
        data.icon = KGlobal::iconLoader()->loadIcon(m_startup->icon(),
                                                    K3Icon::Small,
                                                    K3Icon::SizeMedium,
                                                    K3Icon::DefaultState,
                                                    0, true);
        return;
    }

    QPixmap pixmap;
    if (TaskBarSettings::showThumbnails() &&
        m_filteredTasks.count() == 1)
    {
        Task::TaskPtr t = m_filteredTasks.first();

        pixmap = t->thumbnail(TaskBarSettings::thumbnailMaxDimension());
    }

    if (pixmap.isNull() && !tasks.isEmpty())
    {
        // try to load icon via net_wm
        pixmap = KWin::icon(tasks.last()->window(),
                            K3Icon::SizeMedium,
                            K3Icon::SizeMedium,
                            true);
    }

    // Collect all desktops the tasks are on. Sort naturally.
    QMap<int, QString> desktopMap;
    bool demandsAttention = false;
    bool modified = false;
    bool allDesktops = false;
    Task::List::const_iterator itEnd = m_filteredTasks.constEnd();
    for (Task::List::const_iterator it = m_filteredTasks.constBegin(); it != itEnd; ++it)
    {
        Task::TaskPtr t = *it;
        if (t->demandsAttention())
        {
            demandsAttention = true;
        }

        if (t->isModified())
        {
            modified = true;
        }

        if (t->isOnAllDesktops())
        {
            allDesktops = true;
            desktopMap.clear();
        }
        else if (!allDesktops)
        {
            desktopMap.insert(t->desktop(),
                              TaskManager::self()->desktopName(t->desktop()));
        }
    }

    QString details;

    if (TaskBarSettings::showAllWindows() && KWin::numberOfDesktops() > 1)
    {
        if (desktopMap.isEmpty())
        {
            details.append(i18n("On all desktops"));
        }
        else
        {
            QStringList desktopNames = desktopMap.values();
            details.append(i18n("On %1", desktopNames.join(", ")) + "<br>");
        }
    }

    if (demandsAttention)
    {
        details.append(i18n("Requesting attention") + "<br>");
    }

    QString name = this->name();
    if (modified)
    {
        details.append(i18n("Has unsaved changes"));

        static QString modStr = "[" + i18n( "modified" ) + "]";
        int modStrPos = name.indexOf(modStr);

        if (modStrPos >= 0)
        {
            // +1 because we include a space after the closing brace.
            name.remove(modStrPos, modStr.length() + 1);
        }
    }

    data.message = "</h3><b>" + name + "</b><h3>";
    data.subtext = details;
    data.icon = pixmap;
    data.direction = Plasma::arrowToDirection(arrowType);
}
