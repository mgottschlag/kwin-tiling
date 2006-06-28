/*****************************************************************

Copyright (c) 2000 Matthias Elter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#undef Bool // For enable-final

#include <QMouseEvent>

#include <klocale.h>
#include <kwinmodule.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kprocess.h>
#include <kshell.h>
#include <kwin.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kglobal.h>

#include "dockbarextension.h"
#include "dockbarextension.moc"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <QX11Info>

extern "C"
{
    KDE_EXPORT KPanelExtension* init(QWidget *parent, const QString& configFile)
    {
        KGlobal::locale()->insertCatalog("dockbarextension");
        return new DockBarExtension(configFile, 0, parent);
    }
}

DockBarExtension::DockBarExtension(const QString& configFile,
				   int actions, QWidget *parent)
  : KPanelExtension(configFile, actions, parent)
{
    dragging_container = 0;
    kwin_module = new KWinModule(this);
    connect( kwin_module, SIGNAL( windowAdded(WId) ), SLOT( windowAdded(WId) ) );
    setMinimumSize(DockContainer::sz(), DockContainer::sz());
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    loadContainerConfig();
}

DockBarExtension::~DockBarExtension()
{
    // kill nicely the applets
    for (DockContainer::Vector::const_iterator it = containers.constBegin();
         it != containers.constEnd();
         ++it)
    {
        (*it)->kill();
    }

    if (dragging_container) delete dragging_container;
}

QSize DockBarExtension::sizeHint(Plasma::Position p, QSize) const
{
    if (p == Plasma::Left || p == Plasma::Right)
	return QSize(DockContainer::sz(), DockContainer::sz() * containers.count());
    else
	return QSize(DockContainer::sz() * containers.count(), DockContainer::sz());
}

void DockBarExtension::resizeEvent(QResizeEvent*)
{
    layoutContainers();   
}


void DockBarExtension::windowAdded(WId win)
{
    // try to read WM_COMMAND
    int argc;
    char **argv;
    QString command;
    if (XGetCommand(QX11Info::display(), win, &argv, &argc)) {
	command = KShell::joinArgs(argv, argc);
	XFreeStringList(argv);
    }

    // try to read wm hints
    WId resIconwin = 0;
    XWMHints *wmhints = XGetWMHints(QX11Info::display(), win);
    if (0 != wmhints) { // we managed to read wm hints
	// read IconWindowHint
        bool is_valid = false;
        /* a good dockapp set the icon hint and the state hint,
           if it uses its icon, the window initial state must be "withdrawn"
           if not, then the initial state must be "normal"
           this filters the problematic Eterm whose initial state is "normal" 
           and which has an iconwin.
        */
	if ((wmhints->flags & IconWindowHint) &&
            (wmhints->flags & StateHint)) {
            resIconwin = wmhints->icon_window;
            is_valid = (resIconwin && wmhints->initial_state == 0) ||
                (resIconwin == 0 && wmhints->initial_state == 1);

        /* an alternative is a window who does not have an icon,
           but whose initial state is set to "withdrawn". This has been 
           added for wmxmms... I hope it won't swallow to much windows :-/ 
        */
        } else if ((wmhints->flags & IconWindowHint) == 0 &&
                   (wmhints->flags & StateHint)) {
            is_valid = (wmhints->initial_state == 0);
        }
        XFree(wmhints);
        if (!is_valid) 
            return; // we won't swallow this one
    }
    else
	return;

    // The following if statement was at one point commented out,
    // without a comment as to why. This caused problems like
    // Eterm windows getting swallowed whole. So, perhaps now I'll
    // get bug reports about whatever commenting it out was supposed
    // to fix.
    if (resIconwin == 0)
	resIconwin = win;

    // try to read class hint
    XClassHint hint;
    QString resClass, resName;
    if (XGetClassHint(QX11Info::display(), win, &hint)) {
        resName =  hint.res_name;
        resClass = hint.res_class;
    }
    else {
        kDebug() << "Could not read XClassHint of window " << win << endl;
        return;
    }
    /* withdrawing the window prevents kwin from managing the window,
       which causes the double-launch bug (one instance from the kwin 
       session, and one from the dockbar) bug when kde is restarted */
    if (resIconwin != win) {
		QX11Info info;
        XWithdrawWindow( QX11Info::display(), win, info.screen() );
        while( KWin::windowInfo(win, NET::XAWMState).mappingState() != NET::Withdrawn );
    }

    // add a container
    embedWindow(resIconwin, command.isNull() ? resClass : command, resName, resClass);
    saveContainerConfig();
}

