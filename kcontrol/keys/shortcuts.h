/*
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SHORTCUTS_MODULE_H
#define SHORTCUTS_MODULE_H

#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include "kaccelaction.h"
#include <kcombobox.h>
#include <kkeydialog.h>

class ShortcutsModule : public QWidget
{
	Q_OBJECT
 public:
	ShortcutsModule( QWidget *parent = 0, const char *name = 0 );
	~ShortcutsModule();

	void load();
	void save();
	void defaults();
	QString quickHelp() const;

 protected:
	void initGUI();
	void createActionsGeneral();
	void createActionsSequence();
	void readSchemeNames();
	void saveScheme();
	void resizeEvent(QResizeEvent *e);

 signals:
	void changed( bool );

 protected slots:
	void slotSchemeCur();
	void slotKeyChange();
	void slotSelectScheme( int = 0 );
	void slotSaveSchemeAs();
	void slotRemoveScheme();

 private:
	QTabWidget* m_pTab;
	QRadioButton *m_prbPre, *m_prbNew;
	QComboBox* m_pcbSchemes;
	QPushButton* m_pbtnSave, * m_pbtnRemove;
	int m_nSysSchemes;
	QStringList m_rgsSchemeFiles;
	KAccelActions m_actionsGeneral, m_actionsSequence;//, m_actionsApplication;
	KShortcutList* m_pListGeneral, * m_pListSequence, * m_pListApplication;
	KKeyChooser* m_pkcGeneral, * m_pkcSequence, * m_pkcApplication;
};

#endif // __SHORTCUTS_MODULE_H
