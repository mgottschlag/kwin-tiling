/*
 * $Id$
 *
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __KCMSTYLE_H
#define __KCMSTYLE_H

#include <qtimer.h>
#include <qstring.h>
#include <kcmodule.h>

#include "stylepreview.h"
#include "menupreview.h"

class QCheckBox;
class QComboBox;
class QFrame;
class QGroupBox;
class QListBox;
class QStyle;
class QTabWidget;
class QVBoxLayout; 
class WidgetPreview;
class QSettings;
class QSpacerItem;
class QLabel;
class QSlider;


class KCMStyle : public KCModule
{ 
	Q_OBJECT

public:
	KCMStyle( QWidget* parent = 0, const char* name = 0 );
	~KCMStyle();

	virtual void load();
	virtual void save();
	virtual void defaults();
	virtual const KAboutData* aboutData() const;
	virtual QString quickHelp() const;

protected:
	void loadStyle(void);
	void switchStyle(const QString& styleName);
	void setStyleRecursive(QWidget* w, QStyle* s);

	void loadEffects(void);
	void loadMisc();
	void addWhatsThis();

protected slots:
//	void setDirty();
	void setMacDirty();
	void setEffectsDirty();
	void setToolbarsDirty();
	void setStyleDirty();

	void updateStyleTimer(const QString& style);
	void styleChanged();
	void menuEffectChanged( bool enabled );
	void menuEffectChanged();
	void menuEffectTypeChanged();

private:
	bool m_bMacDirty, m_bEffectsDirty, m_bStyleDirty,
//		 m_bGeneralDirty, 
		 m_bToolbarsDirty;

	QVBoxLayout* mainLayout;
	QTabWidget* tabWidget;
	QWidget *page1, *page2, *page3;
	QVBoxLayout* page1Layout;
	QVBoxLayout* page2Layout;
	QVBoxLayout* page3Layout;

	// Page1 widgets
	QGroupBox* gbWidgetStyle;
	QListBox* lbStyle;
	StylePreview* stylePreview;
	QStyle* appliedStyle;
	QPalette palette;
	QTimer switchStyleTimer;
	QString currentStyle;

	// Page2 widgets
	QGroupBox* gbEffects;
	QCheckBox* cbEnableEffects;

	QFrame* containerFrame;
	QGridLayout* containerLayout;
	QComboBox* comboTooltipEffect;
	QComboBox* comboComboEffect;
	QComboBox* comboMenuEffect;
	QLabel* lblTooltipEffect;
	QLabel* lblComboEffect;
	QLabel* lblMenuEffect;
	QSpacerItem* comboSpacer;

	QFrame* menuContainer;
	QGridLayout* menuContainerLayout;
	MenuPreview* menuPreview;
	QVBox* sliderBox;
	QSlider* slOpacity;
	QComboBox* comboMenuEffectType;
	QLabel* lblMenuEffectType;
	QLabel* lblMenuOpacity;

	// Page3 widgets
	QGroupBox* gbToolbarSettings;
	QGroupBox* gbVisualAppearance;
	
	QCheckBox* cbHoverButtons;
	QCheckBox* cbTransparentToolbars;
	QCheckBox* cbEnableTooltips;
	QComboBox* comboToolbarIcons;
	
	QCheckBox* cbIconsOnButtons;
	QCheckBox* cbTearOffHandles;
	QCheckBox* cbMacMenubar;
};

#endif // __KCMSTYLE_H

// vim: set noet ts=4:
