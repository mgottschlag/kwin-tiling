/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 */

#include <qwidget.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qstring.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcmodule.h>
#include <kpassdlg.h>

#include <kdesu/defaults.h>
#include <kdesu/client.h>

#include "passwords.h"

/**
 * DLL interface.
 */
extern "C" {
    KCModule *create_passwords(QWidget *parent, const char *name) {
	KGlobal::locale()->insertCatalogue("passwords");
	return new KPasswordConfig(parent, name);
    }
}


/**** KPasswordConfig ****/

KPasswordConfig::KPasswordConfig(QWidget *parent, const char *name)
    : KCModule(parent, name)
{
    QVBoxLayout *top = new QVBoxLayout(this, 10, 10);

    // Echo mode
    m_EMGroup = new QButtonGroup(i18n("Echo characters as"), this);
    QWhatsThis::add( m_EMGroup,  i18n("Here you can configure the visual feedback given"
      " when you have to enter a password in kdesu. It does not affect other programs.<p>"
      " You can chose between: <ul><li><em>1 star:</em> for each character entered, an asterisk (*)"
      " appears.</li><li><em>3 stars:</em> for each character entered, three asterisks appear.</li>"
      " <li><em>no echo:</em> you don't get any visual feed back (so people watching can't even"
      " tell how many characters your password has).</li></ul>"));
    top->addWidget(m_EMGroup);
    QVBoxLayout *vbox = new QVBoxLayout(m_EMGroup, 10, 10);
    vbox->addSpacing(10);
    QRadioButton *rb = new QRadioButton(i18n("1 star"), m_EMGroup);
    vbox->addWidget(rb, 0, AlignLeft);
    rb = new QRadioButton(i18n("3 stars"), m_EMGroup);
    vbox->addWidget(rb, 0, AlignLeft);
    rb = new QRadioButton(i18n("no echo"), m_EMGroup);
    vbox->addWidget(rb, 0, AlignLeft);
    connect(m_EMGroup, SIGNAL(clicked(int)), SLOT(slotEchoMode(int)));

    // Keep password

    m_KeepBut = new QCheckBox(i18n("&Remember passwords"), this);
    QWhatsThis::add( m_KeepBut, i18n("If this option is selected, kdesu will remember your passwords"
       " for a given time. This way you don't have to enter your password again everytime you do"
       " something that requires a password. Keep in mind that this option is insecure and may enable"
       " others to do harm to your information and your system.<p>"
       " Please <em>do not</em> use this option if you are working in an insecure environment (e.g. in an open-plan office).<p>"
       " This option does not affect passwords explicitely set in applications, e.g. your mail password"
       " in KMail.") );
    connect(m_KeepBut, SIGNAL(toggled(bool)), SLOT(slotKeep(bool)));
    top->addWidget(m_KeepBut);
    QHBoxLayout *hbox = new QHBoxLayout();
    top->addLayout(hbox);
    QLabel *lbl = new QLabel(i18n("&Timeout"), this);
    lbl->setFixedSize(lbl->sizeHint());
    hbox->addSpacing(20);
    hbox->addWidget(lbl);
    m_TimeoutEdit = new QSpinBox(this);
    QString wtstr = i18n("Here you can specify for how long kdesu will remember your"
       " passwords. A short timeout is more secure than a long timeout.");
    QWhatsThis::add( lbl, wtstr );
    QWhatsThis::add( m_TimeoutEdit, wtstr );
    lbl->setBuddy(m_TimeoutEdit);
    m_TimeoutEdit->setRange(5, 1200);
    m_TimeoutEdit->setSteps(5, 10);
    m_TimeoutEdit->setSuffix(i18n(" minutes"));
    m_TimeoutEdit->setFixedSize(m_TimeoutEdit->sizeHint());
    hbox->addWidget(m_TimeoutEdit);
    hbox->addStretch();

    top->addStretch();

    setButtons(buttons());
    m_pConfig = KGlobal::config();
    load();
}


KPasswordConfig::~KPasswordConfig()
{
}


void KPasswordConfig::load()
{
    KConfigGroupSaver saver(m_pConfig, "Passwords");

    QString val = m_pConfig->readEntry("EchoMode", "x");
    if (val == "OneStar")
	m_Echo = KPasswordEdit::OneStar;
    else if (val == "ThreeStars")
	m_Echo = KPasswordEdit::ThreeStars;
    else if (val == "NoEcho")
	m_Echo = KPasswordEdit::NoEcho;
    else
	m_Echo = defEchoMode;

    m_bKeep = m_pConfig->readBoolEntry("Keep", defKeep);
    m_Timeout = m_pConfig->readNumEntry("Timeout", defTimeout);

    apply();
    emit changed(false);
}


void KPasswordConfig::save()
{
    KConfigGroupSaver saver(m_pConfig, "Passwords");

    QString val;
    if (m_Echo == KPasswordEdit::OneStar)
	val = "OneStar";
    else if (m_Echo == KPasswordEdit::ThreeStars)
	val = "ThreeStars";
    else 
	val = "NoEcho";
    m_pConfig->writeEntry("EchoMode", val, true, true);

    m_pConfig->writeEntry("Keep", m_bKeep, true, true);
    m_Timeout = m_TimeoutEdit->value()*60;
    m_pConfig->writeEntry("Timeout", m_Timeout, true, true);

    m_pConfig->sync();

    if (!m_bKeep) {
	// Try to stop daemon
	KDEsuClient client;
	if (client.ping() != -1)
	    client.stopServer();
    }
    emit changed(false);
}


void KPasswordConfig::defaults()
{
    m_Echo = defEchoMode;
    m_bKeep = defKeep;
    m_Timeout = defTimeout;

    apply();
    emit changed(true);
}


void KPasswordConfig::apply()
{
    m_EMGroup->setButton(m_Echo);
    m_KeepBut->setChecked(m_bKeep);

    m_TimeoutEdit->setValue(m_Timeout/60);
    m_TimeoutEdit->setEnabled(m_bKeep);
}
    

void KPasswordConfig::slotEchoMode(int i)
{
    m_Echo = i;
    emit changed(true);
}


void KPasswordConfig::slotKeep(bool keep)
{
    m_bKeep = keep;
    m_TimeoutEdit->setEnabled(m_bKeep);
    emit changed(true);
}


int KPasswordConfig::buttons()
{
    return KCModule::Help | KCModule::Default | KCModule::Reset |
	   KCModule::Cancel | KCModule::Ok;
}

QString KPasswordConfig::quickHelp()
{
    return i18n("<h1>Passwords</h1> For some actions, like changing the date/time"
       " of your system clock or creating users on your system, you need special"
       " privileges. In these cases a KDE program called 'kdesu' will ask you for"
       " a password. Here you can configure the behavior of kdesu, i.e. what visual"
       " feedback is given when you enter a password and whether kdesu should remember"
       " your passwords for a certain time.<p>"
       " These settings affect kdesu <em>only</em>. This means that e.g. the behavior of"
       " KMail and other programs asking you for passwords can not be configured here.");
}

