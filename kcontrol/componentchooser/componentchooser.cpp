/***************************************************************************
                          componentchooser.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Joseph Wenninger
    email                : jowenn@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License         *
 *                                                                         *
 ***************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>

#include <kapplication.h>
#include <dcopclient.h>
#include "componentchooser.h"
#include "componentchooser.moc"
#include <klineedit.h>
#include <kemailsettings.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <qstringlist.h>
#include <klistbox.h>
#include <ktrader.h>
#include <kservice.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtoolbutton.h>
#include <qfile.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kopenwith.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qradiobutton.h>

class MyListBoxItem: public QListBoxText
{
public:
	MyListBoxItem(const QString& text, const QString &file):QListBoxText(text),File(file){}
	virtual ~MyListBoxItem(){;}
	QString File;
};


//BEGIN  General kpart based Component selection

CfgComponent::CfgComponent(QWidget *parent):ComponentConfig_UI(parent),CfgPlugin(){
	m_lookupDict.setAutoDelete(true);
	m_revLookupDict.setAutoDelete(true);
	connect(ComponentSelector,SIGNAL(activated(const QString&)),this,SLOT(slotComponentChanged(const QString&)));
}

CfgComponent::~CfgComponent(){}

void CfgComponent::slotComponentChanged(const QString&) {
	emit changed(true);
}

void CfgComponent::save(KConfig *cfg) {
  		QString ServiceTypeToConfigure=cfg->readEntry("ServiceTypeToConfigure","");
		KConfig *store = new KConfig(cfg->readEntry("storeInFile","null"));
		store->setGroup(cfg->readEntry("valueSection",""));
		store->writeEntry(cfg->readEntry("valueName","kcm_componenchooser_null"),*m_lookupDict[ComponentSelector->currentText()]);
		store->sync();
		delete store;
}

void CfgComponent::load(KConfig *cfg) {

	ComponentSelector->clear();
	m_lookupDict.clear();
	m_revLookupDict.clear();

	QString ServiceTypeToConfigure=cfg->readEntry("ServiceTypeToConfigure","");

	QString MimeTypeOfInterest=cfg->readEntry("MimeTypeOfInterest","");
	KTrader::OfferList offers = KTrader::self()->query(MimeTypeOfInterest, "'"+ServiceTypeToConfigure+"' in ServiceTypes");

	for (KTrader::OfferList::Iterator tit = offers.begin(); tit != offers.end(); ++tit)
        {
		ComponentSelector->insertItem((*tit)->name());
		m_lookupDict.insert((*tit)->name(),new QString((*tit)->desktopEntryName()));
		m_revLookupDict.insert((*tit)->desktopEntryName(),new QString((*tit)->name()));;
	}
        
	KConfig *store = new KConfig(cfg->readEntry("storeInFile","null"));
        store->setGroup(cfg->readEntry("valueSection",""));               
	QString setting=store->readEntry(cfg->readEntry("valueName","kcm_componenchooser_null"),"");
        delete store;
	if (setting.isEmpty()) setting=cfg->readEntry("defaultImplementation","");
	QString *tmp=m_revLookupDict[setting];
	if (tmp)
		for (int i=0;i<ComponentSelector->count();i++)
			if ((*tmp)==ComponentSelector->text(i))
			{
				ComponentSelector->setCurrentItem(i);
				break;
			}
	emit changed(false);
}

//END  General kpart based Component selection






//BEGIN Email client config
CfgEmailClient::CfgEmailClient(QWidget *parent):EmailClientConfig_UI(parent),CfgPlugin(){
	pSettings = new KEMailSettings();

	connect(kmailCB, SIGNAL(toggled(bool)), SLOT(configChanged()) );
	connect(txtEMailClient, SIGNAL(textChanged(const QString&)), SLOT(configChanged()) );
	connect(chkRunTerminal, SIGNAL(clicked()), SLOT(configChanged()) );
}

CfgEmailClient::~CfgEmailClient() {
	delete pSettings;
}


void CfgEmailClient::load(KConfig *)
{
	QString emailClient = pSettings->getSetting(KEMailSettings::ClientProgram);
	bool useKMail = (emailClient == QString::null);
    
	kmailCB->setChecked(useKMail);
	otherCB->setChecked(!useKMail);
	txtEMailClient->setText(emailClient);
	chkRunTerminal->setChecked((pSettings->getSetting(KEMailSettings::ClientTerminal) == "true"));

	emit changed(false);

}

void CfgEmailClient::configChanged()
{
	emit changed(true);
}

void CfgEmailClient::selectEmailClient()
{
	KURL::List urlList;
	KOpenWithDlg dlg(urlList, i18n("Select preferred email client:"), QString::null, this);
	if (dlg.exec() != QDialog::Accepted) return;
	QString client = dlg.text();

	bool b = client.left(11) == "konsole -e ";
	if (b) client = client.mid(11);
	if (!client.isEmpty())
	{
		chkRunTerminal->setChecked(b);
		txtEMailClient->setText(client);
	}
}


void CfgEmailClient::save(KConfig *)
{
	if (kmailCB->isChecked())
	{
		pSettings->setSetting(KEMailSettings::ClientProgram, QString::null);
		pSettings->setSetting(KEMailSettings::ClientTerminal, "false");
	}
	else
	{
		pSettings->setSetting(KEMailSettings::ClientProgram, txtEMailClient->text());
		pSettings->setSetting(KEMailSettings::ClientTerminal, (chkRunTerminal->isChecked()) ? "true" : "false");
	}

	// insure proper permissions -- contains sensitive data
	QString cfgName(KGlobal::dirs()->findResource("config", "emails"));
	if (!cfgName.isEmpty())
		::chmod(QFile::encodeName(cfgName), 0600);

	kapp->dcopClient()->emitDCOPSignal("KDE_emailSettingsChanged()", QByteArray());

	emit changed(false);
}


//END Email client config



//BEGIN Terminal Emulator Configuration

CfgTerminalEmulator::CfgTerminalEmulator(QWidget *parent):TerminalEmulatorConfig_UI(parent),CfgPlugin(){
	connect(terminalLE,SIGNAL(textChanged(const QString &)), this, SLOT(configChanged()));
	connect(terminalCB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
	connect(otherCB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
}

CfgTerminalEmulator::~CfgTerminalEmulator() {
}

void CfgTerminalEmulator::configChanged()
{
	emit changed(true);
}


void CfgTerminalEmulator::load(KConfig *) {
	KConfig *config = new KConfig("kdeglobals", true);
	config->setGroup("General");
	QString terminal = config->readEntry("TerminalApplication","konsole");
	if (terminal == "konsole")
	{
	   terminalLE->setText("xterm");
	   terminalCB->setChecked(true);
	}
	else
	{
	  terminalLE->setText(terminal);
	  otherCB->setChecked(true);
	}
	delete config;

	emit changed(false);
}

void CfgTerminalEmulator::save(KConfig *) {

	KConfig *config = new KConfig("kdeglobals");
	config->setGroup("General");
	config->writeEntry("TerminalApplication",terminalCB->isChecked()?"konsole":terminalLE->text(), true, true);
	config->sync();
	delete config;

	kapp->dcopClient()->send("klauncher", "klauncher","reparseConfiguration()", QString::null);

	emit changed(false);
}

void CfgTerminalEmulator::selectTerminalApp()
{
	KURL::List urlList;
	KOpenWithDlg dlg(urlList, i18n("Select preferred email client:"), QString::null, this);
	if (dlg.exec() != QDialog::Accepted) return;
	QString client = dlg.text();

	if (!client.isEmpty())
	{
		terminalLE->setText(client);
	}
}

//END Terminal Emulator Configuration









ComponentChooser::ComponentChooser(QWidget *parent, const char *name):
	ComponentChooser_UI(parent,name), configWidget(0) {

	somethingChanged=false;
	latestEditedService="";

        configContainer->setColumnLayout(0, Qt::Vertical );
   	myLayout = new QVBoxLayout( configContainer->layout() );
        myLayout->setAlignment( Qt::AlignTop );
	QStringList dummy;
	QStringList services=KGlobal::dirs()->findAllResources( "data","kcm_componentchooser/*.desktop",false,true,dummy);
	for (QStringList::Iterator it=services.begin();it!=services.end();++it)
	{
		KSimpleConfig *cfg=new KSimpleConfig((*it));
		ServiceChooser->insertItem(new MyListBoxItem(cfg->readEntry("Name",i18n("Unknown")),(*it)));
		delete cfg;

	}
	ServiceChooser->sort();
	connect(ServiceChooser,SIGNAL(executed(QListBoxItem*)),this,SLOT(slotServiceSelected(QListBoxItem*)));
	ServiceChooser->setSelected(0,true);
	slotServiceSelected(ServiceChooser->item(0));

}

void ComponentChooser::slotServiceSelected(QListBoxItem* it) {
	if (!it) return;
	if (somethingChanged) {
		if (KMessageBox::questionYesNo(this,i18n("<qt>You changed the default component of your choice, do want to save that change now ?<BR><BR>Selecting <B>No</B> will discard your changes</qt>"))==KMessageBox::Yes) save();
	}
	KSimpleConfig *cfg=new KSimpleConfig(static_cast<MyListBoxItem*>(it)->File);

	ComponentDescription->setText(cfg->readEntry("Comment",i18n("No description available")));

	QString cfgType=cfg->readEntry("configurationType");
	if (cfgType.isEmpty() || (cfgType=="component"))
	{
		if (!(configWidget && configWidget->qt_cast("CfgComponent")))
		{
			delete configWidget;
			CfgComponent* cfgcomp = new CfgComponent(configContainer);
			configWidget=cfgcomp;
                        cfgcomp->ChooserDocu->setText(i18n("Choose from the list below which component should be used by default for the %1 service.").arg(it->text()));
			configWidget->show();
			myLayout->addWidget(configWidget);
			connect(configWidget,SIGNAL(changed(bool)),this,SLOT(emitChanged(bool)));
		}
	}
	else if (cfgType=="internal_email")
	{
		if (!(configWidget && configWidget->qt_cast("CfgEmailClient")))
		{
			delete configWidget;
			configWidget=new CfgEmailClient(configContainer);
			configWidget->show();
			myLayout->addWidget(configWidget);
			connect(configWidget,SIGNAL(changed(bool)),this,SLOT(emitChanged(bool)));
		}

	}
	else if (cfgType=="internal_terminal")
	{
		if (!(configWidget && configWidget->qt_cast("CfgTerminalEmulator")))
		{
			delete configWidget;
			configWidget=new CfgTerminalEmulator(configContainer);
			configWidget->show();
			myLayout->addWidget(configWidget);
			connect(configWidget,SIGNAL(changed(bool)),this,SLOT(emitChanged(bool)));
		}

	}

	if (configWidget) static_cast<CfgPlugin*>(configWidget->qt_cast("CfgPlugin"))->load(cfg);
	emitChanged(false);
	delete cfg;
	latestEditedService=static_cast<MyListBoxItem*>(it)->File;
}


void ComponentChooser::emitChanged(bool val) {
	somethingChanged=val;
	emit changed(val);
}


ComponentChooser::~ComponentChooser()
{
	delete configWidget;
}

void ComponentChooser::load() {
}

void ComponentChooser::save() {
	KSimpleConfig *cfg=new KSimpleConfig(latestEditedService);
		if (configWidget) static_cast<CfgPlugin*>(configWidget->qt_cast("CfgPlugin"))->save(cfg);
	delete cfg;
}

void ComponentChooser::restoreDefault() {
/*
	txtEMailClient->setText("kmail");
	chkRunTerminal->setChecked(false);

	// Check if -e is needed, I do not think so
	terminalLE->setText("xterm");  //No need for i18n
	terminalCB->setChecked(true);
	emitChanged(false);
*/
}

