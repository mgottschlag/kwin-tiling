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

#include <qdragobject.h>

#include "utils.h"
#include <kio/job.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include "kdm-users.moc"


KDMUsersWidget::KDMUsersWidget(QWidget *parent, const char *name)
    : KCModule(parent, name)
{
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

    usr_to_all = new QPushButton( "<<", this );
    rLayout->addWidget(usr_to_all, 2, 1);
    usr_to_all->setFixedSize( sz );
    connect( usr_to_all, SIGNAL( clicked() ), SLOT( slotUsrToAll() ) );
    connect( usr_to_all, SIGNAL(clicked()), this, SLOT(slotChanged()));

    rLayout->setRowStretch(3, 1);

    all_to_no  = new QPushButton( ">>", this );
    rLayout->addWidget(all_to_no, 5, 1);
    all_to_no->setFixedSize( sz );
    connect( all_to_no, SIGNAL( clicked() ), SLOT( slotAllToNo() ) );
    connect( all_to_no, SIGNAL(clicked()), this, SLOT(slotChanged()));

    no_to_all = new QPushButton( "<<", this );
    rLayout->addWidget(no_to_all, 6, 1);
    no_to_all->setFixedSize( sz );
    connect( no_to_all, SIGNAL( clicked() ), SLOT( slotNoToAll() ) );
    connect( no_to_all, SIGNAL(clicked()), this, SLOT(slotChanged()));

    rLayout->setRowStretch(7, 1);

    alluserlb = new QListBox(this);
    rLayout->addMultiCellWidget(alluserlb, 1, 7, 0, 0);

    userlb = new QListBox(this);
    rLayout->addMultiCellWidget(userlb, 1, 3, 2, 2);

    nouserlb = new QListBox(this);
    rLayout->addMultiCellWidget(nouserlb, 5, 7, 2, 2);

    connect( userlb, SIGNAL( highlighted( int ) ),
             SLOT( slotUserSelected( int ) ) );
    connect( alluserlb, SIGNAL( highlighted( int ) ),
             SLOT( slotUserSelected( int ) ) );
    connect( nouserlb, SIGNAL( highlighted( int ) ),
             SLOT( slotUserSelected( int ) ) );

    QRadioButton *rb;
    QVBoxLayout *lLayout = new QVBoxLayout(main, 10);
    lLayout->addSpacing(20);

    userlabel = new QLabel(" ", this );
    userlabel->setAlignment(AlignCenter);
    lLayout->addWidget(userlabel);

    userbutton = new KIconButton(this);
    userbutton->setAcceptDrops(true);
    userbutton->installEventFilter(this); // for drag and drop
    userbutton->setIcon("default");
    userbutton->setFixedSize(80, 80);
    connect(userbutton, SIGNAL(iconChanged(QString)), SLOT(slotUserPixChanged(QString)));
    connect(userbutton, SIGNAL(iconChanged(QString)), this, SLOT(slotChanged()));
    QToolTip::add(userbutton, i18n("Click or drop an image here"));
    lLayout->addWidget(userbutton);

    usrGroup = new QButtonGroup( this );
    usrGroup->setExclusive( TRUE );
    QVBoxLayout *usrGLayout = new QVBoxLayout( usrGroup, 10 );

    rb = new QRadioButton(i18n("Show only\nselected users"), usrGroup );
    usrGroup->insert( rb );
    usrGLayout->addWidget( rb );
    connect(rb, SIGNAL(clicked()), this, SLOT(slotChanged()));

    rb = new QRadioButton( i18n("Show all users\nbut no-show users"), usrGroup );
    rb->setGeometry( 10, 50, 140, 25 );
    usrGroup->insert( rb );
    usrGLayout->addWidget( rb );
    connect(rb, SIGNAL(clicked()), this, SLOT(slotChanged()));

    lLayout->addWidget( usrGroup );

    shwGroup = new QButtonGroup( this );
    QVBoxLayout *shwGLayout = new QVBoxLayout( shwGroup, 10 );

    cbusrshw = new QCheckBox(i18n("Show users"), shwGroup);
    shwGroup->insert( cbusrshw );
    shwGLayout->addWidget( cbusrshw );
    connect(cbusrshw, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));

    cbusrsrt = new QCheckBox(i18n("Sort users"), shwGroup);
    shwGroup->insert( cbusrsrt );
    shwGLayout->addWidget( cbusrsrt );
    connect(cbusrsrt, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));

    lLayout->addWidget( shwGroup );
    lLayout->addStretch( 1 );

    load();
}


