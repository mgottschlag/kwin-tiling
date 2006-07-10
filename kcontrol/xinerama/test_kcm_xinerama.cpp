#include <QApplication>
#include <QDesktopWidget>

int main( int argc, char** argv )
{
	QApplication app( argc, argv );
	if( QApplication::desktop()->isVirtualDesktop() )
		return 0;
	return 1;
}
