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
#include <KDebug>
#include <KDialog>
#include <KLineEdit>
#include <KLocale>
#include <KGlobalSettings>
#include <KPushButton>
#include <KStandardGuiItem>
#include <KWin>

#include "../plasma/lib/runner.h"

#include "runners/services/servicerunner.h"
#include "runners/sessions/sessionrunner.h"
#include "runners/shell/shellrunner.h"
#include "collapsiblewidget.h"
#include "interface.h"
#include "interfaceadaptor.h"
#include "krunnerapp.h"

//#define FLASH_DIALOG 0

// A little hack of a class to let us easily activate a match

class SearchMatch : public QListWidgetItem
{
    public:
        SearchMatch( QAction* action, Plasma::Runner* runner, QListWidget* parent )
            : QListWidgetItem( parent ),
              m_default( false ),
              m_action( 0 ),
              m_runner( runner )
        {
            setAction( action );
        }

        void activate()
        {
            m_action->activate( QAction::Trigger );
        }

        bool actionEnabled()
        {
            return m_action->isEnabled();
        }

        void setAction( QAction* action )
        {
            m_action = action;
            setIcon( m_action->icon() );
            setText( i18n("%1 (%2)",
                     m_action->text(),
                     m_runner->objectName() ) );

            // in case our new action is now enabled and the old one wasn't, or
            // vice versa
            setDefault( m_default );
        }

