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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kpixmap.h>
#include <kstandarddirs.h>

#include "bgrender.h"
#include "bgadvanced.h"
#include "bgadvanced_ui.h"

/**** BGAdvancedDialog ****/

BGAdvancedDialog::BGAdvancedDialog(KBackgroundRenderer *_r,
	QWidget *parent, bool m_multidesktop)
	: KDialogBase(parent, "BGAdvancedDialog", true, i18n("Advanced Background Settings"),
	Ok, Ok, true), r(_r)
{
   dlg = new BGAdvancedBase(this);
   setMainWidget(dlg);

   if (m_multidesktop)
   {
      dlg->m_listPrograms->header()->setStretchEnabled ( true, 1 );
      dlg->m_listPrograms->setAllColumnsShowFocus(true);

      connect(dlg->m_listPrograms, SIGNAL(clicked(QListViewItem *)),
            SLOT(slotProgramItemClicked(QListViewItem *)));
      connect(dlg->m_listPrograms, SIGNAL(doubleClicked(QListViewItem *)),
            SLOT(slotProgramItemDoubleClicked(QListViewItem *)));
	    
      connect(dlg->m_buttonAdd, SIGNAL(clicked()),
            SLOT(slotAdd()));
      connect(dlg->m_buttonRemove, SIGNAL(clicked()),
            SLOT(slotRemove()));
      connect(dlg->m_buttonModify, SIGNAL(clicked()),
            SLOT(slotModify()));

      // Load programs
      QStringList lst = KBackgroundProgram::list();
      QStringList::Iterator it;
      for (it=lst.begin(); it != lst.end(); it++)
         addProgram(*it);
   }
   else
   {
      dlg->m_groupProgram->hide();
   }

   dlg->m_spinCache->setSteps(512, 1024);
   dlg->m_spinCache->setRange(0, 10240);
   dlg->m_spinCache->setSpecialValueText(i18n("Unlimited"));
   dlg->m_spinCache->setSuffix(i18n(" KB"));

   // Blend modes: make sure these match with kdesktop/bgrender.cc !!
   dlg->m_comboBlend->insertItem(i18n("No Blending"));
   dlg->m_comboBlend->insertItem(i18n("Horizontal Blending"));
   dlg->m_comboBlend->insertItem(i18n("Vertical Blending"));
   dlg->m_comboBlend->insertItem(i18n("Pyramid Blending"));
   dlg->m_comboBlend->insertItem(i18n("Pipecross Blending"));
   dlg->m_comboBlend->insertItem(i18n("Elliptic Blending"));
   dlg->m_comboBlend->insertItem(i18n("Intensity Blending"));
   dlg->m_comboBlend->insertItem(i18n("Saturate Blending"));
   dlg->m_comboBlend->insertItem(i18n("Contrast Blending"));
   dlg->m_comboBlend->insertItem(i18n("Hue Shift Blending"));

   connect( dlg->m_comboBlend, SIGNAL(activated(int)), SLOT(slotBlendMode(int)));
   // connect( dlg->m_comboBlend->listBox(),SIGNAL(highlighted ( int  )), SLOT(slotBlendMode(int)));

   connect( dlg->m_sliderBlend, SIGNAL(valueChanged(int)), SLOT(slotBlendBalance(int)));

   connect( dlg->m_cbBlendReverse, SIGNAL(toggled(bool)), SLOT(slotBlendReverse(bool)));

   connect( dlg->m_cbProgram, SIGNAL(toggled(bool)), 
            dlg->m_listPrograms, SLOT(setEnabled(bool)));
   connect( dlg->m_cbProgram, SIGNAL(toggled(bool)), 
            SLOT(slotProgramChanged()));

   dlg->m_monitorImage->setText(QString::null);
   dlg->m_monitorImage->setPixmap(locate("data", "kcontrol/pics/monitor.png"));
   dlg->m_monitorImage->setFixedSize(dlg->m_monitorImage->sizeHint());
   m_pMonitor = new QWidget(dlg->m_monitorImage, "preview monitor");
   m_pMonitor->setGeometry(23, 14, 151, 115);

   connect(r, SIGNAL(imageDone(int)), SLOT(slotPreviewDone()));

   QString wtstr;
   wtstr = i18n( "In this box you can enter how much memory KDE should use for caching the background(s)."
                 " If you have different backgrounds for the different desktops caching can make"
                 " switching desktops smoother at the expense of higher memory use.");
   QWhatsThis::add( dlg->m_lblCache, wtstr );
   QWhatsThis::add( dlg->m_spinCache, wtstr );

   wtstr = i18n("If you have selected to use wallpaper, you"
	       " can choose various methods of blending the background colors and patterns"
	       " with the wallpaper. The default option, \"No Blending\", means that the"
	       " wallpaper simply obscures the background below.");
   QWhatsThis::add( dlg->m_lblBlend, wtstr );
   QWhatsThis::add( dlg->m_comboBlend, wtstr );

   wtstr = i18n("You can use this slider to control"
	        " the degree of blending. You can experiment by moving the slider and"
		" looking at the effects in the preview image besides.");
   QWhatsThis::add( dlg->m_lblBlendBalance, wtstr );
   QWhatsThis::add( dlg->m_sliderBlend, wtstr );

   QWhatsThis::add( dlg->m_cbBlendReverse, 
           i18n("For some types of blending, you can reverse the role"
	        " of the background and the picture by checking this option.") );

   QWhatsThis::add( m_pMonitor, i18n("In this monitor, you can preview how your settings will look like on a \"real\" desktop.") );

   m_oldBackgroundMode = r->backgroundMode();
   if (m_oldBackgroundMode == KBackgroundSettings::Program)
      m_oldBackgroundMode = KBackgroundSettings::Flat;

   updateUI();
}

