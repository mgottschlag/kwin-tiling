#include <qlayout.h>
#include <qlistview.h>
#include <qmsgbox.h>
#include <qpoint.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <kapp.h>
#include <qpushbt.h>
#include <qfont.h>
#include <kmenubar.h>
#include <ktmainwindow.h>
#include <time.h>
#include <kprocess.h>

#include "ksmbstatus.h"
#include "ksmbstatus.moc"

#include <klocale.h>
#include <stdio.h>


void NetMon::help()
{
//        KNetMon->invokeHTMLHelp("","");                 

}

void NetMon::processLine(char *bufline, int linelen)
{
    char *tok;
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
        else {
	    row = new QListViewItem(list);
	    column=0;
	    tok=strtok(bufline," ");
	    row->setText(column++,tok);
            while (column<5) {
		tok=strtok(NULL," ");
		row->setText(column++,tok);
	    }
        }
      }
    else if (readingpart==locked_files)
    {
      if ((strncmp(bufline,"No",2) == 0) || (linelen<=1))
      { // "No locked files"
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
    }
}

// called when we get some data from smbstatus
//   can be called for any size of buffer (one line, several lines,
//     half of one ...)
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
        processLine(s,len); // process each line
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
    readingpart=connexions;
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
    else { // ok -> count the number of locked files for each pid
        for (row=list->firstChild();row!=0;row=row->itemBelow()) {
	    pid=atoi(row->text(3));
	    row->setText(5,QString("%1").arg((lo)[pid]));
        }
    }
    version->adjustSize();
    delete process;
    list->show();
}


void NetMon::Kill()
{
  QString a;
  a = killrow->text(3);
  kill(a.toUInt(),15);
  update();
}

void NetMon::Killmenu(QListViewItem * row, const QPoint& pos, int /* column */)
{
   // WABA: column not used, is this ok?
   if (row!=0) {
       killrow=row;
       menu->setTitle("//"+row->text(4)+"/"+row->text(0));
       menu->popup(pos);
   }
}

NetMon::NetMon( QWidget * parent, const char * name )
        : KConfigWidget (parent, name)
{
    QBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setAutoAdd(true);
    topLayout->setMargin(SCREEN_XY_OFFSET);
    topLayout->setSpacing(10);
    
    list=new QListView(this,"Hello");
    version=new QLabel(this);
    
    list->setAllColumnsShowFocus(true);
    list->setMinimumSize(425,200);
    list->addColumn(i18n("Service"), 80);
    list->addColumn(i18n("UID"), 70);
    list->addColumn(i18n("GID"), 70);     
    list->addColumn(i18n("PID"), 50);
    list->addColumn(i18n("Machine"),80);
    list->addColumn(i18n("Open Files"),70);
 
    timer = new QTimer(this);
    timer->start(5000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    menu = new KPopupMenu();
    menu->insertItem("&Kill",this,SLOT(Kill()));
    QObject::connect(list,SIGNAL(rightButtonPressed(QListViewItem *,const QPoint &,int)),
		    SLOT(Killmenu(QListViewItem *,const QPoint &,int)));
    update();
}
