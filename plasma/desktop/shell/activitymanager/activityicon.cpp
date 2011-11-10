/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *   Copyright 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "activityicon.h"

#include "activity.h"
#include "activitycontrols.h"
#include "desktopcorona.h"
#include "plasmaapp.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsGridLayout>
#include <QPainter>
#include <QCursor>
#include <QSizePolicy>

#include <KIconLoader>
#include <KIcon>
#include <KStandardDirs>
#include <KGlobalSettings>

#include <Plasma/Label>
#include <Plasma/PushButton>
#include <Plasma/LineEdit>
#include <Plasma/IconWidget>
#include <Plasma/Package>

#include <scripting/layouttemplatepackagestructure.h>
#include "scripting/desktopscriptengine.h"

#define REMOVE_ICON KIcon("edit-delete")
#define STOP_ICON KIcon("media-playback-stop")
#define START_ICON KIcon("media-playback-start")
#define CONFIGURE_ICON KIcon("configure")

class ActivityActionWidget: public QGraphicsWidget {
public:
    ActivityActionWidget(ActivityIcon * parent, const QString & slot,
            const KIcon & icon, const QString & tooltip, const QSize & size = QSize(16, 16))
        : QGraphicsWidget(parent), m_parent(parent), m_slot(slot), m_iconSize(size), m_icon(icon)
    {
        setToolTip(tooltip);
        setMinimumSize(m_iconSize);
        setPreferredSize(m_iconSize);
        setMaximumSize(m_iconSize);

        setGeometry(0, 0, m_iconSize.width(), m_iconSize.height());
        setZValue(1);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        QGraphicsWidget::paint(painter, option, widget);
        painter->drawPixmap(0, 0, m_icon.pixmap(m_iconSize));
    }

    void mousePressEvent(QGraphicsSceneMouseEvent * event)
    {
        QGraphicsWidget::mouseReleaseEvent(event);

        if (event->button() != Qt::LeftButton)
            return;

        QMetaObject::invokeMethod(m_parent, m_slot.toAscii());

        event->accept();
    }

    ActivityIcon * m_parent;
    QString m_slot;
    QSize m_iconSize;
    KIcon m_icon;

};

ActivityIcon::ActivityIcon(const QString &id)
    : AbstractIcon(0),
      m_buttonStop(0),
      m_buttonRemove(0),
      m_buttonStart(0),
      m_buttonConfigure(0),
      m_closable(false),
      m_inlineWidgetAnim(0)
{
    DesktopCorona *c = qobject_cast<DesktopCorona*>(PlasmaApp::self()->corona());
    m_activity = c->activity(id);

    updateButtons();

    connect(this, SIGNAL(clicked(Plasma::AbstractIcon*)), m_activity, SLOT(activate()));
    connect(m_activity, SIGNAL(stateChanged()), this, SLOT(updateButtons()));
    connect(m_activity, SIGNAL(infoChanged()), this, SLOT(updateContents()));
    connect(m_activity, SIGNAL(currentStatusChanged()), this, SLOT(currentStatusChanged()));
    setName(m_activity->name());
    currentStatusChanged();

    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}

ActivityIcon::ActivityIcon(const QString &name, const QString &icon, const QString &plugin)
    : AbstractIcon(0),
      m_buttonStop(0),
      m_buttonRemove(0),
      m_buttonStart(0),
      m_buttonConfigure(0),
      m_icon(icon),
      m_iconName(icon), 
      m_pluginName(plugin),
      m_closable(false),
      m_activity(0),
      m_inlineWidgetAnim(0)
{
    updateButtons();

    connect(this, SIGNAL(clicked(Plasma::AbstractIcon*)), this, SLOT(createActivity()));
    setName(name);
    currentStatusChanged();

    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}

void ActivityIcon::createActivity()
{
    KService::Ptr service = KService::serviceByStorageId(m_pluginName);

    KPluginInfo info(service);
    Plasma::PackageStructure::Ptr structure(new WorkspaceScripting::LayoutTemplatePackageStructure);

    const QString path = KStandardDirs::locate("data", structure->defaultPackageRoot() + '/' + info.pluginName() + '/');
    if (!path.isEmpty()) {
        Plasma::Package package(path, structure);
        const QString scriptFile = package.filePath("mainscript");
        const QStringList startupApps = service->property("X-Plasma-ContainmentLayout-ExecuteOnCreation", QVariant::StringList).toStringList();

        if (!scriptFile.isEmpty() || !startupApps.isEmpty()) {
            PlasmaApp::self()->createActivityFromScript(
                scriptFile,
                name(),
                m_iconName,
                startupApps
            );
        }


        KConfig config("plasma-desktoprc");
        KConfigGroup group(&config, "ActivityManager HiddenTemplates");

        group.writeEntry(m_pluginName, true);
        group.sync();

        emit requestsRemoval(false);
    }
}

