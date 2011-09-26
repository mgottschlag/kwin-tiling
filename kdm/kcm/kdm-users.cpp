/*
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdm-users.h"

#include "helper.h"

#include <KUrl>
#include <KComboBox>
#include <KIconDialog>
#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>
#include <KTemporaryFile>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KStandardGuiItem>
#include <KIO/NetAccess>

#include <QButtonGroup>
#include <QCheckBox>
#include <QDir>
#include <QDragEnterEvent>
#include <QEvent>
#include <QFile>
#include <QGroupBox>
#include <QIntValidator>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QStyle>
#include <QTreeWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <pwd.h>

extern KConfig *config;

extern int handleActionReply(QWidget *parent, const KAuth::ActionReply &reply);

static int executeFaceAction(QWidget *parent, const QVariantMap &helperargs)
{
    parent->setEnabled(false);

    KAuth::Action action("org.kde.kcontrol.kcmkdm.managefaces");
    action.setHelperID("org.kde.kcontrol.kcmkdm");
    action.setArguments(helperargs);

    KAuth::ActionReply reply = action.execute();

    parent->setEnabled(true);

    return handleActionReply(parent, reply);
}

KDMUsersWidget::KDMUsersWidget(QWidget *parent)
    : QWidget(parent)
{
#ifdef __linux__
    struct stat st;
    if (!stat("/etc/debian_version", &st)) { /* debian */
        defminuid = "1000";
        defmaxuid = "29999";
    } else if (!stat("/usr/portage", &st)) { /* gentoo */
        defminuid = "1000";
        defmaxuid = "65000";
    } else if (!stat("/etc/mandrake-release", &st)) { /* mandrake - check before redhat! */
        defminuid = "500";
        defmaxuid = "65000";
    } else if (!stat("/etc/redhat-release", &st)) { /* redhat */
        defminuid = "100";
        defmaxuid = "65000";
    } else /* if (!stat("/etc/SuSE-release", &st)) */ { /* suse */
        defminuid = "500";
        defmaxuid = "65000";
    }
#else
    defminuid = "1000";
    defmaxuid = "65000";
