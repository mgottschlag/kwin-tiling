/***************************************************************************
                          kcm_componentchooser.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Joseph Wenninger
    email                : jowenn@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation                             *
 *                                                                         *
 ***************************************************************************/

#include <kcm_componentchooser.h>
#include <kcm_componentchooser.moc>
#include <qlayout.h>
#include <kglobal.h>
#include <klocale.h>

KCMComponentChooser::KCMComponentChooser( QWidget *parent, const char *name ):
	KCModule(parent,name) {

	(new QVBoxLayout(this))->setAutoAdd(true);
	m_chooser=new ComponentChooser(this,"ComponentChooser");
	connect(m_chooser,SIGNAL(changed(bool)),this,SIGNAL(changed(bool)));
}

void KCMComponentChooser::load(){
	m_chooser->load();
}

void KCMComponentChooser::save(){
	m_chooser->save();
}

void KCMComponentChooser::defaults(){
	m_chooser->restoreDefault();
}

extern "C"
{
    KCModule *create_componentchooser( QWidget *parent, const char * )
    {
        KGlobal::locale()->insertCatalogue("kcmcomponentchooser");
        return new KCMComponentChooser( parent, "kcmcomponentchooser" );
    }
}