void BGAdvancedDialog::setCacheSize(int s)
{
   dlg->m_spinCache->setValue(s);
}

int BGAdvancedDialog::cacheSize()
{
   return dlg->m_spinCache->value();
}

void BGAdvancedDialog::updateUI()
{
   int mode = r->blendMode();

   dlg->m_comboBlend->blockSignals(true);
   dlg->m_sliderBlend->blockSignals(true);

   dlg->m_comboBlend->setCurrentItem(mode);
   bool b = !(mode == KBackgroundSettings::NoBlending);
   dlg->m_sliderBlend->setEnabled( b );
   dlg->m_lblBlendBalance->setEnabled( b );

   b = !(mode < KBackgroundSettings::IntensityBlending);
   dlg->m_cbBlendReverse->setEnabled(b);

   dlg->m_cbBlendReverse->setChecked(r->reverseBlending());

   dlg->m_sliderBlend->setValue( r->blendBalance() / 10 );   

   dlg->m_comboBlend->blockSignals(false);
   dlg->m_sliderBlend->blockSignals(false);

   QString prog = r->KBackgroundProgram::name();
   if ((r->backgroundMode() == KBackgroundSettings::Program)
       && !prog.isEmpty())
   {
      dlg->m_cbProgram->setChecked(true);
      dlg->m_listPrograms->setEnabled(true);
      selectProgram(prog);
   }
   else
   {
      dlg->m_cbProgram->setChecked(false);
      dlg->m_listPrograms->setEnabled(false);
   }
   slotPreviewDone();
}

void BGAdvancedDialog::slotBlendMode(int mode)
{
   if (mode == r->blendMode())
      return;

   bool b = !(mode == KBackgroundSettings::NoBlending);
   dlg->m_sliderBlend->setEnabled( b );
   dlg->m_lblBlendBalance->setEnabled( b );

   b = !(mode < KBackgroundSettings::IntensityBlending);
   dlg->m_cbBlendReverse->setEnabled(b);

   r->stop();
   r->setBlendMode(mode);
   r->start();   
}

void BGAdvancedDialog::slotBlendBalance(int value)
{
   value = value*10;
   if (value == r->blendBalance())
      return;

   r->stop();
   r->setBlendBalance(value);
   r->start();   
}

void BGAdvancedDialog::slotBlendReverse(bool b)
{
   if (b == r->reverseBlending())
      return;
   
   r->stop();
   r->setReverseBlending(b);
   r->start();
}

void BGAdvancedDialog::removeProgram(const QString &name)
{
   if (m_programItems.find(name)) 
   {
      delete m_programItems[name];
      m_programItems.remove(name);
   }
}

void BGAdvancedDialog::addProgram(const QString &name)
{
   removeProgram(name);
   
   KBackgroundProgram prog(name);
   if (prog.command().isEmpty() || (prog.isGlobal() && !prog.isAvailable()))
      return;

   QListViewItem *item = new QListViewItem(dlg->m_listPrograms);
   item->setText(0, prog.name());
   item->setText(1, prog.comment());
   item->setText(2, i18n("%1 min.").arg(prog.refresh()));

   m_programItems.insert(name, item);
}

void BGAdvancedDialog::selectProgram(const QString &name)
{
qWarning("BGAdvancedDialog::selectProgram(%s)", name.latin1());
   if (m_programItems.find(name)) 
   {
      QListViewItem *item = m_programItems[name];
      dlg->m_listPrograms->ensureItemVisible(item);
      dlg->m_listPrograms->setSelected(item, true);
      m_selectedProgram = name;
   }
}

