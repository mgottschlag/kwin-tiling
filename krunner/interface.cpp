/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "interface.h"

#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QShortcut>
#include <QTimer>
#include <QHideEvent>
#include <QClipboard>

#include <KActionCollection>
#include <KHistoryComboBox>
#include <KCompletionBox>
#include <KDebug>
#include <KDialog>
#include <KLineEdit>
#include <KLocale>
#include <KGlobalSettings>
#include <KPushButton>
#include <KStandardGuiItem>
#include <KTitleWidget>
#include <KWindowSystem>

#include <Solid/Device>
#include <Solid/DeviceInterface>

// #include <threadweaver/DebuggingAids.h>
// #include <ThreadWeaver/Thread>
#include <ThreadWeaver/Job>
#include <ThreadWeaver/QueuePolicy>
#include <ThreadWeaver/Weaver>
#include <QMutex>

#include <plasma/abstractrunner.h>

#include "collapsiblewidget.h"
#include "interfaceadaptor.h"
#include "krunnersettings.h"

using ThreadWeaver::Weaver;
using ThreadWeaver::Job;

// A little hack of a class to let us easily activate a match

class SearchMatch : public QListWidgetItem
{
    public:
        SearchMatch(const Plasma::SearchMatch* action, QListWidget* parent)
            : QListWidgetItem( parent ),
              m_default(false),
              m_action(0)
        {
            m_action = action;
            setIcon(m_action->icon());

            if (action->subtext().isEmpty()) {
                setText(i18n("%1 (%2)",
                        m_action->text(),
                        m_action->runner()->objectName()));
            } else {
                setText(i18n("%1 (%2, %3)",
                        m_action->text(),
                        m_action->subtext(),
                        m_action->runner()->objectName()));
            }
        }

        void activate(const Plasma::SearchContext *context) const
        {
            m_action->run(context);
        }

        bool actionEnabled() const
        {
            return m_action->isEnabled();
        }

        bool hasRunOptions() const
        {
            return m_action->runner()->hasRunOptions();
        }

        QString toString() const
        {
            return m_action->data().toString();
        }

        Plasma::SearchMatch::Type actionType() const
        {
            return m_action->type();
        }

        qreal actionRelevance() const
        {
            return m_action->relevance();
        }

        void createRunOptions(QWidget* parent) const
        {
            m_action->runner()->createRunOptions(parent);
        }

        void setDefault(bool def)
        {
            if (m_default == def) {
                return;
            }

            m_default = def;

            if (m_default) {
                if (m_action->isEnabled()) {
                    setText(text().prepend(i18n("Default: ")));
                } else {
                    m_default = false;
                }
            } else {
                setText(text().mid(9));
            }
        }

        bool operator<(const QListWidgetItem & other) const
        {
            // Rules:
            //      0. Default wins. Always.
            //      1. Exact trumps informational
            //      2. Informational trumps possible
            //      3. Higher relevance wins

            const SearchMatch *otherMatch = dynamic_cast<const SearchMatch*>(&other);

            if (!otherMatch) {
                return QListWidgetItem::operator<(other);
            }

            if (otherMatch->m_default) {
                return true;
            }

            Plasma::SearchMatch::Type myType = m_action->type();
            Plasma::SearchMatch::Type otherType = otherMatch->m_action->type();

            if (myType != otherType) {
                return myType < otherType;
            }

            return m_action->relevance() < otherMatch->m_action->relevance();
        }

    private:
        bool m_default;
        const Plasma::SearchMatch* m_action;
};

// Restricts simultaneous jobs of the same type
// Similar to ResourceRestrictionPolicy but check the object type first
class RunnerRestrictionPolicy : public ThreadWeaver::QueuePolicy
{
public:
    ~RunnerRestrictionPolicy();

    static RunnerRestrictionPolicy& instance();

    void setCap(int cap)
    {
        m_cap = cap;
    }
    int cap() const
    {
        return m_cap;
    }

    bool canRun(Job* job);
    void free(Job* job);
    void release(Job* job);
    void destructed(Job* job);
private:
    RunnerRestrictionPolicy();

//     QHash<QString, int> m_runCounts;
    int m_count;
    int m_cap;
    QMutex m_mutex;
};

RunnerRestrictionPolicy::RunnerRestrictionPolicy()
    : QueuePolicy(),
      m_cap(2)
{
}

