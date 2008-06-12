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
#include <QClipboard>
#include <QDesktopWidget>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QLabel>
#include <QShortcut>
#include <QToolButton>
#include <QVBoxLayout>

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

#include "kworkspace/kdisplaymanager.h"

#include "collapsiblewidget.h"
#include "configdialog.h"
#include "interfaceadaptor.h"
#include "krunnersettings.h"
#include "resultscene.h"
#include "resultitem.h"

static const int MIN_WIDTH = 400;

Interface::Interface(QWidget* parent)
    : KRunnerDialog(parent),
      m_configDialog(0),
      m_delayedRun(false),
      m_running(false)
{
    setWindowTitle( i18n("Run Command") );
    setWindowIcon(KIcon("system-run"));

    m_hideResultsTimer.setSingleShot(true);
    connect(&m_hideResultsTimer, SIGNAL(timeout()), this, SLOT(hideResultsArea()));

    QWidget* w = mainWidget();
    m_layout = new QVBoxLayout(w);
    m_layout->setMargin(0);

    QWidget *buttonContainer = new QWidget(w);
    QHBoxLayout *bottomLayout = new QHBoxLayout(buttonContainer);
    bottomLayout->setMargin(0);

    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    QColor buttonBgColor = theme->color(Plasma::Theme::BackgroundColor);
    QString buttonStyleSheet = QString("QToolButton { border: 1px solid %4; border-radius: 4px; padding: 2px;"
                                       " background-color: rgba(%1, %2, %3, %5); }")
                                      .arg(buttonBgColor.red())
                                      .arg(buttonBgColor.green())
                                      .arg(buttonBgColor.blue())
                                      .arg(theme->color(Plasma::Theme::HighlightColor).name(), "50%");
    buttonBgColor = theme->color(Plasma::Theme::TextColor);
    buttonStyleSheet += QString("QToolButton:hover { border: 2px solid %1; }")
                               .arg(theme->color(Plasma::Theme::HighlightColor).name());
    buttonStyleSheet += QString("QToolButton:focus { border: 2px solid %1; }")
                               .arg(theme->color(Plasma::Theme::HighlightColor).name());
    //kDebug() << "stylesheet is" << buttonStyleSheet;

    QToolButton *configButton = new QToolButton(buttonContainer);
    configButton->setStyleSheet(buttonStyleSheet);
//    configButton->setDefault(false);
//    configButton->setAutoDefault(false);
    configButton->setText(i18n("Settings"));
    configButton->setToolTip(i18n("Settings"));
    configButton->setIcon(KIcon("configure"));
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

    QToolButton *activityButton = new QToolButton(buttonContainer);
    activityButton->setStyleSheet(buttonStyleSheet);
//    activityButton->setDefault(false);
//    activityButton->setAutoDefault(false);
    activityButton->setText(i18n("Show System Activity"));
    activityButton->setToolTip(i18n("Show System Activity"));
    activityButton->setIcon(KIcon("utilities-system-monitor"));
    connect(activityButton, SIGNAL(clicked()), qApp, SLOT(showTaskManager()));
    connect(activityButton, SIGNAL(clicked()), this, SLOT(close()));
    bottomLayout->addWidget(activityButton);
    //bottomLayout->addStretch(10);

    QString stringReserver = i18n("Launch");
    stringReserver = i18n("Click to execute the selected item above");
    stringReserver = i18n("Show Options");
    /*
    QString runButtonWhatsThis = i18n( "Click to execute the selected item above" );
    m_runButton = new QToolButton(KGuiItem(i18n("Launch"), "system-run", QString(), runButtonWhatsThis), buttonContainer);
    m_runButton->setEnabled( false );
    m_runButton->setDefault(true);
    m_runButton->setAutoDefault(true);
    connect( m_runButton, SIGNAL( clicked(bool) ), SLOT(run()) );
    bottomLayout->addWidget( m_runButton );
    */
    QToolButton *closeButton = new QToolButton(buttonContainer);
    closeButton->setStyleSheet(buttonStyleSheet);
    //TODO: use a better string for this dialog when we are out of string freeze?
    KGuiItem guiItem = KStandardGuiItem::close();
    closeButton->setText(guiItem.text());
    closeButton->setToolTip(guiItem.text().remove('&'));
    closeButton->setIcon(KIcon("dialog-close"));
//    closeButton->setDefault(false);
//    closeButton->setAutoDefault(false);
    connect(closeButton, SIGNAL(clicked(bool)), SLOT(close()));
    bottomLayout->addWidget(closeButton);

    m_layout->addWidget(buttonContainer);

    m_searchTerm = new KHistoryComboBox(false, w);
    m_searchTerm->setDuplicatesEnabled(false);

    KLineEdit *lineEdit = new KLineEdit(m_searchTerm);
    QAction *focusEdit = new QAction(this);
    focusEdit->setShortcut(Qt::Key_F6);
    connect(focusEdit, SIGNAL(triggered(bool)), lineEdit, SLOT(setFocus()));
    addAction(focusEdit);

    // the order of these next few lines if very important.
    // QComboBox::setLineEdit sets the autoComplete flag on the lineedit,
    // and KComboBox::setAutoComplete resets the autocomplete mode! ugh!
    m_searchTerm->setLineEdit(lineEdit);

    m_completion = new KCompletion();
    lineEdit->setCompletionObject(m_completion);
    lineEdit->setCompletionMode(static_cast<KGlobalSettings::Completion>(KRunnerSettings::queryTextCompletionMode()));
    lineEdit->setClearButtonShown(true);
//    m_layout->addWidget(m_searchTerm);
    bottomLayout->insertWidget(2, m_searchTerm, 10);

    QHBoxLayout *statusLayout = new QHBoxLayout();
    m_descriptionLabel = new QLabel(w);
    m_descriptionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    m_descriptionLabel->hide();
    statusLayout->addWidget(m_descriptionLabel, 10, Qt::AlignLeft | Qt::AlignTop);

    m_previousPage = new QLabel(w);
    m_previousPage->setText("<a href=\"prev\">&lt;&lt;</a>");
    m_previousPage->hide();
    statusLayout->addWidget(m_previousPage, 0, Qt::AlignLeft | Qt::AlignTop);

    m_nextPage = new QLabel(w);
    m_nextPage->setText("<a href=\"next\">&gt;&gt;</a>");
    m_nextPage->hide();
    statusLayout->addWidget(m_nextPage, 0, Qt::AlignLeft | Qt::AlignTop);

    {
        QPalette p = m_descriptionLabel->palette();
        p.setColor(QPalette::WindowText, theme->color(Plasma::Theme::TextColor));
        p.setColor(QPalette::Link, theme->color(Plasma::Theme::TextColor));
        p.setColor(QPalette::LinkVisited, theme->color(Plasma::Theme::TextColor));
        m_descriptionLabel->setPalette(p);
        m_previousPage->setPalette(p);
        m_nextPage->setPalette(p);
    }

    m_layout->addLayout(statusLayout);

    m_dividerLine = new QWidget(w);
    m_dividerLine->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    m_dividerLine->setFixedHeight(1);
    m_dividerLine->setAutoFillBackground(true);
    m_layout->addWidget(m_dividerLine);

    m_resultsView = new QGraphicsView(w);
    m_resultsView->setFrameStyle(QFrame::NoFrame);
    m_resultsView->viewport()->setAutoFillBackground(false);
    m_resultsView->setInteractive(true);
    m_resultsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_resultsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_resultsView->setOptimizationFlag(QGraphicsView::DontSavePainterState);
    m_resultsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    //kDebug() << "size:" << m_resultsView->size() << m_resultsView->minimumSize();
    m_resultsScene = new ResultScene(this);
    m_resultsView->setScene(m_resultsScene);
    m_resultsView->setMinimumSize(m_resultsScene->minimumSizeHint());
    connect(m_resultsScene, SIGNAL(matchCountChanged(int)), this, SLOT(matchCountChanged(int)));
    connect(m_resultsScene, SIGNAL(itemActivated(ResultItem *)), this, SLOT(run(ResultItem *)));
    connect(m_resultsScene, SIGNAL(itemHoverEnter(ResultItem *)), this, SLOT(updateDescriptionLabel(ResultItem *)));
    connect(m_resultsScene, SIGNAL(itemHoverLeave(ResultItem *)), m_descriptionLabel, SLOT(clear()));
    connect(m_previousPage, SIGNAL(linkActivated(const QString&)), m_resultsScene, SLOT(previousPage()));
    connect(m_nextPage, SIGNAL(linkActivated(const QString&)), m_resultsScene, SLOT(nextPage()));

    m_layout->addWidget(m_resultsView);

    connect(m_searchTerm, SIGNAL(editTextChanged(QString)), this, SLOT(queryTextEditted(QString)));
    connect(m_searchTerm, SIGNAL(returnPressed()), this, SLOT(runDefaultResultItem()));

    new InterfaceAdaptor( this );
    QDBusConnection::sessionBus().registerObject( "/Interface", this );

    new QShortcut( QKeySequence( Qt::Key_Escape ), this, SLOT(close()) );

    kDebug() << "size:" << m_resultsView->size() << m_resultsView->minimumSize();
    // we restore the original size, which will set the results view back to its
    // normal size, then we hide the results view and resize the dialog
    setMinimumSize(QSize(MIN_WIDTH , 0));
    KConfigGroup interfaceConfig(KGlobal::config(), "Interface");
    restoreDialogSize(interfaceConfig);
    m_resultsView->hide();
    adjustSize();

    m_layout->addStretch(1);
    setTabOrder(0, configButton);
    setTabOrder(configButton, activityButton);
    setTabOrder(activityButton, m_searchTerm);
    setTabOrder(m_searchTerm, m_previousPage);
    setTabOrder(m_previousPage, m_nextPage);
    setTabOrder(m_nextPage, m_resultsView);
    setTabOrder(m_resultsView, closeButton);
}

void Interface::resizeEvent(QResizeEvent *event)
{
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    int gradientWidth = contentsRect().width() - KDialog::marginHint()*2;
    QLinearGradient gr(0, 0, gradientWidth, 0);
    gr.setColorAt(0, theme->color(Plasma::Theme::BackgroundColor));
    gr.setColorAt(.35, theme->color(Plasma::Theme::TextColor));
    gr.setColorAt(.65, theme->color(Plasma::Theme::TextColor));
    gr.setColorAt(1, theme->color(Plasma::Theme::BackgroundColor));
    {
        QPalette p = palette();
        p.setBrush(QPalette::Background, gr);
        m_dividerLine->setPalette(p);
    }
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

    // TODO: set a nice welcome string when the string freeze lifts
    m_descriptionLabel->clear();

    centerOnScreen();
    KWindowSystem::forceActiveWindow(winId());
    QTimer::singleShot(0, this, SLOT(show()));
}

void Interface::centerOnScreen()
{
    int screen = 0;
    if (QApplication::desktop()->numScreens() > 1) {
        screen = QApplication::desktop()->screenNumber(QCursor::pos());
    }

    if (m_resultsView->isVisibleTo(this)) {
        KDialog::centerOnScreen(this, screen);
        return;
    }

    // center it as if the results view was already visible
    QDesktopWidget *desktop = qApp->desktop();
    QRect r = desktop->screenGeometry(screen);
    int w = width();
    int h = height() + m_resultsView->height();
    move(r.left() + (r.width() / 2) - (w / 2),
         r.top() + (r.height() / 2) - (h / 2));
    kDebug() << "moved to" << pos();
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
    m_delayedRun = false;
    m_searchTerm->setCurrentItem(QString(), true, 0);
    m_descriptionLabel->clear();
    m_resultsScene->clearQuery();
    m_resultsView->hide();
    m_descriptionLabel->hide();
    m_previousPage->hide();
    m_nextPage->hide();
    setMinimumSize(QSize(MIN_WIDTH, 0));
    adjustSize();
}

