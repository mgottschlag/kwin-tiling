/*
  This file is part of the KDE Display Manager Configuration package
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


#include <unistd.h>
#include <sys/types.h>


#include <q3buttongroup.h>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>

#include <QValidator>
#include <qstylefactory.h>
#include <QStyle>
//Added by qt3to4:
#include <QGridLayout>
#include <QEvent>
#include <QHBoxLayout>
#include <QDropEvent>
#include <QVBoxLayout>
#include <QDragEnterEvent>

#include <klocale.h>
#include <klineedit.h>
#include <kimageio.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kiconloader.h>
#include <k3urldrag.h>
#include <kimagefilepreview.h>

#include "kdm-appear.h"
#include "kbackedcombobox.h"

extern KSimpleConfig *config;


KDMAppearanceWidget::KDMAppearanceWidget(QWidget *parent)
  : QWidget(parent)
{
  QString wtstr;

  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->setMargin(KDialog::marginHint());
  vbox->setSpacing(KDialog::spacingHint());

  Q3GroupBox *group = new Q3GroupBox(i18n("Appearance"), this);
  vbox->addWidget(group);

  QGridLayout *grid = new QGridLayout( group);
  grid->setMargin( KDialog::marginHint() );
  grid->setSpacing( KDialog::spacingHint() );
  grid->addRowSpacing(0, group->fontMetrics().height());
  grid->setColumnStretch(0, 1);
  grid->setColumnStretch(1, 1);

  QHBoxLayout *hlay = new QHBoxLayout( );
  hlay->setSpacing(KDialog::spacingHint());
  grid->addLayout(hlay, 1, 0, 1, 2 );
  greetstr_lined = new KLineEdit(group);
  QLabel *label = new QLabel(i18n("&Greeting:"),group);
  label->setBuddy(greetstr_lined);
  hlay->addWidget(label);
  connect(greetstr_lined, SIGNAL(textChanged(const QString&)),
      SLOT(changed()));
  hlay->addWidget(greetstr_lined);
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
  grid->addLayout(hglay, 2, 0, 3, 1);

  label = new QLabel(i18n("Logo area:"),group);
  hglay->addWidget(label, 0, 0);
  QVBoxLayout *vlay = new QVBoxLayout();
  vlay->setSpacing( KDialog::spacingHint() );
  hglay->addLayout(vlay, 0, 1, 1,2);
  noneRadio = new QRadioButton( i18nc("logo area", "&None"), group );
  clockRadio = new QRadioButton( i18n("Show cloc&k"), group );
  logoRadio = new QRadioButton( i18n("Sho&w logo"), group );
  Q3ButtonGroup *buttonGroup = new Q3ButtonGroup( group );
  label->setBuddy( buttonGroup );
  connect( buttonGroup, SIGNAL(clicked(int)), SLOT(slotAreaRadioClicked(int)) );
  connect( buttonGroup, SIGNAL(clicked(int)), SLOT(changed()) );
  buttonGroup->hide();
  buttonGroup->insert(noneRadio, KdmNone);
  buttonGroup->insert(clockRadio, KdmClock);
  buttonGroup->insert(logoRadio, KdmLogo);
  vlay->addWidget(noneRadio);
  vlay->addWidget(clockRadio);
  vlay->addWidget(logoRadio);
  wtstr = i18n("You can choose to display a custom logo (see below), a clock or no logo at all.");
  label->setWhatsThis( wtstr );
  noneRadio->setWhatsThis( wtstr );
  logoRadio->setWhatsThis( wtstr );
  clockRadio->setWhatsThis( wtstr );

  logoLabel = new QLabel(i18n("&Logo:"),group);
  logobutton = new QPushButton(group);
  logoLabel->setBuddy( logobutton );
  logobutton->setAutoDefault(false);
  logobutton->setAcceptDrops(true);
  logobutton->installEventFilter(this); // for drag and drop
  connect(logobutton, SIGNAL(clicked()), SLOT(slotLogoButtonClicked()));
  hglay->addWidget(logoLabel, 1, 0);
  hglay->addWidget(logobutton, 1, 1, Qt::AlignCenter);
  hglay->addRowSpacing(1, 110);
  wtstr = i18n("Click here to choose an image that KDM will display. "
	       "You can also drag and drop an image onto this button "
	       "(e.g. from Konqueror).");
  logoLabel->setWhatsThis( wtstr );
  logobutton->setWhatsThis( wtstr );
  hglay->addRowSpacing( 2, KDialog::spacingHint());
  hglay->setColumnStretch( 3, 1);


  hglay = new QGridLayout();
  hglay->setSpacing( KDialog::spacingHint() );
  grid->addLayout(hglay, 2, 1);

  label = new QLabel(i18n("Position:"),group);
  hglay->addWidget(label, 0, 0, 2, 1, Qt::AlignVCenter);
  QValidator *posValidator = new QIntValidator(0, 100, group);
  QLabel *xLineLabel = new QLabel(i18n("&X:"),group);
  hglay->addWidget(xLineLabel, 0, 1);
  xLineEdit = new QLineEdit (group);
  connect( xLineEdit, SIGNAL( textChanged(const QString&) ), SLOT( changed() ));
  hglay->addWidget(xLineEdit, 0, 2);
  xLineLabel->setBuddy(xLineEdit);
  xLineEdit->setValidator(posValidator);
  QLabel *yLineLabel = new QLabel(i18n("&Y:"),group);
  hglay->addWidget(yLineLabel, 1, 1);
  yLineEdit = new QLineEdit (group);
  connect( yLineEdit, SIGNAL( textChanged(const QString&) ), SLOT( changed() ));
  hglay->addWidget(yLineEdit, 1, 2);
  yLineLabel->setBuddy(yLineEdit);
  yLineEdit->setValidator(posValidator);
  wtstr = i18n("Here you specify the relative coordinates (in percent) of the login dialog's <em>center</em>.");
  label->setWhatsThis( wtstr );
  xLineLabel->setWhatsThis( wtstr );
  xLineEdit->setWhatsThis( wtstr );
  yLineLabel->setWhatsThis( wtstr );
  yLineEdit->setWhatsThis( wtstr );
  hglay->setColumnStretch( 3, 1);
  hglay->setRowStretch( 2, 1);


  hglay = new QGridLayout();
  hglay->setSpacing( KDialog::spacingHint() );
  grid->addLayout(hglay, 3, 1);
  hglay->setColumnStretch(3, 1);

  guicombo = new KBackedComboBox(group);
  guicombo->insertItem( "", i18n("<default>") );
  loadGuiStyles(guicombo);
  label = new QLabel(i18n("GUI s&tyle:"),group);
  label->setBuddy(guicombo);
  connect(guicombo, SIGNAL(activated(int)), SLOT(changed()));
  hglay->addWidget(label, 0, 0);
  hglay->addWidget(guicombo, 0, 1);
  wtstr = i18n("You can choose a basic GUI style here that will be "
        "used by KDM only.");
  label->setWhatsThis( wtstr );
  guicombo->setWhatsThis( wtstr );

  colcombo = new KBackedComboBox(group);
  colcombo->insertItem( "", i18n("<default>") );
  loadColorSchemes(colcombo);
  label = new QLabel(i18n("&Color scheme:"),group);
  label->setBuddy(colcombo);
  connect(colcombo, SIGNAL(activated(int)), SLOT(changed()));
  hglay->addWidget(label, 1, 0);
  hglay->addWidget(colcombo, 1, 1);
  wtstr = i18n("You can choose a basic Color Scheme here that will be "
        "used by KDM only.");
  label->setWhatsThis( wtstr );
  colcombo->setWhatsThis( wtstr );

  echocombo = new KBackedComboBox(group);
  echocombo->insertItem("NoEcho", i18n("No Echo"));
  echocombo->insertItem("OneStar", i18n("One Star"));
  echocombo->insertItem("ThreeStars", i18n("Three Stars"));
  label = new QLabel(i18n("Echo &mode:"),group);
  label->setBuddy(echocombo);
  connect(echocombo, SIGNAL(activated(int)), SLOT(changed()));
  hglay->addWidget(label, 2, 0);
  hglay->addWidget(echocombo, 2, 1);
  wtstr = i18n("You can choose whether and how KDM shows your password when you type it.");
  label->setWhatsThis( wtstr );
  echocombo->setWhatsThis( wtstr );


  // The Language group box
  group = new Q3GroupBox(0, Qt::Vertical, i18n("Locale"), this);
  vbox->addWidget(group);

  langcombo = new KLanguageButton(group);
  loadLanguageList(langcombo);
  connect(langcombo, SIGNAL(activated(const QString &)), SLOT(changed()));
  label = new QLabel(i18n("Languag&e:"),group);
  label->setBuddy(langcombo);
  QGridLayout *hbox = new QGridLayout();
  group->layout()->addItem(hbox);
  hbox->setSpacing( KDialog::spacingHint() );
  hbox->setColumnStretch(1, 1);
  hbox->addWidget(label, 1, 0);
  hbox->addWidget(langcombo, 1, 1);
  wtstr = i18n("Here you can choose the language used by KDM. This setting does not affect"
    " a user's personal settings; that will take effect after login.");
  label->setWhatsThis( wtstr );
  langcombo->setWhatsThis( wtstr );


  vbox->addStretch(1);

}

void KDMAppearanceWidget::makeReadOnly()
{
    disconnect( logobutton, SIGNAL(clicked()),
		this, SLOT(slotLogoButtonClicked()) );
    logobutton->setAcceptDrops(false);
    greetstr_lined->setReadOnly(true);
    noneRadio->setEnabled(false);
    clockRadio->setEnabled(false);
    logoRadio->setEnabled(false);
    xLineEdit->setEnabled(false);
    yLineEdit->setEnabled(false);
    guicombo->setEnabled(false);
    colcombo->setEnabled(false);
    echocombo->setEnabled(false);
    langcombo->setEnabled(false);
}

void KDMAppearanceWidget::loadLanguageList(KLanguageButton *combo)
{
  QStringList langlist = KGlobal::dirs()->findAllResources("locale",
			QLatin1String("*/entry.desktop"));
  langlist.sort();
  for ( QStringList::ConstIterator it = langlist.begin();
	it != langlist.end(); ++it )
  {
    QString fpath = (*it).left((*it).length() - 14);
    int index = fpath.lastIndexOf('/');
    QString nid = fpath.mid(index + 1);

    KSimpleConfig entry(*it);
    entry.setGroup(QLatin1String("KCM Locale"));
    QString name = entry.readEntry(QLatin1String("Name"), i18n("without name"));
    combo->insertLanguage(nid, name, QLatin1String("l10n/"), QString());
  }
}

