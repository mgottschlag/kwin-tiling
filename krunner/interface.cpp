/***************************************************************************
 *   Copyright 2006 by Aaron Seigo <aseigo@kde.org>                        *
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "interface.h"

#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QGraphicsView>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QShortcut>
#include <QClipboard>

#include <KActionCollection>
#include <KHistoryComboBox>
#include <KCompletion>
#include <KCompletionBox>
#include <KDebug>
#include <KDialog>
#include <KLineEdit>
#include <KLocale>
#include <KGlobalSettings>
#include <KPluginInfo>
#include <KPushButton>
#include <KTitleWidget>
#include <KWindowSystem>

#include <plasma/abstractrunner.h>
#include <plasma/runnermanager.h>
#include <plasma/theme.h>

#include "collapsiblewidget.h"
#include "configdialog.h"
#include "interfaceadaptor.h"
#include "krunnersettings.h"
#include "resultscene.h"
#include "resultitem.h"

Interface::Interface(QWidget* parent)
    : KRunnerDialog(parent),
      m_configDialog(0),
      m_running(false)
{
    setWindowTitle( i18n("Run Command") );
    setWindowIcon(KIcon("system-run"));

    QWidget* w = mainWidget();
    m_layout = new QVBoxLayout(w);
    m_layout->setMargin(0);

    m_searchTerm = new KHistoryComboBox(false, w);
    m_searchTerm->setDuplicatesEnabled(false);

    KLineEdit *lineEdit = new KLineEdit(m_searchTerm);

    // the order of these next few lines if very important.
    // QComboBox::setLineEdit sets the autoComplete flag on the lineedit,
    // and KComboBox::setAutoComplete resets the autocomplete mode! ugh!
    m_searchTerm->setLineEdit(lineEdit);

    m_completion = new KCompletion();
    lineEdit->setCompletionObject(m_completion);
    lineEdit->setCompletionMode(static_cast<KGlobalSettings::Completion>(KRunnerSettings::queryTextCompletionMode()));
    lineEdit->setClearButtonShown(true);

    m_layout->addWidget(m_searchTerm);
    m_resultsView = new QGraphicsView(w);
    m_resultsView->setFrameStyle(QFrame::NoFrame);
    m_resultsView->viewport()->setAutoFillBackground(false);
    m_resultsView->setAutoFillBackground(false);
    m_resultsView->setInteractive(true);
    m_resultsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_resultsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_resultsView->setOptimizationFlag(QGraphicsView::DontSavePainterState);
    m_resultsView->setMinimumSize(ResultItem::BOUNDING_SIZE * 4, ResultItem::BOUNDING_SIZE * 2);
    kDebug() << "size:" << m_resultsView->size() << m_resultsView->minimumSize();
    m_layout->addWidget(m_resultsView);

    m_resultsScene = new ResultScene(this);
    m_resultsView->setScene(m_resultsScene);

    m_descriptionLabel = new QLabel(w);
    m_descriptionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    QPalette p = m_descriptionLabel->palette();
    p.setColor(QPalette::WindowText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    m_descriptionLabel->setPalette(p);
    m_layout->addWidget(m_descriptionLabel);

    QWidget *buttonContainer = new QWidget(w);
    QHBoxLayout *bottomLayout = new QHBoxLayout(buttonContainer);
    bottomLayout->setMargin(0);
    KGuiItem configButtonGui = KStandardGuiItem::configure();
    configButtonGui.setText(i18n("Settings"));
    KPushButton *configButton = new KPushButton(configButtonGui, buttonContainer);
    configButton->setDefault(false);
    configButton->setAutoDefault(false);
    connect(configButton, SIGNAL(clicked()), SLOT(showConfigDialog()));
    bottomLayout->addWidget( configButton );

    /*
    KPushButton *m_optionsButton = new KPushButton(KStandardGuiItem::configure(), buttonContainer);
    m_optionsButton->setDefault(false);
    m_optionsButton->setAutoDefault(false);
    m_optionsButton->setText(i18n("Show Options"));
    m_optionsButton->setEnabled(false);
    m_optionsButton->setCheckable(true);
    connect(m_optionsButton, SIGNAL(toggled(bool)), SLOT(showOptions(bool)));
    bottomLayout->addWidget( m_optionsButton );
    */

    KPushButton *activityButton = new KPushButton(buttonContainer);
    activityButton->setDefault(false);
    activityButton->setAutoDefault(false);
    activityButton->setText(i18n("Show System Activity"));
    activityButton->setIcon(KIcon("utilities-system-monitor"));
    connect(activityButton, SIGNAL(clicked()), qApp, SLOT(showTaskManager()));
    connect(activityButton, SIGNAL(clicked()), this, SLOT(close()));
    bottomLayout->addWidget(activityButton);

    //bottomLayout->addStretch();

    QString stringReserver = i18n("Launch");
    stringReserver = i18n("Click to execute the selected item above");
    stringReserver = i18n("Show Options");
    /*
    QString runButtonWhatsThis = i18n( "Click to execute the selected item above" );
    m_runButton = new KPushButton(KGuiItem(i18n("Launch"), "system-run", QString(), runButtonWhatsThis), buttonContainer);
    m_runButton->setEnabled( false );
    m_runButton->setDefault(true);
    m_runButton->setAutoDefault(true);
    connect( m_runButton, SIGNAL( clicked(bool) ), SLOT(run()) );
    bottomLayout->addWidget( m_runButton );
    */
    KPushButton *closeButton = new KPushButton(KStandardGuiItem::close(), buttonContainer);
    closeButton->setDefault(false);
    closeButton->setAutoDefault(false);
    connect(closeButton, SIGNAL(clicked(bool)), SLOT(close()));
    bottomLayout->addWidget(closeButton);

    m_layout->addWidget(buttonContainer);

    connect(m_searchTerm, SIGNAL(editTextChanged(QString)), m_resultsScene, SLOT(launchQuery(QString)));
    connect(m_searchTerm, SIGNAL(returnPressed()), this, SLOT(runDefaultResultItem()));

    connect(m_resultsScene, SIGNAL(itemActivated(ResultItem *)), this, SLOT(run(ResultItem *)));
    connect(m_resultsScene, SIGNAL(itemHoverEnter(ResultItem *)), this, SLOT(updateDescriptionLabel(ResultItem *)));
    connect(m_resultsScene, SIGNAL(itemHoverLeave(ResultItem *)), m_descriptionLabel, SLOT(clear()));

    new InterfaceAdaptor( this );
    QDBusConnection::sessionBus().registerObject( "/Interface", this );

    new QShortcut( QKeySequence( Qt::Key_Escape ), this, SLOT(close()) );

    kDebug() << "size:" << m_resultsView->size() << m_resultsView->minimumSize();
    KConfigGroup interfaceConfig(KGlobal::config(), "Interface");
    restoreDialogSize(interfaceConfig);
    kDebug() << "size:" << m_resultsView->size() << m_resultsView->minimumSize() << size();
}

