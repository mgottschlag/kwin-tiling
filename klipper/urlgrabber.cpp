// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) (C) 2000,2001,2002 by Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <netwm.h>

#include <QTimer>
#include <QX11Info>
#include <QUuid>

#include <kconfig.h>
#include <kdialog.h>
#include <ktextedit.h>
#include <klocale.h>
#include <kmenu.h>
#include <kprocess.h>
#include <kservice.h>
#include <kdebug.h>
#include <kstringhandler.h>
#include <kmacroexpander.h>
#include <kglobal.h>

#include "klippersettings.h"
#include "urlgrabber.h"

// TODO:
// - script-interface?

URLGrabber::URLGrabber()
{
    m_myCurrentAction = 0L;
    m_myMenu = 0L;
    m_myPopupKillTimeout = 8;
    m_trimmed = true;

    m_myPopupKillTimer = new QTimer( this );
    m_myPopupKillTimer->setSingleShot( true );
    connect( m_myPopupKillTimer, SIGNAL( timeout() ),
             SLOT( slotKillPopupMenu() ));

    // testing
    /*
    ClipAction *action;
    action = new ClipAction( "^http:\\/\\/", "Web-URL" );
    action->addCommand("kfmclient exec %s", "Open with Konqi", true);
    action->addCommand("netscape -no-about-splash -remote \"openURL(%s, new-window)\"", "Open with Netscape", true);
    m_myActions->append( action );

    action = new ClipAction( "^mailto:", "Mail-URL" );
    action->addCommand("kmail --composer %s", "Launch kmail", true);
    m_myActions->append( action );

    action = new ClipAction( "^\\/.+\\.jpg$", "Jpeg-Image" );
    action->addCommand("kuickshow %s", "Launch KuickShow", true);
    action->addCommand("kview %s", "Launch KView", true);
    m_myActions->append( action );
    */
}


URLGrabber::~URLGrabber()
{
    qDeleteAll(m_myActions);
    m_myActions.clear();
    delete m_myMenu;
}

//
// Called from Klipper::slotRepeatAction, i.e. by pressing Ctrl-Alt-R
// shortcut. I.e. never from clipboard monitoring
//
void URLGrabber::invokeAction( const QString& clip )
{
    if ( !clip.isEmpty() )
        m_myClipData = clip;
    if ( m_trimmed )
        m_myClipData = m_myClipData.trimmed();

    actionMenu( false );
}


void URLGrabber::setActionList( const ActionList& list )
{
    qDeleteAll(m_myActions);
    m_myActions.clear();
    m_myActions = list;
}


const ActionList& URLGrabber::matchingActions( const QString& clipData )
{
    m_myMatches.clear();

    foreach (ClipAction* action, m_myActions) {
        if ( action->matches( clipData ) )
            m_myMatches.append( action );
    }

    return m_myMatches;
}


bool URLGrabber::checkNewData( const QString& clipData )
{
    // kDebug() << "** checking new data: " << clipData;
    m_myClipData = clipData;
    if ( m_trimmed )
        m_myClipData = m_myClipData.trimmed();

    if ( m_myActions.isEmpty() )
        return false;

    actionMenu( true ); // also creates m_myMatches

    return !m_myMatches.isEmpty();
}


void URLGrabber::actionMenu( bool wm_class_check )
{
    if ( m_myClipData.isEmpty() )
        return;

    ActionList matchingActionsList = matchingActions( m_myClipData );
    ClipCommand *command = 0L;

    if (!matchingActionsList.isEmpty()) {
        // don't react on konqi's/netscape's urls...
        if ( wm_class_check && isAvoidedWindow() )
            return;

        QString item;
        m_myCommandMapper.clear();

        m_myPopupKillTimer->stop();

        m_myMenu = new KMenu;

        connect(m_myMenu, SIGNAL(triggered(QAction*)), SLOT(slotItemSelected(QAction*)));

        foreach (ClipAction* action, matchingActionsList) {
            QListIterator<ClipCommand*> it( action->commands() );
            if ( it.hasNext() )
                m_myMenu->addTitle(KIcon( "klipper" ),
                        i18n("%1 - Actions For: %2", action->description(), KStringHandler::csqueeze(m_myClipData, 45)));
            while (it.hasNext()) {
                command = it.next();
                item = command->description;
                if ( item.isEmpty() )
                    item = command->command;

                QString id = QUuid::createUuid().toString();
                QAction * action = new QAction(this);
                action->setData(id);
                action->setText(item);

                if (!command->pixmap.isEmpty())
                    action->setIcon(KIcon(command->pixmap));

                m_myCommandMapper.insert(id, command);
                m_myMenu->addAction(action);
            }
        }

        // only insert this when invoked via clipboard monitoring, not from an
        // explicit Ctrl-Alt-R
        if ( wm_class_check )
        {
            m_myMenu->addSeparator();
            QAction *disableAction = new QAction(i18n("Disable This Popup"), this);
            connect(disableAction, SIGNAL(triggered()), SIGNAL(sigDisablePopup()));
            m_myMenu->addAction(disableAction);
        }
        m_myMenu->addSeparator();
        // add an edit-possibility
        QAction *editAction = new QAction(KIcon("document-properties"), i18n("&Edit Contents..."), this);
        connect(editAction, SIGNAL(triggered()), SLOT(editData()));
        m_myMenu->addAction(editAction);

        QAction *cancelAction = new QAction(KIcon("dialog-cancel"), i18n("&Cancel"), this);
        connect(cancelAction, SIGNAL(triggered()), m_myMenu, SLOT(hide()));
        m_myMenu->addAction(cancelAction);

        if ( m_myPopupKillTimeout > 0 )
            m_myPopupKillTimer->start( 1000 * m_myPopupKillTimeout );

        emit sigPopup( m_myMenu );
    }
}


