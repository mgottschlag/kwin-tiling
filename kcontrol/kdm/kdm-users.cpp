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
#include <kio_job.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include "kdm-users.moc"



// Destructor
KDMUsersWidget::~KDMUsersWidget()
{
    if(gui) {
        delete userbutton;
        delete usrGroup;
        delete shwGroup;
    }
}

KDMUsersWidget::KDMUsersWidget(QWidget *parent, const char *name, bool init)
    : KConfigWidget(parent, name)
{
    gui = !init;
    loadSettings();
    if(gui)
        setupPage(parent);
}

void KDMUsersWidget::setupPage(QWidget *)
{
    QLabel *a_label = new QLabel(i18n("All users"), this);
    a_label->setFixedSize(a_label->sizeHint());
    QLabel *s_label = new QLabel(i18n("Selected users"), this);
    s_label->setFixedSize(s_label->sizeHint());
    QLabel *n_label = new QLabel(i18n("No-show users"), this);
    n_label->setFixedSize(n_label->sizeHint());
    
    QPushButton *all_to_no, *all_to_usr, *no_to_all, *usr_to_all;
    
    all_to_usr = new QPushButton( ">>", this );
    all_to_usr->setFixedSize(30, 30);
    connect( all_to_usr, SIGNAL( clicked() ), SLOT( slotAllToUsr() ) );
    
    usr_to_all = new QPushButton( "<<", this );
    usr_to_all->setFixedSize(30, 30);
    connect( usr_to_all, SIGNAL( clicked() ), SLOT( slotUsrToAll() ) );
    
    all_to_no  = new QPushButton( ">>", this );
    all_to_no->setFixedSize(30, 30);
    connect( all_to_no, SIGNAL( clicked() ), SLOT( slotAllToNo() ) );
    
    no_to_all = new QPushButton( "<<", this );
    no_to_all->setFixedSize(30, 30);
    connect( no_to_all, SIGNAL( clicked() ), SLOT( slotNoToAll() ) );
    
    QRadioButton *rb;
    
    usrGroup = new QButtonGroup( this );
    usrGroup->setExclusive( TRUE );
    
    rb = new QRadioButton( 
        i18n("Show only\nselected users"), usrGroup );
    rb->setGeometry( 10, 10, 140, 25 );
    if(!showallusers)
        rb->setChecked(true);
    usrGroup->insert( rb, 0 );
    
    rb = new QRadioButton( 
        i18n("Show all users\n but no-show users"), usrGroup );
    rb->setGeometry( 10, 50, 140, 25 );
    if(showallusers)
        rb->setChecked(true);
    usrGroup->insert( rb, 1 );
    usrGroup->adjustSize();
    usrGroup->setMinimumSize(usrGroup->size());
    connect( usrGroup, SIGNAL( clicked( int ) ), SLOT( slotUserShowMode( int ) ) );
    
    shwGroup = new QButtonGroup( this );
    cbusrshw = new QCheckBox(
        i18n("Show users"), shwGroup);
    cbusrshw->setGeometry( 10, 10, 120, 25 );
    if(showusers)
        cbusrshw->setChecked(true);
    connect( cbusrshw, SIGNAL( toggled( bool ) ), SLOT( slotUserShow( bool ) ) );
    cbusrsrt = new QCheckBox(
        i18n("Sort users"), shwGroup);
    cbusrsrt->setGeometry( 10, 40, 120, 25 );
    if(sortusers)
        cbusrsrt->setChecked(true);
    connect( cbusrsrt, SIGNAL( toggled( bool ) ), SLOT( slotUserSort( bool ) ) );
    shwGroup->insert( cbusrshw, 0);
    shwGroup->insert( cbusrsrt, 1);
    shwGroup->adjustSize();
    shwGroup->setMinimumSize(shwGroup->size());
    
    alluserlb = new QListBox(this);
    alluserlb->insertStrList(&allusers);
    userlb = new QListBox(this);
    userlb->insertStrList(&users);
    nouserlb = new QListBox(this);
    nouserlb->insertStrList(&no_users);
    
    connect( userlb, SIGNAL( highlighted( int ) ),
             SLOT( slotUserSelected( int ) ) );
    connect( alluserlb, SIGNAL( highlighted( int ) ),
             SLOT( slotUserSelected( int ) ) );
    connect( nouserlb, SIGNAL( highlighted( int ) ),
             SLOT( slotUserSelected( int ) ) );
    
    userlabel = new QLabel( this );
    userlabel->setMinimumSize(160, 25);
    
    userbutton = new KIconLoaderButton(iconloader, this);
    userbutton->setAcceptDrops(true);
    userbutton->installEventFilter(this); // for drag and drop
    userbutton->setIcon("default.png");
    userbutton->setFixedSize(80, 80);
    connect(userbutton, SIGNAL(iconChanged(const QString&)),
            SLOT(slotUserPixChanged(const QString&)));
    QToolTip::add(userbutton, i18n("Click or drop an image here"));
    
    QBoxLayout *main = new QHBoxLayout( this, 10 );
    QBoxLayout *box1 = new QVBoxLayout();
    QBoxLayout *box2 = new QVBoxLayout();
    QBoxLayout *box3 = new QVBoxLayout();
    QBoxLayout *box4 = new QVBoxLayout();
    
    main->addLayout(box1);
    main->addLayout(box2);
    main->addLayout(box3);
    main->addLayout(box4);
    
    box1->addWidget(a_label);
    box1->addWidget(alluserlb);
    
    box2->addStretch(1);
    box2->addWidget(all_to_usr);
    box2->addStretch(1);
    box2->addWidget(usr_to_all);
    box2->addStretch(1);
    box2->addWidget(all_to_no);
    box2->addStretch(1);
    box2->addWidget(no_to_all);
    
    box3->addWidget(s_label);
    box3->addWidget(userlb);
    box3->addWidget(n_label);
    box3->addWidget(nouserlb);
    
    box4->addWidget(userlabel, 5, AlignLeft);
    box4->addWidget(userbutton, 5, AlignLeft);
    box4->addWidget(usrGroup, 5, AlignLeft);
    box4->addWidget(shwGroup, 5, AlignLeft);
    
    main->activate();
}