RunnerRestrictionPolicy::~RunnerRestrictionPolicy()
{
}

RunnerRestrictionPolicy& RunnerRestrictionPolicy::instance()
{
    static RunnerRestrictionPolicy policy;
    return policy;
}

bool RunnerRestrictionPolicy::canRun(Job* job)
{
    Q_UNUSED(job)
    QMutexLocker l(&m_mutex);
//     QString type = job->objectName();
    if (m_count/*m_runCounts.value(type)*/ > m_cap) {
//         kDebug() << "Denying job " << type << " because of " << m_count/*m_runCounts[type]*/ << " current jobs" << endl;
        return false;
    } else {
//         m_runCounts[type]++;
        ++m_count;
        return true;
    }
}

void RunnerRestrictionPolicy::free(Job* job)
{
    Q_UNUSED(job)
    QMutexLocker l(&m_mutex);
//     QString type = job->objectName();
    --m_count;
//     if (m_runCounts.value(type)) {
//         m_runCounts[type]--;
//     }
}

void RunnerRestrictionPolicy::release(Job* job)
{
    free(job);
}

void RunnerRestrictionPolicy::destructed(Job* job)
{
    Q_UNUSED(job)
}

// Class to run queries in different threads
class FindMatchesJob : public Job
{
public:
    FindMatchesJob(const QString& term, Plasma::AbstractRunner* runner, Plasma::SearchContext* context, QObject* parent = 0);

    int priority() const;

protected:
    void run();
private:
    QString m_term;
    Plasma::SearchContext* m_context;
    Plasma::AbstractRunner* m_runner;
};

FindMatchesJob::FindMatchesJob( const QString& term, Plasma::AbstractRunner* runner, Plasma::SearchContext* context, QObject* parent )
    : ThreadWeaver::Job(parent),
      m_term(term),
      m_context(context),
      m_runner(runner)
{
//     setObjectName( runner->objectName() );
    if (runner->speed() == Plasma::AbstractRunner::SlowSpeed) {
        assignQueuePolicy(&RunnerRestrictionPolicy::instance());
    }
}

void FindMatchesJob::run()
{
//     kDebug() << "Running match for " << m_runner->objectName() << " in Thread " << thread()->id() << endl;
    m_runner->performMatch(*m_context);
}

int FindMatchesJob::priority() const
{
    return m_runner->priority();
}

