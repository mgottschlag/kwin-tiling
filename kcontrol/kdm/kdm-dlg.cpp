/*
  Copyright (C) 1997-1998 Thomas Tanghus (tanghus@earthling.net)

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kdm-dlg.h"

#include "positioner.h"

#include <k3urldrag.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kimagefilepreview.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include <QButtonGroup>
#include <QDragEnterEvent>
#include <QEvent>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QStyle>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

extern KSimpleConfig *config;

KDMDialogWidget::KDMDialogWidget( QWidget *parent )
	: QWidget( parent )
{
	QString wtstr;

	QGridLayout *grid = new QGridLayout( this );
	grid->setMargin( KDialog::marginHint() );
	grid->setSpacing( KDialog::spacingHint() );
	//grid->setColumnStretch( 0, 1 );
	grid->setColumnStretch( 1, 1 );

	QHBoxLayout *hlay = new QHBoxLayout();
	hlay->setSpacing( KDialog::spacingHint() );
	grid->addLayout( hlay, 0, 0, 1, 2 );
	greetstr_lined = new KLineEdit( this );
	QLabel *label = new QLabel( i18n("&Greeting:"), this );
	label->setBuddy( greetstr_lined );
	hlay->addWidget( label );
	connect( greetstr_lined, SIGNAL(textChanged( const QString& )),
	         SIGNAL(changed()) );
	hlay->addWidget( greetstr_lined );
	wtstr = i18n("This is the \"headline\" for KDM's login window. You may want to "
	             "put some nice greeting or information about the operating system here.<p>"
	             "KDM will substitute the following character pairs with the "
	             "respective contents:<br><ul>"
	             "<li>%d -> current display</li>"
	             "<li>%h -> host name, possibly with domain name</li>"
	             "<li>%n -> node name, most probably the host name without domain name</li>"
	             "<li>%s -> the operating system</li>"
	             "<li>%r -> the operating system's version</li>"
	             "<li>%m -> the machine (hardware) type</li>"
	             "<li>%% -> a single %</li>"
	             "</ul>" );
	label->setWhatsThis( wtstr );
	greetstr_lined->setWhatsThis( wtstr );


	QGridLayout *hglay = new QGridLayout();
	hglay->setSpacing( KDialog::spacingHint() );
	grid->addLayout( hglay, 1, 0 );

	label = new QLabel( i18n("Logo area:"), this );
	hglay->addWidget( label, 0, 0 );
	QVBoxLayout *vlay = new QVBoxLayout();
	vlay->setSpacing( KDialog::spacingHint() );
	hglay->addLayout( vlay, 0, 1, 1, 2 );
	noneRadio = new QRadioButton( i18nc("logo area", "&None"), this );
	clockRadio = new QRadioButton( i18n("Show cloc&k"), this );
	logoRadio = new QRadioButton( i18n("Sho&w logo"), this );
	QButtonGroup *buttonGroup = new QButtonGroup( this );
	connect( buttonGroup, SIGNAL(buttonClicked( int )),
	         SLOT(slotAreaRadioClicked( int )) );
	connect( buttonGroup, SIGNAL(buttonClicked( int )), SIGNAL(changed()) );
	buttonGroup->addButton( noneRadio, KdmNone );
	buttonGroup->addButton( clockRadio, KdmClock );
	buttonGroup->addButton( logoRadio, KdmLogo );
	vlay->addWidget( noneRadio );
	vlay->addWidget( clockRadio );
	vlay->addWidget( logoRadio );
	wtstr = i18n("You can choose to display a custom logo (see below), a clock or no logo at all.");
	label->setWhatsThis( wtstr );
	noneRadio->setWhatsThis( wtstr );
	logoRadio->setWhatsThis( wtstr );
	clockRadio->setWhatsThis( wtstr );

	logoLabel = new QLabel( i18n("&Logo:"), this );
	logobutton = new QPushButton( this );
	logoLabel->setBuddy( logobutton );
	logobutton->setAutoDefault( false );
	logobutton->setAcceptDrops( true );
	logobutton->installEventFilter( this ); // for drag and drop
	connect( logobutton, SIGNAL(clicked()), SLOT(slotLogoButtonClicked()) );
	hglay->addWidget( logoLabel, 1, 0, Qt::AlignVCenter );
	hglay->addWidget( logobutton, 1, 1, Qt::AlignCenter );
	hglay->setRowMinimumHeight( 1, 110 );
	wtstr = i18n("Click here to choose an image that KDM will display. "
	             "You can also drag and drop an image onto this button "
	             "(e.g. from Konqueror).");
	logoLabel->setWhatsThis( wtstr );
	logobutton->setWhatsThis( wtstr );


	vlay = new QVBoxLayout();
	grid->addLayout( vlay, 1, 1, 2, 1 );
	vlay->setParent( grid );

	label = new QLabel( i18n("Dialog &position:"), this );
	vlay->addWidget( label );
	positioner = new Positioner( this );
	label->setBuddy( positioner );
	connect( positioner, SIGNAL(positionChanged()), SIGNAL(changed()) );
	vlay->addWidget( positioner );

	grid->setRowStretch( 3, 1 );

}

void KDMDialogWidget::makeReadOnly()
{
	disconnect( logobutton, SIGNAL(clicked()),
	            this, SLOT(slotLogoButtonClicked()) );
	logobutton->setAcceptDrops( false );
	greetstr_lined->setReadOnly( true );
	noneRadio->setEnabled( false );
	clockRadio->setEnabled( false );
	logoRadio->setEnabled( false );
	positioner->makeReadOnly();
}

bool KDMDialogWidget::setLogo(QString logo)
{
	QString flogo = logo.isEmpty() ?
		KStandardDirs::locate( "data", QLatin1String("kdm/pics/kdelogo.png") ) :
		logo;
	QImage p( flogo );
	if (p.isNull())
		return false;
	if (p.width() > 100 || p.height() > 100)
		p = p.scaled( 100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation );
	logobutton->setPixmap( QPixmap::fromImage( p ) );
	uint bd = style()->pixelMetric( QStyle::PM_ButtonMargin ) * 2;
	logobutton->setFixedSize( p.width() + bd, p.height() + bd );
	logopath = logo;
	return true;
}


void KDMDialogWidget::slotLogoButtonClicked()
{
	KFileDialog dialog( KStandardDirs::locate( "data", QLatin1String("kdm/pics/") ),
	                    KImageIO::pattern( KImageIO::Reading ),
	                    this );
	dialog.setOperationMode( KFileDialog::Opening );
	dialog.setMode( KFile::File | KFile::LocalOnly );

	KImageFilePreview *imagePreview = new KImageFilePreview( &dialog );
	dialog.setPreviewWidget( imagePreview );
	if (dialog.exec() == QDialog::Accepted &&
	    setLogo(dialog.selectedFile() ))
		changed();
}


void KDMDialogWidget::slotAreaRadioClicked( int id )
{
	logobutton->setEnabled( id == KdmLogo );
	logoLabel->setEnabled( id == KdmLogo );
}


bool KDMDialogWidget::eventFilter( QObject *, QEvent *e )
{
	if (e->type() == QEvent::DragEnter) {
		iconLoaderDragEnterEvent( (QDragEnterEvent *)e );
		return true;
	}

	if (e->type() == QEvent::Drop) {
		iconLoaderDropEvent( (QDropEvent *)e );
		return true;
	}

	return false;
}

void KDMDialogWidget::iconLoaderDragEnterEvent( QDragEnterEvent *e )
{
	e->setAccepted( K3URLDrag::canDecode( e ) );
}


KUrl *decodeImgDrop( QDropEvent *e, QWidget *wdg );

void KDMDialogWidget::iconLoaderDropEvent( QDropEvent *e )
{
	KUrl pixurl;
	bool istmp;

	KUrl *url = decodeImgDrop( e, this );
	if (url) {

		// we gotta check if it is a non-local file and make a tmp copy at the hd.
		if (!url->isLocalFile()) {
			pixurl.setPath( KGlobal::dirs()->
				resourceDirs("data").last() + "kdm/pics/" + url->fileName() );
			KIO::NetAccess::file_copy( *url, pixurl, parentWidget() );
			istmp = true;
		} else {
			pixurl = *url;
			istmp = false;
		}

		// By now url should be "file:/..."
		if (!setLogo( pixurl.path() )) {
			KIO::NetAccess::del( pixurl, parentWidget() );
			QString msg = i18n("There was an error loading the image:\n"
			                   "%1\n"
			                   "It will not be saved.",
			                   pixurl.path());
			KMessageBox::sorry( this, msg );
		}

		delete url;
	}
}


void KDMDialogWidget::save()
{
	config->setGroup( "X-*-Greeter" );

	config->writeEntry( "GreetString", greetstr_lined->text() );

	config->writeEntry( "LogoArea", noneRadio->isChecked() ? "None" :
	                    logoRadio->isChecked() ? "Logo" : "Clock" );

	config->writeEntry( "LogoPixmap", KGlobal::iconLoader()->iconPath( logopath, K3Icon::Desktop, true ) );

	config->writeEntry( "GreeterPos",
		QString("%1,%2").arg( positioner->x() ).arg( positioner->y() ) );
}


void KDMDialogWidget::load()
{
	config->setGroup( "X-*-Greeter" );

	// Read the greeting string
	greetstr_lined->setText( config->readEntry( "GreetString",
	                                            i18n("Welcome to %s at %n") ) );

	// Regular logo or clock
	QString logoArea = config->readEntry( "LogoArea", "Logo" );
	if (logoArea == "Clock") {
		clockRadio->setChecked( true );
		slotAreaRadioClicked( KdmClock );
	} else if (logoArea == "Logo") {
		logoRadio->setChecked( true );
		slotAreaRadioClicked( KdmLogo );
	} else {
		noneRadio->setChecked( true );
		slotAreaRadioClicked( KdmNone );
	}

	// See if we use alternate logo
	setLogo( config->readEntry( "LogoPixmap" ) );

	QStringList sl = config->readEntry( "GreeterPos", QStringList() );
	if (sl.count() != 2)
		positioner->setPosition( 50, 50 );
	else
		positioner->setPosition( sl.first().toInt(), sl.last().toInt() );
}


void KDMDialogWidget::defaults()
{
	greetstr_lined->setText( i18n("Welcome to %s at %n") );
	logoRadio->setChecked( true );
	slotAreaRadioClicked( KdmLogo );
	setLogo( "" );
	positioner->setPosition( 50, 50 );
}

QString KDMDialogWidget::quickHelp() const
{
	return i18n("<h1>KDM - Dialog</h1> Here you can configure the basic appearance"
	            " of the KDM login manager in dialog mode, i.e. a greeting string, an icon etc.");
}

#include "kdm-dlg.moc"
