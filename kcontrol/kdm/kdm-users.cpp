/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

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


#include <qdragobject.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kapp.h>
#include <ksimpleconfig.h>
#include <kicondialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kimageio.h>
#include <kmessagebox.h>
#include "kdm-users.moc"


KDMUsersWidget::KDMUsersWidget(QWidget *parent, const char *name)
    : KCModule(parent, name)
{
    // WABA: This is ugly!
    QString default_pix(locate("data", "kdm/pics/users/default.png"));
    m_userPixDir = default_pix.left(default_pix.findRev('/')+1);
    m_defaultText = i18n("<default>");

    QString wtstr;
    QHBoxLayout *main = new QHBoxLayout(this, 10);
    QGridLayout *rLayout = new QGridLayout(main, 4, 3, 10);

    QLabel *a_label = new QLabel(i18n("All users"), this);
    rLayout->addWidget(a_label, 0, 0);

    QLabel *s_label = new QLabel(i18n("Selected users"), this);
    rLayout->addWidget(s_label, 0, 2);

    QLabel *n_label = new QLabel(i18n("No-show users"), this);
    rLayout->addWidget(n_label, 4, 2);

    QPushButton *all_to_no, *all_to_usr, *no_to_all, *usr_to_all;

    QSize sz(40, 20);

    all_to_usr = new QPushButton( ">>", this );
    all_to_usr->setFixedSize( sz );
    rLayout->addWidget(all_to_usr, 1, 1);
    connect( all_to_usr, SIGNAL( clicked() ), SLOT( slotAllToUsr() ) );
    connect( all_to_usr, SIGNAL(clicked()), this, SLOT(slotChanged()));
    QWhatsThis::add( all_to_usr, i18n("Click here to add the highlighted user on the left to"
      " the list of selected users on the right, i.e. users that should in any case be shown in KDM's user list.") );

    usr_to_all = new QPushButton( "<<", this );
    rLayout->addWidget(usr_to_all, 2, 1);
    usr_to_all->setFixedSize( sz );
    connect( usr_to_all, SIGNAL( clicked() ), SLOT( slotUsrToAll() ) );
    connect( usr_to_all, SIGNAL(clicked()), this, SLOT(slotChanged()));
    QWhatsThis::add( usr_to_all, i18n("Click here to remove the highlighted user from the list"
      " of selected users."));

    rLayout->setRowStretch(3, 1);

    all_to_no  = new QPushButton( ">>", this );
    rLayout->addWidget(all_to_no, 5, 1);
    all_to_no->setFixedSize( sz );
    connect( all_to_no, SIGNAL( clicked() ), SLOT( slotAllToNo() ) );
    connect( all_to_no, SIGNAL(clicked()), this, SLOT(slotChanged()));
    QWhatsThis::add( all_to_no, i18n("Click here to add the highlighted user on the left to the list"
      " of users explicitly not shown by KDM.") );

    no_to_all = new QPushButton( "<<", this );
    rLayout->addWidget(no_to_all, 6, 1);
    no_to_all->setFixedSize( sz );
    connect( no_to_all, SIGNAL( clicked() ), SLOT( slotNoToAll() ) );
    connect( no_to_all, SIGNAL(clicked()), this, SLOT(slotChanged()));
    QWhatsThis::add( no_to_all, i18n("Click here to remove the highlighted user from the list"
      " of users explicitly not shown by KDM.") );

    rLayout->setRowStretch(7, 1);

    alluserlb = new KListBox(this);
    rLayout->addMultiCellWidget(alluserlb, 1, 7, 0, 0);
    wtstr = i18n("This is the list of users for which no explicit show policy has been set,"
      " i.e. they will only be shown by KDM if the option \"Show all users but no-show users\""
      " has been set.");
    QWhatsThis::add( alluserlb, wtstr );
    QWhatsThis::add( a_label, wtstr );

    userlb = new KListBox(this);
    rLayout->addMultiCellWidget(userlb, 1, 3, 2, 2);
    wtstr = i18n("This is the list of users KDM will show in its login dialog in any case.");
    QWhatsThis::add( userlb, wtstr );
    QWhatsThis::add( s_label, wtstr );

    nouserlb = new KListBox(this);
    rLayout->addMultiCellWidget(nouserlb, 5, 7, 2, 2);
    wtstr = i18n("This is the list of users KDM will not show in its login dialog.");
    QWhatsThis::add( nouserlb, wtstr );
    QWhatsThis::add( n_label, wtstr );

    connect( userlb, SIGNAL( highlighted( const QString & ) ),
             SLOT( slotUserSelected( const QString & ) ) );
    connect( alluserlb, SIGNAL( highlighted( const QString & ) ),
             SLOT( slotUserSelected( const QString & ) ) );
    connect( nouserlb, SIGNAL( highlighted( const QString & ) ),
             SLOT( slotUserSelected( const QString & ) ) );

    QVBoxLayout *lLayout = new QVBoxLayout(main, 10);
    lLayout->addSpacing(20);

    userlabel = new QLabel(" ", this );
    userlabel->setAlignment(AlignCenter);
    lLayout->addWidget(userlabel);

    userbutton = new KIconButton(this);
    userbutton->setAcceptDrops(true);
    userbutton->installEventFilter(this); // for drag and drop
    userbutton->setFixedSize(80, 80);
    connect(userbutton, SIGNAL(iconChanged(QString)), SLOT(slotUserPixChanged(QString)));
    connect(userbutton, SIGNAL(iconChanged(QString)), this, SLOT(slotChanged()));

    QToolTip::add(userbutton, i18n("Click or drop an image here"));
    lLayout->addWidget(userbutton);
    wtstr = i18n("Here you can see the username of the currently selected user and the"
      " image assigned to this user. Click on the image button to select from a list"
      " of images or drag and drop your own image onto the button (e.g. from Konqueror).");
    QWhatsThis::add( userlabel, wtstr );
    QWhatsThis::add( userbutton, wtstr );

    usrGroup = new QButtonGroup( this );
    usrGroup->setExclusive( TRUE );
    QVBoxLayout *usrGLayout = new QVBoxLayout( usrGroup, 10 );

    rbselusr = new QRadioButton(i18n("Show only\n&selected users"), usrGroup );
    usrGroup->insert( rbselusr );
    usrGLayout->addWidget( rbselusr );
    connect(rbselusr, SIGNAL(clicked()), this, SLOT(slotChanged()));
    QWhatsThis::add(rbselusr, i18n("If this option is selected, KDM will only show the users listed"
      " in the \"selected users\" listbox in its login dialog."));

    rballusr = new QRadioButton( i18n("Show all users\nbut &no-show users"), usrGroup );
    rballusr->setGeometry( 10, 50, 140, 25 );
    usrGroup->insert( rballusr );
    usrGLayout->addWidget( rballusr );
    connect(rballusr, SIGNAL(clicked()), this, SLOT(slotChanged()));
    QWhatsThis::add( rballusr, i18n("If this option is selected, KDM will show all users but those"
      " who are listed in the \"no-show users\" listbox."));

    lLayout->addWidget( usrGroup );

    shwGroup = new QButtonGroup( this );
    QVBoxLayout *shwGLayout = new QVBoxLayout( shwGroup, 10 );

    cbusrshw = new QCheckBox(i18n("S&how users"), shwGroup);
    shwGroup->insert( cbusrshw );
    shwGLayout->addWidget( cbusrshw );
    connect(cbusrshw, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
    QWhatsThis::add(cbusrshw, i18n("If this option is selected, KDM will show a list of users in its"
      " login dialog, so users can click on their name and image rather than typing"
      " in their login."));

    cbusrsrt = new QCheckBox(i18n("S&ort users"), shwGroup);
    shwGroup->insert( cbusrsrt );
    shwGLayout->addWidget( cbusrsrt );
    connect(cbusrsrt, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
    QWhatsThis::add(cbusrsrt, i18n("This option tells KDM to alphabetically sort the users"
      " shown in its login dialog.") );

    lLayout->addWidget( shwGroup );
    lLayout->addStretch( 1 );

    load();

    // read only mode
    if (getuid() != 0)
      {
    usrGroup->setEnabled(false);
    shwGroup->setEnabled(false);
    cbusrshw->setEnabled(false);
    cbusrsrt->setEnabled(false);
    userbutton->setEnabled(false);
    alluserlb->setEnabled(false);
    nouserlb->setEnabled(false);
    userlb->setEnabled(false);
    all_to_usr->setEnabled(false);
    usr_to_all->setEnabled(false);
    no_to_all->setEnabled(false);
    all_to_no->setEnabled(false);
      }
}


void KDMUsersWidget::slotUserPixChanged(QString)
{
    QString user(userlabel->text().stripWhiteSpace());
    if(user == m_defaultText)
    {
       user = "default";
       if (KMessageBox::questionYesNo(this, i18n("Save image as default image?"))
            != KMessageBox::Yes)
          return;
    }

    QString userpix = m_userPixDir + user + ".png";
    const QPixmap *p = userbutton->pixmap();
    if(!p)
        return;
    if(!p->save(userpix, "PNG")) {
        QString msg = i18n("There was an error saving the image:\n%1\n")
        .arg(userpix);
        KMessageBox::sorry(this, msg);
    }
    userbutton->adjustSize();
}


bool KDMUsersWidget::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::DragEnter) {
        userButtonDragEnterEvent((QDragEnterEvent *) e);
        return true;
    }

    if (e->type() == QEvent::Drop) {
        userButtonDropEvent((QDropEvent *) e);
        return true;
    }

    return false;
}


