/****************************************************************************
** $Id$
**
** Definition of QXEmbed class
**
** Created :
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QXEMBED_H
#define QXEMBED_H

#include <qwidget.h>

class QXEmbed : public QWidget
{
  Q_OBJECT

public:

  QXEmbed(QWidget *parent=0, const char *name=0);
  ~QXEmbed();

  void embed(WId w);

protected:
  void keyPressEvent( QKeyEvent * );
  void keyReleaseEvent( QKeyEvent * );
  void focusInEvent( QFocusEvent * );
  void focusOutEvent( QFocusEvent * );
  void resizeEvent(QResizeEvent *);
  void showEvent( QShowEvent * );

  bool focusNextPrevChild( bool next );

private:

 void sendFocusIn();
 void sendFocusOut();

  WId  window;

};


#endif
