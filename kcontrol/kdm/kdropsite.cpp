
#include "kdropsite.h"


KDropSite::KDropSite( QWidget * parent ) : QObject( parent ), QDropSite( parent )
{
  //kdDebug() << "KDropSite constructor" << endl;
}

void KDropSite::dragMoveEvent( QDragMoveEvent *e )
{
  //kdDebug() << "dragMove" << endl;
  emit dragMove(e);
}

void KDropSite::dragEnterEvent( QDragEnterEvent *e )
{
  //kdDebug() << "dragEnter" << endl;
  emit dragEnter(e);
}

void KDropSite::dragLeaveEvent( QDragLeaveEvent *e )
{
  //kdDebug() << "dragLeave" << endl;
  emit dragLeave(e);
}

void KDropSite::dropEvent( QDropEvent *e )
{
  //kdDebug() << "drop" << endl;
  emit dropAction(e);
}


#include "kdropsite.moc"

