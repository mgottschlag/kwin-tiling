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

// #include <threadweaver/DebuggingAids.h>
// #include <ThreadWeaver/Thread>
#include <ThreadWeaver/Job>
#include <ThreadWeaver/QueuePolicy>
#include <ThreadWeaver/Weaver>
#include <QMutex>

#include <plasma/abstractrunner.h>

#include "runners/services/servicerunner.h"
#include "runners/sessions/sessionrunner.h"
#include "runners/shell/shellrunner.h"
#include "collapsiblewidget.h"
#include "interfaceadaptor.h"

using ThreadWeaver::Weaver;
using ThreadWeaver::Job;

// A little hack of a class to let us easily activate a match

class SearchMatch : public QListWidgetItem
{
    public:
        SearchMatch(Plasma::SearchMatch* action, QListWidget* parent)
            : QListWidgetItem( parent ),
              m_default(false),
              m_action(0)
        {
            setAction(action);
        }

        void activate()
        {
            m_action->exec();
        }

        bool actionEnabled()
        {
            return m_action->isEnabled();
        }

        bool hasMatchOptions()
        {
            return m_action->runner()->hasMatchOptions();
        }

        void setAction(Plasma::SearchMatch* action)
        {
            m_action = action;
            setIcon(m_action->icon());
            setText(i18n("%1 (%2)",
                    m_action->text(),
                    m_action->runner()->objectName()));

            // in case our new action is now enabled and the old one wasn't, or
            // vice versa
            setDefault(m_default);
        }

        QString toString()
        {
            return m_action->data().toString();
        }

        Plasma::SearchMatch::Type actionType()
        {
            return m_action->type();
        }

        /*Plasma::SearchMatch* action()
        {
            return m_action;
        }*/

        void createMatchOptions(QWidget* parent)
        {
            m_action->runner()->createMatchOptions(parent);
        }

        void setDefault( bool def ) {
            if ( m_default == def ) {
                return;
            }

            m_default = def;

            if ( m_default ) {
                if ( m_action->isEnabled() ) {
                    setText( text().prepend( i18n("Default: ") ) );
                }
            } else {
                setText( text().mid( 9 ) );
            }
        }

    private:
        bool m_default;
        Plasma::SearchMatch* m_action;
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

    QWidget* w = mainWidget();
    m_layout = new QVBoxLayout(w);
    m_layout->setMargin(0);

    m_header = new KTitleWidget(w);
    m_header->setBackgroundRole( QPalette::Base );
    m_layout->addWidget( m_header );

    m_searchTerm = new KHistoryComboBox(false,w);
    m_searchTerm->setDuplicatesEnabled(false);
    KLineEdit *lineEdit = new KLineEdit(m_searchTerm);
    lineEdit->setCompletionObject(m_context.completionObject());
    lineEdit->setClearButtonShown(true);
    m_searchTerm->setLineEdit(lineEdit);
    m_header->setBuddy(m_searchTerm);
    m_layout->addWidget(m_searchTerm);
    connect(m_searchTerm, SIGNAL(textChanged(QString)),
            this, SLOT(queueMatch()));
    connect(m_searchTerm, SIGNAL(returnPressed()),
            this, SLOT(exec()));

    KConfigGroup cg(KGlobal::config(), "General");
    QStringList executions = cg.readEntry("pastqueries", QStringList());
    //Handle updates to the completion object as well
    m_searchTerm->setHistoryItems(executions, true);

    //TODO: temporary feedback, change later with the "icon parade" :)
    m_matchList = new QListWidget(w);
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
    connect( m_runButton, SIGNAL( clicked(bool) ), SLOT(exec()) );
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
    m_runners.append( new ShellRunner( this ) );
    m_runners.append( new ServiceRunner( this ) );
    m_runners.append( new SessionRunner( this ) );
    QStringList whitelist = cg.readEntry( "runners",QStringList() );
    m_runners += Plasma::AbstractRunner::loadRunners( this, whitelist );

    connect(&m_context, SIGNAL(matchesChanged()), this, SLOT(queueUpdates()));

    resetInterface();

//     ThreadWeaver::setDebugLevel(true, 4);
}

