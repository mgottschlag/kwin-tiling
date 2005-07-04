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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef URLGRABBER_H
#define URLGRABBER_H

#include <qptrlist.h>
#include <qintdict.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kprocess.h>


class QTimer;

class KConfig;
class KPopupMenu;

class ClipAction;
struct ClipCommand;
typedef QPtrList<ClipAction> ActionList;
typedef QPtrListIterator<ClipAction> ActionListIterator;

class URLGrabber : public QObject
{
  Q_OBJECT

public:
  URLGrabber( KConfig* config );
  ~URLGrabber();

  /**
   * Checks a given string whether it matches any of the user-defined criteria.
   * If it does, the configured action will be executed.
   * @returns false if the string should be put into the popupmenu or not,
   * otherwise true.
   */
  bool checkNewData( const QString& clipData );
  void invokeAction( const QString& clip = QString::null );

  const ActionList * actionList() const { return myActions; }
  void setActionList( ActionList * );
  void readConfiguration( KConfig * );
  void writeConfiguration( KConfig * );

  int popupTimeout() const { return myPopupKillTimeout; }
  void setPopupTimeout( int timeout ) { myPopupKillTimeout = timeout; }

  const QStringList& avoidWindows() const { return myAvoidWindows; }
  void setAvoidWindows( const QStringList& list ) { myAvoidWindows = list; }

  bool stripWhiteSpace() const { return m_stripWhiteSpace; }
  void setStripWhiteSpace( bool enable ) { m_stripWhiteSpace = enable; }
    
private:
  const ActionList& matchingActions( const QString& );
  void execute( const struct ClipCommand *command ) const;
  void editData();
  bool isAvoidedWindow() const;
  void actionMenu( bool wm_class_check );

  ActionList *myActions;
  ActionList myMatches;
  QStringList myAvoidWindows;
  QString myClipData;
  ClipAction *myCurrentAction;
  QIntDict<ClipCommand> myCommandMapper;
  KPopupMenu *myMenu;
  QTimer *myPopupKillTimer;
  int myPopupKillTimeout;
  bool m_stripWhiteSpace;
  KConfig* m_config;

private slots:
  void slotActionMenu() { actionMenu( true ); }
  void slotItemSelected( int );
  void slotKillPopupMenu();


signals:
    void sigPopup( QPopupMenu * );
    void sigDisablePopup();

};


struct ClipCommand
{
    ClipCommand( const QString &, const QString &, bool = true, const QString & = "" );
    QString command;
    QString description;
    bool isEnabled;
    QString pixmap;
    //  int id; // the index reflecting the position in the list of commands
};

/**
 * Represents one configured action. An action consists of one regular
 * expression, an (optional) description and a list of ClipCommands
 * (a command to be executed, a description and an enabled/disabled flag).
 */
class ClipAction
{
public:
  ClipAction( const QString& regExp, const QString& description );
  ClipAction( const ClipAction& );
  ClipAction( KConfig *kc );
  ~ClipAction();

  void  setRegExp( const QString& r) 	      { myRegExp = QRegExp( r ); }
  QString regExp() 			const { return myRegExp.pattern(); }
  inline bool matches( const QString& string ) const {
      return ( myRegExp.search( string ) != -1 );
  }

  void 	setDescription( const QString& d)     { myDescription = d; }
  const QString& description() 		const { return myDescription; }

  /**
   * Removes all ClipCommands associated with this ClipAction.
   */
  void clearCommands() { myCommands.clear(); }

  void  addCommand( const QString& command, const QString& description, bool, const QString& icon = "" );
  const QPtrList<ClipCommand>& commands() 	const { return myCommands; }

  /**
   * Saves this action to a a given KConfig object
   */
  void save( KConfig * ) const;


private:
  QRegExp 		myRegExp;
  QString 		myDescription;
  QPtrList<ClipCommand> 	myCommands;

};


#endif // URLGRABBER_H
