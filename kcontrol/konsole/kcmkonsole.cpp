/***************************************************************************
                          kcmkonsole.cpp - control module for konsole
                             -------------------
    begin                : mar apr 17 16:44:59 CEST 2001
    copyright            : (C) 2001 by Andrea Rizzi
    email                : rizzi@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kcmkonsole.h"

#include <qlayout.h>
#include <qstringlist.h>

#include <dcopclient.h>

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfontdialog.h>
#include <kgenericfactory.h>
#include "schemaeditor.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qtabwidget.h>

typedef KGenericFactory<KCMKonsole, QWidget> ModuleFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_konsole, ModuleFactory("kcmkonsole") );

KCMKonsole::KCMKonsole(QWidget * parent, const char *name, const QStringList&)
:KCModule(ModuleFactory::instance(), parent, name)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    dialog = new KCMKonsoleDialog(this);
    dialog->line_spacingSB->setRange(0, 8, 1, false);
    dialog->line_spacingSB->setSpecialValueText(i18n("normal line spacing", "Normal"));
    dialog->show();
    topLayout->add(dialog);
    load();

    connect(dialog->terminalSizeHintCB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
    connect(dialog->warnCB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
    connect(dialog->ctrldragCB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
    connect(dialog->cutToBeginningOfLineCB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
    connect(dialog->blinkingCB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
    connect(dialog->frameCB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
    connect(dialog->terminalLE,SIGNAL(textChanged(const QString &)),this,SLOT(configChanged()));
    connect(dialog->terminalCB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
    connect(dialog->startKwritedCB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
    connect(dialog->line_spacingSB,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(dialog->silence_secondsSB,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(dialog->word_connectorLE,SIGNAL(textChanged(const QString &)),this,SLOT(configChanged()));
    connect(dialog->SchemaEditor1, SIGNAL(changed()), this, SLOT(configChanged()));
}

void KCMKonsole::load()
{

    KConfig *config = new KConfig("konsolerc", true);
    config->setDesktopGroup();

    dialog->terminalSizeHintCB->setChecked(config->readBoolEntry("TerminalSizeHint",true));
    dialog->warnCB->setChecked(config->readBoolEntry("WarnQuit",true));
    dialog->ctrldragCB->setChecked(config->readBoolEntry("CtrlDrag",false));
    dialog->cutToBeginningOfLineCB->setChecked(config->readBoolEntry("CutToBeginningOfLine",false));
    dialog->blinkingCB->setChecked(config->readBoolEntry("BlinkingCursor",false));
    dialog->frameCB->setChecked(config->readBoolEntry("has frame",true));
    dialog->line_spacingSB->setValue(config->readUnsignedNumEntry( "LineSpacing", 0 ));
    dialog->silence_secondsSB->setValue(config->readUnsignedNumEntry( "SilenceSeconds", 10 ));
    dialog->word_connectorLE->setText(config->readEntry("wordseps",":@-./_~"));

    dialog->SchemaEditor1->setSchema(config->readEntry("schema"));

    delete config;

    config = new KConfig("kdeglobals", true);
    config->setGroup("General");
    QString terminal = config->readEntry("TerminalApplication","konsole");
    if (terminal == "konsole")
    {
       dialog->terminalLE->setText("xterm");
       dialog->terminalCB->setChecked(true);
    }
    else
    {
       dialog->terminalLE->setText(terminal);
       dialog->terminalCB->setChecked(false);
    }
    delete config;

    config = new KConfig("kwritedrc", true);
    config->setGroup("General");
    dialog->startKwritedCB->setChecked(config->readBoolEntry("Autostart",true));
    delete config;

    emit changed(false);
}

void KCMKonsole::load(const QString & /*s*/)
{

}

void KCMKonsole::configChanged()
{
    emit changed(true);
}

void KCMKonsole::save()
{
    if (dialog->SchemaEditor1->isModified())
    {
       dialog->TabWidget2->showPage(dialog->tab_2);
       dialog->SchemaEditor1->querySave();
    }

    KConfig *config = new KConfig("konsolerc");
    config->setDesktopGroup();

    config->writeEntry("TerminalSizeHint", dialog->terminalSizeHintCB->isChecked());
    config->writeEntry("WarnQuit", dialog->warnCB->isChecked());
    config->writeEntry("CtrlDrag", dialog->ctrldragCB->isChecked());
    config->writeEntry("CutToBeginningOfLine", dialog->cutToBeginningOfLineCB->isChecked());
    config->writeEntry("BlinkingCursor", dialog->blinkingCB->isChecked());
    config->writeEntry("has frame", dialog->frameCB->isChecked());
    config->writeEntry("LineSpacing" , dialog->line_spacingSB->value());
    config->writeEntry("SilenceSeconds" , dialog->silence_secondsSB->value());
    config->writeEntry("wordseps", dialog->word_connectorLE->text());

    config->writeEntry("schema", dialog->SchemaEditor1->schema());

    // that one into kdeglobals
    config->setGroup("General");
    config->writeEntry("TerminalApplication",dialog->terminalCB->isChecked()?"konsole":dialog->terminalLE->text(), true, true);

    delete config;

    config = new KConfig("kwritedrc");
    config->setGroup("General");
    config->writeEntry("Autostart", dialog->startKwritedCB->isChecked());
    delete config;

    emit changed(false);

    DCOPClient *dcc = kapp->dcopClient();
    dcc->send("konsole-*", "konsole", "reparseConfiguration()", QByteArray());
    dcc->send("kdesktop", "default", "configure()", QByteArray());
    dcc->send("klauncher", "klauncher", "reparseConfiguration()", QByteArray());
}

void KCMKonsole::defaults()
{
    dialog->terminalSizeHintCB->setChecked(true);
    dialog->warnCB->setChecked(true);
    dialog->ctrldragCB->setChecked(false);
    dialog->cutToBeginningOfLineCB->setChecked(false);
    dialog->blinkingCB->setChecked(false);
    dialog->frameCB->setChecked(true);
    dialog->terminalCB->setChecked(true);
    dialog->line_spacingSB->setValue(0);
    dialog->silence_secondsSB->setValue(10);

    dialog->word_connectorLE->setText(":@-./_~");

    // Check if -e is needed, I do not think so
    dialog->terminalLE->setText("xterm");  //No need for i18n
    dialog->startKwritedCB->setChecked(true);
    configChanged();

}

QString KCMKonsole::quickHelp() const
{
    return i18n("<h1>Konsole</h1> With this module you can configure Konsole, the KDE terminal"
		" application. You can configure the generic Konsole options (which can also be "
		"configured using the RMB) and you can edit the schemas and sessions "
		"available to Konsole.");
}


const KAboutData * KCMKonsole::aboutData() const
{

 KAboutData *ab=new KAboutData( "kcmkonsole", I18N_NOOP("KCM Konsole"),
    "0.2",I18N_NOOP("KControl module for konsole configuration"), KAboutData::License_GPL,
    "(c) 2001, Andrea Rizzi", 0, 0, "rizzi@kde.org");

  ab->addAuthor("Andrea Rizzi",0, "rizzi@kde.org");
 return ab;

}

#include "kcmkonsole.moc"
