/*
 * ksmbstatus.cpp
 *
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpoint.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <kapp.h>
#include <qfont.h>
#include <time.h>
#include <kprocess.h>

#include "ksmbstatus.h"
#include "ksmbstatus.moc"

#include <klocale.h>
#include <stdio.h>

#include <iostream.h>
#include <qstring.h>
#include <qregexp.h>

#define Before(ttf,in) in.left(in.find(ttf))
#define After(ttf,in)  (in.contains(ttf)?QString(in.mid(in.find(ttf)+QString(ttf).length())):QString(""))

NetMon::NetMon( QWidget * parent, KConfig *config, const char * name )
   : QWidget(parent, name)
   ,configFile(config)
   ,strShare("")
   ,strUser("")
   ,strGroup("")
   ,strMachine("")
   ,strSince("")
   ,strPid("")
   ,iUser(0)
   ,iGroup(0)
   ,iMachine(0)
   ,iPid(0)
{
    QBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setAutoAdd(true);
    topLayout->setMargin(SCREEN_XY_OFFSET);
    topLayout->setSpacing(10);

    list=new QListView(this,"Hello");
    version=new QLabel(this);

    list->setAllColumnsShowFocus(true);
    list->setMinimumSize(425,200);
    list->addColumn(i18n("Type"), 50);
    list->addColumn(i18n("Service"), 80);
    list->addColumn(i18n("Accessed from"),100);
    list->addColumn(i18n("UID"), 70);
    list->addColumn(i18n("GID"), 70);
    list->addColumn(i18n("PID"), 50);
    list->addColumn(i18n("Open Files"),70);

    timer = new QTimer(this);
    timer->start(10000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    menu = new KPopupMenu();
    QObject::connect(list,SIGNAL(rightButtonPressed(QListViewItem *,const QPoint &,int)),
		    SLOT(Killmenu(QListViewItem *,const QPoint &,int)));
    update();
};

void NetMon::help()
{
//        KNetMon->invokeHTMLHelp("","");

}

void NetMon::processNFSLine(char *bufline, int)
{
   QCString line(bufline);
   if (line.contains(":/"))
      new QListViewItem(list,"NFS",After(":",line),Before(":/",line));
};

void NetMon::processSambaLine(char *bufline, int)
{
   QCString line(bufline);
   rownumber++;
   if (rownumber == 2)
      version->setText(bufline); // second line = samba version
   if ((readingpart==header) && line.contains("Service"))
   {
      iUser=line.find("uid");
      iGroup=line.find("gid");
      iPid=line.find("pid");
      iMachine=line.find("machine");
   }
   else if ((readingpart==header) && (line.contains("---")))
   {
      readingpart=connexions;
   }
   else if ((readingpart==connexions) && (int(line.length())>=iMachine))
   {
      strShare=line.mid(0,iUser);
      strUser=line.mid(iUser,iGroup-iUser);
      strGroup=line.mid(iGroup,iPid-iGroup);
      strPid=line.mid(iPid,iMachine-iPid);

      line=line.mid(iMachine,line.length());
      strMachine=line;
      /*strSince=s.after(")");
       strSince=strSince.afterRXwhite();
       strSince=strSince.afterRXwhite();
       strSince=strSince.afterRXwhite();
       strSince=strSince.afterRXwhite();
       strSince=strSince.before(" ");*/
      new QListViewItem(list,"SMB",strShare,strMachine, strUser,strGroup,strPid/*,strSince*/);
   }
   else if (readingpart==connexions)
      readingpart=locked_files;
   else if ((readingpart==locked_files) && (line.find("No ")==0))
      readingpart=finished;
   else if (readingpart==locked_files)
   {
      if ((strncmp(bufline,"Pi", 2) !=0) // "Pid DenyMode ..."
          && (strncmp(bufline,"--", 2) !=0)) // "------------"
      {
         char *tok=strtok(bufline," ");
         int pid=atoi(tok);
         (lo)[pid]++;
      }
   };
