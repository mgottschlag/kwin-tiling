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
#include <QPainter>
#include <QResizeEvent>
#include <QSvgRenderer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QShortcut>
#include <QTimer>

#include <KActionCollection>
#include <KDebug>
#include <KDialog>
#include <KGlobalSettings>
#include <KLineEdit>
#include <KLocale>
#include <KPushButton>
#include <KStandardGuiItem>
#include <KWin>

#include "../plasma/lib/theme.h"
#include "../plasma/lib/runner.h"

#include "runners/services/servicerunner.h"
#include "runners/sessions/sessionrunner.h"
#include "runners/shell/shellrunner.h"
#include "interface.h"
#include "interfaceadaptor.h"
#include "krunnerapp.h"

//#define FLASH_DIALOG 0

// A little hack of a class to let us easily activate a match
class SearchMatch : public QListWidgetItem
{
    public:
        SearchMatch( QAction* action, Plasma::Runner* runner, QListWidget* parent, bool isDefault )
            : QListWidgetItem( parent ),
              m_action( action )
        {
            setIcon( m_action->icon() );

            if ( !action->isEnabled() || !isDefault ) {
                setText( i18n("%1 (%2)",
                         m_action->text(),
                         runner->objectName() ) );
            } else {
                setText( i18n("Default: %1 (%2)",
                         m_action->text(),
                         runner->objectName() ) );
            }
        }

        void exec()
        {
            m_action->activate( QAction::Trigger );
        }

        bool actionEnabled()
        {
            return m_action->isEnabled();
        }

    private:
        QAction* m_action;
};


Interface::Interface(QWidget* parent)
    : QWidget( parent ),
      m_bgRenderer( 0 ),
      m_renderDirty( true ),
      m_defaultMatch( 0 )
{
    setWindowFlags( Qt::Window | Qt::FramelessWindowHint );
    setWindowTitle( i18n("Run Command") );

    connect( &m_searchTimer, SIGNAL(timeout()),
             this, SLOT(fuzzySearch()) );

    m_theme = new Plasma::Theme( this );
    themeChanged();
    connect( m_theme, SIGNAL(changed()), this, SLOT(themeChanged()) );

    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* bottomLayout = new QHBoxLayout(this);

    m_header = new QFrame( this );
    QHBoxLayout* headerLayout = new QHBoxLayout( m_header );
    m_header->setFrameShape( QFrame::StyledPanel );
    m_header->setFrameShadow( QFrame::Plain );
    m_header->setAutoFillBackground( true );
    m_header->setBackgroundRole( QPalette::Base );
    layout->addWidget( m_header );

    m_headerLabel = new QLabel( m_header );
    //TODO: create a action so this can be changed by
    //various processes to give the user feedback
    m_headerLabel->setText( i18n( "Enter the name of an application, location or search term below." ) );
    m_headerLabel->setEnabled( true );
    m_headerLabel->setWordWrap( true );
    m_headerLabel->setAlignment( Qt::AlignCenter );
    headerLayout->addWidget( m_headerLabel );

    m_searchTerm = new KLineEdit( this );
    m_searchTerm->clear();
    m_searchTerm->setClickMessage( i18n( "Enter a command or search term here" ) );
    m_searchTerm->setClearButtonShown( true );
    layout->addWidget( m_searchTerm );
    connect( m_searchTerm, SIGNAL(textChanged(QString)),
            this, SLOT(search(QString)) );
    connect( m_searchTerm, SIGNAL(returnPressed()),
             this, SLOT(exec()) );

    //TODO: temporary feedback, change later with the "icon parade" :)
    m_actionsList = new QListWidget(this);
    connect( m_actionsList, SIGNAL(itemActivated(QListWidgetItem*)),
             SLOT(matchActivated(QListWidgetItem*)));
    layout->addWidget(m_actionsList);

    m_optionsLabel = new QLabel( this );
    m_optionsLabel->setText( i18n( "Options" ) );
    m_optionsLabel->setEnabled( false );
    bottomLayout->addWidget( m_optionsLabel );

    bottomLayout->addStretch();

    QString runButtonWhatsThis = i18n( "Click to execute the selected item above" );
    m_runButton = new KPushButton( KGuiItem( i18n( "Run" ), "run",
                                             QString(), runButtonWhatsThis ),
                                   this );
    m_runButton->setFlat( true );
    m_runButton->setEnabled( false );
    connect( m_runButton, SIGNAL(clicked(bool)), SLOT(exec()) );
    bottomLayout->addWidget( m_runButton );

    m_cancelButton = new KPushButton( KStandardGuiItem::cancel(), this );
    m_cancelButton->setFlat( true );
    connect( m_cancelButton, SIGNAL(clicked(bool)), SLOT(hide()) );
    bottomLayout->addWidget( m_cancelButton );

    layout->addLayout (bottomLayout );

    setWidgetPalettes();
    connect( KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
             SLOT(setWidgetPalettes()) );

    new InterfaceAdaptor( this );
    QDBusConnection::sessionBus().registerObject( "/Interface", this );

    new QShortcut( QKeySequence( Qt::Key_Escape ), this, SLOT(hide()) );

    //FIXME: what size should we be?
    resize(400, 250);


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

    kDebug() << "display() called" << endl;
    m_searchTerm->setFocus();
    m_actionsList->clear() ;
    if ( !term.isEmpty() ) {
        m_searchTerm->setText( term );
    }

    show();
    raise();
    KWin::setOnDesktop( winId(), KWin::currentDesktop() );
    KDialog::centerOnScreen( this );
    KWin::forceActiveWindow( winId() );

    if ( !term.isEmpty() ) {
        search( term );
    }
}

void Interface::hideEvent( QHideEvent* e )
{
    Q_UNUSED( e )

    kDebug() << "hide event" << endl;
    m_searchTerm->clear();
    m_actionsList->clear() ;
    m_runButton->setEnabled( false );
    QWidget::hideEvent( e );
}

void Interface::matchActivated(QListWidgetItem* item)
{
    SearchMatch* match = dynamic_cast<SearchMatch*>(item);

    if ( match && match->actionEnabled() ) {
        match->exec();
        hide();
    }
}

void Interface::search(const QString& t)
{
    m_searchTimer.stop();

    //FIXME: annoyingly, if the same list gets returned, this make it appear
    //       to just "flicker" as the old list goes away and is replaced by
    //       the new. something to think about when implementing the icon
    //       parade
    m_actionsList->clear();
    m_defaultMatch = 0;
    QString term = t.trimmed();

    if ( term.isEmpty() ) {
        m_runButton->setEnabled( false );
        return;
    }

    // get the exact matches
    foreach (Plasma::Runner* runner, m_runners) {
        kDebug() << "\trunner: " << runner->objectName() << endl;
        QAction* exactMatch = runner->exactMatch( term ) ;

        if ( exactMatch ) {
            bool makeDefault = !m_defaultMatch && exactMatch->isEnabled();
            SearchMatch* match = new SearchMatch( exactMatch, runner, m_actionsList, makeDefault );
            if ( makeDefault ) {
                m_defaultMatch = match;
                m_optionsLabel->setEnabled( runner->hasOptions() );
                m_runButton->setEnabled( true );
            }
        }
    }

    if ( !m_defaultMatch ) {
        m_runButton->setEnabled( false );
    }

    m_searchTimer.start( 250 );
}

void Interface::fuzzySearch()
{
    m_searchTimer.stop();

    QString term = m_searchTerm->text().trimmed();

    // get the inexact matches
    foreach ( Plasma::Runner* runner, m_runners ) {
        KActionCollection* matches = runner->matches( term, 10, 0 );
        kDebug() << "\t\tturned up " << matches->actions().count() << " matches " << endl;
        foreach ( QAction* action, matches->actions() ) {
            bool makeDefault = !m_defaultMatch && action->isEnabled();
            kDebug() << "\t\t " << action << ": " << action->text() << " " << !m_defaultMatch << " " << action->isEnabled() << endl;
            SearchMatch* match = new SearchMatch( action, runner, m_actionsList, makeDefault );

            if ( makeDefault ) {
                m_defaultMatch = match;
                m_runButton->setEnabled( true );
                m_optionsLabel->setEnabled( runner->hasOptions() );
            }
        }
    }
}

void Interface::themeChanged()
{
    delete m_bgRenderer;
    kDebug() << "themeChanged() to " << m_theme->themeName()
             << "and we have " << m_theme->imagePath("/background/dialog") << endl;
    m_bgRenderer = new QSvgRenderer( m_theme->imagePath( "/background/dialog" ), this );
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
    m_actionsList->setPalette( widgetPalette );
}

void Interface::updateMatches()
{
    //TODO: implement
}

void Interface::exec()
{
    SearchMatch* match = dynamic_cast<SearchMatch*>( m_actionsList->currentItem() );

    if ( match && match->actionEnabled() ) {
        matchActivated( match );
    } else if ( m_defaultMatch ) {
        matchActivated( m_defaultMatch );
    }
}

void Interface::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());

    if ( KRunnerApp::s_haveCompositeManager ) {
        //kDebug() << "gots us a compmgr!" << m_haveCompositionManager << endl;
        p.save();
        p.setCompositionMode( QPainter::CompositionMode_Source );
        p.fillRect( rect(), Qt::transparent );
        p.restore();
    }

    if ( m_renderDirty ) {
        m_renderedSvg.fill( Qt::transparent );
        QPainter p( &m_renderedSvg );
        p.setRenderHint( QPainter::Antialiasing );
        m_bgRenderer->render( &p);
        p.end();
        m_renderDirty = false;
    }

    p.drawPixmap( 0, 0, m_renderedSvg );
}

void Interface::resizeEvent(QResizeEvent *e)
{
    if ( e->size() != m_renderedSvg.size() ) {
        m_renderedSvg = QPixmap( e->size() );
        m_renderDirty = true;
    }

    QWidget::resizeEvent( e );
}

#include "interface.moc"
