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

#include <QCheckBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QFrame>
#include <QGridLayout>

#include <kconfig.h>
#include <kcolorbutton.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "bgrender.h"
#include "bgadvanced.h"

#include <X11/Xlib.h>
#include <fixx11h.h>

/**** BGAdvancedDialog ****/

BGAdvancedDialog::BGAdvancedDialog(KBackgroundRenderer *_r,
                                   QWidget *parent)
    : KDialog(parent),
      r(_r)
{
    setObjectName("BGAdvancedDialog");
    setModal(true);
    setCaption(i18n("Advanced Background Settings"));
    setButtons(Ok | Cancel);

    dlg = new BGAdvancedBase(this);
    setMainWidget(dlg);

    dlg->m_listPrograms->header()->setResizeMode(1, QHeaderView::Stretch);
    dlg->m_listPrograms->setRootIsDecorated(false);
    dlg->m_listPrograms->setAllColumnsShowFocus(true);

    connect(dlg->m_listPrograms, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SLOT(slotProgramItemClicked(QTreeWidgetItem*)));

    // Load programs
    const QStringList lst = KBackgroundProgram::list();
    QStringList::const_iterator it;
    for (it = lst.constBegin(); it != lst.constEnd(); ++it)
        addProgram(*it);

    // FIXME: support it proper
    dlg->m_buttonAdd->hide();
    dlg->m_buttonRemove->hide();
    dlg->m_buttonModify->hide();
    dlg->m_groupCache->hide();

    connect(dlg->m_cbProgram, SIGNAL(toggled(bool)),
            SLOT(slotEnableProgram(bool)));

    m_oldBackgroundMode = r->backgroundMode();
    if (m_oldBackgroundMode == KBackgroundSettings::Program)
        m_oldBackgroundMode = KBackgroundSettings::Flat;

    dlg->adjustSize();
    updateUI();
}

void BGAdvancedDialog::makeReadOnly()
{
    dlg->m_cbProgram->setEnabled(false);
    dlg->m_listPrograms->setEnabled(false);
}

#if 0
void BGAdvancedDialog::setCacheSize(int s)
{
    dlg->m_spinCache->setValue(s);
}

int BGAdvancedDialog::cacheSize()
{
    return dlg->m_spinCache->value();
}

QColor BGAdvancedDialog::textColor()
{
    return dlg->m_colorText->color();
}

void BGAdvancedDialog::setTextColor(const QColor &color)
{
    dlg->m_colorText->setColor(color);
}

QColor BGAdvancedDialog::textBackgroundColor()
{
    return dlg->m_cbSolidTextBackground->isChecked() ?
           dlg->m_colorTextBackground->color() : QColor();
}

void BGAdvancedDialog::setTextBackgroundColor(const QColor &color)
{
    dlg->m_colorTextBackground->blockSignals(true);
    dlg->m_cbSolidTextBackground->blockSignals(true);
    if (color.isValid()) {
        dlg->m_cbSolidTextBackground->setChecked(true);
        dlg->m_colorTextBackground->setColor(color);
        dlg->m_colorTextBackground->setEnabled(true);
    } else {
        dlg->m_cbSolidTextBackground->setChecked(false);
        dlg->m_colorTextBackground->setColor(Qt::white);
        dlg->m_colorTextBackground->setEnabled(false);
    }
    dlg->m_colorTextBackground->blockSignals(false);
    dlg->m_cbSolidTextBackground->blockSignals(false);
}

bool BGAdvancedDialog::shadowEnabled()
{
    return dlg->m_cbShadow->isChecked();
}

void BGAdvancedDialog::setShadowEnabled(bool enabled)
{
    dlg->m_cbShadow->setChecked(enabled);
}

void BGAdvancedDialog::setTextLines(int lines)
{
    dlg->m_spinTextLines->setValue(lines);
}

int BGAdvancedDialog::textLines() const
{
    return dlg->m_spinTextLines->value();
}

void BGAdvancedDialog::setTextWidth(int width)
{
    dlg->m_spinTextWidth->setValue(width);
}

int BGAdvancedDialog::textWidth() const
{
    return dlg->m_spinTextWidth->value();
}
#endif

void BGAdvancedDialog::updateUI()
{
    QString prog = r->KBackgroundProgram::name();

    dlg->m_cbProgram->blockSignals(true);
    if ((r->backgroundMode() == KBackgroundSettings::Program)
            && !prog.isEmpty()) {
        dlg->m_cbProgram->setChecked(true);
        dlg->m_listPrograms->setEnabled(true);
        dlg->m_buttonAdd->setEnabled(true);
        dlg->m_buttonRemove->setEnabled(true);
        dlg->m_buttonModify->setEnabled(true);
        selectProgram(prog);
    } else {
        dlg->m_cbProgram->setChecked(false);
        dlg->m_listPrograms->setEnabled(false);
        dlg->m_buttonAdd->setEnabled(false);
        dlg->m_buttonRemove->setEnabled(false);
        dlg->m_buttonModify->setEnabled(false);
    }
    dlg->m_cbProgram->blockSignals(false);
}

