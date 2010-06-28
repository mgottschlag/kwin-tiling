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

#include <KAction>
#include <KActionCollection>
#include <KHistoryComboBox>
#include <KCompletion>
#include <KCompletionBox>
#include <KDebug>
#include <KDialog>
#include <KLineEdit>
#include <KLocale>
#include <KGlobalSettings>
#include <KPushButton>
#include <KTitleWidget>
#include <KWindowSystem>

#include <Plasma/AbstractRunner>
#include <Plasma/RunnerManager>
#include <Plasma/Theme>
#include <Plasma/Svg>

#include "krunnerapp.h"
#include "krunnersettings.h"
#include "interfaces/default/resultscene.h"
#include "interfaces/default/resultitem.h"
#include "interfaces/default/krunnertabfilter.h"
#include "interfaces/default/resultsview.h"
#include "toolbutton.h"

static const int MIN_WIDTH = 420;

Interface::Interface(Plasma::RunnerManager *runnerManager, QWidget *parent)
    : KRunnerDialog(runnerManager, parent),
      m_delayedRun(false),
      m_running(false),
      m_queryRunning(false)
{
    m_resultData.processHoverEvents = true;
    m_resultData.mouseHovering = false;

    m_hideResultsTimer.setSingleShot(true);
    connect(&m_hideResultsTimer, SIGNAL(timeout()), this, SLOT(hideResultsArea()));

    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);

    m_buttonContainer = new QWidget(this);
    QHBoxLayout *bottomLayout = new QHBoxLayout(m_buttonContainer);
    bottomLayout->setMargin(0);

    m_configButton = new ToolButton(m_buttonContainer);
    m_configButton->setText(i18n("Settings"));
    m_configButton->setToolTip(i18n("Settings"));
    connect(m_configButton, SIGNAL(clicked()), SLOT(toggleConfigDialog()));
    bottomLayout->addWidget(m_configButton);

    //Set up the system activity button, using the krunner global action, showing the global shortcut in the tooltip
    m_activityButton = new ToolButton(m_buttonContainer);
    KRunnerApp *krunnerApp = KRunnerApp::self();
    QAction *showSystemActivityAction = krunnerApp->actionCollection()->action("Show System Activity");
    m_activityButton->setDefaultAction(showSystemActivityAction);

    updateSystemActivityToolTip();
    connect(showSystemActivityAction, SIGNAL(globalShortcutChanged(const QKeySequence &)), this, SLOT(updateSystemActivityToolTip()));
    connect(showSystemActivityAction, SIGNAL(triggered(bool)), this, SLOT(resetAndClose()));
    bottomLayout->addWidget(m_activityButton);

    m_singleRunnerIcon = new QLabel();
    bottomLayout->addWidget(m_singleRunnerIcon);
    m_singleRunnerDisplayName = new QLabel();
    bottomLayout->addWidget(m_singleRunnerDisplayName);

    m_helpButton = new ToolButton(m_buttonContainer);
    m_helpButton->setText(i18n("Help"));
    m_helpButton->setToolTip(i18n("Information on using this application"));
    connect(m_helpButton, SIGNAL(clicked(bool)), SLOT(showHelp()));
    connect(m_helpButton, SIGNAL(clicked(bool)), SLOT(configCompleted()));
    bottomLayout->addWidget(m_helpButton);

    QSpacerItem* closeButtonSpacer = new QSpacerItem(0,0,QSizePolicy::MinimumExpanding,QSizePolicy::Fixed);
    bottomLayout->addSpacerItem(closeButtonSpacer);

    m_closeButton = new ToolButton(m_buttonContainer);
    KGuiItem guiItem = KStandardGuiItem::close();
    m_closeButton->setText(guiItem.text());
    m_closeButton->setToolTip(guiItem.text().remove('&'));
    connect(m_closeButton, SIGNAL(clicked(bool)), SLOT(resetAndClose()));
    bottomLayout->addWidget(m_closeButton);

    m_layout->addWidget(m_buttonContainer);

    m_searchTerm = new KHistoryComboBox(false, this);
    m_searchTerm->setPalette(QApplication::palette());
    m_searchTerm->setDuplicatesEnabled(false);

    KLineEdit *lineEdit = new KLineEdit(m_searchTerm);
    QAction *focusEdit = new QAction(this);
    focusEdit->setShortcut(Qt::Key_F6);

    // in theory, the widget should detect the direction from the content
    // but this is not available in Qt4.4/KDE 4.2, so the best default for this widget
    // is LTR: as it's more or less a "command line interface"
    // FIXME remove this code when KLineEdit has automatic direction detection of the "paragraph"
    m_searchTerm->setLayoutDirection(Qt::LeftToRight);

    connect(focusEdit, SIGNAL(triggered(bool)), this, SLOT(searchTermSetFocus()));
    addAction(focusEdit);

    // the order of these next few lines if very important.
    // QComboBox::setLineEdit sets the autoComplete flag on the lineedit,
    // and KComboBox::setAutoComplete resets the autocomplete mode! ugh!
    m_searchTerm->setLineEdit(lineEdit);

    m_completion = new KCompletion();
    lineEdit->setCompletionObject(m_completion);
    lineEdit->setCompletionMode(static_cast<KGlobalSettings::Completion>(KRunnerSettings::queryTextCompletionMode()));
    lineEdit->setClearButtonShown(true);
    QStringList pastQueryItems = KRunnerSettings::pastQueries();
    m_searchTerm->setHistoryItems(pastQueryItems);
    m_searchTerm->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_completion->insertItems(pastQueryItems);
    bottomLayout->insertWidget(4, m_searchTerm, 10);

    m_singleRunnerSearchTerm = new KLineEdit(this);
    bottomLayout->insertWidget(4, m_singleRunnerSearchTerm, 10 );

    //kDebug() << "size:" << m_resultsView->size() << m_resultsView->minimumSize();
    m_resultsScene = new ResultScene(&m_resultData, runnerManager, m_searchTerm, this);
    m_resultsView = new ResultsView(m_resultsScene, &m_resultData, this);
    m_layout->addWidget(m_resultsView);

    connect(m_resultsScene, SIGNAL(matchCountChanged(int)), this, SLOT(matchCountChanged(int)));
    connect(m_resultsScene, SIGNAL(itemActivated(ResultItem *)), this, SLOT(run(ResultItem *)));

    connect(lineEdit, SIGNAL(userTextChanged(QString)), this, SLOT(queryTextEdited(QString)));
    connect(m_searchTerm, SIGNAL(returnPressed()), this, SLOT(runDefaultResultItem()));
    connect(m_singleRunnerSearchTerm, SIGNAL(textChanged(QString)), this, SLOT(queryTextEdited(QString)));
    connect(m_singleRunnerSearchTerm, SIGNAL(returnPressed()),  this, SLOT(runDefaultResultItem()));

    KrunnerTabFilter *krunnerTabFilter = new KrunnerTabFilter(m_resultsScene, lineEdit, this);
    m_searchTerm->installEventFilter(krunnerTabFilter);
    m_singleRunnerSearchTerm->installEventFilter(krunnerTabFilter);
    lineEdit->installEventFilter(this);

    themeUpdated();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));

    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(resetAndClose()));

    m_layout->setAlignment(Qt::AlignTop);

    setTabOrder(0, m_configButton);
    setTabOrder(m_configButton, m_activityButton);
    setTabOrder(m_activityButton, m_searchTerm);
    setTabOrder(m_searchTerm, m_resultsView);
    setTabOrder(m_resultsView, m_helpButton);
    setTabOrder(m_helpButton, m_closeButton);

    //kDebug() << "size:" << m_resultsView->size() << m_resultsView->minimumSize() << minimumSizeHint();

    // we restore the original size, which will set the results view back to its
    // normal size, then we hide the results view and resize the dialog

    setMinimumSize(QSize(MIN_WIDTH , 0));

    // we load the last used size; the saved value is the size of the dialog when the
    // results are visible;
    adjustSize();

    if (KGlobal::config()->hasGroup("Interface")) {
        KConfigGroup interfaceConfig(KGlobal::config(), "Interface");
        restoreDialogSize(interfaceConfig);
    }

    m_defaultSize = size();
    m_resultsView->hide();

    m_delayedQueryTimer.setSingleShot(true);
    m_delayedQueryTimer.setInterval(100);
    connect(&m_delayedQueryTimer, SIGNAL(timeout()), this, SLOT(delayedQueryLaunch()));

    QTimer::singleShot(0, this, SLOT(resetInterface()));
}

