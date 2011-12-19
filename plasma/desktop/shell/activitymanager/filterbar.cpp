/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright 2010 Chani Armitage <chani@kde.org>
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

#include "filterbar.h"
#include "desktopcorona.h"
#include "plasmaapp.h"

#include <QGraphicsLinearLayout>
#include <QTimer>

#include <KAuthorized>
#include <KMenu>
#include <KPushButton>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <knewstuff3/downloaddialog.h>

#include <Plasma/Theme>
#include <Plasma/Corona>
#include <Plasma/LineEdit>
#include <Plasma/PushButton>
#include <Plasma/TabBar>
#include <Plasma/Package>

#include <scripting/layouttemplatepackagestructure.h>
#include "scripting/desktopscriptengine.h"

FilterBar::FilterBar(Qt::Orientation orientation, QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_unlockButton(0)
{
    setFocusPolicy(Qt::StrongFocus);

    //init text search
    m_textSearch = new Plasma::LineEdit();

    m_textSearch->setClickMessage(i18n("Enter Search Term"));
    m_textSearch->setAttribute(Qt::WA_NoSystemBackground);
    m_textSearch->setClearButtonShown(true);
    connect(m_textSearch, SIGNAL(textChanged(QString)), this, SIGNAL(searchTermChanged(QString)));

    m_addWidgetsButton = new Plasma::PushButton(this);
    m_addWidgetsButton->setText(i18n("Add Widgets"));
    m_addWidgetsButton->setIcon(KIcon("plasma"));
    connect(m_addWidgetsButton, SIGNAL(clicked()), this, SIGNAL(addWidgetsRequested()));

    if (PlasmaApp::self()->corona()->immutability() != Plasma::SystemImmutable &&
        KAuthorized::authorize("plasma-desktop/add_activities")) {
        m_newActivityButton = new Plasma::PushButton(this);
        m_newActivityButton->setText(i18n("Create Activity"));
        m_newActivityButton->setIcon(KIcon("list-add"));
        m_newActivityMenu = new KMenu();
        connect(m_newActivityMenu, SIGNAL(aboutToShow()), this, SLOT(populateActivityMenu()));
        connect(m_newActivityMenu, SIGNAL(triggered(QAction*)), this, SLOT(createActivity(QAction*)));
        m_newActivityButton->nativeWidget()->setMenu(m_newActivityMenu);
    } else {
        m_newActivityButton = 0;
        m_newActivityMenu = 0;
    }

    //layout
    m_linearLayout = new QGraphicsLinearLayout(this);
    m_linearLayout->addItem(m_textSearch);
    m_linearLayout->addStretch(10);
    if (m_newActivityButton) {
        m_linearLayout->addItem(m_newActivityButton);
    }
    m_linearLayout->addItem(m_addWidgetsButton);

    setOrientation(orientation);
    connect(PlasmaApp::self()->corona(), SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
            this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));
    immutabilityChanged(PlasmaApp::self()->corona()->immutability());
}

FilterBar::~FilterBar()
{
}

Plasma::LineEdit* FilterBar::textSearch()
{
    return m_textSearch;
}

void FilterBar::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)

    QSizeF contentsSize = m_linearLayout->contentsRect().size();
//    m_linearLayout->invalidate();
//    m_linearLayout->activate();

    if(m_orientation == Qt::Horizontal) {
        //don't let it occupy the whole layout width
        m_textSearch->setMaximumWidth(contentsSize.width()/6);
        m_textSearch->setMinimumWidth(contentsSize.width()/6);
    } else {
        //let it occupy the whole width
        m_textSearch->setMaximumWidth(-1);
        m_textSearch->setMinimumWidth(-1);
    }
}

void FilterBar::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    m_linearLayout->setOrientation(orientation);

    if (orientation == Qt::Horizontal) {
        m_textSearch->setPreferredWidth(200);
        m_textSearch->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        m_textSearch->setPreferredHeight(-1);
    } else {
        m_textSearch->setPreferredHeight(30);
        m_textSearch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_textSearch->setPreferredWidth(-1);
    }
}

void FilterBar::setMenuPos()
{
    if (!m_newActivityButton) {
        return;
    }

    QPoint position = PlasmaApp::self()->corona()->popupPosition(m_newActivityButton, m_newActivityMenu->geometry().size());
    m_newActivityMenu->move(position);
}

void FilterBar::setFocus()
{
    m_textSearch->setFlag(ItemIsFocusable);
    m_textSearch->setFocus();
}