        Plasma::Runner* runner()
        {
            return m_runner;
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
        QAction* m_action;
        Plasma::Runner* m_runner;
};

Interface::Interface(QWidget* parent)
    : KRunnerDialog( parent ),
      m_expander( 0 ),
      m_defaultMatch( 0 )
{
    setWindowTitle( i18n("Run Command") );

    connect( &m_searchTimer, SIGNAL(timeout()),
             this, SLOT(fuzzySearch()) );

    m_layout = new QVBoxLayout(this);

    m_header = new QFrame( this );
    QHBoxLayout* headerLayout = new QHBoxLayout( m_header );
    m_header->setFrameShape( QFrame::StyledPanel );
    m_header->setFrameShadow( QFrame::Plain );
    m_header->setAutoFillBackground( true );
    m_header->setBackgroundRole( QPalette::Base );
    m_layout->addWidget( m_header );

    m_headerLabel = new QLabel( m_header );
    //TODO: create a slot so this can be changed by various runners to give the user feedback?
    m_headerLabel->setText( i18n( "Enter the name of an application, location or search term below." ) );
    m_headerLabel->setEnabled( true );
    m_headerLabel->setWordWrap( true );
    m_headerLabel->setAlignment( Qt::AlignCenter );
    headerLayout->addWidget( m_headerLabel );

    m_searchTerm = new KLineEdit( this );
    m_headerLabel->setBuddy( m_searchTerm );
    m_searchTerm->clear();
    m_searchTerm->setClearButtonShown( true );
    m_layout->addWidget( m_searchTerm );
    connect( m_searchTerm, SIGNAL(textChanged(QString)),
            this, SLOT(match(QString)) );
    connect( m_searchTerm, SIGNAL(returnPressed()),
             this, SLOT(exec()) );

    //TODO: temporary feedback, change later with the "icon parade" :)
    m_matchList = new QListWidget(this);
    connect( m_matchList, SIGNAL(itemActivated(QListWidgetItem*)),
             SLOT(matchActivated(QListWidgetItem*)) );
    connect( m_matchList, SIGNAL(itemClicked(QListWidgetItem*)),
             SLOT(setDefaultItem(QListWidgetItem*)) );
    m_layout->addWidget(m_matchList);

    // buttons at the bottom
    QHBoxLayout* bottomLayout = new QHBoxLayout(this);
    m_optionsButton = new KPushButton( KStandardGuiItem::configure(), this );
    m_optionsButton->setText( i18n( "Show Options" ) );
    m_optionsButton->setFlat( true );
    m_optionsButton->setEnabled( false );
    m_optionsButton->setCheckable( true );
    connect( m_optionsButton, SIGNAL(toggled(bool)), SLOT(showOptions(bool)) );
    bottomLayout->addWidget( m_optionsButton );

    bottomLayout->addStretch();

    QString runButtonWhatsThis = i18n( "Click to execute the selected item above" );
    m_runButton = new KPushButton( KGuiItem( i18n( "Launch" ), "launch",
                                             QString(), runButtonWhatsThis ),
                                   this );
    m_runButton->setFlat( true );
    m_runButton->setEnabled( false );
    connect( m_runButton, SIGNAL( clicked(bool) ), SLOT(exec()) );
    bottomLayout->addWidget( m_runButton );

    m_cancelButton = new KPushButton( KStandardGuiItem::cancel(), this );
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
    m_runners += Plasma::Runner::loadRunners( this );

#ifdef FLASH_DIALOG
    QTimer* t = new QTimer(this);
    connect( t, SIGNAL(timeout()), this, SLOT(display()));
    t->start( 250 );
#endif
}

Interface::~Interface()
{
}

void Interface::display( const QString& term)
{
#ifdef FLASH_DIALOG
    static bool s_in = false;

    if (s_in) {
        s_in = false;
        hide();
        return;
    }
    s_in = true;
#endif

    //kDebug() << "display() called, are we visible? " << isVisible() << endl;
    m_searchTerm->setFocus();

    if ( !term.isEmpty() ) {
        m_searchTerm->setText( term );
    }

    KWin::setOnDesktop( winId(), KWin::currentDesktop() );
    KDialog::centerOnScreen( this );
    show();
    KWin::forceActiveWindow( winId() );

    //kDebug() << "about to match now that we've shown " << isVisible() << endl;

    match( term );
}

void Interface::setWidgetPalettes()
{
    // a nice palette to use with the widgets
    QPalette widgetPalette = palette();
    QColor headerBgColor = widgetPalette.color( QPalette::Active,
                                                QPalette::Base );
    headerBgColor.setAlpha( 200 );
    widgetPalette.setColor( QPalette::Active, QPalette::Base, headerBgColor );

    m_header->setPalette( widgetPalette );
    m_searchTerm->setPalette( widgetPalette );
    m_matchList->setPalette( widgetPalette );
}

void Interface::showEvent( QShowEvent* e )
{
    Q_UNUSED( e )

    //kDebug() << "show event" << endl;
}

void Interface::hideEvent( QHideEvent* e )
{
    Q_UNUSED( e )

    //kDebug() << "hide event" << endl;
    showOptions( false );
    m_searchTerm->clear();
    m_matchList->clear() ;
    m_runButton->setEnabled( false );
    m_optionsButton->setEnabled( false );
    e->accept();
}

void Interface::matchActivated(QListWidgetItem* item)
{
    SearchMatch* match = dynamic_cast<SearchMatch*>(item);
    m_optionsButton->setEnabled( match && match->runner()->hasOptions() );

    if ( match && match->actionEnabled() ) {
        kDebug() << "match activated! " << match->text() << endl;
        match->activate();
        hide();
    }
}

void Interface::match(const QString& t)
{
    m_searchTimer.stop();

    m_defaultMatch = 0;
    QString term = t.trimmed();

    if ( term.isEmpty() ) {
        m_matches.clear();
        m_searchMatches.clear();
        m_matchList->clear();
        m_runButton->setEnabled( false );
        m_optionsButton->setEnabled( false );
        showOptions( false );
        return;
    }

    QMap<Plasma::Runner*, SearchMatch*> matches;

    int matchCount = 0;

    // get the exact matches
    foreach ( Plasma::Runner* runner, m_runners ) {
        kDebug() << "\trunner: " << runner->objectName() << endl;
        QAction* exactMatch = runner->exactMatch( term ) ;

        if ( exactMatch ) {
            SearchMatch* match = 0;
            bool makeDefault = !m_defaultMatch && exactMatch->isEnabled();

            QMap<Plasma::Runner*, SearchMatch*>::iterator it = m_matches.find( runner );
            if ( it != m_matches.end() ) {
                match = it.value();
                match->setAction( exactMatch );
                matches[runner] = match;
                m_matches.erase( it );
            } else {
                match = new SearchMatch( exactMatch, runner, 0 );
                m_matchList->insertItem( matchCount, match );
            }

            if ( makeDefault ) {
                match->setDefault( true );
                m_defaultMatch = match;
                m_optionsButton->setEnabled( runner->hasOptions() );
                m_runButton->setEnabled( true );
            }

            ++matchCount;
            matches[runner] = match;
        }
    }

    if ( !m_defaultMatch ) {
        showOptions( false );
        m_runButton->setEnabled( false );
    }

    foreach ( SearchMatch* match, m_matches ) {
        delete match;
    }

    m_matches = matches;
    m_searchTimer.start( 200 );
}

void Interface::fuzzySearch()
{
    m_searchTimer.stop();

    // TODO: we may want to stop this from flickering about as well,
    //       similar to match above
    foreach ( SearchMatch* match, m_searchMatches ) {
        delete match;
    }

    m_searchMatches.clear();

    QString term = m_searchTerm->text().trimmed();

    // get the inexact matches
    foreach ( Plasma::Runner* runner, m_runners ) {
        KActionCollection* matches = runner->matches( term, 10, 0 );
        kDebug() << "\t\tturned up " << matches->actions().count() << " matches " << endl;
        foreach ( QAction* action, matches->actions() ) {
            bool makeDefault = !m_defaultMatch && action->isEnabled();
            kDebug() << "\t\t " << action << ": " << action->text() << " " << !m_defaultMatch << " " << action->isEnabled() << endl;
            SearchMatch* match = new SearchMatch( action, runner, m_matchList );
            m_searchMatches.append( match );

            if ( makeDefault ) {
                m_defaultMatch = match;
                m_defaultMatch->setDefault( true );
                m_runButton->setEnabled( true );
                m_optionsButton->setEnabled( runner->hasOptions() );
            }
        }
    }
}

void Interface::updateMatches()
{
    //TODO: implement
}

void Interface::exec()
{
    SearchMatch* match = dynamic_cast<SearchMatch*>( m_matchList->currentItem() );

    if ( match && match->actionEnabled() ) {
        matchActivated( match );
    } else if ( m_defaultMatch ) {
        matchActivated( m_defaultMatch );
    }
}

void Interface::showOptions(bool show)
{
    //TODO: in the case where we are no longer showing options
    //      should we have the runner delete it's options?
    if ( show ) {
        if ( !m_defaultMatch || !m_defaultMatch->runner()->hasOptions() ) {
            // in this case, there is nothing to show
            return;
        }

        if ( !m_expander ) {
            kDebug() << "creating m_expander" << endl;
            m_expander = new CollapsibleWidget( this );
            connect( m_expander, SIGNAL( collapseCompleted() ),
                     m_expander, SLOT( hide() ) );
            m_layout->insertWidget( 3, m_expander );
        }

        kDebug() << "set inner widget to " << m_defaultMatch->runner()->options() << endl;
        m_expander->setInnerWidget( m_defaultMatch->runner()->options() );
        m_expander->show();
        m_optionsButton->setText( i18n( "Hide Options" ) );
    } else {
        m_optionsButton->setText( i18n( "Show Options" ) );
        resize( 400, 250 );
    }

    if ( m_expander ) {
        //TODO: we need to insert an element into the krunner dialog
        //      that is big enough for the options. this will prevent
        //      other items in the dialog from moving around and look
        //      more "natural"; it should appear as if a "drawer" is
        //      being pulled open, e.g. an expander.
        m_expander->setExpanded( show );
    }
    m_optionsButton->setChecked( show );
}

void Interface::setDefaultItem( QListWidgetItem* item )
{
    if ( !item ) {
        return;
    }

    if ( m_defaultMatch ) {
        m_defaultMatch->setDefault( false );
    }

    m_defaultMatch = dynamic_cast<SearchMatch*>( item );

    bool hasOptions = m_defaultMatch && m_defaultMatch->runner()->hasOptions();
    m_optionsButton->setEnabled( hasOptions );

    if ( m_expander && !hasOptions ) {
        m_expander->hide();
    }
}

#include "interface.moc"