void KDMAppearanceWidget::loadColorSchemes(KBackedComboBox *combo)
{
  // XXX: Global + local schemes
  QStringList list = KGlobal::dirs()->
      findAllResources("data", "kdisplay/color-schemes/*.kcsrc", false, true);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
  {
    KSimpleConfig config(*it, true);
    config.setGroup("Color Scheme");

    QString str;
    if (!(str = config.readEntry("Name")).isEmpty() ||
	!(str = config.readEntry("name")).isEmpty())
    {
	QString str2 = (*it).mid( (*it).lastIndexOf( '/' ) + 1 ); // strip off path
	str2.resize( str2.length() - 6 ); // strip off ".kcsrc
        combo->insertItem( str2, str );
    }
  }
}

void KDMAppearanceWidget::loadGuiStyles(KBackedComboBox *combo)
{
  // XXX: Global + local schemes
  QStringList list = KGlobal::dirs()->
      findAllResources("data", "kstyle/themes/*.themerc", false, true);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
  {
    KSimpleConfig config(*it, true);

    if (!(config.hasGroup("KDE") && config.hasGroup("Misc")))
	continue;

    config.setGroup("Desktop Entry");
    if (config.readEntry("Hidden", false))
	continue;

    config.setGroup("KDE");
    QString str2 = config.readEntry("WidgetStyle");
    if (str2.isNull())
	continue;

    config.setGroup("Misc");
    combo->insertItem( str2, config.readEntry("Name") );
  }
}

