#ifndef __KDMDESKTOP_H__
#define __KDMDESKTOP_H__


#include <kapplication.h>


#include "bgrender.h"


class MyApplication : public KApplication
{
  Q_OBJECT

public:
 
  MyApplication();


private slots:

  void renderDone();

private:
  
  KBackgroundRenderer renderer;

};


#endif