ActivityIcon::~ActivityIcon()
{
}

QPixmap ActivityIcon::pixmap(const QSize &size)
{
    if (m_activity) {
        return m_activity->pixmap(size);
    } else {
        return m_icon.pixmap(size);
    }
}

QMimeData* ActivityIcon::mimeData()
{
    //TODO: how shall we use d&d?
    return 0;
}

class MakeRoomAnimation : public QAbstractAnimation
{
public:
    MakeRoomAnimation(ActivityIcon *icon, qreal addedWidth, QObject *parent)
        : QAbstractAnimation(parent),
          m_icon(icon),
          m_addWidth(addedWidth)
    {
        m_startSize = icon->geometry().size();
        m_icon->getContentsMargins(0, 0, &m_startRightMargin, 0);
    }

    int duration() const
    {
        return 100;
    }

    void updateCurrentTime(int currentTime)
    {
        const qreal delta = currentTime / (float) duration();

        m_icon->setPreferredSize(m_startSize + QSizeF(m_addWidth * delta, 0));

        qreal left, top, right, bottom;
        m_icon->getContentsMargins(&left, &top, &right, &bottom);

        right = m_startRightMargin + m_addWidth * delta;

        m_icon->setContentsMargins(left, top, right, bottom);
    }

private:
    ActivityIcon * m_icon;
    QSizeF m_startSize;
    qreal m_addWidth;
    qreal m_startRightMargin;
};

void ActivityIcon::showInlineWidget(ActivityControls * w)
{
    hideInlineWidget(true);

    connect(w, SIGNAL(closed()), this, SLOT(hideInlineWidget()));

    w->setMaximumSize(QSize(0, size().height()));
    w->adjustSize();
    w->setPos(contentsRect().topRight() + QPoint(4, 0));
    w->setZValue(2);

    m_inlineWidget = w;
    QTimer::singleShot(0, this, SLOT(startInlineAnim()));
}

void ActivityIcon::showRemovalConfirmation()
{
    ActivityControls * w = new ActivityRemovalConfirmation(this);

    if (m_activity)
        connect(w, SIGNAL(removalConfirmed()), m_activity, SLOT(remove()));
    else
        connect(w, SIGNAL(removalConfirmed()), this, SLOT(hideTemplate()));

    showInlineWidget(w);
}

void ActivityIcon::showConfiguration()
{
    if (m_activity)
        showInlineWidget(new ActivityConfiguration(this, m_activity));
}

void ActivityIcon::startInlineAnim()
{
    QGraphicsWidget * w = m_inlineWidget.data();
    //kDebug() << "Booh yah!" << w;
    if (!w) {
        return;
    }

    //kDebug() << w->preferredSize() << w->layout()->preferredSize();
    if (!m_inlineWidgetAnim) {
        m_inlineWidgetAnim = new MakeRoomAnimation(this, w->layout()->preferredSize().width() + 4, this);
        connect(m_inlineWidgetAnim, SIGNAL(finished()), this, SLOT(makeInlineWidgetVisible()));
    }

    m_inlineWidgetAnim->start();
}

void ActivityIcon::hideInlineWidget(bool aboutToShowAnother)
{
    if (m_inlineWidget) {
        m_inlineWidget.data()->deleteLater();
        m_inlineWidget.data()->hide();
    }

    if (m_inlineWidgetAnim && !aboutToShowAnother) {
        m_inlineWidgetAnim->setDirection(QAbstractAnimation::Backward);
        if (m_inlineWidgetAnim->state() != QAbstractAnimation::Running) {
            m_inlineWidgetAnim->start(QAbstractAnimation::DeleteWhenStopped);
            m_inlineWidgetAnim = 0;
        }
    }
}

void ActivityIcon::makeInlineWidgetVisible()
{
    if (m_inlineWidget) {
        m_inlineWidget.data()->show();
    }
}

void ActivityIcon::setClosable(bool closable)
{
    if (closable == m_closable) {
        return;
    }

    m_closable = closable;
    updateButtons();
}

Activity* ActivityIcon::activity()
{
    return m_activity;
}

void ActivityIcon::activityRemoved()
{
    m_activity = 0;
    deleteLater();
}

void ActivityIcon::setGeometry(const QRectF & geometry)
{
    Plasma::AbstractIcon::setGeometry(geometry);
    updateLayout();
}