bool KDMAppearanceWidget::setLogo(QString logo)
{
    QString flogo = logo.isEmpty() ?
                    locate("data", QLatin1String("kdm/pics/kdelogo.png") ) :
                    logo;
    QImage p(flogo);
    if (p.isNull())
        return false;
    if (p.width() > 100 || p.height() > 100)
        p = p.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    logobutton->setPixmap(QPixmap::fromImage(p));
    uint bd = style()->pixelMetric( QStyle::PM_ButtonMargin ) * 2;
    logobutton->setFixedSize(p.width() + bd, p.height() + bd);
    logopath = logo;
    return true;
}


void KDMAppearanceWidget::slotLogoButtonClicked()
{
    KFileDialog dialog(locate("data", QLatin1String("kdm/pics/")),
			 KImageIO::pattern( KImageIO::Reading ),
			 this);
    dialog.setOperationMode( KFileDialog::Opening );
    dialog.setMode( KFile::File | KFile::LocalOnly );

    KImageFilePreview* imagePreview = new KImageFilePreview( &dialog );
    dialog.setPreviewWidget( imagePreview );
    if (dialog.exec() == QDialog::Accepted) {
	if ( setLogo(dialog.selectedFile()) ) {
	    changed();
	}
    }
}


void KDMAppearanceWidget::slotAreaRadioClicked(int id)
{
    logobutton->setEnabled( id == KdmLogo );
    logoLabel->setEnabled( id == KdmLogo );
}


bool KDMAppearanceWidget::eventFilter(QObject *, QEvent *e)
{
  if (e->type() == QEvent::DragEnter) {
    iconLoaderDragEnterEvent((QDragEnterEvent *) e);
    return true;
  }

  if (e->type() == QEvent::Drop) {
    iconLoaderDropEvent((QDropEvent *) e);
    return true;
  }

  return false;
}