Interface::~Interface()
{
    KConfigGroup cg(KGlobal::config(), "General");
    cg.writeEntry("pastqueries", m_searchTerm->historyItems());
    m_context.clearMatches();
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

    display();
    m_header->setText(i18n("Switch users"));
    m_header->setPixmap("system-switch-user");
    m_defaultMatch = 0;
    m_context.setSearchTerm("SESSIONS");
    sessionrunner->match(&m_context);

    foreach (Plasma::SearchMatch *action, m_context.exactMatches()) {
        bool makeDefault = action->type() != Plasma::SearchMatch::InformationalMatch;

        SearchMatch *match = new SearchMatch(action, m_matchList);

        if (makeDefault) {
            m_defaultMatch = match;
            m_defaultMatch->setDefault(true);
            m_runButton->setEnabled(true);
            m_optionsButton->setEnabled(sessionrunner->hasMatchOptions());
        }
    }

    if (!m_defaultMatch) {
        m_matchList->addItem(i18n("No desktop sessions available"));
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
    m_context.setSearchTerm(QString());
    m_searchTerm->setCurrentItem(QString(), true);
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
        return;
    }

    if (!match->actionEnabled()) {
        return;
    }

    QString searchTerm = m_searchTerm->currentText();
    m_searchTerm->addToHistory(searchTerm);

    if (match->actionType() == Plasma::SearchMatch::InformationalMatch) {
        m_searchTerm->setItemText(0, match->toString());
    } else {
        //kDebug() << "match activated! " << match->text();
        match->activate();
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
    //FIXME: this induces rediculously bad flicker
    m_matchList->clear();

    int matchCount = 0;
    m_defaultMatch = 0;
    QList<QList<Plasma::SearchMatch *> > matchLists;
    matchLists << m_context.informationalMatches()
                      << m_context.exactMatches()
                      << m_context.possibleMatches();

    foreach (QList<Plasma::SearchMatch *> matchList, matchLists) {
        foreach (Plasma::SearchMatch *action, matchList) {
            bool makeDefault = !m_defaultMatch && action->isEnabled();

            SearchMatch *match = new SearchMatch(action, 0);
            m_matchList->insertItem(matchCount, match);

            if (makeDefault &&
                action->relevance() > 0 &&
                (action->type() != Plasma::SearchMatch::InformationalMatch ||
                 !action->data().toString().isEmpty())) {
                match->setDefault(true);
                m_defaultMatch = match;
                m_optionsButton->setEnabled(action->runner()->hasMatchOptions());
                m_runButton->setEnabled(true);
            }

            ++matchCount;
        }
    }

    if (!m_defaultMatch) {
        if (m_execQueued && Weaver::instance()->isIdle() ) {
            m_execQueued = false;
        }
        showOptions(false);
        m_runButton->setEnabled(false);
    } else if (m_execQueued) {
        m_execQueued = false;
        exec();
    }
}

void Interface::exec()
{
    if (!m_execQueued && m_searchTerm->completionBox() && m_searchTerm->completionBox()->isVisible()) {
        queueMatch();
        return;
    }

    kDebug() << "match list has" << m_matchList->count() << "items";

    QListWidgetItem* currentMatch = m_matchList->currentItem();
    if (!currentMatch) {
        if (m_defaultMatch) {
            currentMatch = m_defaultMatch;
        } else if (m_matchList->count() < 2) {
            //TODO: the < 2 is a bit of a hack; we *always* get a search option returned,
            //      so if we only have 1 item and it's not selected, guess what it is? ;)
            //      we might be able to do better here.
            m_execQueued = true;
            match();
            return;
        }
    }
    matchActivated(currentMatch);
}

void Interface::showOptions(bool show)
{
    //TODO: in the case where we are no longer showing options
    //      should we have the runner delete it's options?
    if (show) {
        if (!m_defaultMatch || !m_defaultMatch->hasMatchOptions()) {
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
        m_defaultMatch->createMatchOptions(m_optionsWidget);
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
        hasOptions = m_defaultMatch && m_defaultMatch->hasMatchOptions();
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