void URLGrabber::slotItemSelected(QAction *action)
{
    if (m_myMenu)
        m_myMenu->hide(); // deleted by the timer or the next action

    QString id = action->data().toString();

    if (id.isEmpty()) {
        kDebug() << "Klipper: no command associated";
        return;
    }

    QHash<QString, ClipCommand*>::iterator i = m_myCommandMapper.find(id);
    ClipCommand *command = i.value();

    if (command)
        execute(command);
    else
        kDebug() << "Klipper: cannot find associated action";
}


void URLGrabber::execute( const struct ClipCommand *command ) const
{
    if ( command->isEnabled ) {
        QHash<QChar,QString> map;
        map.insert( 's', m_myClipData );
        // commands executed should always have a parent,
        // but a simple check won't hurt...
        if ( command->parent )
        {
            const QStringList matches = command->parent->regExpMatches();
            // support only %0 and the first 9 matches...
            const int numMatches = qMin(10, matches.count());
            for ( int i = 0; i < numMatches; ++i )
                map.insert( QChar( '0' + i ), matches.at( i ) );
        }
        else
        {
            kDebug() << "No parent for" << command->description << "(" << command->command << ")";
        }
        QString cmdLine = KMacroExpander::expandMacrosShellQuote( command->command, map );

        if ( cmdLine.isEmpty() )
            return;

        KProcess proc;
        proc.setShellCommand(cmdLine.trimmed());
        if (!proc.startDetached())
            kDebug() << "Klipper: Could not start process!";
    }
}


void URLGrabber::editData()
{
    m_myPopupKillTimer->stop();
    KDialog *dlg = new KDialog( 0 );
    dlg->setModal( true );
    dlg->setCaption( i18n("Edit Contents") );
    dlg->setButtons( KDialog::Ok | KDialog::Cancel );

    KTextEdit *edit = new KTextEdit( dlg );
    edit->setText( m_myClipData );
    edit->setFocus();
    edit->setMinimumSize( 300, 40 );
    dlg->setMainWidget( edit );
    dlg->adjustSize();

    if ( dlg->exec() == KDialog::Accepted ) {
        m_myClipData = edit->toPlainText();
        QTimer::singleShot( 0, this, SLOT( slotActionMenu() ) );
    }
    else
    {
        m_myMenu->deleteLater();
        m_myMenu = 0;
    }
    delete dlg;
}


void URLGrabber::loadSettings()
{
    m_trimmed = KlipperSettings::stripWhiteSpace();
    m_myAvoidWindows = KlipperSettings::noActionsForWM_CLASS();
    m_myPopupKillTimeout = KlipperSettings::timeoutForActionPopups();

    qDeleteAll(m_myActions);
    m_myActions.clear();

    KConfigGroup cg(KGlobal::config(), "General");
    int num = cg.readEntry("Number of Actions", 0);
    QString group;
    for ( int i = 0; i < num; i++ ) {
        group = QString("Action_%1").arg( i );
        m_myActions.append( new ClipAction( KGlobal::config(), group ) );
    }
}

void URLGrabber::saveSettings() const
{
    KConfigGroup cg(KGlobal::config(), "General");
    cg.writeEntry( "Number of Actions", m_myActions.count() );

    int i = 0;
    QString group;
    foreach (ClipAction* action, m_myActions) {
        group = QString("Action_%1").arg( i );
        action->save( KGlobal::config(), group );
        ++i;
    }

    KlipperSettings::setNoActionsForWM_CLASS(m_myAvoidWindows);
}

