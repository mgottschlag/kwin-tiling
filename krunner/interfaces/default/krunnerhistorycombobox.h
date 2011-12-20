/***************************************************************************
 *   Copyright 2009 by Jacopo De Simoi <wilderkde@gmail.com>               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


#ifndef KRUNNERHISTORYCOMBOBOX_H
#define KRUNNERHISTORYCOMBOBOX_H


#include <KDebug>

#include <QKeyEvent>
#include <QFocusEvent>

#include <KHistoryComboBox>

class KrunnerHistoryComboBox : public KHistoryComboBox
{
    Q_OBJECT

public: 
    explicit KrunnerHistoryComboBox(bool useCompletion, QWidget *parent = 0);
    ~KrunnerHistoryComboBox();
    void setLineEdit(QLineEdit* edit);

public slots:
    void addToHistory(const QString& item);

signals:
    void queryTextEdited(QString);

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    virtual void wheelEvent(QWheelEvent *);

private:
    void discardCompletion();

private slots:
    void currentIndexChanged(const QString &);
    void connectLineEdit();

private:
    bool m_addingToHistory;
};

#endif
