// (c) Torben Weis 1998
// (c) David Faure 1998

#include <stdio.h>

#include <kcontrol.h>
#include <klocale.h>

#include "rootopts.h"

KConfig *g_pConfig;

class KDesktopControlApplication: public KControlApplication
{
public:
  KDesktopControlApplication( int &argc, char **argv );

  virtual void init();
  virtual void apply();
  virtual void defaultValues();

private:
  KRootOptions *m_pRootOptions;
};

KDesktopControlApplication::KDesktopControlApplication( int &argc, char **argv )
: KControlApplication( argc, argv, "kcmkdesktop" )
{
  m_pRootOptions = 0L;
  
  if ( !runGUI() )
    return;
  
  if ( !pages || pages->contains( "icons" ) )
   addPage( m_pRootOptions = new KRootOptions( dialog, "icons" ), i18n( "&Desktop Icons" ), "kdesktop-1.html" );
     
  if ( m_pRootOptions )
     dialog->show();
  else
  {
    fprintf(stderr, i18n("usage: %s [-init | {icons}]\n").ascii(), argv[0] );;
    justInit = true;
  }
}

void KDesktopControlApplication::init()
{
  if ( m_pRootOptions )
    m_pRootOptions->loadSettings();
}

void KDesktopControlApplication::defaultValues()
{
  if ( m_pRootOptions )
    m_pRootOptions->defaultSettings();
}

void KDesktopControlApplication::apply()
{
  if ( m_pRootOptions )
    m_pRootOptions->applySettings();
}

int main(int argc, char **argv )
{
  g_pConfig = new KConfig( "kdesktoprc", false, false );
  KDesktopControlApplication app( argc, argv );
  
  app.setTitle( i18n( "KDE Desktop Configuration" ) );
  
  int ret = 0;
  if ( app.runGUI() )
    ret = app.exec();
  
  delete g_pConfig;
  return ret;
}