void KDMUsersWidget::userButtonDragEnterEvent(QDragEnterEvent *e)
{
    e->accept(QUriDrag::canDecode(e));
}


void KDMUsersWidget::userButtonDropEvent(QDropEvent *e)
{
    QStringList uris;

    if (QUriDrag::decodeToUnicodeUris(e, uris) && (uris.count() > 0)) {
        KURL url(*uris.begin());

        QString pixpath;
        QString user(userlabel->text());
        bool istmp = false;

        QString mimetype = KImageIO::mimeType(url.url());

        if( !KImageIO::isSupported(mimetype, KImageIO::Reading) )
        {
            QString msg =  i18n("Sorry, but\n"
                "%1\n"
                "does not seem to be an image file\n"
                "The following image types are understood:\n"
                "%2")
            .arg(url.url())
            .arg(KImageIO::types(KImageIO::Reading).join(", "));
            KMessageBox::sorry( this, msg);
        } else {
            // we gotta check if it is a non-local file and make a tmp copy at the hd.
            if( url.isLocalFile() )
            {
                pixpath = url.path();
            } else {
        KIO::NetAccess::download(url, pixpath);
                istmp = true;
            }
            // Finally we've got an image file to add to the user
            QPixmap p(pixpath);
            if (istmp)
                KIO::NetAccess::removeTempFile(pixpath);
            if(!p.isNull()) { // Save image as <userpixdir>/<user>.<ext>
                userbutton->setPixmap(p);
                slotUserPixChanged(user);
            } else {
                QString msg = i18n("There was an error loading the image:\n"
                   "%1\n"
                   "It will not be saved...")
            .arg(url.prettyURL());
                KMessageBox::sorry(this, msg);
            }
        }
    }
}


