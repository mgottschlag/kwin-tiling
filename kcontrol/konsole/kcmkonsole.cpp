
#include "kcmkonsole.h"
#include "kcmkonsoledialog.h"

#include <kfontdialog.h>
#include <kdialog.h>
#include <qlayout.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include "schemaeditor.h"

#include <qcheckbox.h>
#include <qcombobox.h>

KCMKonsole::KCMKonsole(QWidget * parent, const char *name)
:KCModule(parent, name)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    dialog = new KCMKonsoleDialog(this);
    dialog->show();
    topLayout->add(dialog);
    load();

    connect(dialog->fontPB, SIGNAL(clicked()), this, SLOT(setupFont()));
}

void KCMKonsole::load()
{

    KConfig *config = new KConfig("konsolerc", false, true);
    config->setGroup("options");


    dialog->fullScreenCB->setChecked(config->readBoolEntry("Fullscreen",false));
    dialog->showMenuBarCB->setChecked(config->readEntry("MenuBar","Enabled") == "Enabled");
    dialog->warnCB->setChecked(config->readBoolEntry("WarnQuit",true));
    dialog->showFrameCB->setChecked(config->readBoolEntry("has frame",true));
    dialog->scrollBarCO->setCurrentItem(config->readNumEntry("scrollbar",1));
    dialog->fontCO->setCurrentItem(config->readNumEntry("font"));
    currentFont = config->readFontEntry("defaultfont");

    dialog->SchemaEditor1->setSchema(config->readEntry("schema"));

    config = new KConfig("kdeglobals", false, true);
    config->setGroup("General");
    dialog->terminalLE->setText(config->readEntry("TerminalApplication","konsole"));
    dialog->terminalCB->setChecked(config->readEntry("TerminalApplication","konsole")!="konsole");
    
}

void KCMKonsole::load(const QString & s)
{

}

void KCMKonsole::setupFont()
{

    // Example string, may be nice somthing like "[root@localhost]$ rm / -rf"
    QString example = i18n("[root@localhost]$ ");
    if (KFontDialog::getFontAndText(currentFont, example))
	dialog->fontCO->setCurrentItem(0);
}



void KCMKonsole::configChanged()
{
}

void KCMKonsole::save()
{

    KConfig *config = new KConfig("konsolerc", false, true);
    config->setGroup("options");

    config->writeEntry("Fullscreen", dialog->fullScreenCB->isChecked());
    config->writeEntry("MenuBar", dialog->showMenuBarCB->isChecked()? "Enabled" : "Disabled");
    config->writeEntry("WarnQuit", dialog->warnCB->isChecked());
    config->writeEntry("has frame", dialog->showFrameCB->isChecked());
    config->writeEntry("scrollbar", dialog->scrollBarCO->currentItem());
    config->writeEntry("font", dialog->fontCO->currentItem());

    config->writeEntry("defaultfont", currentFont);

    config->writeEntry("schema", dialog->SchemaEditor1->schema());

    config->sync();		//is it necessary ?

    config = new KConfig("kdeglobals", false, true);
    config->setGroup("General");
    config->writeEntry("TerminalApplication",dialog->terminalCB->isChecked()?dialog->terminalLE->text():"konsole");
    
    config->sync();		//is it necessary ?

}

void KCMKonsole::defaults()
{
    dialog->fullScreenCB->setChecked(false);
    dialog->showMenuBarCB->setChecked(true);
    dialog->warnCB->setChecked(true);
    dialog->showFrameCB->setChecked(true);
    dialog->scrollBarCO->setCurrentItem(true);
    dialog->terminalCB->setChecked(false);
    dialog->terminalLE->setText(i18n( "xterm -e " ) );
    configChanged();
}

QString KCMKonsole::quickHelp() const
{
    return i18n("<h1>konsole</h1> With this module you can configure konsole, the KDE terminal"
		" application. In this module you can configure the generic konsole options, "
		"that you can configure using the RMB too, and you can edit the konsole schema.");
}


const KAboutData * KCMKonsole::aboutData() const
{
 
 KAboutData *ab=new KAboutData( "kcmkonsole", I18N_NOOP("KCM Konsole"),
    "0.2",I18N_NOOP("KControl module for konsole configuration"), KAboutData::License_GPL,
    "(c) 2001, Andrea Rizzi", 0, 0, "rizzi@kde.org");
 
  ab->addAuthor("Andrea Rizzi",0, "rizzi@kde.org");
 return ab;
      
}

extern "C" {
    KCModule *create_konsole(QWidget * parent, const char *name) {
	KGlobal::locale()->insertCatalogue("kcmkonsole");
	return new KCMKonsole(parent, name);
    };
}



#include "kcmkonsole.moc"