void ActivityIcon::updateLayout()
{
    QRectF rect = contentsRect();

    rect.adjust(
            (rect.width() - iconSize()) / 2,
            rect.height() - iconSize(),
            - (rect.width() - iconSize()) / 2,
            0
            );

    if (m_buttonStop) {
        m_buttonStop->setGeometry(QRectF(
            rect.topRight() - QPointF(m_buttonStop->m_iconSize.width(), 0),
            m_buttonStop->m_iconSize
        ));
    }

    if (m_buttonRemove) {
        m_buttonRemove->setGeometry(QRectF(
            rect.topRight() - QPointF(m_buttonRemove->m_iconSize.width(), 0),
            m_buttonRemove->m_iconSize
        ));
    }

    if (m_buttonConfigure) {
        m_buttonConfigure->setGeometry(QRectF(
            rect.bottomRight() - QPointF(m_buttonConfigure->m_iconSize.width(), m_buttonConfigure->m_iconSize.height()),
            m_buttonConfigure->m_iconSize
        ));
    }

    if (m_buttonStart) {
        m_buttonStart->setGeometry(QRectF(
            rect.center() - QPointF(m_buttonStart->m_iconSize.width() / 2, m_buttonStart->m_iconSize.height() / 2),
            m_buttonStart->m_iconSize
        ));
    }
}

void ActivityIcon::updateButtons()
{
    if (m_activity) {

        if (!m_buttonConfigure) {
            m_buttonConfigure = new ActivityActionWidget(this, "showConfiguration", CONFIGURE_ICON, i18n("Configure activity"));
        }

        #define DESTROY_ACTIVITY_ACTION_WIDIGET(A) \
        if (A) {                               \
            A->hide();                         \
            A->deleteLater();                  \
            A = 0;                             \
        }

        switch (m_activity->state()) {
            case KActivityInfo::Running:
                DESTROY_ACTIVITY_ACTION_WIDIGET(m_buttonStart);
                DESTROY_ACTIVITY_ACTION_WIDIGET(m_buttonRemove);

                if (m_closable) {
                    if (!m_buttonStop) {
                        m_buttonStop = new ActivityActionWidget(this, "stopActivity", STOP_ICON, i18n("Stop activity"));
                    }
                } else {
                    DESTROY_ACTIVITY_ACTION_WIDIGET(m_buttonStop);
                }
                break;

            case KActivityInfo::Stopped:
                DESTROY_ACTIVITY_ACTION_WIDIGET(m_buttonStop);

                if (!m_buttonRemove) {
                    m_buttonRemove = new ActivityActionWidget(this, "showRemovalConfirmation", REMOVE_ICON, i18n("Stop activity"));
                }

                if (!m_buttonStart) {
                    m_buttonStart = new ActivityActionWidget(this, "startActivity", START_ICON, i18n("Start activity"), QSize(32, 32));
                }
                break;

            case KActivityInfo::Invalid:
                DESTROY_ACTIVITY_ACTION_WIDIGET(m_buttonConfigure);
                // no break

            default: //transitioning or invalid: don't let the user mess with it
                DESTROY_ACTIVITY_ACTION_WIDIGET(m_buttonStart);
                DESTROY_ACTIVITY_ACTION_WIDIGET(m_buttonRemove);
                DESTROY_ACTIVITY_ACTION_WIDIGET(m_buttonStop);
        }

        #undef DESTROY_ACTIVITY_ACTION_WIDIGET

    } else {
        if (!m_buttonRemove) {
            m_buttonRemove = new ActivityActionWidget(this, "showRemovalConfirmation", REMOVE_ICON, i18n("Stop activity"));
        }
    }

    updateLayout();
}

void ActivityIcon::stopActivity()
{
    if (m_activity) {
        m_activity->close();
    }
}

void ActivityIcon::startActivity()
{
    emit clicked(this);
}

void ActivityIcon::updateContents()
{
    if (m_activity)
        setName(m_activity->name());
    update();
}

void ActivityIcon::currentStatusChanged()
{
    if (m_activity)
        setSelected(m_activity->isCurrent());
}

void ActivityIcon::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    if (m_inlineWidget && m_inlineWidget.data()->hidesContents()) {
        paintBackground(painter, option, widget);
        return;
    }

    AbstractIcon::paint(painter, option, widget);
}

void ActivityIcon::hideTemplate()
{
    KConfig config("plasma-desktoprc");
    KConfigGroup group(&config, "ActivityManager HiddenTemplates");

    group.writeEntry(m_pluginName, true);
    group.sync();

    emit requestsRemoval(true);
}

#include "activityicon.moc"