#endif

    m_userPixDir = config->group("X-*-Greeter").readEntry("FaceDir",
            QString(KStandardDirs::installPath("data") + "kdm/faces" + '/'));

    if (!getpwnam("nobody"))
        KMessageBox::sorry(this, i18n(
            "User 'nobody' does not exist. "
            "Displaying user images will not work in KDM."));

    m_defaultText = i18n("<placeholder>default</placeholder>");

    minGroup = new QGroupBox(i18nc(
        "@title:group UIDs belonging to system users like 'cron'", "System U&IDs"), this);
    minGroup->setWhatsThis(i18n(
        "Users with a UID (numerical user identification) outside this range "
        "will not be listed by KDM and this setup dialog. "
        "Note that users with the UID 0 (typically root) are not affected by "
        "this and must be explicitly excluded in \"Inverse selection\" mode."));
    QSizePolicy sp_ign_fix(QSizePolicy::Ignored, QSizePolicy::Fixed);
    QValidator *valid = new QIntValidator(0, INT_MAX, minGroup);
    QLabel *minlab = new QLabel(i18nc("UIDs", "Below:"), minGroup);
    leminuid = new KLineEdit(minGroup);
    minlab->setBuddy(leminuid);
    leminuid->setSizePolicy(sp_ign_fix);
    leminuid->setValidator(valid);
    connect(leminuid, SIGNAL(textChanged(QString)), SIGNAL(changed()));
    connect(leminuid, SIGNAL(textChanged(QString)), SLOT(slotMinMaxChanged()));
    QLabel *maxlab = new QLabel(i18nc("UIDs", "Above:"), minGroup);
    lemaxuid = new KLineEdit(minGroup);
    maxlab->setBuddy(lemaxuid);
    lemaxuid->setSizePolicy(sp_ign_fix);
    lemaxuid->setValidator(valid);
    connect(lemaxuid, SIGNAL(textChanged(QString)), SIGNAL(changed()));
    connect(lemaxuid, SIGNAL(textChanged(QString)), SLOT(slotMinMaxChanged()));
    QGridLayout *grid = new QGridLayout(minGroup);
    grid->addWidget(minlab, 0, 0);
    grid->addWidget(leminuid, 0, 1);
    grid->addWidget(maxlab, 1, 0);
    grid->addWidget(lemaxuid, 1, 1);

    usrGroup = new QGroupBox(i18nc("@title:group", "Users"), this);
    cbshowlist = new QCheckBox(i18nc("... of users", "Show list"), usrGroup);
    cbshowlist->setWhatsThis(i18n(
        "If this option is checked, KDM will show a list of users, so users can "
        "click on their name or image rather than typing in their login."));
    cbcomplete = new QCheckBox(i18nc("user ...", "Autocompletion"), usrGroup);
    cbcomplete->setWhatsThis(i18n(
        "If this option is checked, KDM will automatically complete "
        "user names while they are typed in the line edit."));
    cbinverted = new QCheckBox(i18nc(
        "@option:check mode of the user selection", "Inverse selection"), usrGroup);
    cbinverted->setWhatsThis(i18n(
        "This option specifies how the users for \"Show list\" and \"Autocompletion\" "
        "are selected in the \"Select users and groups\" list: "
        "If not checked, select only the checked users. "
        "If checked, select all non-system users, except the checked ones."));
    cbusrsrt = new QCheckBox(i18n("Sor&t users"), usrGroup);
    cbusrsrt->setWhatsThis(i18n(
        "If this is checked, KDM will alphabetically sort the user list. "
        "Otherwise users are listed in the order they appear in the password file."));
    QButtonGroup *buttonGroup = new QButtonGroup(usrGroup);
    buttonGroup->setExclusive(false);
    connect(buttonGroup, SIGNAL(buttonClicked(int)), SLOT(slotShowOpts()));
    connect(buttonGroup, SIGNAL(buttonClicked(int)), SIGNAL(changed()));
    buttonGroup->addButton(cbshowlist);
    buttonGroup->addButton(cbcomplete);
    buttonGroup->addButton(cbinverted);
    buttonGroup->addButton(cbusrsrt);
    QBoxLayout *box = new QVBoxLayout(usrGroup);
    box->addWidget(cbshowlist);
    box->addWidget(cbcomplete);
    box->addWidget(cbinverted);
    box->addWidget(cbusrsrt);

    wstack = new QStackedWidget(this);
    s_label = new QLabel(i18n("S&elect users and groups:"), this);
    s_label->setBuddy(wstack);
    optinlv = new QTreeWidget(this);
    optinlv->setRootIsDecorated(false);
    optinlv->setHeaderLabel(i18n("Selected Users"));
    optinlv->setWhatsThis(i18n(
        "KDM will show all checked users. Entries denoted with '@' are user groups. "
        "Checking a group is like checking all users in that group."));
    wstack->addWidget(optinlv);
    connect(optinlv, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SLOT(slotUpdateOptIn(QTreeWidgetItem*)));
    connect(optinlv, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SIGNAL(changed()));
    optoutlv = new QTreeWidget(this);
    optoutlv->setRootIsDecorated(false);
    optoutlv->setHeaderLabel(i18n("Excluded Users"));
    optoutlv->setWhatsThis(i18n(
        "KDM will show all non-checked non-system users. Entries denoted with '@' "
        "are user groups. Checking a group is like checking all users in that group."));
    wstack->addWidget(optoutlv);
    connect(optoutlv, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SLOT(slotUpdateOptOut(QTreeWidgetItem*)));
    connect(optoutlv, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            SIGNAL(changed()));

    faceGroup = new QGroupBox(i18nc(
        "@title:group source for user faces", "User Image Source"), this);
    faceGroup->setWhatsThis(i18n(
        "Here you can specify where KDM will obtain the images that represent users. "
        "\"System\" represents the global folder; these are the pictures you can set below. "
        "\"User\" means that KDM should read the user's $HOME/.face.icon file. "
        "The two selections in the middle define the order of preference if both sources are available."));
    rbadmonly = new QRadioButton(i18nc("@option:radio image source", "System"), faceGroup);
    rbprefadm = new QRadioButton(i18nc("@option:radio image source", "System, user"), faceGroup);
    rbprefusr = new QRadioButton(i18nc("@option:radio image source", "User, system"), faceGroup);
    rbusronly = new QRadioButton(i18nc("@option:radio image source", "User"), faceGroup);
    buttonGroup = new QButtonGroup(faceGroup);
    connect(buttonGroup, SIGNAL(buttonClicked(int)), SLOT(slotFaceOpts()));
    connect(buttonGroup, SIGNAL(buttonClicked(int)), SIGNAL(changed()));
    buttonGroup->addButton(rbadmonly);
    buttonGroup->addButton(rbprefadm);
    buttonGroup->addButton(rbprefusr);
    buttonGroup->addButton(rbusronly);
    box = new QVBoxLayout(faceGroup);
    box->addWidget(rbadmonly);
    box->addWidget(rbprefadm);
    box->addWidget(rbprefusr);
    box->addWidget(rbusronly);

    QGroupBox *picGroup = new QGroupBox(i18nc(
        "@title:group user face assignments", "User Images"), this);
    usercombo = new KComboBox(picGroup);
    usercombo->setWhatsThis(i18n("The user the image below belongs to."));
    connect(usercombo, SIGNAL(activated(int)),
            SLOT(slotUserSelected()));
    QLabel *userlabel = new QLabel(i18n("User:"), picGroup);
    userlabel->setBuddy(usercombo);
    userbutton = new QPushButton(picGroup);
    userbutton->setAcceptDrops(true);
    userbutton->installEventFilter(this); // for drag and drop
    uint sz = style()->pixelMetric(QStyle::PM_ButtonMargin) * 2 + 48;
    userbutton->setFixedSize(sz, sz);
    connect(userbutton, SIGNAL(clicked()),
            SLOT(slotUserButtonClicked()));
    userbutton->setToolTip(i18n("Click or drop an image here"));
    userbutton->setWhatsThis(i18n(
        "Here you can see the image assigned to the user selected in the combo "
        "box above. Click on the image button to select from a list of images "
        "or drag and drop your own image on to the button (e.g. from Konqueror)."));
    rstuserbutton = new QPushButton(i18nc(
        "@action:button assign default user face", "R&eset"), picGroup);
    rstuserbutton->setWhatsThis(i18n(
        "Click this button to make KDM use the default image for the selected user."));
    connect(rstuserbutton, SIGNAL(clicked()),
            SLOT(slotUnsetUserPix()));
    QGridLayout *hlpl = new QGridLayout(picGroup);
    hlpl->setSpacing(KDialog::spacingHint());
    hlpl->addWidget(userlabel, 0, 0);
    hlpl->addWidget(usercombo, 0, 1); // XXX this makes the layout too wide
    hlpl->addWidget(userbutton, 1, 0, 1, 2, Qt::AlignHCenter);
    hlpl->addWidget(rstuserbutton, 2, 0, 1, 2, Qt::AlignHCenter);

    QHBoxLayout *main = new QHBoxLayout(this);
    main->setSpacing(10);

    QVBoxLayout *lLayout = new QVBoxLayout();
    main->addItem(lLayout);
    lLayout->setSpacing(10);
    lLayout->addWidget(minGroup);
    lLayout->addWidget(usrGroup);
    lLayout->addStretch(1);

    QVBoxLayout *mLayout = new QVBoxLayout();
    main->addItem(mLayout);
    mLayout->setSpacing(10);
    mLayout->addWidget(s_label);
    mLayout->addWidget(wstack);
    mLayout->setStretchFactor(wstack, 1);
    main->setStretchFactor(mLayout, 1);

    QVBoxLayout *rLayout = new QVBoxLayout();
    main->addItem(rLayout);
    rLayout->setSpacing(10);
    rLayout->addWidget(faceGroup);
    rLayout->addWidget(picGroup);
    rLayout->addStretch(1);
}

