#include "componentchooser.h"
#include "componentchooser.moc"
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

ComponentChooser::~ComponentChooser() { ; }

void ComponentChooser::load() {
;
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
}

void ComponentChooser::restoreDefault() {
;
}