void DockBarExtension::layoutContainers()
{
    int i = 0;
    for (DockContainer::Vector::const_iterator it = containers.constBegin();
         it != containers.constEnd();
         ++it)
    {
        if (orientation() == Qt::Horizontal)
            (*it)->move(DockContainer::sz() * i, 0);
        else
            (*it)->move(0, DockContainer::sz() * i);
        i++;
    }
}

void DockBarExtension::embedWindow(WId win, QString command, QString resName, QString resClass)
{
    if (win == 0) return; 
    DockContainer* container = 0;
   
    for (DockContainer::Vector::const_iterator it = containers.constBegin();
         it != containers.constEnd();
         ++it)
    {
        DockContainer* c = *it;
        if (c->embeddedWinId() == 0 && c->resName() == resName && c->resClass() == resClass) {
            container = c;
            break;
        }
    }
    
    if (container == 0) {
	container = new DockContainer(command, this, resName, resClass);
	addContainer(container);
    }
    
    container->embed(win);
    layoutContainers();
    emit updateLayout();
    if (KStandardDirs::findExe(KShell::splitArgs(container->command()).front()).isEmpty()) {
        container->askNewCommand();
    }
 }

void DockBarExtension::addContainer(DockContainer* c, int pos)
{
    if (pos == -1)
    {
        containers.append(c);
    }
    else
    {
        DockContainer::Vector::iterator it = containers.begin();

        for (int i = 0; i < pos && it != containers.end(); ++i)
        {
            ++it;
        }
        ++it;

        containers.insert(it, c);
    }
    connect(c, SIGNAL(embeddedWindowDestroyed(DockContainer*)), 
            SLOT(embeddedWindowDestroyed(DockContainer*)));
    connect(c, SIGNAL(settingsChanged(DockContainer*)), 
            SLOT(settingsChanged(DockContainer*)));
    c->resize(DockContainer::sz(), DockContainer::sz());
    c->show();
}

void DockBarExtension::removeContainer(DockContainer* c)
{
    DockContainer::Vector::iterator it = qFind(containers.begin(), containers.end(), c);

    if (it == containers.end())
    {
        return;
    }

    containers.erase(it);
    delete c;
    layoutContainers();
}

void DockBarExtension::embeddedWindowDestroyed(DockContainer* c)
{
    removeContainer(c);
    saveContainerConfig();
    emit updateLayout();
}

void DockBarExtension::settingsChanged(DockContainer *)
{
    saveContainerConfig();
}

void DockBarExtension::saveContainerConfig()
{
    QStringList applet_list;
    KConfig *conf = config();
    unsigned count = 0;

    for (DockContainer::Vector::const_iterator it = containers.constBegin();
         it != containers.constEnd();
         ++it)
    {
        DockContainer* c = *it;
        if (!c->command().isEmpty())
        {
            QString applet_gid = QString("Applet_%1").arg(QString::number(count));
            applet_list.append(applet_gid);
            conf->setGroup(applet_gid);
            conf->writePathEntry("Command", c->command());
            conf->writePathEntry("resName", c->resName());
            conf->writeEntry("resClass", c->resClass());
            ++count;
        }
    }
    conf->setGroup("General");
    conf->writeEntry("Applets", applet_list);
    conf->deleteEntry("Commands"); // cleanup old config    
    conf->sync();
}

