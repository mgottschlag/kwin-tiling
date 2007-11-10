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
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QShortcut>
#include <QTimer>
#include <QHideEvent>

#include <KActionCollection>
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

#include <plasma/abstractrunner.h>

#include "runners/services/servicerunner.h"
#include "runners/sessions/sessionrunner.h"
#include "runners/shell/shellrunner.h"
#include "collapsiblewidget.h"
#include "interfaceadaptor.h"

//#define FLASH_DIALOG 0

// A little hack of a class to let us easily activate a match

class SearchMatch : public QListWidgetItem
{
    public:
        SearchMatch(Plasma::SearchAction* action, QListWidget* parent)
            : QListWidgetItem( parent ),
              m_default(false),
              m_action(0)
        {
            setAction(action);
        }

        void activate()
        {
            m_action->activate( QAction::Trigger );
        }

        bool actionEnabled()
        {
            return m_action->isEnabled();
        }

        void setAction(Plasma::SearchAction* action)
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

        Plasma::SearchAction* action()
        {
            return m_action;
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
        Plasma::SearchAction* m_action;
};

Interface::Interface(QWidget* parent)
    : KRunnerDialog( parent ),
      m_nextRunner(0),
      m_expander(0),
      m_optionsWidget(0),
      m_defaultMatch(0)
{
    setWindowTitle( i18n("Run Command") );

    KConfigGroup cg(KGlobal::config(), "General");
    m_executions = cg.readEntry("pastqueries", m_executions);

    m_matchTimer.setSingleShot(true);
    connect(&m_matchTimer, SIGNAL(timeout()), this, SLOT(match()));

    QWidget* w = mainWidget();
    m_layout = new QVBoxLayout(w);
    m_layout->setMargin(0);

    m_header = new KTitleWidget(w);
    m_header->setBackgroundRole( QPalette::Base );
    m_layout->addWidget( m_header );

    m_searchTerm = new KLineEdit(w);
    m_header->setBuddy(m_searchTerm);
    m_searchTerm->clear();
    m_searchTerm->setClearButtonShown(true);
    m_searchTerm->setCompletionObject(m_context.completionObject());
    m_layout->addWidget(m_searchTerm);
    connect(m_searchTerm, SIGNAL(textChanged(QString)),
            this, SLOT(queueMatch()));
    connect(m_searchTerm, SIGNAL(returnPressed()),
            this, SLOT(exec()));

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
    m_optionsButton->setFlat( true );
    m_optionsButton->setEnabled( false );
    m_optionsButton->setCheckable( true );
    connect( m_optionsButton, SIGNAL(toggled(bool)), SLOT(showOptions(bool)) );
    bottomLayout->addWidget( m_optionsButton );

    KPushButton *activityButton = new KPushButton(w);
    activityButton->setText(i18n("Show System Activity"));
    activityButton->setIcon(KIcon("ksysguard"));
    activityButton->setFlat(true);
    connect(activityButton, SIGNAL(clicked()), qApp, SLOT(showTaskManager()));
    connect(activityButton, SIGNAL(clicked()), this, SLOT(hide()));
    bottomLayout->addWidget(activityButton);

    bottomLayout->addStretch();

    QString runButtonWhatsThis = i18n( "Click to execute the selected item above" );
    m_runButton = new KPushButton(KGuiItem(i18n( "Launch" ), "launch",
                                           QString(), runButtonWhatsThis),
                                  w);
    m_runButton->setFlat( true );
    m_runButton->setEnabled( false );
    connect( m_runButton, SIGNAL( clicked(bool) ), SLOT(exec()) );
    bottomLayout->addWidget( m_runButton );

    m_cancelButton = new KPushButton(KStandardGuiItem::cancel(), w);
    m_cancelButton->setFlat( true );
    connect( m_cancelButton, SIGNAL(clicked(bool)), SLOT(hide()) );
    bottomLayout->addWidget( m_cancelButton );

    m_layout->addLayout (bottomLayout );

    new InterfaceAdaptor( this );
    QDBusConnection::sessionBus().registerObject( "/Interface", this );

    new QShortcut( QKeySequence( Qt::Key_Escape ), this, SLOT(hide()) );

    //FIXME: what size should we be?
    resize(400, 250);

    setWidgetPalettes();
    connect( KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
             SLOT(setWidgetPalettes()) );

    //TODO: how should we order runners, particularly ones loaded from plugins?
    m_runners.append( new ShellRunner( this ) );
    m_runners.append( new ServiceRunner( this ) );
    m_runners.append( new SessionRunner( this ) );
    m_runners += Plasma::AbstractRunner::loadRunners( this );

    resetInterface();

#ifdef FLASH_DIALOG
    QTimer* t = new QTimer(this);
    connect( t, SIGNAL(timeout()), this, SLOT(display()));
    t->start( 250 );
#endif
}

Interface::~Interface()
{
    KConfigGroup cg(KGlobal::config(), "General");
    cg.writeEntry("pastqueries", m_executions);
}

void Interface::display(const QString& term)
{
    m_searchTerm->setFocus();

    if ( !term.isEmpty() ) {
        m_searchTerm->setText( term );
    }

    KWindowSystem::setOnDesktop( winId(), KWindowSystem::currentDesktop() );
    KDialog::centerOnScreen( this );
    show();
    KWindowSystem::forceActiveWindow( winId() );

    match();
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
    m_header->setPixmap("user");
    m_context.setSearchTerm("SESSIONS");
    sessionrunner->match(&m_context);

    foreach (Plasma::SearchAction *action, m_context.exactMatches()) {
        bool makeDefault = action->type() != Plasma::SearchAction::InformationalMatch;

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
    m_context.setSearchTerm(QString());
    m_searchTerm->clear();
    m_matchList->clear();
    m_runButton->setEnabled( false );
    m_optionsButton->setEnabled( false );
    showOptions( false );
}

void Interface::hideEvent( QHideEvent* e )
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

    Plasma::SearchAction *action = match->action();
    m_optionsButton->setEnabled(action->runner()->hasMatchOptions());

    if (!action->isEnabled()) {
        return;
    }

    if (action->type() == Plasma::SearchAction::InformationalMatch) {
        m_searchTerm->setText(action->data().toString());
    } else {
        //kDebug() << "match activated! " << match->text();
        match->activate();
        hide();
    }
}

void Interface::queueMatch()
{
    // start over when the timer fires on the first runner
    m_nextRunner = 0;
    
    // (re)start the timer
    m_matchTimer.start(200);

    QString term = m_searchTerm->text().trimmed();
    if (!term.isEmpty()) {
        m_context.setSearchTerm(term);
    }
}

void Interface::match()
{
    //FIXME: this induces rediculously bad flicker
    m_matchList->clear();

    int matchCount = 0;
    QString term = m_searchTerm->text().trimmed();

    if (term.isEmpty()) {
        resetInterface();
        return;
    }

    m_context.addStringCompletions(m_executions);

    // get the exact matches
    m_runners[m_nextRunner]->match(&m_context);

    //foreach (Plasma::AbstractRunner* runner, m_runners) {
    //    //kDebug() << "\trunner: " << runner->objectName();
    //    runner->match(&m_context);
    //}

    QList<QList<Plasma::SearchAction *> > matchLists;
    matchLists << m_context.informationalMatches()
               << m_context.exactMatches()
               << m_context.possibleMatches();

    foreach (QList<Plasma::SearchAction *> matchList, matchLists) {
        foreach (Plasma::SearchAction *action, matchList) {
            bool makeDefault = !m_defaultMatch && action->isEnabled();

            SearchMatch *match = new SearchMatch(action, 0);
            m_matchList->insertItem(matchCount, match);

            if (makeDefault &&
                (action->type() != Plasma::SearchAction::InformationalMatch ||
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
        showOptions(false);
        m_runButton->setEnabled(false);
    }

    if (++m_nextRunner >= m_runners.size())
    {
        m_nextRunner = 0;
        m_defaultMatch = 0;
    }
    else
    {
        // start the timer over so we will
        // process the rest of the runners
        m_matchTimer.start(0);
    }

}

void Interface::exec()
{
    match();

    if (m_searchTerm->completionBox() && m_searchTerm->completionBox()->isVisible()) {
        return;
    }

    SearchMatch* match = dynamic_cast<SearchMatch*>(m_matchList->currentItem());
    if (!match) {
        return;
    }
    QString searchTerm = m_searchTerm->text();

    if (!m_executions.contains(searchTerm)) {
        m_executions << searchTerm;

        //TODO: how many items should we remember exactly?
        if (m_executions.size() > 100) {
            m_executions.pop_front();
        }
    }

    if (match && match->actionEnabled()) {
        matchActivated(match );
    } else if (m_defaultMatch) {
        matchActivated(m_defaultMatch);
    }
}

void Interface::showOptions(bool show)
{
    //TODO: in the case where we are no longer showing options
    //      should we have the runner delete it's options?
    if (show) {
        if (!m_defaultMatch || !m_defaultMatch->action()->runner()->hasMatchOptions()) {
            // in this case, there is nothing to show
            return;
        }

        if (!m_expander) {
            //kDebug() << "creating m_expander";
            m_expander = new CollapsibleWidget( this );
            m_expander->show();
            connect( m_expander, SIGNAL( collapseCompleted() ),
                     m_expander, SLOT( hide() ) );
            m_layout->insertWidget( 3, m_expander );
        }

        //kDebug() << "set inner widget to " << m_defaultMatch->runner()->options();
        delete m_optionsWidget;
        m_optionsWidget = new QWidget(this);
        m_defaultMatch->action()->runner()->createMatchOptions(m_optionsWidget);
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
        hasOptions = m_defaultMatch && m_defaultMatch->action()->runner()->hasMatchOptions();
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