void BGAdvancedDialog::slotAdd()
{
   KProgramEditDialog dlg;
   dlg.exec();
   if (dlg.result() == QDialog::Accepted) 
   {
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
   if (prog.isGlobal()) 
   {
      KMessageBox::sorry(this,
		i18n("Unable to remove the program! The program is global "
		"and can only be removed by the System Administrator."),
		i18n("Cannot Remove Program"));
      return;
   }
   if (KMessageBox::warningContinueCancel(this,
	    i18n("Are you sure you want to remove the program `%1'?")
	    .arg(prog.name()),
	    i18n("Remove Background Program"),
	    i18n("&Remove")) != KMessageBox::Continue)
      return;

   prog.remove();
   removeProgram(m_selectedProgram);
   m_selectedProgram = QString::null;
}

/*
 * Modify a program.
 */
void BGAdvancedDialog::slotModify()
{
   if (m_selectedProgram.isEmpty())
      return;

   KProgramEditDialog dlg(m_selectedProgram);
   dlg.exec();
   if (dlg.result() == QDialog::Accepted) 
   {
      QString program = dlg.program();
      if (program != m_selectedProgram) 
      {
         KBackgroundProgram prog(m_selectedProgram);
         prog.remove();
	 removeProgram(m_selectedProgram);
      }
      addProgram(dlg.program());
      selectProgram(dlg.program());
   }
}

void BGAdvancedDialog::slotProgramItemClicked(QListViewItem *item)
{
   if (item)
      m_selectedProgram = item->text(0);
   slotProgramChanged();
}

void BGAdvancedDialog::slotProgramItemDoubleClicked(QListViewItem *item)
{
   slotProgramItemClicked(item);
   slotModify();
}

void BGAdvancedDialog::slotProgramChanged()
{
   r->stop();

   r->setProgram(m_selectedProgram);
   if (dlg->m_cbProgram->isChecked() && !m_selectedProgram.isEmpty())
      r->setBackgroundMode(KBackgroundSettings::Program);
   else
      r->setBackgroundMode(m_oldBackgroundMode);
   
   r->start();   
}

void BGAdvancedDialog::slotPreviewDone()
{
   KPixmap pm;
   if (QPixmap::defaultDepth() < 15)
      pm.convertFromImage(*r->image(), KPixmap::LowColor);
   else
      pm.convertFromImage(*r->image());

   m_pMonitor->setBackgroundPixmap(pm);
}


/**** KProgramEditDialog ****/

KProgramEditDialog::KProgramEditDialog(QString program, QWidget *parent, char *name)
    : KDialogBase(parent, name, true, i18n("Configure Background Program"),
	Ok | Cancel, Ok, true)
{
    QFrame *frame = makeMainWidget();

    QGridLayout *grid = new QGridLayout(frame, 6, 2, 0, spacingHint());
    grid->addColSpacing(1, 300);

    QLabel *lbl = new QLabel(i18n("&Name:"), frame);
    grid->addWidget(lbl, 0, 0);
    m_NameEdit = new QLineEdit(frame);
    lbl->setBuddy(m_NameEdit);
    grid->addWidget(m_NameEdit, 0, 1);

    lbl = new QLabel(i18n("&Comment:"), frame);
    grid->addWidget(lbl, 1, 0);
    m_CommentEdit = new QLineEdit(frame);
    lbl->setBuddy(m_CommentEdit);
    grid->addWidget(m_CommentEdit, 1, 1);

    lbl = new QLabel(i18n("&Command:"), frame);
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
    m_RefreshEdit->setSteps(5, 10);
    m_RefreshEdit->setSuffix(i18n(" minutes"));
    m_RefreshEdit->setFixedSize(m_RefreshEdit->sizeHint());
    lbl->setBuddy(m_RefreshEdit);
    grid->addWidget(m_RefreshEdit, 5, 1, AlignLeft);

    m_Program = program;
    if (m_Program.isEmpty()) {
	KBackgroundProgram prog(i18n("New Command"));
	int i = 1;
	while (!prog.command().isEmpty())
	    prog.load(i18n("New Command <%1>").arg(i++));
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


QString KProgramEditDialog::program()
{
    return m_NameEdit->text();
}

void KProgramEditDialog::slotOk()
{
    QString s = m_NameEdit->text();
    if (s.isEmpty()) {
	KMessageBox::sorry(this, i18n("You did not fill in the `Name' field.\n"
		"This is a required field."));
	return;
    }

    KBackgroundProgram prog(s);
    if ((s != m_Program) && !prog.command().isEmpty()) {
	int ret = KMessageBox::warningYesNo(this,
	    i18n("There is already a program with the name `%1'.\n"
	    "Do you want to overwrite it?").arg(s));
	if (ret != KMessageBox::Yes)
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
    accept();
}


#include "bgadvanced.moc"