void Interface::resizeEvent(QResizeEvent *event)
{
    //m_resultsView->resize(size());
    m_resultsScene->resize(m_resultsView->width(), m_resultsView->height());
    KRunnerDialog::resizeEvent(event);
}

Interface::~Interface()
{
    KRunnerSettings::setPastQueries(m_searchTerm->historyItems());
    KRunnerSettings::setQueryTextCompletionMode(m_searchTerm->completionMode());
    KConfigGroup interfaceConfig(KGlobal::config(), "Interface");
    saveDialogSize(interfaceConfig);
}

void Interface::clearHistory()
{
    m_searchTerm->clearHistory();
    KRunnerSettings::setPastQueries(m_searchTerm->historyItems());
}

void Interface::display(const QString& term)
{
    m_searchTerm->setFocus();

    if (!term.isEmpty()) {
        m_searchTerm->setItemText(0, term);
    }

    KWindowSystem::setOnDesktop(winId(), KWindowSystem::currentDesktop());

    int screen = 0;
    if (QApplication::desktop()->numScreens() > 1) {
        screen = QApplication::desktop()->screenNumber(QCursor::pos());
    }

    KDialog::centerOnScreen(this, screen);
    show();
    KWindowSystem::forceActiveWindow(winId());
}

void Interface::displayWithClipboardContents()
{
   QString clipboardData = QApplication::clipboard()->text(QClipboard::Selection);
   display(clipboardData);
}

void Interface::setWidgetPalettes()
{
    // a nice palette to use with the widgets
    QPalette widgetPalette = palette();
    QColor bgColor = widgetPalette.color( QPalette::Active,
                                                QPalette::Base );
    bgColor.setAlpha( 200 );
    widgetPalette.setColor( QPalette::Base, bgColor );

    m_searchTerm->setPalette( widgetPalette );
}

void Interface::resetInterface()
{
    m_searchTerm->setCurrentItem(QString(), true, 0);
    m_descriptionLabel->clear();
    m_resultsScene->clearQuery();
}

void Interface::closeEvent(QCloseEvent* e)
{
    if (!m_running) {
        resetInterface();
    }
    e->accept();
}

void Interface::run(ResultItem *item)
{
    if (!item || item->group() < Plasma::QueryMatch::PossibleMatch) {
        return;
    }

    kDebug() << item->name() << item->id();
    m_searchTerm->addToHistory(m_searchTerm->currentText());

    if (item->group() == Plasma::QueryMatch::InformationalMatch) {
        QString info = item->data();

        if (!info.isEmpty()) {
            m_searchTerm->setItemText(0, info);
            m_searchTerm->setCurrentIndex(0);
        }
        return;
    }

    m_running = true;
    close();
    m_resultsScene->run(item);
    m_running = false;
    resetInterface();
}

void Interface::runDefaultResultItem()
{
    run(m_resultsScene->defaultResultItem());
}

void Interface::updateDescriptionLabel(ResultItem *item)
{
    if (item->description() != "") {
        m_descriptionLabel->setText(item->name() + ": " + item->description());
    } else {
        m_descriptionLabel->setText(item->name());
    }
}

void Interface::switchUser()
{
    //TODO: ugh, magic strings. See sessions/sessionrunner.cpp
    display(QString());
    //TODO: create a "single runner" mode
    //m_header->setText(i18n("Switch users"));
    //m_header->setPixmap("system-switch-user");

    KService::Ptr service = KService::serviceByStorageId("plasma-runner-sessions.desktop");
    KPluginInfo info(service);

    if (info.isValid()) {
        m_resultsScene->launchQuery("SESSIONS", info.pluginName());
    }
}

void Interface::showConfigDialog()
{
    if (!m_configDialog) {
        m_configDialog = new KRunnerConfigDialog(m_resultsScene->manager());
        connect(m_configDialog, SIGNAL(finished()), this, SLOT(configCompleted()));
    }

    KWindowSystem::setOnDesktop(m_configDialog->winId(), KWindowSystem::currentDesktop());
    KWindowSystem::activateWindow(m_configDialog->winId());
    m_configDialog->show();
}

void Interface::configCompleted()
{
    m_configDialog->deleteLater();
    m_configDialog = 0;
}

#include "interface.moc"