void KDMUsersWidget::slotUserPixChanged(const QString& )
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

bool KDMUsersWidget::eventFilter(QObject */*o*/, QEvent *e)
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
                KIOJob *iojob = new KIOJob(); // will autodelete itself
                iojob->setGUImode( KIOJob::NONE );
                iojob->copy(url.url(), pixurl.ascii());
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
                // Copy the file. NB: network transparent
                KIOJob *iojob = new KIOJob(); // will autodelete itself
                iojob->setGUImode( KIOJob::NONE );
                if(istmp)
                    iojob->move(url.url(), userpixurl.ascii());
                else
                    iojob->copy(url.url(), userpixurl.ascii());
            } else {
                msg  = i18n("There was an error loading the image:\n>");
                msg += url.path();
                msg += i18n("<\nIt will not be saved...");
                KMessageBox::sorry(this, msg);
            }
        }
    }
}

void KDMUsersWidget::slotUserShow(bool show)
{
    showusers = show;
}

void KDMUsersWidget::slotUserSort(bool sort)
{
    sortusers = sort;
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

void KDMUsersWidget::slotUserShowMode( int m )
{
    showallusers = m;
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

void KDMUsersWidget::applySettings()
{
    //debug("KDMUsersWidget::applySettings()");
    KSimpleConfig *c = new KSimpleConfig(locate("config", "kdmrc"));
    
    c->setGroup("KDM");
    
    c->writeEntry( "UserView", showusers );
    c->writeEntry( "SortUsers", sortusers );
    
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

void KDMUsersWidget::loadSettings()
{
    iconloader = KGlobal::iconLoader();
    QString str;
    
    // Get config object
    KSimpleConfig *c = new KSimpleConfig(locate("config", "kdmrc"));
    c->setGroup("KDM");
    
    // Read users from kdmrc and /etc/passwd
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
    
    struct passwd *ps;
#define CHECK_STRING( x) (x != 0 && x[0] != 0)
    setpwent();
    while( (ps = getpwent()) != 0) {
        kapp->processEvents(50);
        if( CHECK_STRING(ps->pw_dir) && CHECK_STRING(ps->pw_shell) &&
            ( no_users.contains( ps->pw_name) == 0)) {
            kapp->processEvents(50);
            // we might have a real user, insert him/her
            allusers.inSort( ps->pw_name);
        }
    }
    endpwent();
#undef CHECK_STRING
    
    sortusers = c->readNumEntry("SortUsers", true);
    showusers = c->readNumEntry("UserView", true);
    
    delete c;
}


