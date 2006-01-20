/***************************************************************************
                          kstylepage.h  -  description
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

#ifndef KSTYLEPAGE_H
#define KSTYLEPAGE_H

#include <qcolor.h>
#include "kstylepagedlg.h"

/**Abstract class for the style page
  *@author Ralf Nolden
  */
class Q3ListViewItem;

class KStylePage : public KStylePageDlg  {
	Q_OBJECT

public:
	KStylePage(QWidget *parent=0, const char *name=0);
	~KStylePage();
	void save(bool curSettings=true);
	/** resets to KDE style as default */
	void setDefaults();
	/** set the preview-widgets' style to the currently selected */
	void switchPrevStyle();

private:
	QString origStyle;
	QString origKWinStyle;
	QString origIcons;
	QString defaultKWinStyle;
	QString currentStyle;
	KConfig* ckwin;
	struct colorSet {
		QString colorFile, bgMode;
		int contrast;
		QColor usrCol1, usrCol2;
		QColor foreground;
		QColor background;
		QColor windowForeground;
		QColor windowBackground;
		QColor selectForeground;
		QColor selectBackground;
		QColor buttonForeground;
		QColor buttonBackground;
		QColor linkColor;
		QColor visitedLinkColor;
		QColor activeForeground;
		QColor inactiveForeground;
		QColor activeBackground;
		QColor inactiveBackground;
		QColor activeBlend;
		QColor inactiveBlend;
		QColor activeTitleBtnBg;
		QColor inactiveTitleBtnBg;
                QColor alternateBackground;
	} usrColors, currentColors;
	// first, the KDE 2 default color values
	QColor widget;
	QColor kde34Blue;
        QColor inactiveBackground;
        QColor activeBackground;
	QColor button;
	QColor link;
	QColor visitedLink;
        QColor activeBlend;
        QColor activeTitleBtnBg;
        QColor inactiveTitleBtnBg;
        QColor inactiveForeground;
        QColor alternateBackground;

	Q3ListViewItem * kde;
	Q3ListViewItem * classic;
	Q3ListViewItem * keramik;
	Q3ListViewItem * cde;
	Q3ListViewItem * win;
	Q3ListViewItem * platinum;

	QStyle *appliedStyle;

	// widget-style existence
	bool kde_hc_exist, kde_def_exist, kde_keramik_exist, kde_light_exist,
		cde_exist, win_exist, platinum_exist, kde_plastik_exist;

	// kwin-style-existence
	bool kwin_keramik_exist, kwin_default_exist, kwin_system_exist,
		kwin_win_exist, kwin_cde_exist, kwin_quartz_exist, kwin_plastik_exist;

	// icon-theme-existence
	bool icon_crystalsvg_exist, icon_kdeclassic_exist, icon_Locolor_exist;

public Q_SLOTS: // Public slots
	/** to be connected to the OS page. Catches either KDE, CDE, win or mac and pre-sets the style.  */
	void presetStyle(const QString& style);

private:
	void saveColors(bool curSettings=true);
	void saveStyle(bool curSettings=true);
	void saveKWin(bool curSettings=true);
	void saveIcons(bool curSettings=true);
	void getAvailability();
	void getUserDefaults();
	void initColors();
	void liveUpdate();
	void getColors(colorSet *set, bool colorfile );
	void setStyleRecursive(QWidget* w, QPalette &, QStyle* s);
	void changeCurrentStyle();
	QPalette createPalette();

private Q_SLOTS:
	void slotCurrentChanged();
};

#endif
