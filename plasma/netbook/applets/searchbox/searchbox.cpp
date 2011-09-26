/*
 *   Copyright 2009 by Artur Duque de Souza <asouza@kde.org>
 *
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

#include "searchbox.h"

#include <QTimer>

#include <Plasma/LineEdit>
#include <Plasma/IconWidget>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>
#include <Plasma/Containment>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

#include <KDebug>
#include <KIcon>
#include <KLineEdit>
#include <KIconLoader>

#include <QGraphicsLinearLayout>


SearchBox::SearchBox(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args), m_widget(0), m_search(0),
      m_closeIcon(0)
{
    setPopupIcon("edit-find");
    setPassivePopup(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

void SearchBox::init()
{
    // tooltip stuff
    Plasma::ToolTipContent toolTipData = Plasma::ToolTipContent();
    toolTipData.setAutohide(true);
    toolTipData.setMainText(name());
    toolTipData.setImage(KIcon("edit-find"));

    Plasma::ToolTipManager::self()->registerWidget(this);
    Plasma::ToolTipManager::self()->setContent(this, toolTipData);
}

SearchBox::~SearchBox()
{
}

QGraphicsWidget *SearchBox::graphicsWidget()
{
    if (m_widget) {
        return m_widget;
    }

    m_search = new Plasma::LineEdit();
    m_search->nativeWidget()->setClearButtonShown(true);
    m_search->nativeWidget()->setClickMessage(i18n("Enter your query here"));
    connect(m_search, SIGNAL(returnPressed()), this, SLOT(query()));
    connect(m_search->nativeWidget(), SIGNAL(textChanged(QString)), this, SLOT(delayedQuery()));

    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    connect(m_searchTimer, SIGNAL(timeout()), this, SLOT(query()));


    m_closeIcon = new Plasma::IconWidget();
    m_closeIcon->setIcon("dialog-close");
    m_closeIcon->setPreferredSize(KIconLoader::SizeSmallMedium,
                                  KIconLoader::SizeSmallMedium);
    connect(m_closeIcon, SIGNAL(clicked()), this, SLOT(hidePopup()));

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout();
    layout->addItem(m_search);
    layout->addItem(m_closeIcon);
    layout->setStretchFactor(m_search, 4);

    m_widget = new QGraphicsWidget(this);
    m_widget->setLayout(layout);
    m_widget->setPreferredWidth(300);
    m_widget->installEventFilter(this);

    return m_widget;
}

void SearchBox::popupEvent(bool shown)
{
    if (!m_search) {
        return;
    }

    if (shown) {
        focusEditor();
    } else {
        m_search->setText(QString());
    }
}

void SearchBox::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);

    focusEditor();
}

bool SearchBox::eventFilter(QObject* watched, QEvent *event)
{
    Q_UNUSED(watched);

    //FIXME: is there a way to do it in PopupApplet?
    if (event->type() == QEvent::FocusIn) {
        focusEditor();
    }
    return false;
}

void SearchBox::focusEditor()
{
    m_search->clearFocus();
    m_search->setFocus();
    m_search->nativeWidget()->clearFocus();
    m_search->nativeWidget()->setFocus();
}

void SearchBox::delayedQuery()
{
    m_searchTimer->start(500);
}

void SearchBox::query()
{
    Plasma::Service *service = dataEngine("searchlaunch")->serviceForSource("query");
    KConfigGroup ops = service->operationDescription("query");
    ops.writeEntry("query", m_search->text());
    KJob *job = service->startOperationCall(ops);
    connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
}

#include "searchbox.moc"
