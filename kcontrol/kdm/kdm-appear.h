/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#ifndef __KDMAPPEAR_H__
#define __KDMAPPEAR_H__

#include <qdir.h>
#include <qimage.h>
#include <qfileinfo.h>
#include <kcolorbtn.h>
#include <kurl.h>

#include "klangcombo.h"

class KLineEdit;

class KDMAppearanceWidget : public KConfigWidget
{
	Q_OBJECT

public:
	KDMAppearanceWidget(QWidget *parent, const char *name, bool init = false);
	~KDMAppearanceWidget();

        void loadSettings();
        void applySettings();
	void setupPage(QWidget*);
	bool eventFilter(QObject *, QEvent *);

protected:
	void iconLoaderDragEnterEvent(QDragEnterEvent *event);
	void iconLoaderDropEvent(QDropEvent *event);

private slots:
        void slotSetGUI(int);
        void slotLogoPixChanged(const QString&);
        void slotLogoPixTextChanged();

private:
        KIconLoader *iconloader;
        KIconLoaderButton *logobutton;
        KLineEdit    *greetstr_lined, *logo_lined;
	QComboBox    *guicombo;
	QString      greetstr, logopath, logofile, guistr;
        bool         gui;
        KLanguageCombo *langcombo;
};

#endif


