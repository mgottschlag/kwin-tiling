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
#include <QShortcut>
#include <QTimer>

#include <KActionCollection>
#include <KDebug>
#include <KDialog>
#include <KLineEdit>
#include <KLocale>
#include <KServiceTypeTrader>
#include <KWin>

#include "../plasma/lib/theme.h"

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
        SearchMatch( QAction* action, Runner* runner, QListWidget* parent )
            : QListWidgetItem( parent ),
              m_action( action )
        {
            setIcon( m_action->icon() );

            if ( parent->item( 0 ) != this ) {
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

    private:
        QAction* m_action;
};


Interface::Interface(QWidget* parent)
    : QWidget( parent ),
      m_haveCompositionManager( false ),
      m_bgRenderer( 0 ),
      m_renderDirty( true )
{
    setWindowFlags( Qt::Window | Qt::FramelessWindowHint );
    setWindowTitle( i18n("Run Command") );

    m_theme = new Plasma::Theme( this );
    themeChanged();
    connect( m_theme, SIGNAL(changed()), this, SLOT(themeChanged()) );

    QVBoxLayout* layout = new QVBoxLayout(this);

    m_searchTerm = new KLineEdit( this );
    m_searchTerm->clear();
    m_searchTerm->setClearButtonShown( true );
    layout->addWidget(m_searchTerm);
    connect(m_searchTerm, SIGNAL(textChanged(QString)),
            this, SLOT(search(QString)));
    connect(m_searchTerm, SIGNAL(returnPressed()),
            this, SLOT(exec()));

    m_matches = new QWidget(this);
    layout->addWidget(m_matches);

    //TODO: temporary feedback, change later with the "icon parade" :)
    m_actionsList = new QListWidget(this);
    connect( m_actionsList, SIGNAL(itemActivated(QListWidgetItem*)),
             SLOT(matchActivated(QListWidgetItem*)));
    layout->addWidget(m_actionsList);

    m_optionsLabel = new QLabel(this);
    m_optionsLabel->setText("Options");
    m_optionsLabel->setEnabled(false);
    layout->addWidget(m_optionsLabel);

    m_haveCompositionManager = KRunnerApp::s_haveCompositeManager;

    new InterfaceAdaptor( this );
    QDBusConnection::sessionBus().registerObject( "/Interface", this );

    new QShortcut( QKeySequence( Qt::Key_Escape ), this, SLOT(hide()) );

    //FIXME: what size should we be?
    resize(400, 250);

    loadRunners();

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
    m_searchTerm->setFocus( );
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
    QWidget::hideEvent( e );
}

void Interface::matchActivated(QListWidgetItem* item)
{
    SearchMatch* match = dynamic_cast<SearchMatch*>(item);

    if ( match ) {
        match->exec();
        hide();
    }
}

void Interface::search(const QString& t)
{
    QListWidgetItem* item ;
    QString term = t.trimmed();
    Runner* firstMatch = 0;

    m_actionsList->clear() ;

    // get the exact matches
    foreach (Runner* runner, m_runners) {
        kDebug() << "\trunner: " << runner->objectName() << endl;
        QAction* exactMatch = runner->exactMatch( term ) ;

        if ( exactMatch ) {
            item = new SearchMatch( exactMatch,
                                    runner,
                                    m_actionsList );
            if ( !firstMatch ) {
                firstMatch = runner;
            }
        }
    }

    // get the inexact matches
    foreach (Runner* runner, m_runners) {
        KActionCollection* matches = runner->matches( term, 10, 0 );
        kDebug() << "\t\tturned up " << matches->actions().count() << " matches " << endl;
        foreach ( QAction* action, matches->actions() ) {
            kDebug() << "\t\t " << action << ": " << action->text() << endl;
            item = new SearchMatch( action,
                                    runner,
                                    m_actionsList );

            if ( !firstMatch ) {
                firstMatch = runner;
            }
        }
    }

    m_optionsLabel->setEnabled( firstMatch && firstMatch->hasOptions() );
}

void Interface::themeChanged()
{
    delete m_bgRenderer;
    kDebug() << "themeChanged() to " << m_theme->themeName()
             << "and we have " << m_theme->imagePath("/background/dialog") << endl;
    m_bgRenderer = new QSvgRenderer( m_theme->imagePath( "/background/dialog" ), this );
}

void Interface::updateMatches()
{
    //TODO: implement
}

void Interface::exec()
{
    matchActivated( m_actionsList->item( 0 ) );
}

void Interface::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());

    if ( m_haveCompositionManager ) {
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

void Interface::loadRunners()
{
    // ha! ha! get it? _load_ _runner_?! oh, i kill me.
    // but seriously, that game was the shiznit back in the day

    foreach ( Runner* runner, m_runners ) {
        delete runner;
    }
    m_runners.clear();

    //TODO: how should we order runners, particularly ones loaded from plugins?
    m_runners.append( new ServiceRunner( this ) );
    m_runners.append( new ShellRunner( this ) );
    m_runners.append( new SessionRunner( this ) );

    KService::List offers = KServiceTypeTrader::self()->query( "KRunner/Runner" );
    foreach ( KService::Ptr service, offers ) {
        Runner* runner = KService::createInstance<Runner>( service, this );
        if ( runner ) {
            kDebug() << "loaded runner : " << service->name() << endl ;
            m_runners.append( runner );
        }
        else {
            kDebug() << "failed to load runner : " << service->name() << endl ;
        }
    }
}

#include "interface.moc"