void KDMUsersWidget::slotShowOpts()
{
    bool en = cbshowlist->isChecked() || cbcomplete->isChecked();
    cbinverted->setEnabled(en);
    cbusrsrt->setEnabled(en);
    wstack->setEnabled(en);
    wstack->setCurrentWidget(cbinverted->isChecked() ? optoutlv : optinlv);
    en = cbshowlist->isChecked();
    faceGroup->setEnabled(en);
    if (!en) {
        usercombo->setEnabled(false);
        userbutton->setEnabled(false);
        rstuserbutton->setEnabled(false);
    } else
        slotFaceOpts();
}

void KDMUsersWidget::slotFaceOpts()
{
    bool en = !rbusronly->isChecked();
    usercombo->setEnabled(en);
    userbutton->setEnabled(en);
    if (en)
        slotUserSelected();
    else
        rstuserbutton->setEnabled(false);
}

void KDMUsersWidget::slotUserSelected()
{
    QString user = usercombo->currentText();
    QImage p;
    if (user != m_defaultText && p.load(m_userPixDir + user + ".face.icon"))
        rstuserbutton->setEnabled(true);
    else {
        p.load(m_userPixDir + ".default.face.icon");
        rstuserbutton->setEnabled(false);
    }
    userbutton->setIcon(QPixmap::fromImage(p.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}


void KDMUsersWidget::checkFacesDir()
{
    QDir testDir(m_userPixDir);
    if (!testDir.exists()) {
        QVariantMap helperargs;
        helperargs["subaction"] = Helper::CreateFacesDir;

        if (executeFaceAction(parentWidget(), helperargs))
            KMessageBox::sorry(this,
                i18n("Unable to create folder %1", testDir.absolutePath()));
    }
}

void KDMUsersWidget::changeUserPix(const QString &pix)
{
    QString user(usercombo->currentText());

    checkFacesDir();

    if (user == m_defaultText) {
        user = ".default";
        if (KMessageBox::questionYesNo(this, i18n("Save image as default?"),
                                       QString(), KStandardGuiItem::save(),
                                       KStandardGuiItem::cancel()) != KMessageBox::Yes)
            return;
    }

    QImage p(pix);
    if (p.isNull()) {
        KMessageBox::sorry(this,
            i18n("There was an error while loading the image\n%1", pix));
        return;
    }

    p = p.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    KTemporaryFile sourceFile;
    sourceFile.open();
    QString source = sourceFile.fileName();
    p.save(source, "PNG");
    QFile::setPermissions(source, sourceFile.permissions() | QFile::ReadOther);

    QVariantMap helperargs;
    helperargs["subaction"] = Helper::InstallFace;
    helperargs["user"] = user;
    helperargs["sourcefile"] = source;

    if (executeFaceAction(parentWidget(), helperargs))
        KMessageBox::error(this,
            i18n("There was an error while saving the image:\n%1",
                 m_userPixDir + user + ".face.icon"));

    slotUserSelected();
}

void KDMUsersWidget::slotUserButtonClicked()
{
    KIconDialog dlg;
    dlg.setCustomLocation(KStandardDirs::installPath("data") + "kdm/pics/users");
    dlg.setup(KIconLoader::NoGroup, KIconLoader::Any, false, 48, true, true, false);
    QString ic = dlg.openDialog();
    if (ic.isEmpty())
        return;
    changeUserPix(ic);
}

void KDMUsersWidget::slotUnsetUserPix()
{
    QString user(usercombo->currentText());

    checkFacesDir();

    QVariantMap helperargs;
    helperargs["subaction"] = Helper::RemoveFace;
    helperargs["user"] = user;

    if (executeFaceAction(parentWidget(), helperargs))
        KMessageBox::error(this,
            i18n("There was an error while removing the image:\n%1",
                 m_userPixDir + user + ".face.icon"));

    slotUserSelected();
}

bool KDMUsersWidget::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::DragEnter) {
        QDragEnterEvent *ee = (QDragEnterEvent *)e;
        ee->setAccepted(KUrl::List::canDecode(ee->mimeData()));
        return true;
    }

    if (e->type() == QEvent::Drop) {
        userButtonDropEvent((QDropEvent *)e);
        return true;
    }

    return false;
}

