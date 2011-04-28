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

class KListConfirmationDialogPrivate;

#include <QVariant>
#include <QList>
#include <QDialog>

#include <KIcon>

/**
 *
 */
class KListConfirmationDialog: public QDialog {
    Q_OBJECT

public:
    KListConfirmationDialog(
            const QString & title = QString(),
            const QString & message = QString(),
            const QString & confirm = QString(),
            const QString & cancel = QString(),
            QWidget * parent = 0, Qt::WindowFlags f = 0);
    virtual ~KListConfirmationDialog();

    void addItem(const KIcon & icon, const QString & title,
            const QString & description, const QVariant & data, bool preselect = false);


    void exec();

protected:
    void showEvent(QShowEvent * event);

protected Q_SLOTS:
    void confirm();
    void cancel();

Q_SIGNALS:
    void selected(QList < QVariant > items);

private:
    void show();

    class KListConfirmationDialogPrivate * const d;
};