bool Interface::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (KWindowSystem::activeWindow() != winId()) {
            // this overcomes problems with click-to-focus and being a Dock window
            KWindowSystem::forceActiveWindow(winId());
            searchTermSetFocus();
        }
    }

    return KRunnerDialog::eventFilter(obj, event);
}

void Interface::saveDialogSize(KConfigGroup &group)
{
    group.writeEntry("Size", size());
}

void Interface::restoreDialogSize(KConfigGroup &group)
{
    resize(group.readEntry("Size", size()));
}

void Interface::updateSystemActivityToolTip()
{
    /* Set the tooltip for the Show System Activity button to include the global shortcut */
    KRunnerApp *krunnerApp = KRunnerApp::self();
    KAction *showSystemActivityAction = dynamic_cast<KAction *>(krunnerApp->actionCollection()->action("Show System Activity"));
    if (showSystemActivityAction) {
        QString shortcut = showSystemActivityAction->globalShortcut().toString();
        if (shortcut.isEmpty()) {
            m_activityButton->setToolTip( showSystemActivityAction->toolTip() );
        } else {
            m_activityButton->setToolTip( i18nc("tooltip, shortcut", "%1 (%2)", showSystemActivityAction->toolTip(), shortcut));
        }
    }
}

void Interface::setConfigWidget(QWidget *w)
{
    //FIXME: would like to use kephal here, but it doesn't provide an availableGeometry call
    const int screenId = qApp->desktop()->screenNumber(this); //Kephal::ScreenUtils::screenId(geometry().center());
    const int maxHeight = qApp->desktop()->availableGeometry(screenId).height(); //Kephal::Screens::self()->screen(screenId)->geometry().height();

    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    const int padding = top + bottom + m_activityButton->height();
    resize(width(), qMin(maxHeight, qMax(w->sizeHint().height() + padding, m_defaultSize.height())));

    m_resultsView->hide();
    m_searchTerm->setEnabled(false);
    m_layout->addWidget(w);

    connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(configWidgetDestroyed()));
}