KUrl *decodeImgDrop(QDropEvent *e, QWidget *wdg);

void KDMUsersWidget::userButtonDropEvent(QDropEvent *e)
{
    KUrl *url = decodeImgDrop(e, this);
    if (url) {
        QString pixpath;
        KIO::NetAccess::download(*url, pixpath, parentWidget());
        changeUserPix(pixpath);
        KIO::NetAccess::removeTempFile(pixpath);
        delete url;
    }
}

void KDMUsersWidget::save()
{
    KConfigGroup configGrp = config->group("X-*-Greeter");

    configGrp.writeEntry("MinShowUID", leminuid->text());
    configGrp.writeEntry("MaxShowUID", lemaxuid->text());

    configGrp.writeEntry("UserList", cbshowlist->isChecked());
    configGrp.writeEntry("UserCompletion", cbcomplete->isChecked());
    configGrp.writeEntry("ShowUsers",
                         cbinverted->isChecked() ? "NotHidden" : "Selected");
    configGrp.writeEntry("SortUsers", cbusrsrt->isChecked());

    configGrp.writeEntry("HiddenUsers", hiddenUsers);
    configGrp.writeEntry("SelectedUsers", selectedUsers);

    configGrp.writeEntry("FaceSource",
                         rbadmonly->isChecked() ? "AdminOnly" :
                         rbprefadm->isChecked() ? "PreferAdmin" :
                         rbprefusr->isChecked() ? "PreferUser" : "UserOnly");
}