void KDMUsersWidget::slotAllToNo()
{
    int id = alluserlb->currentItem();
    if (id < 0)
       return;
    QString user = alluserlb->currentText();
    if (user == m_defaultText)
       return;
    nouserlb->insertItem(user);
    nouserlb->sort();
    alluserlb->removeItem(id);
}


void KDMUsersWidget::slotNoToAll()
{
    int id = nouserlb->currentItem();
    if (id < 0)
       return;
    QString user = nouserlb->currentText();
    alluserlb->insertItem(user);
    alluserlb->sort();
    nouserlb->removeItem(id);
}


void KDMUsersWidget::slotAllToUsr()
{
    int id = alluserlb->currentItem();
    if (id < 0)
       return;
    QString user = alluserlb->currentText();
    if (user == m_defaultText)
       return;
    userlb->insertItem(user);
    userlb->sort();
    alluserlb->removeItem(id);
}


void KDMUsersWidget::slotUsrToAll()
{
    int id = userlb->currentItem();
    if (id < 0)
       return;
    QString user = userlb->currentText();
    alluserlb->insertItem(user);
    alluserlb->sort();
    userlb->removeItem(id);
}


void KDMUsersWidget::slotUserSelected(const QString &user)
{
   userlabel->setText(user);

   QString name;
   if (user == m_defaultText)
      name = m_userPixDir + "default.png";
   else
      name = m_userPixDir + user + ".png";
   QPixmap p( name );
   if(p.isNull())
      p = QPixmap(m_userPixDir+"default.png");
   userbutton->setPixmap(p);
   userbutton->adjustSize();
}