#if 0
void BGAdvancedDialog::removeProgram(const QString &name)
{
    delete m_programItems.take(name);
}
#endif

void BGAdvancedDialog::addProgram(const QString &name)
{
#if 0
    removeProgram(name);
#endif

    KBackgroundProgram prog(name);
    if (prog.command().isEmpty() || (prog.isGlobal() && !prog.isAvailable()))
        return;

    QTreeWidgetItem *item = new QTreeWidgetItem(dlg->m_listPrograms);
    item->setText(0, prog.name());
    item->setText(1, prog.comment());
    item->setText(2, i18n("%1 min.", prog.refresh()));

    m_programItems.insert(name, item);
}

void BGAdvancedDialog::selectProgram(const QString &name)
{
    if (QTreeWidgetItem *item = m_programItems[name]) {
        dlg->m_listPrograms->scrollToItem(item);
        item->setSelected(true);
        m_selectedProgram = name;
    }
}

#if 0
void BGAdvancedDialog::slotAdd()
{
    KProgramEditDialog dlg(QString(), this);
    dlg.exec();
    if (dlg.result() == QDialog::Accepted) {
        QString program = dlg.program();
        addProgram(program);
        selectProgram(program);
    }
}

void BGAdvancedDialog::slotRemove()
{
    if (m_selectedProgram.isEmpty())
        return;

    KBackgroundProgram prog(m_selectedProgram);
    if (prog.isGlobal()) {
        KMessageBox::sorry(this,
           i18n("Unable to remove the program: the program is global "
                "and can only be removed by the system administrator."),
           i18n("Cannot Remove Program"));
        return;
    }
    if (KMessageBox::warningContinueCancel(this,
           i18n("Are you sure you want to remove the program `%1'?",
                prog.name()),
           i18n("Remove Background Program"),
           KGuiItem(i18n("&Remove"))) != KMessageBox::Continue)
        return;

    prog.remove();
    removeProgram(m_selectedProgram);
    m_selectedProgram.clear();
}

/*
 * Modify a program.
 */
void BGAdvancedDialog::slotModify()
{
    if (m_selectedProgram.isEmpty())
        return;

    KProgramEditDialog dlg(m_selectedProgram, this);
    dlg.exec();
    if (dlg.result() == QDialog::Accepted) {
        QString program = dlg.program();
        if (program != m_selectedProgram) {
            KBackgroundProgram prog(m_selectedProgram);
            prog.remove();
            removeProgram(m_selectedProgram);
        }
        addProgram(dlg.program());
        selectProgram(dlg.program());
    }
}
#endif

void BGAdvancedDialog::slotProgramItemClicked(QTreeWidgetItem *item)
{
    if (item)
        m_selectedProgram = item->text(0);
    slotProgramChanged();
}

#if 0
void BGAdvancedDialog::slotProgramItemDoubleClicked(QTreeWidgetItem *item)
{
    slotProgramItemClicked(item);
    slotModify();
}
#endif

void BGAdvancedDialog::slotProgramChanged()
{
//   r->stop();

    r->setProgram(m_selectedProgram);
    if (dlg->m_cbProgram->isChecked() && !m_selectedProgram.isEmpty())
        r->setBackgroundMode(KBackgroundSettings::Program);
    else
        r->setBackgroundMode(m_oldBackgroundMode);

// We have no preview, no need to start the renderer
//   r->start();
}

void BGAdvancedDialog::slotEnableProgram(bool b)
{
    dlg->m_listPrograms->setEnabled(b);
    if (b) {
        if (QTreeWidgetItem *cur = dlg->m_listPrograms->currentItem()) {
            dlg->m_listPrograms->blockSignals(true);
            cur->setSelected(true);
            dlg->m_listPrograms->scrollToItem(cur);
            dlg->m_listPrograms->blockSignals(false);
            slotProgramItemClicked(cur);
        }
    } else {
        slotProgramChanged();
    }
}

