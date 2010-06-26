/*

Dialog class that handles input focus in absence of a wm

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2003 Oswald Buddenhagen <ossi@kde.org>


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/


#ifndef FDIALOG_H
#define FDIALOG_H

#include <QDialog>
#include <QMessageBox>

class QFrame;

class FDialog : public QDialog {
    Q_OBJECT
    typedef QDialog inherited;

  public:
    FDialog(QWidget *parent = 0, bool framed = true);
    virtual int exec();
    static void fitInto(const QRect &scr, QRect &grt);

  signals:
    void ready();

  protected:
    virtual void resizeEvent(QResizeEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    void adjustGeometry();

  private:
    QFrame *winFrame;
    bool readyEmitted;
};

#define errorbox QMessageBox::Critical
#define sorrybox QMessageBox::Warning
#define infobox QMessageBox::Information

class KFMsgBox : public FDialog {
    typedef FDialog inherited;

  private:
    KFMsgBox(QWidget *parent, QMessageBox::Icon type, const QString &text);

  public:
    static void box(QWidget *parent, QMessageBox::Icon type,
                    const QString &text);
};

#endif /* FDIALOG_H */
