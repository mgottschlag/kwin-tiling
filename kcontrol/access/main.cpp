#include <kcmdlineargs.h>
#include <klocale.h>

#include "kaccess.h"


int main(int argc, char *argv[])
{
 
  if (!KAccessApp::start())
    return 0;

  KAccessApp app;
  return app.exec();		   
}