/**** KProgramEditDialog ****/
#if 0
KProgramEditDialog::KProgramEditDialog(const QString &program, QWidget *parent, char *name)
    : KDialog(parent)
{
    setObjectName(name);
    setModal(true);
    setCaption(i18n("Configure Background Program"));
    setButtons(Ok | Cancel);

    QFrame *frame = new QFrame(this);
    setMainWidget(frame);

    QGridLayout *grid = new QGridLayout(frame);
    grid->setSpacing(spacingHint());
    grid->setMargin(0);
    grid->setColumnMinimumWidth(1, 300);

    QLabel *lbl = new QLabel(i18n("&Name:"), frame);
    grid->addWidget(lbl, 0, 0);
    m_NameEdit = new QLineEdit(frame);
    lbl->setBuddy(m_NameEdit);
    grid->addWidget(m_NameEdit, 0, 1);

    lbl = new QLabel(i18n("Co&mment:"), frame);
    grid->addWidget(lbl, 1, 0);
    m_CommentEdit = new QLineEdit(frame);
    lbl->setBuddy(m_CommentEdit);
    grid->addWidget(m_CommentEdit, 1, 1);

    lbl = new QLabel(i18n("Comman&d:"), frame);
    grid->addWidget(lbl, 2, 0);
    m_CommandEdit = new QLineEdit(frame);
    lbl->setBuddy(m_CommandEdit);
    grid->addWidget(m_CommandEdit, 2, 1);

    lbl = new QLabel(i18n("&Preview cmd:"), frame);
    grid->addWidget(lbl, 3, 0);
    m_PreviewEdit = new QLineEdit(frame);
    lbl->setBuddy(m_PreviewEdit);
    grid->addWidget(m_PreviewEdit, 3, 1);

    lbl = new QLabel(i18n("&Executable:"), frame);
    grid->addWidget(lbl, 4, 0);
    m_ExecEdit = new QLineEdit(frame);
    lbl->setBuddy(m_ExecEdit);
    grid->addWidget(m_ExecEdit, 4, 1);

    lbl = new QLabel(i18n("&Refresh time:"), frame);
    grid->addWidget(lbl, 5, 0);
    m_RefreshEdit = new QSpinBox(frame);
    m_RefreshEdit->setRange(5, 60);
    m_RefreshEdit->setSingleStep(5);
    m_RefreshEdit->setSuffix(i18n(" min"));
    m_RefreshEdit->setFixedSize(m_RefreshEdit->sizeHint());
    lbl->setBuddy(m_RefreshEdit);
    grid->addWidget(m_RefreshEdit, 5, 1, Qt::AlignLeft);

    m_Program = program;
    if (m_Program.isEmpty()) {
        KBackgroundProgram prog(i18n("New Command"));
        int i = 1;
        while (!prog.command().isEmpty())
            prog.load(i18n("New Command <%1>", i++));
        m_NameEdit->setText(prog.name());
        m_NameEdit->setSelection(0, 100);
        m_RefreshEdit->setValue(15);
        return;
    }

    // Fill in the fields
    m_NameEdit->setText(m_Program);
    KBackgroundProgram prog(m_Program);
    m_CommentEdit->setText(prog.comment());
    m_ExecEdit->setText(prog.executable());
    m_CommandEdit->setText(prog.command());
    m_PreviewEdit->setText(prog.previewCommand());
    m_RefreshEdit->setValue(prog.refresh());
}


QString KProgramEditDialog::program()const
{
    return m_NameEdit->text();
}

void KProgramEditDialog::accept()
{
    QString s = m_NameEdit->text();
    if (s.isEmpty()) {
        KMessageBox::sorry(this, i18n("You did not fill in the `Name' field.\n"
                                      "This is a required field."));
        return;
    }

    KBackgroundProgram prog(s);
    if ((s != m_Program) && !prog.command().isEmpty()) {
        int ret = KMessageBox::warningContinueCancel(this,
                  i18n("There is already a program with the name `%1'.\n"
                       "Do you want to overwrite it?", s), QString(), KGuiItem(i18n("Overwrite")));
        if (ret != KMessageBox::Continue)
            return;
    }

    if (m_ExecEdit->text().isEmpty()) {
        KMessageBox::sorry(this, i18n("You did not fill in the `Executable' field.\n"
                                      "This is a required field."));
        return;
    }
    if (m_CommandEdit->text().isEmpty()) {
        KMessageBox::sorry(this, i18n("You did not fill in the `Command' field.\n"
                                      "This is a required field."));
        return;
    }

    prog.setComment(m_CommentEdit->text());
    prog.setExecutable(m_ExecEdit->text());
    prog.setCommand(m_CommandEdit->text());
    prog.setPreviewCommand(m_PreviewEdit->text());
    prog.setRefresh(m_RefreshEdit->value());

    prog.writeSettings();

    KDialog::accept();
}
#endif

#include "bgadvanced.moc"
