#ifndef helper_h
#define helper_h

inline void checkInsertPos( QPopupMenu *popup, const QString & str,
                                   int &index )
{
  if ( index == -1 )
    return;

  int a = 0;
  int b = popup->count();
  while ( a < b )
  {
    int w = ( a + b ) / 2;

    int id = popup->idAt( w );
    int j = str.compare( popup->text( id ) );

    if ( j > 0 )
      a = w + 1;
    else
      b = w;
  }

  index = a; // it doesn't really matter ... a == b here.

  Q_ASSERT( a == b );
}

inline QPopupMenu * checkInsertIndex( QPopupMenu *popup,
                            const QStringList *tags, const QString &submenu )
{
  int pos = tags->findIndex( submenu );

  QPopupMenu *pi = 0;
  if ( pos != -1 )
  {
    QMenuItem *p = popup->findItem( pos );
    pi = p ? p->popup() : 0;
  }
  if ( !pi )
    pi = popup;

  return pi;
}



#endif
