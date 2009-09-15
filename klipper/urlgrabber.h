// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

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
#ifndef URLGRABBER_H
#define URLGRABBER_H

#include <QHash>
#include <QRegExp>

class QTimer;

class KConfig;
class KMenu;

class ClipAction;
struct ClipCommand;
typedef QList<ClipAction*> ActionList;

class URLGrabber : public QObject
{
  Q_OBJECT

public:
  URLGrabber();
  ~URLGrabber();

  /**
   * Checks a given string whether it matches any of the user-defined criteria.
   * If it does, the configured action will be executed.
   * @returns false if the string should be put into the popupmenu or not,
   * otherwise true.
   */
  bool checkNewData( const QString& clipData );
  void invokeAction( const QString& clip = QString() );

  ActionList actionList() const { return m_myActions; }
  void setActionList( const ActionList& );

  void loadSettings();
  void saveSettings() const;

  int popupTimeout() const { return m_myPopupKillTimeout; }
  void setPopupTimeout( int timeout ) { m_myPopupKillTimeout = timeout; }

  QStringList excludedWMClasses() const { return m_myAvoidWindows; }
  void setExcludedWMClasses( const QStringList& list ) { m_myAvoidWindows = list; }

  bool trimmed() const { return m_trimmed; }
  void setStripWhiteSpace( bool enable ) { m_trimmed = enable; }

private:
  const ActionList& matchingActions( const QString& );
  void execute( const ClipAction *action, int commandIdx ) const;
  bool isAvoidedWindow() const;
  void actionMenu( bool wm_class_check );
  void matchingMimeActions(const QString& clipData);

  ActionList m_myActions;
  ActionList m_myMatches;
  QStringList m_myAvoidWindows;
  QString m_myClipData;
  ClipAction *m_myCurrentAction;

  // holds mappings of menu action IDs to action commands (action+cmd index in it)
  QHash<QString, QPair<ClipAction*, int> > m_myCommandMapper;
  KMenu *m_myMenu;
  QTimer *m_myPopupKillTimer;
  int m_myPopupKillTimeout;
  bool m_trimmed;

private Q_SLOTS:
  void slotActionMenu() { actionMenu( true ); }
  void slotItemSelected(QAction *action);
  void slotKillPopupMenu();
  void editData();


Q_SIGNALS:
    void sigPopup( QMenu * );
    void sigDisablePopup();

};


struct ClipCommand
{
    ClipCommand( const QString & command, const QString & description,
            bool = true, const QString & icon = QString() );

    QString command;
    QString description;
    bool isEnabled;
    QString pixmap;
};

/**
 * Represents one configured action. An action consists of one regular
 * expression, an (optional) description and a list of ClipCommands
 * (a command to be executed, a description and an enabled/disabled flag).
 */
class ClipAction
{
public:
  explicit ClipAction( const QString& regExp = QString(),
                       const QString& description = QString() );

  ClipAction( KSharedConfigPtr kc, const QString& );
  ~ClipAction();

  void  setRegExp( const QString& r) { m_myRegExp = QRegExp( r ); }
  QString regExp() const             { return m_myRegExp.pattern(); }

  bool matches( const QString& string ) const { return ( m_myRegExp.indexIn( string ) != -1 ); }

  QStringList regExpMatches() const { return m_myRegExp.capturedTexts(); }

  void setDescription( const QString& d) { m_myDescription = d; }
  QString description() const            { return m_myDescription; }

  /**
   * Removes all ClipCommands associated with this ClipAction.
   */
  void clearCommands() { m_myCommands.clear(); }

  void  addCommand( const QString& command,
                    const QString& description,
                    bool isEnabled = true,
                    const QString& icon = QString() );

  /**
   * Replaces command at index @p idx with command @p newCmd
   */
  void replaceCommand( int idx, const ClipCommand& newCmd );

  /**
   * Returns command by its index in command list
   */
  ClipCommand command(int idx) const { return m_myCommands.at(idx); }

  QList<ClipCommand> commands() const { return m_myCommands; }

  /**
   * Saves this action to a a given KConfig object
   */
  void save( KSharedConfigPtr, const QString& ) const;


private:
  QRegExp m_myRegExp;
  QString m_myDescription;
  QList<ClipCommand> m_myCommands;

};


#endif // URLGRABBER_H