void KDMUsersWidget::save()
{
    KSimpleConfig *c = new KSimpleConfig(locate("config", "kdmrc"));

    c->setGroup("KDM");

    c->writeEntry( "UserView", cbusrshw->isChecked() );
    c->writeEntry( "SortUsers", cbusrsrt->isChecked() );

    showallusers = rballusr->isChecked();

    if(nouserlb->count() > 0) {
        QString nousrstr;
        for(uint i = 0; i < nouserlb->count(); i++) {
            nousrstr.append(nouserlb->text(i));
            nousrstr.append(",");
        }
        c->writeEntry( "NoUsers", nousrstr );
    }

    if((userlb->count() > 0) && (!showallusers)) {
        QString usrstr;
        for(uint i = 0; i < userlb->count(); i++) {
            usrstr.append(userlb->text(i));
            usrstr.append(",");
        }
        c->writeEntry( "Users", usrstr );
    }

    delete c;
}


void KDMUsersWidget::load()
{
    iconloader = KGlobal::iconLoader();
    QString str;

    // Get config object
    KSimpleConfig *c = new KSimpleConfig(locate("config", "kdmrc"));
    c->setGroup("KDM");

    // Read users from kdmrc and /etc/passwd
    QStringList users, no_users;
    users = c->readListEntry( "Users");
    showallusers = users.isEmpty();
    no_users = c->readListEntry( "NoUsers");

    userlb->clear();
    userlb->insertStringList(users);
    nouserlb->clear();
    nouserlb->insertStringList(no_users);

    QStringList allusers;
    allusers.append(m_defaultText);
    struct passwd *ps;
#define CHECK_STRING( x) (x != 0 && x[0] != 0)
    setpwent();

    // kapp->processEvents(50) makes layout calculation to fail
    // do we really need them here?

    while( (ps = getpwent()) != 0) {
        //  kapp->processEvents(50);
        if( CHECK_STRING(ps->pw_dir) && CHECK_STRING(ps->pw_shell) &&
            ( no_users.contains( ps->pw_name) == 0)) {
            //  kapp->processEvents(50);
            // we might have a real user, insert him/her
            allusers.append( QString::fromLocal8Bit(ps->pw_name));
        }
    }
    endpwent();
#undef CHECK_STRING
    allusers.sort();
    alluserlb->clear();
    alluserlb->insertStringList(allusers);

    cbusrsrt->setChecked(c->readNumEntry("SortUsers", true));
    cbusrshw->setChecked(c->readNumEntry("UserView", true));
    if (showallusers) rballusr->setChecked(true);
    else rbselusr->setChecked(true);

    delete c;
    alluserlb->setCurrentItem(0);
    slotUserSelected(alluserlb->currentText());
}


void KDMUsersWidget::defaults()
{
}


void KDMUsersWidget::slotChanged()
{
  emit KCModule::changed(true);
}
