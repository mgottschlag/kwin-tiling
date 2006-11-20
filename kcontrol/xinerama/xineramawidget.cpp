#include "xineramawidget.h"
// TODO replace comment below with license
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

XineramaWidget::XineramaWidget( QWidget* parent )
    : QWidget( parent ), Ui_XineramaWidget()
{
    setupUi( this );

}


void XineramaWidget::emitConfigChanged()
{
emit configChanged();
}
#include "xineramawidget.moc"