void Interface::configWidgetDestroyed()
{
    QTimer::singleShot(0, this, SLOT(cleanupAfterConfigWidget()));
}

void Interface::cleanupAfterConfigWidget()
{
    m_searchTerm->setEnabled(true);
    resetInterface();
    searchTermSetFocus();
}

void Interface::resizeEvent(QResizeEvent *event)
{
    // We set m_defaultSize only when the event is spontaneous, i.e. when the user resizes the
    // window, or if they are manually resizing it
    if ((freeFloating() && event->spontaneous()) || isManualResizing()) {
        m_defaultSize = size();
    }

    m_resultsView->resize(m_buttonContainer->width(), m_resultsView->height());
    m_resultsScene->setWidth(m_resultsView->width());
    KRunnerDialog::resizeEvent(event);
}

Interface::~Interface()
{
    KRunnerSettings::setPastQueries(m_searchTerm->historyItems());
    KRunnerSettings::setQueryTextCompletionMode(m_searchTerm->completionMode());
    KRunnerSettings::self()->writeConfig();

    // Before saving the size we resize to the default size, with the results container shown.
    resize(m_defaultSize);
    KConfigGroup interfaceConfig(KGlobal::config(), "Interface");
    saveDialogSize(interfaceConfig);
    KGlobal::config()->sync();
}


void Interface::searchTermSetFocus()
{
    if (m_runnerManager->singleMode()) {
        m_singleRunnerSearchTerm->setFocus();
    } else {
        m_searchTerm->setFocus();
    }
}


void Interface::themeUpdated()
{
    //reset the icons
    m_helpButton->setIcon(m_iconSvg->pixmap("help"));
    m_configButton->setIcon(m_iconSvg->pixmap("configure"));
    m_activityButton->setIcon(m_iconSvg->pixmap("status"));
    m_closeButton->setIcon(m_iconSvg->pixmap("close"));
}

