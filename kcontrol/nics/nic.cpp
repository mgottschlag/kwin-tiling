/*
 * nic.cpp
 *
 *  Copyright (C) 2001 Alexander Neundorf <neundorf@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "config.h"

#include "nic.h"

#include <klocale.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include <qptrlist.h>
#include <qstring.h>
#include <qlayout.h>

#include <qtimer.h>

#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>

typedef KGenericFactory<KCMNic, QWidget > KCMNicFactory;
K_EXPORT_COMPONENT_FACTORY (libkcm_nic, KCMNicFactory("kcmnic") );

struct MyNIC
{
   QString name;
   QString addr;
   QString netmask;
   QString state;
   QString type;
};

typedef QPtrList<MyNIC> NICList;

NICList* findNICs();

KCMNic::KCMNic(QWidget *parent, const char * name, const QStringList &)
   :KCModule(parent,name)
{
   QVBoxLayout *box=new QVBoxLayout(this);
   box->setMargin(KDialog::marginHint());
   box->setSpacing(KDialog::spacingHint());
   m_list=new QListView(this);
   box->addWidget(m_list);
   m_list->addColumn(i18n("Name"));
   m_list->addColumn(i18n("IP address"));
   m_list->addColumn(i18n("Network mask"));
   m_list->addColumn(i18n("Type"));
   m_list->addColumn(i18n("State"));
   QHBoxLayout *hbox=new QHBoxLayout(this);
   m_updateButton=new QPushButton(i18n("&Update"),this);
   hbox->addWidget(m_updateButton);
   hbox->addStretch(1);
   box->addLayout(hbox);
   QTimer* timer=new QTimer(this);
   timer->start(60000);
   connect(m_updateButton,SIGNAL(clicked()),this,SLOT(update()));
   connect(timer,SIGNAL(timeout()),this,SLOT(update()));
   update();
};

void KCMNic::update()
{
   m_list->clear();
   NICList *nics=findNICs();
   nics->setAutoDelete(true);
   for (MyNIC* tmp=nics->first(); tmp!=0; tmp=nics->next())
      new QListViewItem(m_list,tmp->name, tmp->addr, tmp->netmask, tmp->type, tmp->state);
   delete nics;
};


NICList* findNICs()
{
   NICList* nl=new NICList;
   nl->setAutoDelete(true);

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

   char buf[8*1024];
   struct ifconf ifc;
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_req = (struct ifreq *) buf;
	int result=ioctl(sockfd, SIOCGIFCONF, &ifc);

   for (char* ptr = buf; ptr < buf + ifc.ifc_len; )
   {
      struct ifreq *ifr =(struct ifreq *) ptr;
      int len = sizeof(struct sockaddr);
#ifdef	HAVE_SOCKADDR_SA_LEN
      if (ifr->ifr_addr.sa_len > len)
         len = ifr->ifr_addr.sa_len;		/* length > 16 */
#endif
      ptr += sizeof(ifr->ifr_name) + len;	/* for next one in buffer */

      int flags;
      struct sockaddr_in *sinptr;
      MyNIC *tmp=0;
      switch (ifr->ifr_addr.sa_family)
      {
      case AF_INET:
         sinptr = (struct sockaddr_in *) &ifr->ifr_addr;
         flags=0;

         struct ifreq ifcopy;
         ifcopy=*ifr;
         result=ioctl(sockfd,SIOCGIFFLAGS,&ifcopy);
         flags=ifcopy.ifr_flags;

         tmp=new MyNIC;
         tmp->name=ifr->ifr_name;
         if ((flags & IFF_UP) == IFF_UP)
            tmp->state=i18n("Up");
         else
            tmp->state=i18n("Down");

         if ((flags & IFF_BROADCAST) == IFF_BROADCAST)
            tmp->type=i18n("Broadcast");
         else if ((flags & IFF_POINTOPOINT) == IFF_POINTOPOINT)
            tmp->type=i18n("Point to Point");
         else if ((flags & IFF_MULTICAST) == IFF_MULTICAST)
            tmp->type=i18n("Multicast");
         else if ((flags & IFF_LOOPBACK) == IFF_LOOPBACK)
            tmp->type=i18n("Loopback");
         else
            tmp->type=i18n("Unknown");

         tmp->addr=inet_ntoa(sinptr->sin_addr);

         ifcopy=*ifr;
         result=ioctl(sockfd,SIOCGIFNETMASK,&ifcopy);
         if (result==0)
         {
            sinptr = (struct sockaddr_in *) &ifcopy.ifr_addr;
            tmp->netmask=inet_ntoa(sinptr->sin_addr);
         }
         else
            tmp->netmask=i18n("Unknown");
         nl->append(tmp);
         break;

      default:
         break;
      }
   }
   return nl;
};
/*
extern "C"
{

  KCModule *create_nic(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmnic");
    return new KCMNic(parent, name);
  }
}
*/


#include "nic.moc"

