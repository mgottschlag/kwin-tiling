
#include "kcmioslaveinfo.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qhbox.h>

#include <kprotocolinfo.h>
#include <klocale.h>
#include <kglobal.h>


KCMIOSlaveInfo::KCMIOSlaveInfo(QWidget *parent, const char * name)
:KCModule(parent,name)
,m_ioslavesLb(0)
{
   setButtons(Help);
   QVBoxLayout *layout=new QVBoxLayout(this,10,15);

   QLabel* label=new QLabel(i18n("Available IOSlaves"),this);

   QHBox *hbox=new QHBox(this);
   m_ioslavesLb=new QListBox(hbox);
   //TODO make something useful after 2.1 is released
   //m_info=new QTextView(hbox);
/*   QWidget *dummy=new QWidget(hbox);
   hbox->setStretchFactor(dummy,1);
   hbox->setStretchFactor(m_ioslavesLb,1);*/
   hbox->setSpacing(15);

   layout->addWidget(label);
   layout->addWidget(hbox);

   QStringList protocols=KProtocolInfo::protocols();
   for (QStringList::Iterator it=protocols.begin(); it!=protocols.end(); it++)
   {
      m_ioslavesLb->insertItem(*it);
   };
   m_ioslavesLb->sort();
   //connect(m_ioslavesLb,SIGNAL(highlighted( const QString&)),this,SLOT(showInfo(const QString&)));
   //connect(m_ioslavesLb,SIGNAL(highlighted( QListBoxItem *item )),this,SLOT(showInfo(QListBoxItem *item)));
   //showInfo(m_ioslavesLb->text(0));
   //showInfo(m_ioslavesLb->firstItem());
};

QString KCMIOSlaveInfo::quickHelp() const
{
   return i18n("Gives you an overview over the installed ioslaves.");
}

void KCMIOSlaveInfo::showInfo(const QString& protocol)
{
   //m_info->setText(QString("Some info about protocol %1:/ ...").arg(protocol));
};

/*void KCMIOSlaveInfo::showInfo(QListBoxItem *item)
{
   if (item==0)
      return;
   m_info->setText(QString("Some info about protocol %1 :/ ...").arg(item->text()));
};*/


extern "C"
{

  KCModule *create_ioslaveinfo(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmioslaveinfo");
    return new KCMIOSlaveInfo(parent, name);
  }
}

#include "kcmioslaveinfo.moc"