Interface::Interface(QWidget* parent)
    : KRunnerDialog( parent ),
      m_expander(0),
      m_optionsWidget(0),
      m_defaultMatch(0),
      m_execQueued(false)
{
    setWindowTitle( i18n("Run Command") );
    setWindowIcon(KIcon("system-run"));

    m_matchTimer.setSingleShot(true);
    connect(&m_matchTimer, SIGNAL(timeout()), this, SLOT(match()));

    m_updateTimer.setSingleShot(true);
    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateMatches()));

    const int numProcs = qMax(Solid::Device::listFromType(Solid::DeviceInterface::Processor).count(), 1);
    const int numThreads = qMin(KRunnerSettings::maxThreads(), 2 + ((numProcs - 1) * 2));
    //kDebug() << "setting up" << numThreads << "threads for" << numProcs << "processors";
    Weaver::instance()->setMaximumNumberOfThreads(numThreads);

    QWidget* w = mainWidget();
    m_layout = new QVBoxLayout(w);
    m_layout->setMargin(0);

    m_header = new KTitleWidget(w);
    m_header->setBackgroundRole( QPalette::Base );
    m_layout->addWidget( m_header );

    m_searchTerm = new KHistoryComboBox(false,w);
    m_searchTerm->setDuplicatesEnabled(false);

    KLineEdit *lineEdit = new KLineEdit(m_searchTerm);

    // the order of these next few lines if very important.
    // QComboBox::setLineEdit sets the autoComplete flag on the lineedit,
    // and KComboBox::setAutoComplete resets the autocomplete mode! ugh!
    m_searchTerm->setLineEdit(lineEdit);
    lineEdit->setCompletionObject(m_context.completionObject());
    lineEdit->setCompletionMode(static_cast<KGlobalSettings::Completion>(KRunnerSettings::queryTextCompletionMode()));
    lineEdit->setClearButtonShown(true);

    m_header->setBuddy(m_searchTerm);
    m_layout->addWidget(m_searchTerm);
    connect(m_searchTerm, SIGNAL(editTextChanged(QString)),
            this, SLOT(queueMatch()));
    connect(m_searchTerm, SIGNAL(returnPressed()),
            this, SLOT(run()));

    QStringList executions = KRunnerSettings::pastQueries();
    //Handle updates to the completion object as well
    m_searchTerm->setHistoryItems(executions, true);

    //TODO: temporary feedback, change later with the "icon parade" :)
    m_matchList = new QListWidget(w);
    //m_matchList->setSortingEnabled(true);

    connect( m_matchList, SIGNAL(itemActivated(QListWidgetItem*)),
             SLOT(matchActivated(QListWidgetItem*)) );
    connect( m_matchList, SIGNAL(itemClicked(QListWidgetItem*)),
             SLOT(setDefaultItem(QListWidgetItem*)) );
    m_layout->addWidget(m_matchList);

    // buttons at the bottom
    QHBoxLayout* bottomLayout = new QHBoxLayout(w);
    m_optionsButton = new KPushButton(KStandardGuiItem::configure(), this);
    m_optionsButton->setText( i18n( "Show Options" ) );
    m_optionsButton->setEnabled( false );
    m_optionsButton->setCheckable( true );
    connect( m_optionsButton, SIGNAL(toggled(bool)), SLOT(showOptions(bool)) );
    bottomLayout->addWidget( m_optionsButton );

    KPushButton *activityButton = new KPushButton(w);
    activityButton->setText(i18n("Show System Activity"));
    activityButton->setIcon(KIcon("utilities-system-monitor"));
    connect(activityButton, SIGNAL(clicked()), qApp, SLOT(showTaskManager()));
    connect(activityButton, SIGNAL(clicked()), this, SLOT(close()));
    bottomLayout->addWidget(activityButton);

    bottomLayout->addStretch();

    QString runButtonWhatsThis = i18n( "Click to execute the selected item above" );
    m_runButton = new KPushButton(KGuiItem(i18n( "Launch" ), "system-run",
                                           QString(), runButtonWhatsThis),
                                  w);
    m_runButton->setEnabled( false );
    m_runButton->setDefault(true);
    connect( m_runButton, SIGNAL( clicked(bool) ), SLOT(run()) );
    bottomLayout->addWidget( m_runButton );

    m_cancelButton = new KPushButton(KStandardGuiItem::cancel(), w);
    connect( m_cancelButton, SIGNAL(clicked(bool)), SLOT(close()) );
    bottomLayout->addWidget( m_cancelButton );

    m_layout->addLayout (bottomLayout );

    new InterfaceAdaptor( this );
    QDBusConnection::sessionBus().registerObject( "/Interface", this );

    new QShortcut( QKeySequence( Qt::Key_Escape ), this, SLOT(close()) );

    //FIXME: what size should we be?
    resize(400, 250);

    //setWidgetPalettes();
    //connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
    //        SLOT(setWidgetPalettes()));

    //TODO: how should we order runners, particularly ones loaded from plugins?
    QStringList whitelist = KRunnerSettings::runners();
    m_runners += Plasma::AbstractRunner::loadAll(this, whitelist);

    connect(&m_context, SIGNAL(matchesChanged()), this, SLOT(queueUpdates()));

    resetInterface();

//     ThreadWeaver::setDebugLevel(true, 4);
}

