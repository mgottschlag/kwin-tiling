/*
 * main.h
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __keys_main_h
#define __keys_main_h

#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <kaccelbase.h>
#include <kcmodule.h>
#include <kcombobox.h>
#include <kkeydialog.h>

class KKeyModule;

class KeyModule : public KCModule
{
	Q_OBJECT
 public:
	KeyModule( QWidget *parent, const char *name );

	void load();
	void save();
	void defaults();
	QString quickHelp() const;

 protected:
	void init();
	void createActionsGeneral();
	void createActionsSequence();
	void createActionsApplication();
	void readSchemeNames();
	void saveScheme();
	void resizeEvent(QResizeEvent *e);

 protected slots:
	void slotSchemeCur();
	void slotKeyChange();
	void slotSelectScheme( int iItem );
	void slotSaveSchemeAs();
	void slotRemoveScheme();
	void moduleChanged(bool state);
	void tabChanged(QWidget *);

 private:
	QTabWidget* m_pTab;
	QRadioButton *m_prbCur, *m_prbNew, *m_prbPre;
	KComboBox* m_pcbSchemes;
	QPushButton* m_pbtnSave, * m_pbtnRemove;
	int m_nSysSchemes;
	QStringList m_rgsSchemeFiles;
	KAccelActions m_actionsGeneral, m_actionsSequence, m_actionsApplication;
	KKeyChooser* m_pkcGeneral, * m_pkcSequence, * m_pkcApplication;
};

#endif