void Interface::clearHistory()
{
    m_searchTerm->clearHistory();
    KRunnerSettings::setPastQueries(m_searchTerm->historyItems());
}

void Interface::display(const QString &term)
{
    if (!term.isEmpty() || !isVisible() ||
        m_runnerManager->singleMode() != m_singleRunnerIcon->isVisible()) {
        resetInterface();
    }

    positionOnScreen();
    searchTermSetFocus();

    if (m_runnerManager->singleMode()) {
        if (term.isEmpty()) {
            // We need to manually trigger queryTextEdited, since
            // with an empty query it won't be triggered; still we need it
            // to launch the query
            queryTextEdited(QString());
        } else {
            m_singleRunnerSearchTerm->setText(term);
        }
    } else if (!term.isEmpty()) {
        m_searchTerm->setItemText(0, term);
    }
}

void Interface::resetInterface()
{
    setStaticQueryMode(false);
    m_delayedRun = false;
    m_searchTerm->setCurrentItem(QString(), true, 0);
    m_singleRunnerSearchTerm->clear();
    if (!m_running) {
        m_resultsScene->clearQuery();
    }
    resetResultsArea();
    resize(qMax(minimumSizeHint().width(), m_defaultSize.width()), minimumSizeHint().height());
}

void Interface::showHelp()
{
    QMap<QString, Plasma::QueryMatch> matches;
    QList<Plasma::AbstractRunner*> runnerList;

    Plasma::AbstractRunner *singleRunner = m_runnerManager->singleModeRunner();
    if (singleRunner) {
        runnerList << singleRunner;
    } else {
        runnerList = m_runnerManager->runners();
    }


    foreach (Plasma::AbstractRunner *runner, runnerList) {
        int count = 0;
        QIcon icon(runner->icon());
        if (icon.isNull()) {
            icon = KIcon("system-run");
        }

        foreach (const Plasma::RunnerSyntax &syntax, runner->syntaxes()) {
            Plasma::QueryMatch match(0);
            match.setType(Plasma::QueryMatch::InformationalMatch);
            match.setIcon(icon);
            match.setText(syntax.exampleQueriesWithTermDescription().join(", "));
            match.setSubtext(syntax.description() + "\n" +
                             i18n("(From %1, %2)", runner->name(), runner->description()));
            match.setData(syntax.exampleQueries().first());
            matches.insert(runner->name() + QString::number(++count), match);
        }
    }

    m_resultsScene->setQueryMatches(matches.values());
}

void Interface::setStaticQueryMode(bool staticQuery)
{
    // don't show the search and other control buttons in the case of a static querymatch
    const bool visible = !staticQuery;
    Plasma::AbstractRunner *singleRunner = m_runnerManager->singleModeRunner();
    m_configButton->setVisible(visible && !singleRunner);
    m_activityButton->setVisible(visible && !singleRunner);
    m_helpButton->setVisible(visible);
    m_searchTerm->setVisible(visible && !singleRunner);
    m_singleRunnerSearchTerm->setVisible(visible && singleRunner);
    if (singleRunner) {
        m_singleRunnerIcon->setPixmap(singleRunner->icon().pixmap(QSize(22, 22)));
        m_singleRunnerDisplayName->setText(singleRunner->name());
    }
    m_singleRunnerIcon->setVisible(singleRunner);
    m_singleRunnerDisplayName->setVisible(singleRunner);
}

void Interface::hideEvent(QHideEvent *e)
{
    resetInterface();
    KRunnerDialog::hideEvent(e);
}

