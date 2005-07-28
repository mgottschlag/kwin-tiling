/*
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
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __KCMSTYLE_H
#define __KCMSTYLE_H

#include <qhash.h>
#include <qstring.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QGridLayout>
#include <Q3Frame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <kcmodule.h>

#include "stylepreview.h"
#include "menupreview.h"

class KComboBox;
class QCheckBox;
class QComboBox;
class Q3Frame;
class Q3GroupBox;
class QLabel;
class Q3ListBox;
class Q3ListViewItem;
class QSettings;
class QSlider;
class QSpacerItem;
class QStyle;
class QTabWidget;
class QVBoxLayout;
class StyleConfigDialog;
class WidgetPreview;

struct StyleEntry {
	QString name;
	QString desc;
	QString configPage;
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

protected:
	bool findStyle( const QString& str, int& combobox_item );
	void switchStyle(const QString& styleName, bool force = false);
	void setStyleRecursive(QWidget* w, QStyle* s);

	void loadStyle( KConfig& config );
	void loadEffects( KConfig& config );
	void loadMisc( KConfig& config );
	void addWhatsThis();

protected slots:
	void styleSpecificConfig();
	void updateConfigButton();
	
	void setEffectsDirty();
	void setToolbarsDirty();
	void setStyleDirty();

	void styleChanged();
	void menuEffectChanged( bool enabled );
	void menuEffectChanged();
	void menuEffectTypeChanged();

private:
	QString currentStyle();

	bool m_bEffectsDirty, m_bStyleDirty, m_bToolbarsDirty;
	QHash <QString,StyleEntry*> styleEntries;
	QMap  <QString,QString>     nameToStyleKey;

	QVBoxLayout* mainLayout;
	QTabWidget* tabWidget;
	QWidget *page1, *page2, *page3;
	QVBoxLayout* page1Layout;
	QVBoxLayout* page2Layout;
	QVBoxLayout* page3Layout;

	// Page1 widgets
	Q3GroupBox* gbWidgetStyle;
	QVBoxLayout* gbWidgetStyleLayout;
	QHBoxLayout* hbLayout;
	KComboBox* cbStyle;
	QPushButton* pbConfigStyle;
	QLabel* lblStyleDesc;
	StylePreview* stylePreview;
	QStyle* appliedStyle;
	QPalette palette;

	// Page2 widgets
	QCheckBox* cbEnableEffects;

	Q3Frame* containerFrame;
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

	Q3Frame* menuContainer;
	QGridLayout* menuContainerLayout;
	MenuPreview* menuPreview;
	Q3VBox* sliderBox;
	QSlider* slOpacity;
	QComboBox* comboMenuEffectType;
	QLabel* lblMenuEffectType;
	QLabel* lblMenuOpacity;
	QCheckBox* cbMenuShadow;

	// Page3 widgets
	Q3GroupBox* gbVisualAppearance;

	QCheckBox* cbHoverButtons;
	QCheckBox* cbTransparentToolbars;
	QCheckBox* cbEnableTooltips;
	QComboBox* comboToolbarIcons;

	QCheckBox* cbIconsOnButtons;
	QCheckBox* cbTearOffHandles;
};

#endif // __KCMSTYLE_H

// vim: set noet ts=4:
