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
class QListViewItem;
class QStyle;
class QTabWidget;
class QVBoxLayout;
class WidgetPreview;
class QSettings;
class QSpacerItem;
class QLabel;
class QSlider;
class KComboBox;

struct StyleEntry {
	QString name;
	QString desc;
	bool hidden;
};

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
	bool findStyle( const QString& str, int& combobox_item );
	void switchStyle(const QString& styleName);
	void setStyleRecursive(QWidget* w, QStyle* s);

	void loadStyle( KSimpleConfig& config );
	void loadEffects( KSimpleConfig& config );
	void loadMisc( KSimpleConfig& config );
	void addWhatsThis();

protected slots:
	void setEffectsDirty();
	void setToolbarsDirty();
	void setStyleDirty();

	void styleChanged();
	void menuEffectChanged( bool enabled );
	void menuEffectChanged();
	void menuEffectTypeChanged();

private:
	bool m_bEffectsDirty, m_bStyleDirty, m_bToolbarsDirty;
	QDict<StyleEntry> styleEntries;

	QVBoxLayout* mainLayout;
	QTabWidget* tabWidget;
	QWidget *page1, *page2, *page3;
	QVBoxLayout* page1Layout;
	QVBoxLayout* page2Layout;
	QVBoxLayout* page3Layout;

	// Page1 widgets
	QGroupBox* gbWidgetStyle;
	QVBoxLayout* gbWidgetStyleLayout;
	QHBoxLayout* hbLayout;
	KComboBox* cbStyle;
	QPushButton* pbConfigStyle;
	QLabel* lblStyleDesc;
	StylePreview* stylePreview;
	QStyle* appliedStyle;
	QPalette palette;
	QString currentStyle;

	// Page2 widgets
	QGroupBox* gbEffects;
	QCheckBox* cbEnableEffects;

	QFrame* containerFrame;
	QGridLayout* containerLayout;
	QComboBox* comboTooltipEffect;
	QComboBox* comboComboEffect;
	QComboBox* comboMenuEffect;
	QComboBox* comboMenuHandle;

	QLabel* lblTooltipEffect;
	QLabel* lblComboEffect;
	QLabel* lblMenuEffect;
	QLabel* lblMenuHandle;
	QSpacerItem* comboSpacer;

	QFrame* menuContainer;
	QGridLayout* menuContainerLayout;
	MenuPreview* menuPreview;
	QVBox* sliderBox;
	QSlider* slOpacity;
	QComboBox* comboMenuEffectType;
	QLabel* lblMenuEffectType;
	QLabel* lblMenuOpacity;
	QCheckBox* cbMenuShadow;

	// Page3 widgets
	QGroupBox* gbToolbarSettings;
	QGroupBox* gbVisualAppearance;

	QCheckBox* cbHoverButtons;
	QCheckBox* cbTransparentToolbars;
	QCheckBox* cbEnableTooltips;
	QComboBox* comboToolbarIcons;

	QCheckBox* cbIconsOnButtons;
	QCheckBox* cbTearOffHandles;
};

#endif // __KCMSTYLE_H

// vim: set noet ts=4:
