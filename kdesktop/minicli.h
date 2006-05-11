/* This file is part of the KDE project

   Copyright (C) 1999-2002,2003 Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2000 Malte Starostik <starosti@zedat.fu-berlin.de>
   Copyright (C) 2003 Sven Leiber <s.leiber@web.de>

   Kdesu integration:
   Copyright (C) 2000 Geert Jansen <jansen@kde.org>

   Original authors:
   Copyright (C) 1997 Matthias Ettrich <ettrich@kde.org>
   Copyright (C) 1997 Torben Weis [ Added command completion ]
   Copyright (C) 1999 Preston Brown <pbrown@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef MINICLI_H
#define MINICLI_H

#include <QString>
#include <qstringlist.h>

#include <kdialog.h>
#include <kservice.h>

namespace Ui
{
  class MinicliDlgUI;
}

class QTimer;
class QWidget;
class KURIFilterData;

class Minicli : public KDialog
{
  Q_OBJECT

public:
  Minicli( QWidget *parent=0, const char *name=0 );
  virtual ~Minicli();

  void setCommand(const QString& command);
  void reset();
  void saveConfig();
  void clearHistory();
  
  virtual void show();
  virtual QSize sizeHint() const;

protected Q_SLOTS:
  virtual void accept();
  virtual void reject();
  void updateAuthLabel();

protected:
  void loadConfig();
  bool needsKDEsu();
  virtual void keyPressEvent( QKeyEvent* );
  virtual void fontChange( const QFont & );

private Q_SLOTS:
  void slotAdvanced();
  void slotParseTimer();
  void slotPriority(int);
  void slotRealtime(bool);
  void slotTerminal(bool);
  void slotChangeUid(bool);
  void slotChangeScheduler(bool);
  void slotCmdChanged(const QString&);

private:
  void setIcon();
  int runCommand();
  void parseLine( bool final );
  QString terminalCommand (const QString&, const QString&);
  QString calculate(const QString &exp);
  void notifyServiceStarted(KService::Ptr service);


  int m_iPriority;
  int m_iScheduler;

  QString m_iconName;
  QString m_prevIconName;
  QStringList m_terminalAppList;
  QStringList m_middleFilters;
  QStringList m_finalFilters;

  QTimer* m_parseTimer;
  QWidget* m_FocusWidget;
  Ui::MinicliDlgUI* m_dlg;
  QWidget* m_dlgWidget;
  KURIFilterData* m_filterData;

  // Cached values
  QString m_prevUser;
  QString m_prevPass;
  bool m_prevChecked;
  bool m_prevCached;
  bool m_autoCheckedRunInTerm;
};
#endif