// find out whether the active window's WM_CLASS is in our avoid-list
// digged a little bit in netwm.cpp
bool URLGrabber::isAvoidedWindow() const
{
    Display *d = QX11Info::display();
    static Atom wm_class = XInternAtom( d, "WM_CLASS", true );
    static Atom active_window = XInternAtom( d, "_NET_ACTIVE_WINDOW", true );
    Atom type_ret;
    int format_ret;
    unsigned long nitems_ret, unused;
    unsigned char *data_ret;
    long BUFSIZE = 2048;
    bool ret = false;
    Window active = 0L;
    QString wmClass;

    // get the active window
    if (XGetWindowProperty(d, DefaultRootWindow( d ), active_window, 0l, 1l,
                           False, XA_WINDOW, &type_ret, &format_ret,
                           &nitems_ret, &unused, &data_ret)
        == Success) {
        if (type_ret == XA_WINDOW && format_ret == 32 && nitems_ret == 1) {
            active = *((Window *) data_ret);
        }
        XFree(data_ret);
    }
    if ( !active )
        return false;

    // get the class of the active window
    if ( XGetWindowProperty(d, active, wm_class, 0L, BUFSIZE, False, XA_STRING,
                            &type_ret, &format_ret, &nitems_ret,
                            &unused, &data_ret ) == Success) {
        if ( type_ret == XA_STRING && format_ret == 8 && nitems_ret > 0 ) {
            wmClass = QString::fromUtf8( (const char *) data_ret );
            ret = (m_myAvoidWindows.indexOf( wmClass ) != -1);
        }

        XFree( data_ret );
    }

    return ret;
}


void URLGrabber::slotKillPopupMenu()
{
    if ( m_myMenu && m_myMenu->isVisible() )
    {
        if ( m_myMenu->geometry().contains( QCursor::pos() ) &&
             m_myPopupKillTimeout > 0 )
        {
            m_myPopupKillTimer->start( 1000 * m_myPopupKillTimeout );
            return;
        }
    }

    if ( m_myMenu ) {
        m_myMenu->deleteLater();
        m_myMenu = 0;
    }
}

///////////////////////////////////////////////////////////////////////////
////////

ClipCommand::ClipCommand(ClipAction *_parent, const QString &_command, const QString &_description,
                         bool _isEnabled, const QString &_icon)
    : parent(_parent),
      command(_command),
      description(_description),
      isEnabled(_isEnabled)
{
    int len = command.indexOf(" ");
    if (len == -1)
        len = command.length();

    if (!_icon.isEmpty())
        pixmap = _icon;
    else
    {
    KService::Ptr service= KService::serviceByDesktopName(command.left(len));
    if (service)
        pixmap = service->icon();
    else
        pixmap.clear();
    }
}


ClipAction::ClipAction( const QString& regExp, const QString& description )
    : m_myRegExp( regExp ), m_myDescription( description )
{
}


ClipAction::ClipAction( const ClipAction& action )
{
    m_myRegExp      = action.m_myRegExp;
    m_myDescription = action.m_myDescription;

    ClipCommand *command = 0L;
    QListIterator<ClipCommand*> it( m_myCommands );
    while (it.hasNext()) {
        command = it.next();
        addCommand(command->command, command->description, command->isEnabled);
    }
}


ClipAction::ClipAction( KSharedConfigPtr kc, const QString& group )
    : m_myRegExp( kc->group(group).readEntry("Regexp") ),
      m_myDescription (kc->group(group).readEntry("Description") )
{
    KConfigGroup cg(kc, group);

    int num = cg.readEntry( "Number of commands", 0 );

    // read the commands
    for ( int i = 0; i < num; i++ ) {
        QString _group = group + "/Command_%1";
        KConfigGroup _cg(kc, _group.arg(i));

        addCommand( _cg.readPathEntry( "Commandline", QString() ),
                    _cg.readEntry( "Description" ), // i18n'ed
                    _cg.readEntry( "Enabled" , false),
                    _cg.readEntry( "Icon") );
    }
}


ClipAction::~ClipAction()
{
    qDeleteAll(m_myCommands);
}


void ClipAction::addCommand( const QString& command,
                             const QString& description, bool enabled, const QString& icon )
{
    if ( command.isEmpty() )
        return;

    struct ClipCommand *cmd = new ClipCommand( this, command, description, enabled, icon );
    //    cmd->id = m_myCommands.count(); // superfluous, I think...
    m_myCommands.append( cmd );
}


// precondition: we're in the correct action's group of the KConfig object
void ClipAction::save( KSharedConfigPtr kc, const QString& group ) const
{
    KConfigGroup cg(kc, group);
    cg.writeEntry( "Description", description() );
    cg.writeEntry( "Regexp", regExp() );
    cg.writeEntry( "Number of commands", m_myCommands.count() );

    struct ClipCommand *cmd;
    QListIterator<struct ClipCommand*> it( m_myCommands );

    // now iterate over all commands of this action
    int i = 0;
    while (it.hasNext()) {
        cmd = it.next();
        QString _group = group + "/Command_%1";
        KConfigGroup cg(kc, _group.arg(i));

        cg.writePathEntry( "Commandline", cmd->command );
        cg.writeEntry( "Description", cmd->description );
        cg.writeEntry( "Enabled", cmd->isEnabled );

        ++i;
    }
}

#include "urlgrabber.moc"
