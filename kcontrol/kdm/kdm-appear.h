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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#ifndef __KDMAPPEAR_H__
#define __KDMAPPEAR_H__


#include <qdir.h>
#include <qimage.h>
#include <qfileinfo.h>
#include <qpushbutton.h>

#include <kcolorbutton.h>
#include <kurl.h>
#include <kfiledialog.h>


#include "klanguagebutton.h"

class QComboBox;
class KBackedComboBox;
class QLabel;
class QRadioButton;
class QLineEdit;
class KLineEdit;


class KDMAppearanceWidget : public QWidget
{
	Q_OBJECT

public:
	KDMAppearanceWidget(QWidget *parent, const char *name=0);

	void load();
	void save();
	void defaults();
	void makeReadOnly();
	QString quickHelp() const;

	void loadColorSchemes(KBackedComboBox *combo);
	void loadGuiStyles(KBackedComboBox *combo);
	void loadLanguageList(KLanguageButton *combo);

	bool eventFilter(QObject *, QEvent *);

signals:
	void changed( bool state );

protected:
	void iconLoaderDragEnterEvent(QDragEnterEvent *event);
	void iconLoaderDropEvent(QDropEvent *event);
	bool setLogo(QString logo);

private slots:
	void slotAreaRadioClicked(int id);
	void slotLogoButtonClicked();
	void changed();

private:
	enum { KdmNone, KdmClock, KdmLogo };
	QLabel      *logoLabel;
	QPushButton *logobutton;
	KLineEdit    *greetstr_lined;
	QString      logopath;
	QRadioButton *noneRadio;
	QRadioButton *clockRadio;
	QRadioButton *logoRadio;
	QLineEdit    *xLineEdit;
	QLineEdit    *yLineEdit;
	KBackedComboBox    *guicombo;
	KBackedComboBox    *colcombo;
	KBackedComboBox    *echocombo;
	KLanguageButton *langcombo;

};

#endif
