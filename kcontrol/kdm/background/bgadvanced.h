/* vi: ts=8 sts=4 sw=4

   This file is part of the KDE project, module kcmbackground.

   Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef BGADVANCED_H
#define BGADVANCED_H

#include <Qt3Support/Q3Dict>
#include <Qt3Support/Q3CheckListItem>

#include <kdialog.h>

#include "ui_bgadvanced_ui.h"

class QLineEdit;
class QSpinBox;

class KBackgroundRenderer;
class KBackgroundProgram;

class BGAdvancedBase : public QWidget, public Ui::BGAdvancedBase
{
public:
  BGAdvancedBase( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};

class BGAdvancedDialog : public KDialog
{
   Q_OBJECT
public:
   BGAdvancedDialog(KBackgroundRenderer *_r, QWidget *parent, bool _kdmMode);

#if 0
   void setCacheSize(int s);
   int cacheSize();
   QColor textColor();
   void setTextColor(const QColor &color);
   QColor textBackgroundColor();
   void setTextBackgroundColor(const QColor &color);
   bool shadowEnabled();
   void setShadowEnabled(bool enabled);
   void setTextLines(int lines);
   int textLines() const;
   void setTextWidth(int width);
   int textWidth() const;
#endif

   void updateUI();

   void makeReadOnly();

#if 0
public Q_SLOTS:
   void slotAdd();
   void slotRemove();
   void slotModify();
#endif

protected:
   void addProgram(const QString &name);
#if 0
   void removeProgram(const QString &name);
#endif
   void selectProgram(const QString &name);

protected Q_SLOTS:
   void slotProgramItemClicked(Q3ListViewItem *item);
#if 0
   void slotProgramItemDoubleClicked(Q3ListViewItem *item);
#endif
   void slotProgramChanged();
   void slotEnableProgram(bool b);

private:
   KBackgroundRenderer *r;

   BGAdvancedBase *dlg;

   QWidget *m_pMonitor;
   Q3Dict<Q3ListViewItem> m_programItems;
   QString m_selectedProgram;
   int m_oldBackgroundMode;
   bool m_kdmMode;
};

#if 0
/**
 * Dialog to edit a background program.
 */
class KProgramEditDialog: public KDialog
{
    Q_OBJECT

public:
    explicit KProgramEditDialog(bool kdmMode, const QString &program=QString(),
                                QWidget *parent=0L, char *name=0L);

    /** The program name is here in case the user changed it */
    QString program()const;

public Q_SLOTS:
    virtual void accept();

private:
    QString m_Program;
    QLineEdit *m_NameEdit, *m_CommentEdit;
    QLineEdit *m_ExecEdit, *m_CommandEdit;
    QLineEdit *m_PreviewEdit;
    QSpinBox *m_RefreshEdit;
    KBackgroundProgram *m_Prog;
    bool m_kdmMode;
};
#endif

#endif