void KDMUsersWidget::slotUserPixChanged(QString)
{
    debug("KDMUsersWidget::slotUserPixChanged()");
    QString msg, user(userlabel->text());
    if(user.isEmpty()) {
        if (KMessageBox::questionYesNo(this, i18n("Save image as default image?"))
            == KMessageBox::Yes)
            user = "default";
        else
            return;
    }
    QString userpix = locate("icon", user + ".png");
    const QPixmap *p = userbutton->pixmap();
    if(!p)
        return;
    if(!p->save(userpix, "PNG")) {
        msg  = i18n("There was an error saving the image:\n>");
        msg += userpix;
        msg += i18n("<");
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

        QString filename = url.filename();
        QString msg, userpixname;
        QStringList dirs = KGlobal::dirs()->findDirs("data", "kdm/pics/");
        QString local = KGlobal::dirs()->saveLocation("data", "kdm/pics/", false);
        QStringList::ConstIterator it = dirs.begin();
        if ((*it).left(local.length()) == local)
            it++;
        QString pixurl("file:"+ *it);
        QString user(userlabel->text());
        QString userpixurl = pixurl + "users/";
        int last_dot_idx = filename.findRev('.');
        bool istmp = false;

        // CC: Now check for the extension
        QString ext(".png .xpm .xbm");
        //#ifdef HAVE_LIBGIF
        ext += " .gif";
        //#endif
#ifdef HAVE_LIBJPEG
        ext += " .jpg";
#endif

        if( !ext.contains(filename.right(filename.length()-
                                         last_dot_idx), false) ) {
            msg =  i18n("Sorry, but \n");
            msg += filename;
            msg += i18n("\ndoes not seem to be an image file");
            msg += i18n("\nPlease use files with these extensions\n");
            msg += ext;
            KMessageBox::sorry( this, msg);
        } else {
            // we gotta check if it is a non-local file and make a tmp copy at the hd.
            if( !url.isLocalFile() ) {
                pixurl += url.filename();
		KIO::file_copy(url, pixurl);
                url = pixurl;
                istmp = true;
            }
            // By now url should be "file:/..."
            if (user.isEmpty()) {
                if (KMessageBox::questionYesNo(this, i18n("Save image as default image?"))
                    == KMessageBox::Yes)
                    user = "default";
                else
                    return;
            }
            // Finally we've got an image file to add to the user
            QPixmap p(url.path());
            if(!p.isNull()) { // Save image as <userpixdir>/<user>.<ext>
                userbutton->setPixmap(p);
                userbutton->adjustSize();
                userpixurl += user;
                userpixurl += filename.right( filename.length()-(last_dot_idx) );
                //debug("destination: %s", userpixurl.ascii());
                if(istmp)
                    KIO::file_move(url, userpixurl);
                else
                    KIO::file_copy(url, userpixurl);
            } else {
                msg  = i18n("There was an error loading the image:\n>");
                msg += url.path();
                msg += i18n("<\nIt will not be saved...");
                KMessageBox::sorry(this, msg);
            }
        }
    }
}


void KDMUsersWidget::slotAllToNo()
{
    int id = alluserlb->currentItem();
    if(id >= 0) {
        nouserlb->inSort(alluserlb->text(id));
        alluserlb->removeItem(id);
    }
}