Interface::~Interface()
{
    KRunnerSettings::setPastQueries(m_searchTerm->historyItems());
    KRunnerSettings::setQueryTextCompletionMode(m_searchTerm->completionMode());
    m_context.removeAllMatches();
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

    if (!isVisible()) {
        queueMatch();
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

void Interface::switchUser()
{
    Plasma::AbstractRunner *sessionrunner = 0;
    foreach (Plasma::AbstractRunner* runner, m_runners) {
        if (qstrcmp(runner->metaObject()->className(), "SessionRunner") == 0) {
            sessionrunner = runner;
            break;
        }
    }

    if (!sessionrunner) {
        kDebug() << "Could not find the Sessionrunner; not showing any sessions!";
        return;
    }
    //TODO: ugh, magic strings. See sessions/sessionrunner.cpp
    display("SESSIONS");
    m_header->setText(i18n("Switch users"));
    m_header->setPixmap("system-switch-user");
    m_defaultMatch = 0;
    m_context.reset();
    m_context.setSearchTerm("SESSIONS");
    sessionrunner->match(&m_context);

    foreach (const Plasma::SearchMatch *action, m_context.matches()) {
        bool makeDefault = !m_defaultMatch && action->type() != Plasma::SearchMatch::InformationalMatch;

        SearchMatch *match = new SearchMatch(action, m_matchList);

        if (makeDefault) {
            m_defaultMatch = match;
            m_defaultMatch->setDefault(true);
            m_runButton->setEnabled(true);
            m_optionsButton->setEnabled(sessionrunner->hasRunOptions());
        }
    }

    if (!m_defaultMatch) {
        m_matchList->addItem(i18n("No desktop sessions available"));
    } else {
        m_matchList->sortItems(Qt::DescendingOrder);
    }
}

void Interface::setWidgetPalettes()
{
    // a nice palette to use with the widgets
    QPalette widgetPalette = palette();
    QColor headerBgColor = widgetPalette.color( QPalette::Active,
                                                QPalette::Base );
    headerBgColor.setAlpha( 200 );
    widgetPalette.setColor( QPalette::Base, headerBgColor );

    m_header->setPalette( widgetPalette );
    m_searchTerm->setPalette( widgetPalette );
    m_matchList->setPalette( widgetPalette );
}

void Interface::resetInterface()
{
    m_header->setText(i18n("Enter the name of an application, location or search term below."));
    m_header->setPixmap("system-search");
    m_defaultMatch = 0;
    m_context.reset();
    m_searchTerm->setCurrentItem(QString(), true, 0);
    m_matchList->clear();
    m_runButton->setEnabled( false );
    m_optionsButton->setEnabled( false );
    showOptions( false );
    m_matchTimer.stop();
}

void Interface::closeEvent(QCloseEvent* e)
{
    resetInterface();
    e->accept();
}

void Interface::matchActivated(QListWidgetItem* item)
{
    SearchMatch* match = dynamic_cast<SearchMatch*>(item);

    if (!match) {
        //kDebug() << "not a SearchMatch object";
        return;
    }

    if (!match->actionEnabled()) {
        //kDebug() << "item not enabled";
        return;
    }

    QString searchTerm = m_searchTerm->currentText();
    m_searchTerm->addToHistory(searchTerm);

    if (match->actionType() == Plasma::SearchMatch::InformationalMatch) {
        //kDebug() << "informational match activated" << match->toString();
        m_searchTerm->setItemText(m_searchTerm->currentIndex(), match->toString());
    } else {
        //kDebug() << "match activated! " << match->text();
        QString term = m_searchTerm->currentText().trimmed();
        m_context.setSearchTerm(term);

        match->activate(&m_context);
        close();
    }
}

void Interface::queueMatch()
{
    if (m_matchTimer.isActive()) {
        return;
    }
    Weaver::instance()->dequeue();
    m_matchTimer.start(200);
}

void Interface::queueUpdates()
{
    if (m_updateTimer.isActive()) {
        return;
    }
    //Wait 100ms between updating matches
    m_matchList->clear();
    m_updateTimer.start(100);
}

void Interface::match()
{
    // If ThreadWeaver is idle, it is safe to clear previous jobs
    if ( Weaver::instance()->isIdle() ) {
        qDeleteAll( m_searchJobs );
        m_searchJobs.clear();
    }
    m_defaultMatch = 0;

    QString term = m_searchTerm->currentText().trimmed();

    if (term.isEmpty()) {
        resetInterface();
        m_execQueued = false;
        return;
    }

    if (m_context.searchTerm() == term) {
        // we already are searching for this!
        return;
    }

    m_context.reset();
    m_context.setSearchTerm(term);
    m_context.addStringCompletions(m_searchTerm->historyItems());

    foreach (Plasma::AbstractRunner* runner, m_runners) {
        Job *job = new FindMatchesJob(term, runner, &m_context, this);
        Weaver::instance()->enqueue( job );
        m_searchJobs.append( job );
    }
}

void Interface::updateMatches()
{
    m_matchList->clear();
    m_defaultMatch = 0;

    foreach (const Plasma::SearchMatch *action, m_context.matches()) {
        SearchMatch *match = new SearchMatch(action, m_matchList);

        if (action->isEnabled() && action->relevance() > 0 &&
            (!m_defaultMatch || *m_defaultMatch < *match) &&
            (action->type() != Plasma::SearchMatch::InformationalMatch ||
             !action->data().toString().isEmpty())) {
            if (m_defaultMatch) {
                m_defaultMatch->setDefault(false);
            }

            match->setDefault(true);
            m_defaultMatch = match;
            m_optionsButton->setEnabled(action->runner()->hasRunOptions());
            m_runButton->setEnabled(true);
        }
    }

    m_matchList->sortItems(Qt::DescendingOrder);

    if (!m_defaultMatch) {
        m_execQueued = false;
        showOptions(false);
        m_runButton->setEnabled(false);
    } else if (m_execQueued) {
        m_execQueued = false;
        run();
    }
}

void Interface::run()
{
    if (!m_execQueued && m_searchTerm->completionBox() && m_searchTerm->completionBox()->isVisible()) {
        queueMatch();
        return;
    }

    //kDebug() << "match list has" << m_matchList->count() << "items";
    if (m_matchList->count() < 1) {
        if (m_searchTerm->currentText().length() > 0) {
            m_execQueued = true;
            match();
        }
        return;
    }

    QListWidgetItem* currentMatch = m_matchList->currentItem();
    if (!currentMatch) {
        if (m_defaultMatch) {
            //kDebug() << "exec'ing default match";
            currentMatch = m_defaultMatch;
        } else {
            for (int i = 0; i < m_matchList->count(); ++i) {
                SearchMatch* match = dynamic_cast<SearchMatch*>(m_matchList->item(i));
                if (match && match->actionEnabled() && match->actionRelevance() > 0 &&
                    match->actionType() != Plasma::SearchMatch::HelperMatch) {
                    currentMatch = match;
                    break;
                }
            }
            //kDebug() << "exec'ing first plausable item" << currentMatch;
        }
    }

    if (currentMatch) {
        matchActivated(currentMatch);
    }
}

void Interface::showOptions(bool show)
{
    //TODO: in the case where we are no longer showing options
    //      should we have the runner delete it's options?
    if (show) {
        if (!m_defaultMatch || !m_defaultMatch->hasRunOptions()) {
            // in this case, there is nothing to show
            return;
        }

        if (!m_expander) {
            //kDebug() << "creating m_expander";
            m_expander = new CollapsibleWidget( this );
            m_expander->show();
            connect( m_expander, SIGNAL( collapseCompleted() ),
                     m_expander, SLOT( close() ) );
            m_layout->insertWidget( 3, m_expander );
        }

        //kDebug() << "set inner widget to " << m_defaultMatch->runner()->options();
        delete m_optionsWidget;
        m_optionsWidget = new QWidget(this);
        m_defaultMatch->createRunOptions(m_optionsWidget);
        m_optionsButton->setText( i18n( "Hide Options" ) );
    } else {
        delete m_optionsWidget;
        m_optionsWidget = 0;
        m_optionsButton->setText( i18n( "Show Options" ) );
        resize( 400, 250 );
    }

    if ( m_expander ) {
        //TODO: we need to insert an element into the krunner dialog
        //      that is big enough for the options. this will prevent
        //      other items in the dialog from moving around and look
        //      more "natural"; it should appear as if a "drawer" is
        //      being pulled open, e.g. an expander.
        m_expander->setInnerWidget(m_optionsWidget);
        m_expander->setVisible(show);
        m_expander->setExpanded(show);
    }

    m_optionsButton->setChecked(show);
}

void Interface::setDefaultItem( QListWidgetItem* item )
{
    bool hasOptions = false;

    if (item) {
        if (m_defaultMatch) {
            m_defaultMatch->setDefault(false);
        }

        m_defaultMatch = dynamic_cast<SearchMatch*>(item);
        if (!m_defaultMatch) {
            return;
        }
        hasOptions = m_defaultMatch && m_defaultMatch->hasRunOptions();
    }

    m_optionsButton->setEnabled(hasOptions);

    if (hasOptions) {
        if (m_expander && m_expander->isExpanded()) {
            m_optionsButton->setText(i18n("Hide Options"));
        } else {
            m_optionsButton->setText(i18n("Show Options"));
        }
    } else {
        m_optionsButton->setText(i18n("Show Options"));
        m_optionsButton->setChecked(false);
        if (m_expander) {
            m_expander->hide();
        }
    }
}

#include "interface.moc"
