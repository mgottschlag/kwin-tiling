#include <fcntl.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kshell.h>

#include "ksystraycmd.h"

#include <X11/Xlib.h>
#include <QX11Info>
#ifndef KDE_USE_FINAL
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#endif
#undef FocusOut
#undef FocusIn
#undef KeyPress
#undef KeyRelease


int main( int argc, char *argv[] )
{
  KAboutData aboutData( "ksystraycmd", 0, ki18n( "KSysTrayCmd" ),
			"KSysTrayCmd 0.1",
			ki18n( "Allows any application to be kept in the system tray" ),
			KAboutData::License_GPL,
			ki18n("(C) 2001-2002 Richard Moore (rich@kde.org)") );
  aboutData.addAuthor( ki18n("Richard Moore"), KLocalizedString(), "rich@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  options.add("!+command", ki18n("Command to execute"));
  // "!" means: all options after command are treated as arguments to the command
  options.add("window <regexp>", ki18n("A regular expression matching the window title\n"
                  "If you do not specify one, then the very first window\n"
				 "to appear will be taken - not recommended."));
  options.add("wid <int>", ki18n("The window id of the target window\n"
                  "Specifies the id of the window to use. If the id starts with 0x\n"
			   "it is assumed to be in hex."));
  options.add("hidden", ki18n( "Hide the window to the tray on startup" ));
  options.add("startonshow", ki18n( "Wait until we are told to show the window before\n"
			      "executing the command" ));
  options.add("tooltip <text>", ki18n( "Sets the initial tooltip for the tray icon" ));
  options.add("keeprunning", ki18n( "Keep the tray icon even if the client exits. This option\n"
			 "has no effect unless startonshow is specified." ));
  options.add("ownicon", ki18n( "Do not use window's icon in systray, but ksystraycmd one's\n"
             "(should be used with --icon to specify ksystraycmd icon)" ));
  options.add("ontop", ki18n( "Try to keep the window above other windows"));
  options.add("quitonhide", ki18n( "Quit the client when we are told to hide the window.\n"
             "This has no effect unless startonshow is specified and implies keeprunning." ));
  /*options.add("menuitem <item>", ki18n( "Adds a custom entry to the tray icon menu\n"
      "The item should have the form text:command." ));*/
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app;

  //
  // Setup the tray icon from the arguments.
  //
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  KSysTrayCmd cmd;

  // Read the window id
  QString wid = args->getOption( "wid" );
  if ( !wid.isEmpty() ) {
      int base = 10;
      if ( wid.startsWith( "0x" ) ) {
	  base = 16;
	  wid = wid.right( wid.length() - 2 );
      }

      bool ok=true;
      ulong w = wid.toULong( &ok, base );
      if ( ok )
	  cmd.setTargetWindow( w );
      else {
	  kWarning() << "KSysTrayCmd: Got bad win id" ;
      }
  }

  // Read window title regexp
  QString title = args->getOption( "window" );
  if ( !title.isEmpty() )
      cmd.setPattern( title );

  if ( title.isEmpty() && wid.isEmpty() && (args->count() == 0) )
    KCmdLineArgs::usageError(i18n("No command or window specified"));

  // Read the command
  QString command;
  for ( int i = 0; i < args->count(); i++ )
    command += KShell::quoteArg(args->arg(i)) + ' ';
  if ( !command.isEmpty() )
      cmd.setCommand( command );

  // Tooltip
  QString tip = args->getOption( "tooltip" );
  if ( !tip.isEmpty() )
    cmd.setDefaultTip( tip );

  // Keep running flag
  if ( args->isSet( "keeprunning" )  )
    cmd.setNoQuit( true );

  if ( args->isSet( "quitonhide" ) ) {
    cmd.setNoQuit( true );
	cmd.setQuitOnHide( true );
  }

  // Start hidden
  if ( args->isSet( "hidden" ) )
    cmd.hideWindow();

  // On top
  if ( args->isSet( "ontop" ) )
    cmd.setOnTop(true);

  // Use ksystraycmd icon
  if ( args->isSet( "ownicon" ) )
    cmd.setOwnIcon(true);

  // Lazy invocation flag
  if ( args->isSet( "startonshow" ) ) {
    cmd.setStartOnShow( true );
    cmd.show();
  }
  else {
    if ( !cmd.start() )
      return 1;
  }

  fcntl(ConnectionNumber(QX11Info::display()), F_SETFD, 1);
  args->clear();

  return app.exec();
}