void DockBarExtension::loadContainerConfig()
{
    KConfig *conf = config();
    conf->setGroup("General");
    QStringList applets = conf->readEntry("Applets", QStringList() );
    
    QStringList fail_list;
    for (QStringList::Iterator it = applets.begin(); it != applets.end(); ++it) {
        if (!conf->hasGroup(*it)) continue;
        conf->setGroup(*it);
        QString cmd = conf->readPathEntry("Command");
        QString resName  = conf->readPathEntry("resName");
        QString resClass = conf->readEntry("resClass");
        if (cmd.isEmpty() || resName.isEmpty() || resClass.isEmpty()) continue;

        DockContainer* c = new DockContainer(cmd, this, resName, resClass );
        addContainer(c);
	
        KProcess proc;
        proc << KShell::splitArgs( cmd );
        if (!proc.start(KProcess::DontCare)) {
            fail_list.append(cmd);
            removeContainer(c);
        }
    }
    if (!fail_list.isEmpty())
        KMessageBox::queuedMessageBox(0, KMessageBox::Information, i18n("The following dockbar applets could not be started: %1", fail_list.join(", ")), i18n("kicker: information"), 0);
    saveContainerConfig();
}

int DockBarExtension::findContainerAtPoint(const QPoint& p)
{
    int i = 0;
    for (DockContainer::Vector::const_iterator it = containers.constBegin();
         it != containers.constEnd();
         ++it, ++i)
    {
        if ((*it)->geometry().contains(p))
        {
            return i;
        }
    }

    return -1;
}

void DockBarExtension::mousePressEvent(QMouseEvent *e ) {
    if (e->button() == Qt::LeftButton) {
        // Store the position of the mouse clic.
        mclic_pos = e->pos();
    } else if (e->button() == Qt::RightButton) {
        int pos = findContainerAtPoint(e->pos());
        if (pos != -1) containers.at(pos)->popupMenu(e->globalPos());
    }
}

void DockBarExtension::mouseReleaseEvent(QMouseEvent *e ) {
    if (e->button() != Qt::LeftButton) return;
    if (dragging_container) {  
        releaseMouse();
        original_container->embed(dragging_container->embeddedWinId());
        delete dragging_container; dragging_container = 0;
        layoutContainers();
        saveContainerConfig();
    }
}

void DockBarExtension::mouseMoveEvent(QMouseEvent *e) {
    if (! (e->state() & Qt::LeftButton) ) return;
    if (dragging_container == 0) {
        // Check whether the user has moved far enough.
        int delay = QApplication::startDragDistance();
        if ( (mclic_pos - e->pos()).manhattanLength() > delay ) {
            int pos = findContainerAtPoint(e->pos());
            original_container = 0;
            if (pos > -1) {
                original_container = containers.at(pos);
                mclic_dock_pos = e->pos() - original_container->pos();
                dragged_container_original_pos = pos;
                dragging_container = new DockContainer(original_container->command(), 0, original_container->resName(), original_container->resClass(), true);
                dragging_container->show();
                dragging_container->embed(original_container->embeddedWinId());
                grabMouse();
            }
        }
    }
    if (dragging_container) {
        dragging_container->move(e->globalPos() - mclic_dock_pos);
        
        // change layout of other containers 
        QPoint dragpos(dragging_container->pos()), 
            barpos(mapToGlobal(pos()));
        int pdrag1,pdrag2,psz;
        pdrag1 = dragpos.x() - barpos.x() + DockContainer::sz()/2;
        pdrag2 = dragpos.y() - barpos.y() + DockContainer::sz()/2;
        if (orientation() == Qt::Vertical) {
            int tmp = pdrag1; pdrag1 = pdrag2; pdrag2 = tmp;
            psz = height();
        } else psz = width();
        if (pdrag2 >= 0 && pdrag2 < DockContainer::sz() && pdrag1 >= 0 && pdrag1 < psz)
            pdrag1 /= DockContainer::sz();
        else
            pdrag1 = dragged_container_original_pos;


        DockContainer::Vector::iterator it = qFind(containers.begin(), containers.end(), original_container);

        if (it == containers.end())
        {
            return;
        }

        DockContainer::Vector::iterator target = containers.begin();
        for (int i = 0; i < pdrag1 && target != containers.end(); ++i)
        {
            ++target;
        }

        containers.erase(it);
        containers.insert(target, original_container);
        layoutContainers();
    }
}
