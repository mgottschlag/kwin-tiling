#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qvaluelist.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kaboutdialog.h>
#include <kbugreport.h>
#include <klocale.h>

#include "aboutwidget.h"
#include "version.h"

AboutWidget::AboutWidget( QWidget *parent, const char *name ) : QVBox( parent, name )
{
    const KAboutData *aboutData = kapp->aboutData();

    KAboutContainer *about = new KAboutContainer( this, "about widget" );
    QString title = "<qt><b><center>";
    title += aboutData->programName() + " " + QString::fromLatin1(klipper_version) + "</center></b>- ";
    title += aboutData->shortDescription() + " -</qt>";
    QLabel *titleLabel = new QLabel( title, about );
    titleLabel->setAlignment( titleLabel->alignment() & ~WordBreak );
    titleLabel->setMinimumHeight( 50 ); // just some vertical spacing...
    about->addWidget( titleLabel );

    QValueList<KAboutPerson> authors = aboutData->authors();
    authors += aboutData->credits();
    QValueListConstIterator<KAboutPerson> it = authors.begin();
    for ( ; it != authors.end(); ++it ) {
        KAboutPerson person = *it;
        about->addPerson( person.name(), person.emailAddress(), person.webAddress(),
                          person.task() );
    }

    QValueList<KAboutTranslator> translators = aboutData->translators();
    QValueListConstIterator<KAboutTranslator> it2 = translators.begin();
    for ( ; it2 != translators.end(); ++it2 ) {
        KAboutTranslator trans = *it2;
        about->addPerson( trans.name(), trans.emailAddress(),
                          QString::null, i18n("Translation") );
    }
    
    QWidget *widget = new QWidget( this );
    QHBoxLayout *layout = new QHBoxLayout( widget );
    m_bugReport = new QPushButton( i18n("&Report a Bug or Wish..."), 
                                   widget, "bugreport button" );
    layout->insertWidget( 0, m_bugReport, 1, AlignRight );
    connect( m_bugReport, SIGNAL( clicked() ), SLOT( slotBugreport() ));
}

AboutWidget::~AboutWidget()
{
}

void AboutWidget::slotBugreport()
{
    KBugReport bugreport( this );
    bugreport.exec();
}

#include "aboutwidget.moc"
