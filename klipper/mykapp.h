#ifndef MYKAPP_H
#define MYKAPP_H

#include <kuniqueapp.h>
#include <kglobalaccel.h>

class MyKApplication : public KUniqueApplication
{
public:

  MyKApplication() :
    KUniqueApplication() { accel = 0L; }
  ~MyKApplication() {}

  void setGlobalKeys( KGlobalAccel *a ) { accel = a; }

private:

  KGlobalAccel *accel;

  inline bool x11EventFilter( XEvent *e )
  {
    if ( accel )
      {
	if ( accel->x11EventFilter( e ) )
	  return true;
      }

    return KApplication::x11EventFilter( e );
  }

};

#endif