/* char *tok;
   int pid;
   QListViewItem *row;
   int column;

   rownumber++;
   if (rownumber == 2)
      version->setText(bufline); // second line = samba version
    else if ((rownumber >= 5) && (readingpart==connexions))
    {

       if (linelen<=1)
          readingpart=locked_files; // stop after first empty line.
       else
       {
          row = new QListViewItem(list);
          column=0;
          tok=strtok(bufline," ");
          row->setText(column++,tok);
          while (column<5)
          {
             tok=strtok(NULL," ");
             row->setText(column++,tok);
          }
       }
    }
    else if (readingpart==locked_files)
    {
       if ((strncmp(bufline,"No",2) == 0) || (linelen<=1))
       {
          // "No locked files"
          readingpart=finished;
          //debug("finished");
       }
       else
          if ((strncmp(bufline,"Pi", 2) !=0) // "Pid DenyMode ..."
              && (strncmp(bufline,"--", 2) !=0)) // "------------"
          {
             tok=strtok(bufline," ");
             pid=atoi(tok);
             (lo)[pid]++;
          }
    }*/
}

// called when we get some data from smbstatus
// can be called for any size of buffer (one line, several lines,
// half of one ...)
void NetMon::slotReceivedData(KProcess *, char *buffer, int )
{
   char s[250],*start,*end;
   size_t len;
   start = buffer;
   while ((end = strchr(start,'\n'))) // look for '\n'
   {
      len = end-start;
      strncpy(s,start,len);
      s[len] = '\0';
      //debug(s); debug("**");
      if (readingpart==nfs)
         processNFSLine(s,len);
      else
         processSambaLine(s,len); // process each line
      start=end+1;
   }
   // here we could save the remaining part of line, if ever buffer
   // doesn't end with a '\n' ... but will this happen ?
}

void NetMon::update()
{
   int pid,n;
   QListViewItem *row;
   KProcess * process = new KProcess();

   for (n=0;n<65536;n++) lo[n]=0;
   list->clear();
   /* Re-read the Contents ... */

   rownumber=0;
   readingpart=header;
   nrpid=0;
   connect(process,
           SIGNAL(receivedStdout(KProcess *, char *, int)),
           SLOT(slotReceivedData(KProcess *, char *, int)));
   *process << "smbstatus"; // the command line
   //debug("update");
   if (!process->start(KProcess::Block,KProcess::Stdout)) // run smbstatus
      version->setText(i18n("Error: Unable to run smbstatus"));
   else if (rownumber==0) // empty result
      version->setText(i18n("Error: Unable to open configuration file \"smb.conf\""));
   else
   {
      // ok -> count the number of locked files for each pid
      for (row=list->firstChild();row!=0;row=row->itemBelow())
      {
//         cerr<<"NetMon::update: this should be the pid: "<<row->text(5)<<endl;
         pid=atoi(row->text(5));
         row->setText(6,QString("%1").arg((lo)[pid]));
      }
   }
   delete process;

   readingpart=nfs;
   process=new KProcess();
   *process<<"showmount"<<"-a"<<"localhost";
   connect(process,SIGNAL(receivedStdout(KProcess *, char *, int)),SLOT(slotReceivedData(KProcess *, char *, int)));
   if (!process->start(KProcess::Block,KProcess::Stdout)) // run smbstatus
      version->setText(version->text()+i18n(" Error: Unable to run showmount"));
   delete process;
   process=0;

   version->adjustSize();
   list->show();
}

void NetMon::Kill()
{
   QString a(killrow->text(5));
//   cerr<<"NetMon::killing -"<<a<<"-"<<endl;
   kill(a.toUInt(),15);
   update();
}

void NetMon::Killmenu(QListViewItem * row, const QPoint& pos, int /* column */)
{
   // WABA: column not used, is this ok?
   if (row!=0)
   {
       killrow=row;
       menu->clear();
       menu->insertItem("&Kill",this,SLOT(Kill()));
       menu->setTitle("//"+row->text(2)+"/"+row->text(1));
       menu->popup(pos);
   }
}