void FilterBar::populateActivityMenu()
{
    if (!m_newActivityButton) {
        return;
    }

    QTimer::singleShot(0, this, SLOT(setMenuPos()));
    if (!m_newActivityMenu->actions().isEmpty()) {
        // already populated.
        return;
    }

    QMenu *templatesMenu = m_newActivityMenu->addMenu(i18n("Templates"));
    QMap<QString, QAction*> sorted; //qmap sorts alphabetically

    //regular plugins
    KPluginInfo::List plugins = Plasma::Containment::listContainmentsOfType("desktop");
    foreach (const KPluginInfo& info, plugins) {
        if (info.property("NoDisplay").toBool()) {
            continue;
        }
        QAction *action;
        if (info.pluginName() == "desktop") { //suggest this one for newbies
            action = m_newActivityMenu->addAction(KIcon(info.icon()), i18n("Empty Desktop"));
        } else {
            action = new QAction(KIcon(info.icon()), info.name(), templatesMenu);
            sorted.insert(info.name(), action);
        }
        action->setData(info.pluginName());
    }

    //templates
    const QString constraint = QString("[X-Plasma-Shell] == '%1' and 'desktop' ~in [X-Plasma-ContainmentCategories]")
                                      .arg(KGlobal::mainComponent().componentName());
    KService::List templates = KServiceTypeTrader::self()->query("Plasma/LayoutTemplate", constraint);
    foreach (const KService::Ptr &service, templates) {
        KPluginInfo info(service);
        Plasma::PackageStructure::Ptr structure(new WorkspaceScripting::LayoutTemplatePackageStructure);
        const QString path = KStandardDirs::locate("data", structure->defaultPackageRoot() + '/' + info.pluginName() + '/');
        if (!path.isEmpty()) {
            Plasma::Package package(path, structure);
            const QString scriptFile = package.filePath("mainscript");
            const QStringList startupApps = service->property("X-Plasma-ContainmentLayout-ExecuteOnCreation", QVariant::StringList).toStringList();

            if (!scriptFile.isEmpty() || !startupApps.isEmpty()) {
                QAction *action = new QAction(KIcon(info.icon()), info.name(), templatesMenu);
                QVariantList data;
                data << scriptFile << info.name() << info.icon() << startupApps;
                action->setData(data);
                sorted.insert(info.name(), action);
            }
        }
    }

    //set up sorted menu
    foreach (QAction *action, sorted) {
        templatesMenu->addAction(action);
    }

    //clone
    QAction *action = m_newActivityMenu->addAction(KIcon("edit-copy"), i18n("Clone current activity"));
    action->setData(0);

    //ghns
    templatesMenu->addSeparator();
    action = templatesMenu->addAction(KIcon("get-hot-new-stuff"), i18n("Get New Templates..."));
    action->setData(1);
}

void FilterBar::createActivity(QAction *action)
{
    if (!m_newActivityButton) {
        return;
    }

    QVariant::Type type = action->data().type();
    if (type == QVariant::String) {
        QString plugin = action->data().toString();
        PlasmaApp::self()->createActivity(plugin);
    } else if (type == QVariant::List) {
        QVariantList data = action->data().toList();
        PlasmaApp::self()->createActivityFromScript(data[0].toString(),
                                                    data[1].toString(),
                                                    data[2].toString(),
                                                    data[3].toStringList());
    } else if (action->data().toInt() == 0) {
        PlasmaApp::self()->cloneCurrentActivity();
    } else { //ghns
        KNS3::DownloadDialog *dialog = new KNS3::DownloadDialog( "activities.knsrc", 0 );
        connect(dialog, SIGNAL(accepted()), m_newActivityMenu, SLOT(clear()));
        connect(dialog, SIGNAL(accepted()), dialog, SLOT(deleteLater()));
        dialog->show();
    }
}

void FilterBar::immutabilityChanged(Plasma::ImmutabilityType immutability)
{
    bool visible = immutability == Plasma::Mutable;
    m_addWidgetsButton->setVisible(visible);
    if (visible) {
        m_linearLayout->addItem(m_addWidgetsButton);
    } else {
        m_linearLayout->removeItem(m_addWidgetsButton);
    }

    if (immutability == Plasma::UserImmutable) {
        if (!m_unlockButton) {
            m_unlockButton = new Plasma::PushButton(this);
            m_unlockButton->setAction(PlasmaApp::self()->corona()->action("lock widgets"));
            m_linearLayout->addItem(m_unlockButton);
        }
    } else if (m_unlockButton) {
        m_unlockButton->deleteLater();
        m_unlockButton = 0;
    }
}

#include "filterbar.moc"