void KDMUsersWidget::updateOptList(QTreeWidgetItem *item, QStringList &list)
{
    if (!item)
        return;
    int ind = list.indexOf(item->text(0));
    if (item->checkState(0) == Qt::Checked) {
        if (ind < 0)
            list.append(item->text(0));
    } else {
        if (ind >= 0)
            list.removeAt(ind);
    }
}

void KDMUsersWidget::slotUpdateOptIn(QTreeWidgetItem *item)
{
    updateOptList(item, selectedUsers);
}

void KDMUsersWidget::slotUpdateOptOut(QTreeWidgetItem *item)
{
    updateOptList(item, hiddenUsers);
}

void KDMUsersWidget::slotClearUsers()
{
    optinlv->clear();
    optoutlv->clear();
    usercombo->clear();
    usercombo->addItem(m_defaultText);
}

void KDMUsersWidget::slotAddUsers(const QMap<QString, int> &users)
{
    QMap<QString, int>::const_iterator it;
    for (it = users.begin(); it != users.end(); ++it) {
        const QString *name = &it.key();
        (new QTreeWidgetItem(optinlv, QStringList() << *name))->
            setCheckState(0, selectedUsers.contains(*name) ? Qt::Checked : Qt::Unchecked);
        (new QTreeWidgetItem(optoutlv, QStringList() << *name))->
            setCheckState(0, hiddenUsers.contains(*name) ? Qt::Checked : Qt::Unchecked);
        if ((*name)[0] != '@')
            usercombo->addItem(*name);
    }
    optinlv->sortItems(0, Qt::AscendingOrder);
    optoutlv->sortItems(0, Qt::AscendingOrder);
    usercombo->model()->sort(0);
    slotUserSelected();
}

void KDMUsersWidget::slotDelUsers(const QMap<QString, int> &users)
{
    QMap<QString, int>::const_iterator it;
    for (it = users.begin(); it != users.end(); ++it) {
        const QString *name = &it.key();
        int idx = usercombo->findText(*name);
        if (idx != -1)
            usercombo->removeItem(idx);
        qDeleteAll(optinlv->findItems(*name, Qt::MatchExactly | Qt::MatchCaseSensitive));
        qDeleteAll(optoutlv->findItems(*name, Qt::MatchExactly | Qt::MatchCaseSensitive));
    }
}

void KDMUsersWidget::load()
{
    QString str;

    KConfigGroup configGrp = config->group("X-*-Greeter");

    selectedUsers = configGrp.readEntry("SelectedUsers", QStringList());
    hiddenUsers = configGrp.readEntry("HiddenUsers", QStringList());

    leminuid->setText(configGrp.readEntry("MinShowUID", defminuid));
    lemaxuid->setText(configGrp.readEntry("MaxShowUID", defmaxuid));

    cbshowlist->setChecked(configGrp.readEntry("UserList", true));
    cbcomplete->setChecked(configGrp.readEntry("UserCompletion", false));
    cbinverted->setChecked(configGrp.readEntry("ShowUsers") != "Selected");
    cbusrsrt->setChecked(configGrp.readEntry("SortUsers", true));

    QString ps = configGrp.readEntry("FaceSource");
    if (ps == QLatin1String("UserOnly"))
        rbusronly->setChecked(true);
    else if (ps == QLatin1String("PreferUser"))
        rbprefusr->setChecked(true);
    else if (ps == QLatin1String("PreferAdmin"))
        rbprefadm->setChecked(true);
    else
        rbadmonly->setChecked(true);

    slotUserSelected();

    slotShowOpts();
    slotFaceOpts();
}

void KDMUsersWidget::defaults()
{
    leminuid->setText(defminuid);
    lemaxuid->setText(defmaxuid);
    cbshowlist->setChecked(true);
    cbcomplete->setChecked(false);
    cbinverted->setChecked(true);
    cbusrsrt->setChecked(true);
    rbadmonly->setChecked(true);
    hiddenUsers.clear();
    selectedUsers.clear();
    slotShowOpts();
    slotFaceOpts();
}

void KDMUsersWidget::slotMinMaxChanged()
{
    emit setMinMaxUID(leminuid->text().toInt(), lemaxuid->text().toInt());
}

#include "kdm-users.moc"
