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

#include "stylepreview.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qtable.h>

#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>


StylePreview::StylePreview( QWidget* parent, const char* name )
	: QFrame( parent, name )
{
	setFrameStyle( StyledPanel | Sunken );

	// Add all the nice preview widgets.
	previewLayout = new QGridLayout( this, 1, 1, 
		KDialog::marginHint(), KDialog::spacingHint() );

	// ButtonGroup
	bg = new QButtonGroup( 1, Qt::Horizontal, i18n("ButtonGroup"), this );
	bg->layout()->setSpacing( KDialog::spacingHint() );
	bg->layout()->setMargin( KDialog::marginHint() );
	rb1 = new QRadioButton( i18n("RadioButton1"), bg );
	rb1->setChecked( TRUE );
	rb2 = new QRadioButton( i18n("RadioButton2"), bg );
	cb = new QCheckBox( i18n("CheckBox"), bg );
	cb->setChecked( TRUE );

	// All other LHS items
	scrollbar = new QScrollBar( Qt::Horizontal, this );
	slider = new QSlider( Qt::Horizontal, this );
	slider->setValue( 50 );
	progress = new QProgressBar( this );
	progress->setProgress( 70 );

	// All RHS items
	le = new QLineEdit( i18n("LineEdit"), this );
	combo = new QComboBox( FALSE, this );
	combo->insertItem( i18n("ComboBox") );
	spin = new QSpinBox( 50, 100, 1, this );
	button = new QPushButton( i18n("PushButton"), this );
	table = new QTable( 4, 2, this );
	table->setLineWidth(1);
	table->setSelectionMode( QTable::NoSelection );

	// Add LHS Items to the layout
	previewLayout->addMultiCellWidget( bg, 0, 3, 0, 0 );	// ButtonGroup
	previewLayout->addWidget( scrollbar, 4, 0 );			// ScrollBar
	previewLayout->addWidget( slider,    5, 0 );			// Slider
	previewLayout->addWidget( progress,  6, 0 );			// ProgressBar

	// Add RHS Items to the layout
	previewLayout->addMultiCellWidget( le,    0, 0, 1, 2 );	// LineEdit
	previewLayout->addMultiCellWidget( combo, 1, 1, 1, 2 );	// ComboBox
	previewLayout->addWidget( spin,   2, 1 );				// SpinBox
	previewLayout->addWidget( button, 2, 2 );				// PushButton
	previewLayout->addMultiCellWidget( table, 3, 6, 1, 2 );	// Table

	// Ensure that the user can't toy with the child widgets.
	// Method borrowed from Qt's qtconfig.
	QObjectList* l = queryList("QWidget");
	QObjectListIt it(*l);
	QObject* obj;
	while ((obj = it.current()) != 0)
	{
		++it;
		obj->installEventFilter(this);
		((QWidget*)obj)->setFocusPolicy(NoFocus);
	}
}

StylePreview::~StylePreview()
{
}

bool StylePreview::eventFilter( QObject* obj, QEvent* ev )
{
	switch( ev->type() )
	{
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
		case QEvent::MouseMove:
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
		case QEvent::Enter:
		case QEvent::Leave:
		case QEvent::Wheel:
			return TRUE; // ignore
		default:
			break;
	}
	return FALSE;
}

// vim: set noet ts=4:
