/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "klistconfirmationdialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QStyledItemDelegate>

#include <KIconLoader>

class KListConfirmationDialogPrivate {
public:
    QVBoxLayout * mainLayout;
    QHBoxLayout * buttonsLayout;

    QPushButton * buttonConfirm;
    QPushButton * buttonCancel;

    QListWidget * listItems;
    QLabel * labelCaption;
    int iconSize;
};

class KListConfirmationDialogListDelegate: public QStyledItemDelegate {
public:
    KListConfirmationDialogListDelegate(int iconSize)
        : m_iconSize(iconSize)
    {
    }

    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        QStyleOptionViewItem o = option;
        o.decorationSize = QSize(m_iconSize, m_iconSize);
        QStyledItemDelegate::paint(painter, o, index);
    }

private:
    int m_iconSize;

};

KListConfirmationDialog::KListConfirmationDialog(
            const QString & title, const QString & message,
            const QString & confirm, const QString & cancel,
            QWidget * parent, Qt::WindowFlags f)
    : d(new KListConfirmationDialogPrivate())
{
    setWindowTitle(title);

    d->mainLayout = new QVBoxLayout(this);

    d->mainLayout->addWidget(d->labelCaption = new QLabel(message, this));
    d->mainLayout->addWidget(d->listItems = new QListWidget(this));
    d->mainLayout->addLayout(d->buttonsLayout = new QHBoxLayout());
    d->buttonsLayout->addStretch(1);

    d->buttonsLayout->setContentsMargins(0, 0, 0, 0);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);

    d->labelCaption->setWordWrap(true);
    d->labelCaption->setContentsMargins(8, 8, 8, 8);

    d->buttonsLayout->addWidget(d->buttonConfirm = new QPushButton("blah"));
    d->buttonsLayout->addWidget(d->buttonCancel  = new QPushButton("blah"));

    d->buttonConfirm->setText(confirm);
    d->buttonCancel->setText(cancel);

    d->iconSize = KIconLoader::global()->currentSize(KIconLoader::Dialog);

    if (d->iconSize < 16) d->iconSize = KIconLoader::SizeMedium;

    d->listItems->setItemDelegate(new  KListConfirmationDialogListDelegate(d->iconSize));

    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    connect(d->buttonConfirm, SIGNAL(clicked()),
            this, SLOT(confirm()));
    connect(d->buttonCancel, SIGNAL(clicked()),
            this, SLOT(cancel()));
}

KListConfirmationDialog::~KListConfirmationDialog()
{
    delete d;
}

void KListConfirmationDialog::addItem(const KIcon & icon, const QString & title,
            const QString & description, const QVariant & data, bool preselect)
{
    QListWidgetItem * item = new QListWidgetItem(icon, title + (description.isNull() ? QString() : "\n" + description), d->listItems);
    item->setCheckState(preselect ? Qt::Checked : Qt::Unchecked);
    item->setSizeHint(QSize(d->iconSize * 3/2, d->iconSize * 3/2));
    item->setData(Qt::UserRole, description);
    item->setData(Qt::UserRole + 1, data);
    d->listItems->addItem(item);
}

void KListConfirmationDialog::showEvent(QShowEvent * event)
{
    // we want to update the size
    int count = d->listItems->count();

    if (count > 5) count = 5;

    resize(size().width(),
            count * d->iconSize * 3 / 2   // height for the list
            + d->buttonsLayout->sizeHint().height() // height for the buttons
            + d->labelCaption->sizeHint().height() // height for the label
            + 32);

    // if (count == d->listItems->count()) {
    //     d->listItems->setStyleSheet("QListWidget { border-left: none; }");
    // }
}

void KListConfirmationDialog::show()
{
}

void KListConfirmationDialog::exec()
{
    QDialog::show();
}

void KListConfirmationDialog::confirm()
{
    QList < QVariant > result;

    foreach (QListWidgetItem * item, d->listItems->selectedItems()) {
        result << item->data(Qt::UserRole + 1);
    }

    selected(result);
    deleteLater();
}

void KListConfirmationDialog::cancel()
{
    QList < QVariant > result;
    selected(result);
    deleteLater();
}

