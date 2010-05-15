/*
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 * Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
 *
 * Portions Copyright (C) TrollTech AS.
 *
 * Based on kcmdisplay
 * Copyright (C) 1997-2002 kcmdisplay Authors.
 * (see Help -> About Style Settings)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KCMSTYLE_H
#define KCMSTYLE_H

#include <QHash>
#include <QLayout>
#include <QMap>

#include <kcmodule.h>
#include <kvbox.h>

#include "ui_theme.h"

class KComboBox;
class KConfig;
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class StylePreview;
class QTabWidget;

class ThemeModel;

class KCMStyle : public KCModule
{
	Q_OBJECT

public:
	KCMStyle( QWidget* parent, const QVariantList& );
	~KCMStyle();

	virtual void load();
	virtual void save();
	virtual void defaults();

protected Q_SLOTS:
	void loadDesktopTheme();

	void setDesktopThemeDirty();
	
	void getNewThemes();

	void tabChanged(int);

private:
	bool m_bDesktopThemeDirty;

	QVBoxLayout* mainLayout;
	QTabWidget* tabWidget;
	QWidget *page0;
	
	//Page0
	Ui::theme themeUi;
	ThemeModel* m_themeModel;
        bool m_isNetbook;
        bool m_workspaceThemeTabActivated;
};

#endif // __KCMSTYLE_H

// vim: set noet ts=4:
