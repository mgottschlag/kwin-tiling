/*
  $Id$
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
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/


#include <unistd.h>
#include <sys/types.h>


#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qdragobject.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qvalidator.h>
#include <qstylefactory.h>

#include <klocale.h>
#include <klineedit.h>
#include <kimageio.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kiconloader.h>

#include "kdm-appear.h"


extern KSimpleConfig *config;


KDMAppearanceWidget::KDMAppearanceWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  QString wtstr;

  QVBoxLayout *vbox = new QVBoxLayout(this, KDialog::marginHint(),
                      KDialog::spacingHint(), "vbox");
  QGroupBox *group = new QGroupBox(i18n("Appearance"), this);
  vbox->addWidget(group, 1);

  QGridLayout *grid = new QGridLayout( group, 5, 2, KDialog::spacingHint(),
                       KDialog::spacingHint(), "grid");
  grid->addRowSpacing(0, group->fontMetrics().height());
  grid->setColStretch(0, 1);
  grid->setColStretch(1, 1);

  QWidget *hlp = new QWidget( group );
  grid->addMultiCellWidget(hlp, 1,1, 0,1);
  QHBoxLayout *hlay = new QHBoxLayout( hlp, KDialog::spacingHint() );
  greetstr_lined = new KLineEdit(hlp);
  QLabel *label = new QLabel(greetstr_lined, i18n("&Greeting:"), hlp);
  hlay->addWidget(label);
  connect(greetstr_lined, SIGNAL(textChanged(const QString&)),
      SLOT(changed()));
  hlay->addWidget(greetstr_lined);
  wtstr = i18n("This is the string KDM will display in the login window. "
           "You may want to put here some nice greeting or information "
           "about the operating system.<p> KDM will replace the string "
           "'%n' with the actual host name of the computer "
           "running the X server. Especially in networks this is a good "
           "idea." );
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( greetstr_lined, wtstr );


  hlp = new QWidget( group );
  grid->addMultiCellWidget(hlp, 2,4, 0,0);
  QGridLayout *hglay = new QGridLayout( hlp, 3, 4, KDialog::spacingHint() );

  label = new QLabel(i18n("Logo area:"), hlp);
  hglay->addWidget(label, 0, 0);
  QWidget *helper = new QWidget( hlp );
  hglay->addMultiCellWidget(helper, 0,0, 1,2);
  QVBoxLayout *vlay = new QVBoxLayout( helper, KDialog::spacingHint() );
  noneRadio = new QRadioButton( i18n("&None"), helper );
  clockRadio = new QRadioButton( i18n("Show cloc&k"), helper );
  logoRadio = new QRadioButton( i18n("Sho&w logo"), helper );
  QButtonGroup *buttonGroup = new QButtonGroup( helper );
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
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( noneRadio, wtstr );
  QWhatsThis::add( logoRadio, wtstr );
  QWhatsThis::add( clockRadio, wtstr );

  logoLabel = new QLabel(i18n("&Logo:"), hlp);
  logobutton = new QPushButton(hlp);
  logoLabel->setBuddy( logobutton );
  logobutton->setAutoDefault(false);
  logobutton->setAcceptDrops(true);
  logobutton->installEventFilter(this); // for drag and drop
  logobutton->setMinimumSize(24, 24);
  logobutton->setMaximumSize(108, 108);
  connect(logobutton, SIGNAL(clicked()), SLOT(slotLogoButtonClicked()));
  hglay->addWidget(logoLabel, 1, 0);
  hglay->addWidget(logobutton, 1, 1, AlignCenter);
  hglay->addRowSpacing(1, 110);
  wtstr = i18n("Click here to choose an image that KDM will display. "
	       "You can also drag and drop an image onto this button "
	       "(e.g. from Konqueror).");
  QWhatsThis::add( logoLabel, wtstr );
  QWhatsThis::add( logobutton, wtstr );
  hglay->addRowSpacing( 2, KDialog::spacingHint());
  hglay->setColStretch( 3, 1);


  hlp = new QWidget( group );
  grid->addWidget(hlp, 2, 1);
  hglay = new QGridLayout( hlp, 2, 3, KDialog::spacingHint() );

  label = new QLabel(i18n("Position:"), hlp);
  hglay->addWidget(label, 0, 0);
  helper = new QWidget( hlp );
  hglay->addWidget(helper, 0, 1);
  vlay = new QVBoxLayout( helper, KDialog::spacingHint() );
  posCenterRadio = new QRadioButton( i18n("Cente&red"), helper );
  posSpecifyRadio = new QRadioButton( i18n("Spec&ify"), helper );
  buttonGroup = new QButtonGroup( helper );
  label->setBuddy( buttonGroup );
  connect( buttonGroup, SIGNAL(clicked(int)), SLOT(slotPosRadioClicked(int)) );
  connect( buttonGroup, SIGNAL(clicked(int)), SLOT(changed()) );
  buttonGroup->hide();
  buttonGroup->insert(posCenterRadio, 0);
  buttonGroup->insert(posSpecifyRadio, 1);
  vlay->addWidget(posCenterRadio);
  vlay->addWidget(posSpecifyRadio);
  wtstr = i18n("You can choose whether the login dialog should be centered"
	       " or placed at specified coordinates.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( posCenterRadio, wtstr );
  QWhatsThis::add( posSpecifyRadio, wtstr );
  helper = new QWidget( hlp );
  hglay->addWidget(helper, 1, 1, AlignHCenter | AlignTop);
  QGridLayout *glay = new QGridLayout( helper, 2, 2, KDialog::spacingHint() );
  QValidator *posValidator = new QIntValidator(0, 999, helper);
  xLineLabel = new QLabel(i18n("&X"), helper);
  glay->addWidget(xLineLabel, 0, 0);
  xLineEdit = new QLineEdit (helper);
  glay->addWidget(xLineEdit, 0, 1);
  xLineLabel->setBuddy(xLineEdit);
  xLineEdit->setValidator(posValidator);
  yLineLabel = new QLabel(i18n("&Y"), helper);
  glay->addWidget(yLineLabel, 1, 0);
  yLineEdit = new QLineEdit (helper);
  glay->addWidget(yLineEdit, 1, 1);
  yLineLabel->setBuddy(yLineEdit);
  yLineEdit->setValidator(posValidator);
  wtstr = i18n("Here you specify the coordinates of the login dialog's <em>center</em>.");
  QWhatsThis::add( xLineLabel, wtstr );
  QWhatsThis::add( xLineEdit, wtstr );
  QWhatsThis::add( yLineLabel, wtstr );
  QWhatsThis::add( yLineEdit, wtstr );
  hglay->setColStretch( 2, 1);
  hglay->setRowStretch( 2, 1);


  hlp = new QWidget( group );
  grid->addWidget(hlp, 3, 1);
  hglay = new QGridLayout( hlp, 2, 3, KDialog::spacingHint() );
  hglay->setColStretch(2, 1);

  guicombo = new KComboBox(false, hlp);
  guicombo->insertStringList(QStyleFactory::keys());
  label = new QLabel(guicombo, i18n("GUI s&tyle:"), hlp);
  connect(guicombo, SIGNAL(activated(int)), SLOT(changed()));
  hglay->addWidget(label, 0, 0);
  hglay->addWidget(guicombo, 0, 1);
  wtstr = i18n("You can choose a basic GUI style here that will be "
        "used by KDM only.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( guicombo, wtstr );

  label = new QLabel(i18n("Echo &mode:"), hlp);
  echocombo = new QComboBox(false, hlp);
  label->setBuddy( echocombo );
  echocombo->insertItem(i18n("No Echo"));
  echocombo->insertItem(i18n("One Star"));
  echocombo->insertItem(i18n("Three Stars"));
  connect(echocombo, SIGNAL(activated(int)), SLOT(changed()));
  hglay->addWidget(label, 1, 0);
  hglay->addWidget(echocombo, 1, 1);
  wtstr = i18n("You can choose whether and how KDM shows your password when you type it.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( echocombo, wtstr );


  // The Language group box
  group = new QGroupBox(0, Vertical, i18n("Locale"), this);
  vbox->addWidget(group, 1);

  QGridLayout *hbox = new QGridLayout( group->layout(), 2, 2, KDialog::spacingHint() );
  hbox->setColStretch(1, 1);

  label = new QLabel(i18n("Languag&e:"), group);
  hbox->addWidget(label, 1, 0);

  langcombo = new KLanguageButton(group);
  label->setBuddy( langcombo );
  langcombo->setFixedHeight( langcombo->sizeHint().height() );
  hbox->addWidget(langcombo, 1, 1);
  connect(langcombo, SIGNAL(activated(int)), SLOT(changed()));

  wtstr = i18n("Here you can choose the language used by KDM. This setting doesn't affect"
    " a user's personal settings that will take effect after login.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( langcombo, wtstr );


  vbox->addStretch(1);

  loadLanguageList(langcombo);

}

void KDMAppearanceWidget::makeReadOnly()
{
    logobutton->setEnabled(false);
    greetstr_lined->setReadOnly(true);
    noneRadio->setEnabled(false);
    clockRadio->setEnabled(false);
    logoRadio->setEnabled(false);
    posCenterRadio->setEnabled(false);
    posSpecifyRadio->setEnabled(false);
    xLineEdit->setEnabled(false);
    yLineEdit->setEnabled(false);
    guicombo->setEnabled(false);
    echocombo->setEnabled(false);
    langcombo->setEnabled(false);
}

void KDMAppearanceWidget::loadLanguageList(KLanguageButton *combo)
{
  combo->clear();
  QStringList langlist = KGlobal::dirs()->findAllResources("locale",
			QString::fromLatin1("*/entry.desktop"));
  langlist.sort();
  for ( QStringList::ConstIterator it = langlist.begin();
	it != langlist.end(); ++it )
  {
    QString fpath = (*it).left((*it).length() - 14);
    int index = fpath.findRev('/');
    QString nid = fpath.mid(index + 1);

    KSimpleConfig entry(*it);
    entry.setGroup(QString::fromLatin1("KCM Locale"));
    QString name = entry.readEntry(QString::fromLatin1("Name"), i18n("without name"));
    combo->insertLanguage(nid, name, QString::fromLatin1("l10n/"), QString::null);
  }
}

bool KDMAppearanceWidget::setLogo(QString logo)
{
    QString flogo = logo.isEmpty() ?
	locate("data", QString::fromLatin1("kdm/pics/kdelogo.png") ) :
	logo;
    QPixmap p(flogo);
    if (p.isNull())
	return false;
    logobutton->setPixmap(p);
    logobutton->adjustSize();
//    resize(width(), height());
    logopath = logo;
    return true;
}


void KDMAppearanceWidget::slotLogoButtonClicked()
{
    KImageIO::registerFormats();
    QString fileName = KFileDialog::getOpenFileName(
	locate("data", QString::fromLatin1("kdm/pics/") ),
	KImageIO::pattern());
    if (!fileName.isEmpty())
	if (setLogo(fileName))
	    changed();
}


void KDMAppearanceWidget::slotAreaRadioClicked(int id)
{
    logobutton->setEnabled( id == KdmLogo );
    logoLabel->setEnabled( id == KdmLogo );
}


void KDMAppearanceWidget::slotPosRadioClicked(int id)
{
    xLineEdit->setEnabled( id != 0 );
    xLineLabel->setEnabled( id != 0 );
    yLineEdit->setEnabled( id != 0 );
    yLineLabel->setEnabled( id != 0 );
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
  e->accept(QUriDrag::canDecode(e));
}


