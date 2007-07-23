/***************************************************************************
 *   Copyright (C) 2007 by Stephen Leaf                                    *
 *   smileaf@smileaf.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "listdelegate.h"

#include <QtGui>

ListDelegate::ListDelegate(QObject *parent)
	: QItemDelegate(parent)
{
}

QWidget *ListDelegate::createEditor(QWidget *parent,
									const QStyleOptionViewItem & /* option */,
									const QModelIndex &index) const
{
	QComboBox *comboBox = new QComboBox(parent);
	if (index.column() == 1) {
		comboBox->addItem(tr("Normal"));
		comboBox->addItem(tr("Not Normal..."));
	} else if (index.column() == 2) {
		comboBox->addItem(tr("Off"));
		comboBox->addItem(tr("On"));
	}

	connect(comboBox, SIGNAL(activated(int)), this, SLOT(emitCommitData()));

	return comboBox;
}

void ListDelegate::setEditorData(QWidget *editor,
								const QModelIndex &index) const
{
	QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
	if (!comboBox)
		return;

	int pos = comboBox->findText(index.model()->data(index).toString(),
								Qt::MatchExactly);
	comboBox->setCurrentIndex(pos);
}

void ListDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
								const QModelIndex &index) const
{
	QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
	if (!comboBox)
		return;

	model->setData(index, comboBox->currentText());
}

void ListDelegate::emitCommitData()
{
	emit commitData(qobject_cast<QWidget *>(sender()));
}