void Interface::closeEvent(QCloseEvent *e)
{
    if (!m_running) {
        resetInterface();
    } else {
        m_delayedRun = false;
        m_resultsView->hide();
        m_descriptionLabel->hide();
        m_previousPage->hide();
        m_nextPage->hide();
        setMinimumSize(QSize(MIN_WIDTH, 0));
        adjustSize();
    }
    e->accept();
}

void Interface::run(ResultItem *item)
{
    if (!item || item->group() < Plasma::QueryMatch::PossibleMatch) {
        m_delayedRun = true;
        return;
    }

    kDebug() << item->name() << item->id();
    m_delayedRun = false;
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

void Interface::queryTextEditted(const QString &query)
{
    if (query.isEmpty()) {
        resetInterface();
    } else {
        m_resultsScene->launchQuery(query);
    }
}

void Interface::updateDescriptionLabel(ResultItem *item)
{
    m_descriptionLabel->setVisible(item);
    if (!item) {
        m_descriptionLabel->clear();
    } else if (item->description().isEmpty()) {
        m_descriptionLabel->setText(item->name());
    } else {
        m_descriptionLabel->setText(item->name() + ": " + item->description());
    }
}

void Interface::switchUser()
{
    KService::Ptr service = KService::serviceByStorageId("plasma-runner-sessions.desktop");
    KPluginInfo info(service);

    if (info.isValid()) {
        SessList sessions;

        if (sessions.isEmpty()) {
            // no sessions to switch between, let's just start up another session directly
            Plasma::AbstractRunner *sessionRunner = m_resultsScene->manager()->runner(info.pluginName());
            if (sessionRunner) {
                Plasma::QueryMatch switcher(sessionRunner);
                sessionRunner->run(*m_resultsScene->manager()->searchContext(), switcher);
            }
        } else {
            display(QString());
            //TODO: create a "single runner" mode
            //m_header->setText(i18n("Switch users"));
            //m_header->setPixmap("system-switch-user");

            //TODO: ugh, magic strings. See sessions/sessionrunner.cpp
            m_resultsScene->launchQuery("SESSIONS", info.pluginName());
        }
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

void Interface::matchCountChanged(int count)
{
    bool show = count > 0;
    m_hideResultsTimer.stop();
    bool pages = m_resultsScene->pageCount() > 1;
    m_previousPage->setVisible(pages);
    m_nextPage->setVisible(pages);

    if (show && m_delayedRun) {
        kDebug() << "delayed run with" << count << "items";
        runDefaultResultItem();
        return;
    }

    if (m_resultsView->isVisible() == show) {
        return;
    }

    if (show) {
        m_resultsView->show();
        setMinimumSize(QSize(MIN_WIDTH, 0));
        adjustSize();
    } else {
        m_hideResultsTimer.start(2000);
    }
}

void Interface::hideResultsArea()
{
    m_resultsView->hide();
    m_descriptionLabel->hide();
    m_previousPage->hide();
    m_nextPage->hide();
    setMinimumSize(QSize(MIN_WIDTH, 0));
    adjustSize();
}

#include "interface.moc"