void Interface::run(ResultItem *item)
{
    if (!item || !item->isValid() || item->group() < Plasma::QueryMatch::PossibleMatch) {
        m_delayedRun = true;
        return;
    }

    kDebug() << item->name() << item->id();
    m_delayedRun = false;

    if (item->group() == Plasma::QueryMatch::InformationalMatch) {
        QString info = item->data();
        int editPos = info.length();

        if (!info.isEmpty()) {
            if (item->isQueryPrototype()) {
                // lame way of checking to see if this is a Help Button generated match!
                int index = info.indexOf(":q:");

                if (index != -1) {
                    editPos = index;
                    info.replace(":q:", "");
                }
            }

            QStringList history = m_searchTerm->historyItems();
            history.prepend(m_searchTerm->currentText().trimmed());
            kDebug() << m_searchTerm->currentText() << history;
            m_searchTerm->setHistoryItems(history);
            m_searchTerm->setCurrentIndex(0);
            m_searchTerm->lineEdit()->setText(info);
            m_searchTerm->lineEdit()->setCursorPosition(editPos);
            QApplication::clipboard()->setText(info);
        }
        return;
    }

    //TODO: check if run is succesful before adding the term to history
    m_searchTerm->addToHistory(m_searchTerm->currentText().trimmed());

    m_running = true;
    // must run the result first before clearing the interface
    // in a way that will cause the results scene to be cleared and
    // the RunnerManager to be cleared of context as a result
    close();
    m_resultsScene->run(item);
    m_running = false;

    resetInterface();
}

void Interface::resetAndClose()
{
    resetInterface();
    close();
}


void Interface::runDefaultResultItem()
{
    if (m_queryRunning || m_delayedQueryTimer.isActive()) {
        m_delayedRun = true;
    } else {
        run(m_resultsScene->defaultResultItem());
    }
}

void Interface::queryTextEdited(const QString &query)
{
    m_delayedRun = false;

    if (query.isEmpty() && !m_runnerManager->singleMode()) {
        m_delayedQueryTimer.stop();
        resetInterface();
        m_queryRunning = false;
    } else {
        m_delayedQueryTimer.start();
    }
}

void Interface::delayedQueryLaunch()
{
    const QString query = (m_runnerManager->singleMode() ? m_singleRunnerSearchTerm->userText()
                                                         : static_cast<KLineEdit*>(m_searchTerm->lineEdit())->userText());
    QString runnerId;
    if (m_runnerManager->singleMode()) {
        runnerId = m_runnerManager->singleModeRunnerId();
    }
    if (!query.isEmpty() || m_runnerManager->singleMode()) {
        m_queryRunning = m_resultsScene->launchQuery(query, runnerId) || m_queryRunning; //lazy OR?
    }
}

void Interface::matchCountChanged(int count)
{
    m_queryRunning = false;
    bool show = count > 0;
    m_hideResultsTimer.stop();

    if (show && m_delayedRun) {
        kDebug() << "delayed run with" << count << "items";
        runDefaultResultItem();
        return;
    }

    if (show) {
        //kDebug() << "showing!" << minimumSizeHint();

        QSize s = m_defaultSize;
        const int minHeight = minimumSizeHint().height();
        const int resultsHeight = m_resultsScene->viewableHeight();
        //kDebug() << minHeight << resultsHeight << s.height();
        if (minHeight + resultsHeight < s.height()) {
            s.setHeight(minHeight + resultsHeight);
            m_resultsView->resize(m_resultsView->width(), resultsHeight + 2);
        }
        resize(s);

        if (!m_resultsView->isVisible()) {
            // Next 2 lines are a workaround to allow arrow
            // keys navigation in krunner's result list.
            // Patch submited in bugreport #211578
            QEvent event(QEvent::WindowActivate);
            QApplication::sendEvent(m_resultsView, &event);

            m_resultsView->show();
        }
        //m_resultsScene->resize(m_resultsView->width(), qMax(m_resultsView->height(), int(m_resultsScene->height())));
        //kDebug() << s << size();
    } else {
        //kDebug() << "hiding ... eventually";
        m_delayedRun = false;
        m_hideResultsTimer.start(1000);
    }
}

void Interface::hideResultsArea()
{
    searchTermSetFocus();
    resetResultsArea();
    resize(qMax(minimumSizeHint().width(), m_defaultSize.width()), minimumSizeHint().height());
}

void Interface::resetResultsArea()
{
    m_resultsView->hide();
    setMinimumSize(QSize(MIN_WIDTH, 0));
    adjustSize();
}

#include "interface.moc"