KURL *decodeImgDrop(QDropEvent *e, QWidget *wdg);

void KDMAppearanceWidget::iconLoaderDropEvent(QDropEvent *e)
{
    KURL pixurl;
    bool istmp;

    KURL *url = decodeImgDrop(e, this);
    if (url) {

	// we gotta check if it is a non-local file and make a tmp copy at the hd.
	if(!url->isLocalFile()) {
	    pixurl = "file:" +
		     KGlobal::dirs()->resourceDirs("data").last() +
		     "kdm/pics/" + url->fileName();
	    KIO::NetAccess::copy(*url, pixurl);
	    istmp = true;
	} else {
	    pixurl = *url;
	    istmp = false;
	}

	// By now url should be "file:/..."
	if (!setLogo(pixurl.path())) {
	    KIO::NetAccess::del(pixurl);
	    QString msg = i18n("There was an error loading the image:\n"
			       "%1\n"
			       "It will not be saved.")
			       .arg(pixurl.path());
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

  config->writeEntry("LogoPixmap", KGlobal::iconLoader()->iconPath(logopath, KIcon::Desktop, true));

  config->writeEntry("GUIStyle", guicombo->currentText());

  config->writeEntry("EchoMode", echocombo->currentItem() == 0 ? "NoEcho" :
			    echocombo->currentItem() == 1 ? "OneStar" :
							    "ThreeStars");

  config->writeEntry("GreeterPosFixed", posSpecifyRadio->isChecked());
  config->writeEntry("GreeterPosX", xLineEdit->text());
  config->writeEntry("GreeterPosY", yLineEdit->text());

  config->writeEntry("Language", langcombo->currentTag());
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
  setLogo( config->readEntry("LogoPixmap", ""));

  // Check the GUI type
  guicombo->setCurrentItem (config->readEntry("GUIStyle", "Default"), true, 0);

  // Check the echo mode
  QString echostr = config->readEntry("EchoMode", "OneStar");
  if (echostr == "ThreeStars")
    echocombo->setCurrentItem(2);
  else if (echostr == "OneStar")
    echocombo->setCurrentItem(1);
  else  // "NoEcho"
    echocombo->setCurrentItem(0);

  if (config->readBoolEntry("GreeterPosFixed", false)) {
    posSpecifyRadio->setChecked(true);
    slotPosRadioClicked(1);
  } else {
    posCenterRadio->setChecked(true);
    slotPosRadioClicked(0);
  }
  xLineEdit->setText( config->readEntry("GreeterPosX", "100"));
  yLineEdit->setText( config->readEntry("GreeterPosY", "100"));

  // get the language
  langcombo->setCurrentItem(config->readEntry("Language", "C"));
}


void KDMAppearanceWidget::defaults()
{
  greetstr_lined->setText( i18n("Welcome to %s at %n") );
  logoRadio->setChecked( true );
  slotAreaRadioClicked( KdmLogo );
  posCenterRadio->setChecked( true );
  slotPosRadioClicked( 0 );
  setLogo( "" );
  guicombo->setCurrentItem( QString::fromLatin1("Default"), true, 0 );
  echocombo->setCurrentItem( 1 );

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
