#ifndef main_included
#define main_included
 
#include <kpopmenu.h>
#include <qtimer.h>
#include <qlabel.h>
#include <kcontrol.h>
#include <kprocess.h>

#define SCREEN_XY_OFFSET 20

class QListView;
class QListViewItem;

class NetMon : public KConfigWidget
{
Q_OBJECT
public:
   NetMon(QWidget *parent, const char * name=NULL);

   void applySettings() {};
   void loadSettings() {};    

private:
   QListView *list;
   QLabel *version;
   QTimer *timer;
   KPopupMenu *menu;
   QListViewItem *killrow;
   int rownumber;
   enum {connexions, locked_files, finished} readingpart;
   int lo[65536];
   int nrpid;
   void processLine(char *bufline, int linelen);
   
private slots:
   void update();
   void help();
   void Kill();
   void Killmenu(QListViewItem *row, const QPoint &pos, int column);
   void slotReceivedData(KProcess *proc, char *buffer, int buflen);
   
};

#endif // main_included                        