void KDMAppearanceWidget::iconLoaderDragEnterEvent(QDragEnterEvent *e)
{
  e->setAccepted(K3URLDrag::canDecode(e));
}


KUrl *decodeImgDrop(QDropEvent *e, QWidget *wdg);

void KDMAppearanceWidget::iconLoaderDropEvent(QDropEvent *e)
{
    KUrl pixurl;
    bool istmp;

    KUrl *url = decodeImgDrop(e, this);
    if (url) {

	// we gotta check if it is a non-local file and make a tmp copy at the hd.
	if(!url->isLocalFile()) {
	    pixurl.setPath(KGlobal::dirs()->resourceDirs("data").last() +
		     "kdm/pics/" + url->fileName());
	    KIO::NetAccess::copy(*url, pixurl, parentWidget());
	    istmp = true;
	} else {
	    pixurl = *url;
	    istmp = false;
	}

	// By now url should be "file:/..."
	if (!setLogo(pixurl.path())) {
	    KIO::NetAccess::del(pixurl, parentWidget());
	    QString msg = i18n("There was an error loading the image:\n"
			       "%1\n"
			       "It will not be saved.",
			        pixurl.path());
	    KMessageBox::sorry(this, msg);
	}

	delete url;
    }
}


void KDMAppearanceWidget::save()
{
  config->setGroup("X-*-Greeter");

  config->writeEntry("GreetString", greetstr_lined->text());

  config->writeEntry("LogoArea", noneRadio->isChecked() ? "None" :
			    logoRadio->isChecked() ? "Logo" : "Clock" );

  config->writeEntry("LogoPixmap", KGlobal::iconLoader()->iconPath(logopath, K3Icon::Desktop, true));

  config->writeEntry("GUIStyle", guicombo->currentId());

  config->writeEntry("ColorScheme", colcombo->currentId());

  config->writeEntry("EchoMode", echocombo->currentId());

  config->writeEntry("GreeterPos", xLineEdit->text() + ',' + yLineEdit->text());

  config->writeEntry("Language", langcombo->current());
}


void KDMAppearanceWidget::load()
{
  config->setGroup("X-*-Greeter");

  // Read the greeting string
  greetstr_lined->setText(config->readEntry("GreetString", i18n("Welcome to %s at %n")));

  // Regular logo or clock
  QString logoArea = config->readEntry("LogoArea", "Logo" );
  if (logoArea == "Clock") {
    clockRadio->setChecked(true);
    slotAreaRadioClicked(KdmClock);
  } else if (logoArea == "Logo") {
    logoRadio->setChecked(true);
    slotAreaRadioClicked(KdmLogo);
  } else {
    noneRadio->setChecked(true);
    slotAreaRadioClicked(KdmNone);
  }

  // See if we use alternate logo
  setLogo(config->readEntry("LogoPixmap"));

  // Check the GUI type
  guicombo->setCurrentId(config->readEntry("GUIStyle"));

  // Check the Color Scheme
  colcombo->setCurrentId(config->readEntry("ColorScheme"));

  // Check the echo mode
  echocombo->setCurrentId(config->readEntry("EchoMode", "OneStar"));

  QStringList sl = config->readEntry( "GreeterPos", QStringList() );
  if (sl.count() != 2) {
    xLineEdit->setText( "50" );
    yLineEdit->setText( "50" );
  } else {
    xLineEdit->setText( sl.first() );
    yLineEdit->setText( sl.last() );
  }

  // get the language
  langcombo->setCurrentItem(config->readEntry("Language", "C"));
}


void KDMAppearanceWidget::defaults()
{
  greetstr_lined->setText( i18n("Welcome to %s at %n") );
  logoRadio->setChecked( true );
  slotAreaRadioClicked( KdmLogo );
  setLogo( "" );
  guicombo->setCurrentId( "" );
  colcombo->setCurrentId( "" );
  echocombo->setCurrentItem( "OneStar" );

  xLineEdit->setText( "50" );
  yLineEdit->setText( "50" );

  langcombo->setCurrentItem( "en_US" );
}

QString KDMAppearanceWidget::quickHelp() const
{
  return i18n("<h1>KDM - Appearance</h1> Here you can configure the basic appearance"
    " of the KDM login manager, i.e. a greeting string, an icon etc.<p>"
    " For further refinement of KDM's appearance, see the \"Font\" and \"Background\" "
    " tabs.");
}


void KDMAppearanceWidget::changed()
{
  emit changed(true);
}

#include "kdm-appear.moc"