void KDMUsersWidget::slotNoToAll()
{
    int id = nouserlb->currentItem();
    if(id >= 0) {
        alluserlb->inSort(nouserlb->text(id));
        nouserlb->removeItem(id);
    }
}


void KDMUsersWidget::slotAllToUsr()
{
    int id = alluserlb->currentItem();
    if(id >=0) {
        userlb->inSort(alluserlb->text(id));
        alluserlb->removeItem(id);
    }
}


void KDMUsersWidget::slotUsrToAll()
{
    int id = userlb->currentItem();
    if(id >= 0) {
        alluserlb->inSort(userlb->text(id));
        userlb->removeItem(id);
    }
}


void KDMUsersWidget::slotUserSelected(int)
{
    QString default_pix(locate("data", "kdm/pics/users/default.png"));
    QString user_pix_dir = default_pix.left(default_pix.findRev('/')-1);

    QString name;
    QListBox *lb;

    // Get the listbox with the focus
    // If this is not a listbox we segfault :-(
    QWidget *w = kapp->focusWidget();
    if(!w)               // Had to add this otherwise I can't find the listbox
        w = focusWidget(); // when the app is swallowed in kcontrol.

    // Maybe this is enough?
    if(w->isA("QListBox")) {
        kapp->processEvents();
        // There seems to be an error in QListBox. When a listbox item is selected
        // for the first time it emits a "highlighted()" signal with the correct
        // index an emmidiatly after a signal with index 0.
        // Therefore I have to use currentItem instead of the index emitted.
        lb = (QListBox*)w;
        name = user_pix_dir + lb->text(lb->currentItem()) + ".png";
        QPixmap p( name );
        if(p.isNull())
            p = QPixmap(default_pix);
        userbutton->setPixmap(p);
        userbutton->adjustSize();
        userlabel->setText(lb->text(lb->currentItem()));
    }
    else
        debug("Not a QListBox");
}


void KDMUsersWidget::save()
{
    //debug("KDMUsersWidget::applySettings()");
    KSimpleConfig *c = new KSimpleConfig(locate("config", "kdmrc"));

    c->setGroup("KDM");

    c->writeEntry( "UserView", cbusrshw->isChecked() );
    c->writeEntry( "SortUsers", cbusrsrt->isChecked() );

    if(nouserlb->count() > 0) {
        QString nousrstr;
        for(uint i = 0; i < nouserlb->count(); i++) {
            nousrstr.append(nouserlb->text(i));
            nousrstr.append(";");
        }
        c->writeEntry( "NoUsers", nousrstr );
    }

    if((userlb->count() > 0) && (!showallusers)) {
        QString usrstr;
        for(uint i = 0; i < userlb->count(); i++) {
            usrstr.append(userlb->text(i));
            usrstr.append(";");
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
    QStrList users, no_users;
    str = c->readEntry( "Users");
    if(!str.isEmpty()) {
        semsplit( str, users);
        showallusers = false;
    }
    else
        showallusers = true;
    str = c->readEntry( "NoUsers");
    if(!str.isEmpty())
        semsplit( str, no_users);	
    userlb->clear();
    userlb->insertStrList(&users);
    nouserlb->clear();
    nouserlb->insertStrList(&no_users);

    QStrList allusers;
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
            allusers.inSort( ps->pw_name);
        }
    }
    endpwent();
#undef CHECK_STRING
    alluserlb->clear();
    alluserlb->insertStrList(&allusers);

    cbusrsrt->setChecked(c->readNumEntry("SortUsers", true));
    cbusrshw->setChecked(c->readNumEntry("UserView", true));
    usrGroup->setButton(showallusers ? 0 : 1);

    delete c;
}


void KDMUsersWidget::defaults()
{
}


void KDMUsersWidget::slotChanged()
{
  emit KCModule::changed(true);
}
