/*
 * $Id$
 *
 * Style Preview Widget
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 *
 * Portions Copyright (C) 2000 TrollTech AS.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __STYLEPREVIEW_H
#define __STYLEPREVIEW_H

#include <qframe.h>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGridLayout;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QRadioButton;
class QScrollBar;
class QSlider;
class QSpinBox;
class QTable;

class StylePreview : public QFrame
{
	Q_OBJECT

public:
	StylePreview( QWidget* parent, const char* name = 0 );
	~StylePreview();

	bool eventFilter( QObject* obj, QEvent* ev );		

private:
	QGridLayout* previewLayout;
	QButtonGroup* bg;
	QRadioButton *rb1, *rb2;
	QCheckBox* cb;
	QScrollBar* scrollbar;
	QSlider* slider;
	QProgressBar* progress;
	QLineEdit* le;
	QComboBox* combo;
	QSpinBox* spin;
	QPushButton* button;
	QTable* table;
};

// vim: set noet ts=4:
#endif // __STYLEPREVIEW_H

