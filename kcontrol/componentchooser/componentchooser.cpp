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
#include <qlistbox.h>
#include <ktrader.h>
#include <kservice.h>
#include <qcombobox.h>
#include <kmessagebox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtoolbutton.h>
#include <qfile.h>
#include <kurl.h>
#include <kopenwith.h>

class MyListBoxItem: public QListBoxText
{
public:
	MyListBoxItem(const QString& text, const QString &file):QListBoxText(text),File(file){}
	virtual ~MyListBoxItem(){;}
	QString File;
};

ComponentChooser::ComponentChooser(QWidget *parent, const char *name):
	ComponentChooser_UI(parent,name) {

	somethingChanged=false;
	m_lookupDict.setAutoDelete(true);
	m_revLookupDict.setAutoDelete(true);
	latestEditedService="";

	QStringList dummy;
	QStringList services=KGlobal::dirs()->findAllResources( "data","kcm_componentchooser/*.desktop",false,true,dummy);
	for (QStringList::Iterator it=services.begin();it!=services.end();++it)
	{
		KSimpleConfig *cfg=new KSimpleConfig((*it));
		ServiceChooser->insertItem(new MyListBoxItem(cfg->readEntry("Name",i18n("Unknown")),(*it)));
		delete cfg;

	}
	connect(ServiceChooser,SIGNAL(selected(QListBoxItem*)),this,SLOT(slotServiceSelected(QListBoxItem*)));
	ServiceChooser->setSelected(0,true);
	slotServiceSelected(ServiceChooser->item(0));
	connect(ComponentSelector,SIGNAL(activated(const QString&)),this,SLOT(slotComponentChanged(const QString&)));

	pSettings = new KEMailSettings();
	load();

	connect(txtEMailClient, SIGNAL(textChanged(const QString&)), SLOT(configChanged()) );
	connect(chkRunTerminal, SIGNAL(clicked()), SLOT(configChanged()) );
	connect(btnSelectEmail, SIGNAL(clicked()), SLOT(selectEmailClient()) );

	connect(terminalLE,SIGNAL(textChanged(const QString &)),this,SLOT(configChanged()));
	connect(terminalCB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
}

void ComponentChooser::slotServiceSelected(QListBoxItem* it) {
	if (!it) return;
	if (somethingChanged) {
		if (KMessageBox::questionYesNo(this,"<qt>You changed the default component of your choice, do want to save that change now ?<BR><BR>Selecting <B>No</B> will discard your changes</qt>")==KMessageBox::Yes) save();
	}
	ComponentSelector->clear();
	m_lookupDict.clear();
	m_revLookupDict.clear();
	KSimpleConfig *cfg=new KSimpleConfig(static_cast<MyListBoxItem*>(it)->File);
	QString ServiceTypeToConfigure=cfg->readEntry("ServiceTypeToConfigure","");
	ComponentDescription->setText(cfg->readEntry("Comment",i18n("No description available")));
	if (ServiceTypeToConfigure.isEmpty()) {
		// This will be handled by a plugin later
	}
	else
	{
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
	}
	latestEditedService=static_cast<MyListBoxItem*>(it)->File;
	emitChanged(false);
	delete cfg;
}


void ComponentChooser::emitChanged(bool val) {
	somethingChanged=val;
	emit changed(val);
}

void ComponentChooser::slotComponentChanged(const QString& str) {
	emitChanged(true);
}

ComponentChooser::~ComponentChooser()
{
	delete pSettings;
}

void ComponentChooser::load() {
	txtEMailClient->setText(pSettings->getSetting(KEMailSettings::ClientProgram));
	chkRunTerminal->setChecked((pSettings->getSetting(KEMailSettings::ClientTerminal) == "true"));

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
	  terminalCB->setChecked(false);
	}
	delete config;
}

void ComponentChooser::save() {
	KSimpleConfig *cfg=new KSimpleConfig(latestEditedService);

	QString ServiceTypeToConfigure=cfg->readEntry("ServiceTypeToConfigure","");
	if (ServiceTypeToConfigure.isEmpty())
	{
		//plugin
	}
	else
	{
		KConfig *store = new KConfig(cfg->readEntry("storeInFile","null"));
		store->setGroup(cfg->readEntry("valueSection",""));
		store->writeEntry(cfg->readEntry("valueName","kcm_componenchooser_null"),*m_lookupDict[ComponentSelector->currentText()]);
		store->sync();
		delete store;
	}
	delete cfg;

	pSettings->setSetting(KEMailSettings::ClientProgram, txtEMailClient->text());
	pSettings->setSetting(KEMailSettings::ClientTerminal, (chkRunTerminal->isChecked()) ? "true" : "false");

	// insure proper permissions -- contains sensitive data
	QString cfgName(KGlobal::dirs()->findResource("config", "emails"));
	if (!cfgName.isEmpty())
		::chmod(QFile::encodeName(cfgName), 0600);

	kapp->dcopClient()->emitDCOPSignal("KDE_emailSettingsChanged()", QByteArray());


	KConfig *config = new KConfig("kdeglobals");
	config->setGroup("General");
	config->writeEntry("TerminalApplication",terminalCB->isChecked()?"konsole":terminalLE->text(), true, true);
	delete config;

	emitChanged(false);
}

void ComponentChooser::restoreDefault() {
	txtEMailClient->setText("kmail");
	chkRunTerminal->setChecked(false);

	// Check if -e is needed, I do not think so
	terminalLE->setText("xterm");  //No need for i18n
	terminalCB->setChecked(true);
	emitChanged(false);
}

void ComponentChooser::configChanged()
{
	emitChanged(true);
}

void ComponentChooser::selectEmailClient()
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
