/***************************************************************************
                          componentchooser.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Joseph Wenninger
    email                : jowenn@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License verstion 2 as    *
 *   published by the Free Software Foundation                             *
 *                                                                         *
 ***************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>

#include "componentchooser.h"
#include "componentchooser.moc"

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>

#include <QtDBus/QtDBus>

#include <kapplication.h>
#include <kemailsettings.h>
#include <kipc.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kopenwith.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kmimetypetrader.h>
#include <kurlrequester.h>
#include <ktoolinvocation.h>
#include <klauncher_iface.h>
#include <QtDBus/QtDBus>


class MyListBoxItem: public Q3ListBoxText
{
public:
	MyListBoxItem(const QString& text, const QString &file):Q3ListBoxText(text),File(file){}
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
		// yes, this can happen if there are NO KTrader offers for this component
		if (!m_lookupDict[ComponentSelector->currentText()])
			return;

		QString ServiceTypeToConfigure=cfg->readEntry("ServiceTypeToConfigure");
		KConfig *store = new KConfig(cfg->readPathEntry("storeInFile","null"));
		store->setGroup(cfg->readEntry("valueSection"));
		store->writePathEntry(cfg->readEntry("valueName","kcm_componenchooser_null"),*m_lookupDict[ComponentSelector->currentText()]);
		store->sync();
		delete store;
		emit changed(false);
}

void CfgComponent::load(KConfig *cfg) {

	ComponentSelector->clear();
	m_lookupDict.clear();
	m_revLookupDict.clear();

	QString ServiceTypeToConfigure=cfg->readEntry("ServiceTypeToConfigure");

	QString MimeTypeOfInterest=cfg->readEntry("MimeTypeOfInterest");
	KService::List offers = KMimeTypeTrader::self()->query(MimeTypeOfInterest, '\''+ServiceTypeToConfigure+"' in ServiceTypes");

	for (KService::List::Iterator tit = offers.begin(); tit != offers.end(); ++tit)
        {
		ComponentSelector->addItem((*tit)->name());
		m_lookupDict.insert((*tit)->name(),new QString((*tit)->desktopEntryName()));
		m_revLookupDict.insert((*tit)->desktopEntryName(),new QString((*tit)->name()));
	}

	KConfig *store = new KConfig(cfg->readPathEntry("storeInFile","null"));
        store->setGroup(cfg->readEntry("valueSection"));
	QString setting=store->readEntry(cfg->readEntry("valueName","kcm_componenchooser_null"), QString());
        delete store;
	if (setting.isEmpty()) setting=cfg->readEntry("defaultImplementation", QString());
	QString *tmp=m_revLookupDict[setting];
	if (tmp)
		for (int i=0;i<ComponentSelector->count();i++)
			if ((*tmp)==ComponentSelector->itemText(i))
			{
				ComponentSelector->setCurrentIndex(i);
				break;
			}
	emit changed(false);
}

void CfgComponent::defaults()
{
    //todo
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

void CfgEmailClient::defaults()
{
    load(0L);
}

void CfgEmailClient::load(KConfig *)
{
	QString emailClient = pSettings->getSetting(KEMailSettings::ClientProgram);
	bool useKMail = (emailClient.isEmpty());

	kmailCB->setChecked(useKMail);
	otherCB->setChecked(!useKMail);
	txtEMailClient->setText(emailClient);
	txtEMailClient->setFixedHeight(txtEMailClient->sizeHint().height());
	chkRunTerminal->setChecked((pSettings->getSetting(KEMailSettings::ClientTerminal) == "true"));

	emit changed(false);

}

void CfgEmailClient::configChanged()
{
	emit changed(true);
}

void CfgEmailClient::selectEmailClient()
{
	KUrl::List urlList;
	KOpenWithDlg dlg(urlList, i18n("Select preferred email client:"), QString(), this);
	// hide "Do not &close when command exits" here, we don't need it for a mail client
	dlg.hideNoCloseOnExit();
	if (dlg.exec() != QDialog::Accepted) return;
	QString client = dlg.text();

	// get the preferred Terminal Application
	KConfigGroup confGroup( KGlobal::config(), QLatin1String("General") );
	QString preferredTerminal = confGroup.readPathEntry("TerminalApplication", QLatin1String("konsole"));
	preferredTerminal += QLatin1String(" -e ");

	int len = preferredTerminal.length();
	bool b = client.left(len) == preferredTerminal;
	if (b) client = client.mid(len);
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
		pSettings->setSetting(KEMailSettings::ClientProgram, QString());
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
	QDBusMessage message = QDBusMessage::createSignal("/Component", "org.kde.Kcontrol", "KDE_emailSettingsChanged" );
	QDBusConnection::sessionBus().send(message);
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

void CfgTerminalEmulator::defaults()
{
    load(0L);
}


void CfgTerminalEmulator::load(KConfig *) {
	KConfig *config = new KConfig("kdeglobals", true);
	config->setGroup("General");
	QString terminal = config->readPathEntry("TerminalApplication","konsole");
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
	config->writePathEntry("TerminalApplication", terminalCB->isChecked()?"konsole":terminalLE->text(), KConfigBase::Normal|KConfigBase::Global);
	config->sync();
	delete config;

	KIPC::sendMessageAll(KIPC::SettingsChanged);
        KToolInvocation::klauncher()->reparseConfiguration();

	emit changed(false);
}

void CfgTerminalEmulator::selectTerminalApp()
{
	KUrl::List urlList;
	KOpenWithDlg dlg(urlList, i18n("Select preferred terminal application:"), QString(), this);
	// hide "Run in &terminal" here, we don't need it for a Terminal Application
	dlg.hideRunInTerminal();
	if (dlg.exec() != QDialog::Accepted) return;
	QString client = dlg.text();

	if (!client.isEmpty())
	{
		terminalLE->setText(client);
	}
}

//END Terminal Emulator Configuration

//BEGIN Browser Configuration

CfgBrowser::CfgBrowser(QWidget *parent) : BrowserConfig_UI(parent),CfgPlugin(){
        connect(lineExec,SIGNAL(textChanged(const QString &)),this,SLOT(configChanged()));
	connect(radioKIO,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
	connect(radioExec,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
}

CfgBrowser::~CfgBrowser() {
}

void CfgBrowser::configChanged()
{
	emit changed(true);
}

void CfgBrowser::defaults()
{
	load(0L);
}


void CfgBrowser::load(KConfig *) {
	KConfig *config = new KConfig("kdeglobals", true);
	config->setGroup("General");
	QString exec = config->readEntry("BrowserApplication");
	if (exec.isEmpty())
	{
	   radioKIO->setChecked(true);
	   m_browserExec = exec;
	   m_browserService = 0;
	}
	else
	{
	   radioExec->setChecked(true);
	   if (exec.startsWith("!"))
	   {
	      m_browserExec = exec.mid(1);
	      m_browserService = 0;
	   }
	   else
	   {
	      m_browserService = KService::serviceByStorageId( exec );
	      if (m_browserService)
  	         m_browserExec = m_browserService->desktopEntryName();
  	      else
  	         m_browserExec.clear();
	   }
	}

	lineExec->setText(m_browserExec);
	delete config;

	emit changed(false);
}

void CfgBrowser::save(KConfig *) {

	KConfig *config = new KConfig("kdeglobals");
	config->setGroup("General");
	QString exec;
	if (radioExec->isChecked())
	{
	   exec = lineExec->text();
	   if (m_browserService && (exec == m_browserExec))
	      exec = m_browserService->storageId(); // Use service
	   else
	      exec = '!' + exec; // Litteral command
	}
	config->writePathEntry("BrowserApplication", exec, KConfigBase::Normal|KConfigBase::Global);
	config->sync();
	delete config;

	KIPC::sendMessageAll(KIPC::SettingsChanged);

	emit changed(false);
}

void CfgBrowser::selectBrowser()
{
	KUrl::List urlList;
	KOpenWithDlg dlg(urlList, i18n("Select preferred Web browser application:"), QString(), this);
	if (dlg.exec() != QDialog::Accepted) return;
	m_browserService = dlg.service();
	if (m_browserService)
	   m_browserExec = m_browserService->desktopEntryName();
	else
	   m_browserExec = dlg.text();

	lineExec->setText(m_browserExec);
}

//END Terminal Emulator Configuration

ComponentChooser::ComponentChooser(QWidget *parent, const char *name):
	ComponentChooser_UI(parent,name), configWidget(0) {

	static_cast<QGridLayout*>(layout())->setRowStretch(1, 1);
	somethingChanged=false;
	latestEditedService="";

	QStringList dummy;
	QStringList services=KGlobal::dirs()->findAllResources( "data","kcm_componentchooser/*.desktop",false,true,dummy);
	for (QStringList::Iterator it=services.begin();it!=services.end();++it)
	{
		KSimpleConfig cfg(*it);
		ServiceChooser->insertItem(new MyListBoxItem(cfg.readEntry("Name",i18n("Unknown")),(*it)));

	}
	ServiceChooser->setFixedWidth(ServiceChooser->sizeHint().width());
	ServiceChooser->sort();
	connect(ServiceChooser,SIGNAL(highlighted(Q3ListBoxItem*)),this,SLOT(slotServiceSelected(Q3ListBoxItem*)));
	ServiceChooser->setSelected(0,true);
	slotServiceSelected(ServiceChooser->item(0));

}

void ComponentChooser::slotServiceSelected(Q3ListBoxItem* it) {
	if (!it) return;

	if (somethingChanged) {
		if (KMessageBox::questionYesNo(this,i18n("<qt>You changed the default component of your choice, do want to save that change now ?</qt>"),QString(),KStdGuiItem::save(),KStdGuiItem::discard())==KMessageBox::Yes) save();
	}
	KSimpleConfig cfg(static_cast<MyListBoxItem*>(it)->File);

	ComponentDescription->setText(cfg.readEntry("Comment",i18n("No description available")));
	ComponentDescription->setMinimumSize(ComponentDescription->sizeHint());


	QString cfgType=cfg.readEntry("configurationType");
	QWidget *newConfigWidget = 0;
	if (cfgType.isEmpty() || (cfgType=="component"))
	{
		if (!(configWidget && qobject_cast<CfgComponent*>(configWidget)))
		{
			CfgComponent* cfgcomp = new CfgComponent(configContainer);
                        cfgcomp->ChooserDocu->setText(i18n("Choose from the list below which component should be used by default for the %1 service.", it->text()));
			newConfigWidget = cfgcomp;
		}
                else
                {
                        static_cast<CfgComponent*>(configWidget)->ChooserDocu->setText(i18n("Choose from the list below which component should be used by default for the %1 service.", it->text()));
                }
	}
	else if (cfgType=="internal_email")
	{
		if (!(configWidget && qobject_cast<CfgEmailClient*>(configWidget)))
		{
			newConfigWidget = new CfgEmailClient(configContainer);
		}

	}
	else if (cfgType=="internal_terminal")
	{
		if (!(configWidget && qobject_cast<CfgTerminalEmulator*>(configWidget)))
		{
			newConfigWidget = new CfgTerminalEmulator(configContainer);
		}

	}
	else if (cfgType=="internal_browser")
	{
		if (!(configWidget && qobject_cast<CfgBrowser*>(configWidget)))
		{
			newConfigWidget = new CfgBrowser(configContainer);
		}

	}

	if (newConfigWidget)
	{
		configContainer->addWidget(newConfigWidget);
		configContainer->raiseWidget(newConfigWidget);
		configContainer->removeWidget(configWidget);
		delete configWidget;
		configWidget=newConfigWidget;
		connect(configWidget,SIGNAL(changed(bool)),this,SLOT(emitChanged(bool)));
	        configContainer->setMinimumSize(configWidget->sizeHint());
	}

	if (configWidget)
		dynamic_cast<CfgPlugin*>(configWidget)->load(&cfg);

	emitChanged(false);
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
	if( configWidget )
	{
		CfgPlugin * plugin = dynamic_cast<CfgPlugin*>( configWidget );
		if( plugin )
		{
			KSimpleConfig cfg(latestEditedService);
			plugin->load( &cfg );
		}
	}
}

void ComponentChooser::save() {
	if( configWidget )
	{
		CfgPlugin* plugin = dynamic_cast<CfgPlugin*>( configWidget );
		if( plugin )
		{
			KSimpleConfig cfg(latestEditedService);
			plugin->save( &cfg );
		}
	}
}

void ComponentChooser::restoreDefault() {
    if (configWidget)
    {
        dynamic_cast<CfgPlugin*>(configWidget)->defaults();
        emitChanged(true);
    }

/*
	txtEMailClient->setText("kmail");
	chkRunTerminal->setChecked(false);

	// Check if -e is needed, I do not think so
	terminalLE->setText("xterm");  //No need for i18n
	terminalCB->setChecked(true);
	emitChanged(false);
*/
}

// vim: sw=4 ts=4 noet
