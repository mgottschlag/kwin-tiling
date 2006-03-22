/***************************************************************************
                          krefinepage.cpp  -  description
                             -------------------
    begin                : Tue May 22 2001
    copyright            : (C) 2001 by Ralf Nolden
    email                : nolden@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qlabel.h>
#include <qpushbutton.h>

#include <krun.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>
#include "krefinepage.h"
#include "kpersonalizer.h"


KRefinePage::KRefinePage(QWidget *parent, const char *name ) : KRefinePageDlg(parent,name) {
   px_finishSidebar->setPixmap(UserIcon("step5.png"));
   connect( pb_kcontrol, SIGNAL(clicked()), SLOT(startKControl()) );
   if( KPersonalizer::beforeSession()) {
       pb_kcontrol->hide();
       lb_kcontrol->hide();
   }
   px_kcontrol->setPixmap(KGlobal::iconLoader()->loadIcon("kcontrol", K3Icon::Panel, K3Icon::SizeMedium));
}
KRefinePage::~KRefinePage(){
}
/** starts kcontrol via krun when the user presses the
start control center button on page 5. */
void KRefinePage::startKControl(){
  KRun::runCommand("kcontrol");
}
#include "krefinepage.moc"
