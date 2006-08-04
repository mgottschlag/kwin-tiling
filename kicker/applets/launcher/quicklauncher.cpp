/*****************************************************************

Copyright (c) 2000 Bill Nagel
Copyright (c) 2004 Dan Bullok <dan.devel@bullok.com>

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

#include <QPainter>
#include <QMenu>
#include <QSlider>
#include <QTimer>
#include <QToolTip>
//Added by qt3to4:
#include <QPixmap>
#include <QDragLeaveEvent>
#include <QPaintEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QList>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kaboutapplication.h>
#include <kaboutdata.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <k3urldrag.h>
#include <kdebug.h>


#include <algorithm>
#include <list>
#include <math.h>
#include <set>
#include <assert.h>

#include "configdlg.h"
#include "popularity.h"
#include "quicklauncher.h"
#include "quickbutton.h"
#include "quickaddappsmenu.h"
#include "quickbuttongroup.h"

typedef ButtonGroup::iterator ButtonIter;
const ButtonGroup::Index NotFound=ButtonGroup::NotFound;
const ButtonGroup::Index Append=ButtonGroup::Append;

#ifdef DEBUG
   #define DEBUGSTR kDebug()
#else
   #define DEBUGSTR kndDebug()
#endif

extern "C"
{
    KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
        KGlobal::locale()->insertCatalog("quicklauncher");
        return new QuickLauncher(configFile, Plasma::Normal,
                                 Plasma::Preferences, parent);
    }
}

QuickLauncher::QuickLauncher(const QString& configFile, Plasma::Type type, int actions,
                             QWidget *parent) :
    KPanelApplet(configFile, type, actions, parent)
{
    //DCOPObject::setObjId("QuickLauncherApplet");
    DEBUGSTR << endl << endl << endl << "------------" << flush;
    DEBUGSTR << "QuickLauncher::QuickLauncher(" << configFile << ",...)" <<
                endl << flush;

    m_settings = new Prefs(sharedConfig());
    m_settings->readConfig();

    m_needsSave = false;
    m_needsRefresh = false;
    m_refreshEnabled = false;

    m_configDialog = 0;
    m_popup = 0;
    m_appletPopup = 0;
    m_removeAppsMenu = 0;

    m_dragAccepted = false;

    m_buttons = new ButtonGroup;
    m_manager = new FlowGridManager;
    m_newButtons = 0;
    m_oldButtons = 0;
    m_dragButtons = 0;

    m_actionCollection = new KActionCollection(this);
    m_configAction = new KAction(KIcon("configure"), i18n("Configure Quicklauncher..."), m_actionCollection, "configure");
    connect(m_configAction, SIGNAL(triggered(bool)), SLOT(slotConfigure()));

    m_saveTimer = new QTimer(this);
    connect(m_saveTimer, SIGNAL(timeout()), this, SLOT(saveConfig()));

    m_popularity = new PopularityStatistics();

    setBackgroundOrigin(AncestorOrigin);

    loadConfig();

    buildPopupMenu();
    m_minPanelDim = std::max(16, m_settings->iconDimChoices()[1]);
    refreshContents();
    setRefreshEnabled(true);

    setAcceptDrops(true);
    //this->setToolTip( i18n("Drop applications here"));
    DEBUGSTR << "    QuickLauncher::QuickLauncher(" << configFile <<
                ",...) END" << endl << flush;
#if 0
    DCOPClient *dcopClient = KApplication::dcopClient();
    dcopClient->connectDCOPSignal(0, "appLauncher",
        "serviceStartedByStorageId(QString,QString)",
        "QuickLauncherApplet",
        "serviceStartedByStorageId(QString,QString)",
        false);
#endif
    kDebug() << "Quicklauncher registered DCOP signal" << endl;
}


//TODO:?  Drag/drop more than one item at a time

QuickLauncher::~QuickLauncher()
{
    KGlobal::locale()->removeCatalog("quicklauncher");
    setCustomMenu(0);
    delete m_popup;
    delete m_appletPopup;
    delete m_removeAppsMenu;
    delete m_popularity;
    clearTempButtons();
    if (m_buttons)
    {
        m_buttons->deleteContents();
        delete m_buttons;
    }
    delete m_actionCollection;
}

// Builds, connects _popup menu
void QuickLauncher::buildPopupMenu()
{
    QuickAddAppsMenu *addAppsMenu = new QuickAddAppsMenu(this, this);
    m_popup = new QMenu(this);
    m_popup->insertItem(i18n("Add Application"), addAppsMenu);
    m_popup->addAction( m_configAction );

    m_appletPopup = new QMenu(this);
    m_appletPopup->insertItem(i18n("Add Application"), addAppsMenu);
    m_removeAppsMenu = new QMenu(this);
    connect(m_removeAppsMenu, SIGNAL(aboutToShow()),
            SLOT(fillRemoveAppsMenu()));
    connect(m_removeAppsMenu, SIGNAL(activated(int)),
            SLOT(removeAppManually(int)));
    m_appletPopup->insertItem(i18n("Remove Application"), m_removeAppsMenu);

    m_appletPopup->addSeparator();
    m_appletPopup->insertItem(i18n("About"), this, SLOT(about()));
    setCustomMenu(m_appletPopup);
}


// Fill the remove apps menu
void QuickLauncher::fillRemoveAppsMenu()
{
    m_removeAppsMenu->clear();
    ButtonIter iter(m_buttons->begin());
    int i = 0;
    while (iter != m_buttons->end())
    {
        QString text = (*iter)->toolTip();
        if (text.isEmpty())
        {
            text = (*iter)->url();
            if (text.isEmpty())
            {
                text = i18n("Unknown");
            }
        }
        m_removeAppsMenu->insertItem(QIcon((*iter)->icon()), text, i);
        ++iter;
        ++i;
    }
}

void QuickLauncher::slotSettingsDialogChanged()
{
    // Update conserve space setting
    setConserveSpace(m_settings->conserveSpace());
    m_popularity->setHistoryHorizon(m_settings->historyHorizon()/100.0);
    slotAdjustToCurrentPopularity();
    kDebug() << "Icon size: " << m_settings->iconDim() << endl;
    refreshContents();

    saveConfig();
}

void QuickLauncher::action(Plasma::Action a)
{
    if (a == Plasma::Preferences)
    {
        slotConfigure();
    }
    else
    {
        KPanelApplet::action(a);
    }
}

void QuickLauncher::slotConfigure()
{
    if (!m_configDialog)
    {
        m_configDialog = new ConfigDlg(this, "configdialog",
            m_settings, SIZE_AUTO, KPageDialog::Plain, KDialog::Ok |
            KDialog::Cancel | KDialog::Apply | KDialog::Default);
        connect(m_configDialog, SIGNAL(settingsChanged()),
            this, SLOT(slotSettingsDialogChanged()));
    }

    m_configDialog->show();
}


int QuickLauncher::findApp(QuickButton *button)
{
    if (m_buttons->empty())
    {
        return NotFound;
    }
    int pos = m_buttons->findValue(button);
    return pos;
}


int QuickLauncher::findApp(QString url)
{
    if (m_buttons->empty())
    {
        return NotFound;
    }
    int pos=m_buttons->findDescriptor(url);
    return pos;
}

void QuickLauncher::removeAppManually(int index)
{
    removeApp(index, true);
}

void QuickLauncher::removeApp(int index, bool manuallyRemoved)
{
    if (m_buttons->empty())
    {
        return;
    }
    if (!m_buttons->isValidIndex(index))
    {
        kWarning() << "    removeApp (" << index <<
            ") *******WARNING****** index=" << index << "is out of bounds." <<
            endl << flush;
        return;
    }
    DEBUGSTR << "Removing button.  index=" << index << " url='" <<
        (*m_buttons)[index]->url() << "'" << endl << flush;

    QString removeAppUrl = (*m_buttons)[index]->url();
    QString removeAppMenuId = (*m_buttons)[index]->menuId();

    delete (*m_buttons)[index];
    m_buttons->eraseAt(index);
    refreshContents();

    if (int(m_buttons->size()) < m_settings->autoAdjustMinItems() && manuallyRemoved)
    {
        m_settings->setAutoAdjustMinItems(m_buttons->size());
    }

    if (manuallyRemoved)
    {
        m_popularity->moveToBottom(removeAppMenuId);
        slotAdjustToCurrentPopularity();
    }

    saveConfig();
}


void QuickLauncher::removeApp(QString url, bool manuallyRemoved)
{
    int index = findApp(url);
    if (index == NotFound)
    {
        kDebug() << "removeApp: Not found: " << url << endl;
        return;
    }
    removeApp(index, manuallyRemoved);
}


void QuickLauncher::removeAppManually(QuickButton *button)
{
    int index = findApp(button);
    if (index == NotFound)
    {
        return;
    }
    removeApp(index, true);
}


int QuickLauncher::widthForHeight(int h) const
{
    FlowGridManager temp_manager = *m_manager;
    temp_manager.setFrameSize(QSize(h,h));
    temp_manager.setOrientation(Qt::Horizontal); // ??? probably not necessary
    if (temp_manager.isValid())
    {
        return temp_manager.frameSize().width();
    }
    return m_minPanelDim;
}


int QuickLauncher::heightForWidth(int w) const
{
    FlowGridManager temp_manager=*m_manager;
    temp_manager.setFrameSize(QSize(w,w));
    temp_manager.setOrientation(Qt::Vertical); // ??? probably not necessary
    if (temp_manager.isValid())
    {
        return temp_manager.frameSize().height();
    }
    return m_minPanelDim;
}


int QuickLauncher::dimension() const
{
    if (orientation()==Qt::Vertical)
    {
        return size().width();
    }
    return size().height();
}

void QuickLauncher::addApp(QString url, bool manuallyAdded)
{
    assert(m_buttons);
    QString newButtonId = QuickURL(url).menuId();
    if (m_appOrdering.find(newButtonId) == m_appOrdering.end())
    {
        m_appOrdering[newButtonId] = m_appOrdering.size();
    }
    uint appPos;
    for (appPos = 0; appPos < m_buttons->size(); ++appPos)
    {
        QString buttonId = (*m_buttons)[appPos]->menuId();
        if (m_appOrdering[buttonId] >= m_appOrdering[newButtonId])
        {
            break;
        }
    }
    addApp(url, appPos, manuallyAdded);
}

QuickButton* QuickLauncher::createButton(QString url)
{
    QuickButton* newButton=new QuickButton(url, m_configAction, m_actionCollection, this);
    connect(newButton, SIGNAL(executed(QString)),
            this, SLOT(slotOwnServiceExecuted(QString)));
    connect(newButton, SIGNAL(stickyToggled(bool)),
            this, SLOT(slotStickyToggled()));
    newButton->setPopupDirection(popupDirection());
    return newButton;
}

void QuickLauncher::addApp(QString url, int index, bool manuallyAdded)
{
    DEBUGSTR << endl <<"About to add: url='" << url <<
                "' index=" << index << endl << flush;
    QuickButton *newButton;
    if (!m_buttons->isValidInsertIndex(index))
    {
        kWarning() << "    *******WARNING****** index=" << index <<
            "is out of bounds." << endl << flush;
        index = m_buttons->lastIndex();
    }
    int old = findApp(QuickURL(url).url());
    if (old != NotFound)
    {
        if (index == old)
        {
            return;
        }
        if (index > old)
        {
            index--;
        }
        newButton = (*m_buttons)[old];
        m_buttons->eraseAt(old);
    }
    else
    {
        newButton = createButton(url);
    }
    m_buttons->insertAt(index, newButton);
    DEBUGSTR << "Added: url='"<<url<<"' index="<<index<<endl<<endl<<flush;
    refreshContents();

    if (manuallyAdded)
    {
        newButton->setSticky(true);
        if (int(m_buttons->size()) > m_settings->autoAdjustMaxItems())
        {
            m_settings->setAutoAdjustMaxItems(m_buttons->size());
        }
    }

    updateInsertionPosToStatusQuo();
    saveConfig();
}

void QuickLauncher::updateInsertionPosToStatusQuo()
{
    // Update the app ordering map, so that next time,
    // addApp(url,manAdded) (without index) will insert the
    // item at the same position again.
    std::list<QString> appList;
    std::set<int> posList;
    //kDebug() << "Rearranging application order. Before:" << endl;
    for (uint n = 0; n < m_buttons->size(); ++n)
    {
        QString buttonId = (*m_buttons)[n]->menuId();
        appList.push_back(buttonId);
        if (m_appOrdering.find(buttonId) == m_appOrdering.end())
        {
            m_appOrdering[buttonId] = m_appOrdering.size();
        }
        posList.insert(m_appOrdering[buttonId]);
        //kDebug() << m_appOrdering[buttonId] << " = " << buttonId << endl;
    }
    //kDebug() << "After:" << endl;
    while (posList.size() > 0)
    {
        assert(appList.size() > 0);
        m_appOrdering[*appList.begin()] = *posList.begin();
        kDebug() << *posList.begin() << " = " << *appList.begin() << endl;
        posList.erase(posList.begin());
        appList.pop_front();
    }
    //kDebug() << "Done." << endl;
}

void QuickLauncher::addAppBeforeManually(QString url, QString sender)
{
    if (sender.isNull())
    {
        addApp(url, Append, true);
    }
    int pos = findApp(sender);
    if (pos < 0)
    {
        pos = Append;
    }
    DEBUGSTR << "QuickLauncher::addAppBefore(" << url <<
                "," << sender << "):  pos=" << pos << endl << flush;
    addApp(url, pos, true);
}


void QuickLauncher::about()
{
    KAboutData about("quicklauncher", I18N_NOOP("Quick Launcher"), "2.0",
                     I18N_NOOP("A simple application launcher"),
                     KAboutData::License_GPL_V2,
                     "(C) 2000 Bill Nagel\n(C) 2004 Dan Bullok\n(C) 2005 Fred Schaettgen");
    KAboutApplication a(&about, this);
    a.exec();
}


void QuickLauncher::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton)
    {
        m_popup->popup(e->globalPos());
    }
}

void QuickLauncher::resizeEvent(QResizeEvent*)
{
    refreshContents();
}

void QuickLauncher::dragEnterEvent(QDragEnterEvent *e)
{
    DEBUGSTR << "QuickLauncher::dragEnterEvent(pos=" << e->pos() <<
        " type=" << e->type() << ")" << endl << flush;
    m_dragAccepted=false;
    KUrl::List kurlList;
    if (!isDragEnabled() || !K3URLDrag::decode(e, kurlList))
    {
        e->setAccepted(false);
        return;
    }

    if (kurlList.size()<=0)
    {
        e->setAccepted(false);
        return;
    }
    m_dragButtons=new ButtonGroup;
    m_oldButtons=new ButtonGroup(*m_buttons);

    QString url;
    KUrl::List::ConstIterator it = kurlList.begin();
    for ( ; it != kurlList.end(); ++it )
    {
        url = QuickURL((*it).url()).url();
        kDebug() << "    Drag Object='"<<url<<"' " << (*it).url() << endl;
        int pos = m_buttons->findDescriptor(url);
        if (pos != NotFound)
        {
            // if it's already in m_buttons, take it out
            m_dragButtons->push_back(m_buttons->takeFrom(pos));
        }
        else
        {
            // otherwise, create a new one
            QuickButton* button = createButton(url);
            button->setSticky(true);
            m_dragButtons->push_back(button);
        }
    }
    if (m_dragButtons->size() > 0)
    {
        //make sure we can drag at least one button.
        m_dragAccepted=true;
        m_newButtons=new ButtonGroup(*m_buttons);
        m_dropPos=NotFound;
        e->setAccepted(true);
        return;
    }
    e->setAccepted(false);
    clearTempButtons();
}


void QuickLauncher::dragMoveEvent(QDragMoveEvent *e)
{
    if (!m_dragAccepted)
    {
        kWarning() << "QuickLauncher::dragMoveEvent: Drag is not accepted." <<
            m_dragAccepted << endl << flush;
        e->setAccepted(false);
        return;
    }

    e->setAccepted(true);
    int pos=m_manager->indexNearest(e->pos());
    if (pos == m_dropPos)
    {
        return;// Already been inserted here, no need to update
    }

    if (m_newButtons->isValidInsertIndex(pos))
    {
        mergeButtons(pos);
        m_dropPos=pos;
    }
    refreshContents();
}


void QuickLauncher::dragLeaveEvent(QDragLeaveEvent *e)
{
    DEBUGSTR << "QuickLauncher::dragLeaveEvent(type=" <<
        e->type() << ")" << endl << flush;
    if (!m_dragAccepted)
    {
        return;
    }

    // No drop.  Return to starting state.
    std::swap(m_buttons,m_oldButtons);
    clearTempButtons();

    refreshContents();
    saveConfig();
}


void QuickLauncher::dropEvent(QDropEvent *e)
{
    DEBUGSTR << "QuickLauncher::dropEvent(pos=" << e->pos() <<
        " type=" << e->type() << ")" << endl << flush;
    if (!m_dragAccepted)
    {
        e->setAccepted(false);
        return;
    }

    if (e->source() == 0)
    {
        for (uint n=0; n<m_dragButtons->size(); ++n)
        {
            (*m_dragButtons)[n]->setSticky(true);
        }
    }

    clearTempButtons();
    refreshContents();
    saveConfig();
    updateInsertionPosToStatusQuo();
}

// insert dragbuttons at index in m_newButtons.  Put result in m_buttons
void QuickLauncher::mergeButtons(int index)
{
    if (!m_newButtons->isValidInsertIndex(index))
    {
        index=m_newButtons->size();
    }

    m_buttons->clear();
    (*m_buttons) = (*m_newButtons);
    m_buttons->insertAt(index, *m_dragButtons);
    refreshContents();
}

void QuickLauncher::clearTempButtons()
{
    std::set<QuickButton*> allButtons;
    //put all the m_buttons in a set (removes duplicates automatically
    if (m_newButtons)
    {
        allButtons.insert(m_newButtons->begin(),m_newButtons->end());
    }
    if (m_oldButtons)
    {
        allButtons.insert(m_oldButtons->begin(),m_oldButtons->end());
    }
    if (m_dragButtons)
    {
        allButtons.insert(m_dragButtons->begin(),m_dragButtons->end());
    }

    //delete temp ButtonGroups
    delete m_newButtons; m_newButtons=0;
    delete m_oldButtons; m_oldButtons=0;
    delete m_dragButtons; m_dragButtons=0;

    //if an element allButtons is NOT in m_buttons (the ones we keep), delete it
    std::set<QuickButton *>::iterator iter = allButtons.begin();
    while (iter != allButtons.end())
    {
        if (findApp(*iter) == NotFound)
        {
            delete *iter;
        }
        ++iter;
    }
    m_dragAccepted = false;
    m_dropPos = NotFound;
}

void QuickLauncher::refreshContents()
{
    int idim, d(dimension());
    // determine button size
    if (m_settings->iconDim() == SIZE_AUTO)
    {
        if (d < 18)
        {
             idim = std::min(16,d);
        }
        else if (d < 64)
        {
            idim = 16;
        }
        else if (d < 80)
        {
            idim = 20;
        }
        else if (d < 122)
        {
            idim = 24;
        }
        else
        {
            idim = 28;
        }
    }
    else
    {
        idim=std::min(m_settings->iconDim(), d);
    }
    m_space = std::max((idim/8)-1, 0);
    m_border = m_space;
    m_buttonSize = QSize(idim, idim);
    m_manager->setOrientation(orientation());
    m_manager->setNumItems(m_buttons->size());
    m_manager->setFrameSize(size());
    m_manager->setItemSize(m_buttonSize);
    m_manager->setSpaceSize(QSize(m_space, m_space));
    m_manager->setBorderSize(QSize(m_border, m_border));
    if (!m_refreshEnabled)
    {
        m_needsRefresh=true;
        return;
    }
    if (!m_manager->isValid())
    {
        kDebug()<<endl<<"******WARNING******    Layout is invalid."<<
            endl << flush;
        m_manager->dump();
        return;
    }

    unsigned index;
    QPoint pos;
    setUpdatesEnabled(false);
    m_buttons->setUpdatesEnabled(false);
    for (index = 0; index < m_buttons->size(); index++)
    {
        pos = m_manager->pos(index);
        QuickButton *button = (*m_buttons)[index];
        button->resize(m_manager->itemSize());
        button->move(pos.x(), pos.y());
        button->setDragging(false);
        button->setEnableDrag(isDragEnabled());
        button->setDynamicModeEnabled(m_settings->autoAdjustEnabled());
    }
    if (m_newButtons)
    {
        m_newButtons->setDragging(false);
    }
    if (m_dragButtons)
    {
        m_dragButtons->setDragging(true);
    }
    m_buttons->show();
    setUpdatesEnabled(true);
    update();
    m_buttons->setUpdatesEnabled(true);
    updateGeometry();
    emit updateLayout();
    updateStickyHighlightLayer();
}


void QuickLauncher::setDragEnabled(bool enable)
{
    m_settings->setDragEnabled(enable);
}

void QuickLauncher::setConserveSpace(bool conserve_space)
{
    m_manager->setConserveSpace(conserve_space);
    if (conserve_space)
    {
        m_manager->setSlack(FlowGridManager::SpaceSlack,
                            FlowGridManager::SpaceSlack);
    }
    else
    {
        m_manager->setSlack(FlowGridManager::ItemSlack,
                            FlowGridManager::ItemSlack);
    }
    refreshContents();
}

class SortByPopularity {
public:
    bool operator()(const QuickLauncher::PopularityInfo& a,
                   const QuickLauncher::PopularityInfo& b)
    {
        return a.popularity < b.popularity;
    }
};

void QuickLauncher::loadConfig()
{
    DEBUGSTR << "QuickLauncher::loadConfig()" << endl << flush;
    //KConfig *c = config();
    //c->setGroup("General");
    setConserveSpace(m_settings->conserveSpace());
    setDragEnabled(m_settings->dragEnabled());
    /*DEBUGSTR << "    IconDim="<<m_iconDim << endl << flush;
    DEBUGSTR << "    ConserveSpace=" << (m_manager->conserveSpace()) <<
        endl << flush;
    DEBUGSTR << "    DragEnabled=" << isDragEnabled() << endl << flush;*/
    QStringList volatileButtons = m_settings->volatileButtons();
    QStringList urls = m_settings->buttons();
    kDebug() << "GetButtons " << urls.join("/") << endl;
    QStringList::Iterator iter(urls.begin());
    int n = 0;
    while (iter != urls.end()) {
        QString url = *iter;
        addApp(url, n, false);
        ++iter;
        ++n;
    }

    // Restore sticky state
    for (n=0; n<int(m_buttons->size()); ++n)
    {
        QuickButton* button = (*m_buttons)[n];
        if (volatileButtons.contains(button->menuId()) == false)
        {
            button->setSticky(true);
        }
        button->setDynamicModeEnabled(m_settings->autoAdjustEnabled());
    }

    m_popularity->readConfig(m_settings);
    m_popularity->setHistoryHorizon(m_settings->historyHorizon()/100.0);

    QStringList serviceNames = m_settings->serviceNames();
    QList<int> insPos = m_settings->serviceInspos();
    for (int n=std::min(serviceNames.size(),insPos.size())-1; n>=0; --n)
    {
        m_appOrdering[serviceNames[n]] = insPos[n];
    }
}

void QuickLauncher::saveConfig()
{
    if (!m_refreshEnabled)
    {
        m_needsSave=true;
        return;
    }
    QStringList urls, volatileUrls;
    ButtonIter iter = m_buttons->begin();
    while (iter != m_buttons->end()) {
        if ((*iter)->sticky() == false)
        {
            volatileUrls.append((*iter)->menuId());
        }
        urls.append((*iter)->menuId());
        ++iter;
    }
    m_settings->setButtons(urls);
    kDebug() << "SetButtons " << urls.join("/") << endl;
    m_settings->setVolatileButtons(volatileUrls);
    m_settings->setConserveSpace(m_manager->conserveSpace());
    m_settings->setDragEnabled(isDragEnabled());

    m_popularity->writeConfig(m_settings);

    // m_popularity must have written the current service list by now
    QStringList serviceNames = m_settings->serviceNames();
    QList<int> insertionPositions;
    for (int n=0; n<int(serviceNames.size()); ++n)
    {
        if (m_appOrdering.find(serviceNames[n]) != m_appOrdering.end())
        {
            insertionPositions.push_back(m_appOrdering[serviceNames[n]]);
        }
    }
    m_settings->setServiceInspos(insertionPositions);

    m_settings->writeConfig();
}


void QuickLauncher::setRefreshEnabled(bool enable)
{
    m_refreshEnabled=enable;
    if (m_refreshEnabled)
    {
        if (m_needsSave) {
           saveConfig();
        }
        if (m_needsRefresh) {
            refreshContents();
        }
    }
}

void QuickLauncher::serviceStartedByStorageId(QString /*starter*/, QString storageId)
{
    KService::Ptr service = KService::serviceByStorageId(storageId);
    if (service->icon() == QString())
    {
        kDebug() << storageId << " has no icon. Makes no sense to add it.";
        return;
    }
    QuickURL url = QuickURL(KStandardDirs::locate("apps", service->desktopEntryPath()));
    QString desktopMenuId(url.menuId());
    kDebug() << "storageId=" << storageId << " desktopURL=" << desktopMenuId << endl;
    // A service was started somwhere else. If the quicklauncher contains
    // this service too, we flash the icon
    QuickButton *startedButton = 0;
    std::set<QString> buttonIdSet;
    for (uint n = 0; n < m_buttons->size(); ++n)
    {
        QuickButton *button = (*m_buttons)[n];
        QString buttonMenuId = button->menuId();
        buttonIdSet.insert(buttonMenuId);
        if (desktopMenuId == buttonMenuId)
        {
           kDebug() << "QuickLauncher: I know that one: " << storageId << endl;
           button->flash();
           startedButton = button;
        }
    }

    // Update popularity info.
    // We do this even if autoadjust is disabled
    // so there are sane values to start with if it's turned on.
    m_popularity->useService(desktopMenuId);

    if (m_settings->autoAdjustEnabled())
    {
        QTimer::singleShot(0, this, SLOT(slotAdjustToCurrentPopularity()));
    }
}

void QuickLauncher::slotAdjustToCurrentPopularity()
{
    // TODO: Shrink immediately if buttons->size() > maxItems
    kDebug() << "Starting popularity update" << endl;
    PopularityStatistics* stats = m_popularity;
    int minItems = m_settings->autoAdjustMinItems();
    int maxItems = m_settings->autoAdjustMaxItems();

    static const double hysteresisFactor = 0.90;
    double minAddPopularity = 0;
    for (int n = 0; n < maxItems; ++n)
    {
        // All items with a popularity not less than 0.75 of the average
        // of the first maxItems apps are included in the list
        double belowAvgAllowed = 0.75;
        minAddPopularity += (belowAvgAllowed * stats->popularityByRank(n)) / maxItems;
    }
    double minDelPopularity = minAddPopularity * hysteresisFactor;
    std::map<QString, QuickButton*> removeableApps;
    std::set<QString> existingApps;
    int numApps = m_buttons->size();
    for (int n = 0; n < int(m_buttons->size()); ++n)
    {
        QuickButton *button = (*m_buttons)[n];
        if (((stats->popularityByRank(stats->rankByService(button->menuId())) <
             minDelPopularity) || m_settings->autoAdjustEnabled()==false) &&
            (button->sticky() == false))
        {
            removeableApps[button->menuId()] = button;
            --numApps;
        }
        existingApps.insert(button->menuId());
    }
    for (int n = 0;
         (numApps < minItems && stats->popularityByRank(n) > 0) ||
         (numApps < maxItems && stats->popularityByRank(n) > minAddPopularity);
         ++n)
    {
        QString app = m_popularity->serviceByRank(n);
        if (existingApps.find(app) == existingApps.end())
        {
            addApp(QuickURL(m_popularity->serviceByRank(n)).url(), false);
            kDebug() << "Adding app " << app << endl;
            ++numApps;
        }
        else if (removeableApps.find(app) != removeableApps.end())
        {
            removeableApps.erase(app);
            ++numApps;
        }
    }
    while (removeableApps.size() > 0)
    {
        removeApp(findApp(removeableApps.begin()->second), false);
        kDebug() << "Removing app " << removeableApps.begin()->first << endl;
        removeableApps.erase(removeableApps.begin()->first);
    }
    kDebug() << "done popularity update" << endl;
    m_settings->setAutoAdjustMinItems(minItems);
    m_settings->setAutoAdjustMaxItems(maxItems);

    // TODO: Think of something better than that:
    m_saveTimer->start(10000,true);
}

void QuickLauncher::slotOwnServiceExecuted(QString serviceMenuId)
{
    m_popularity->useService(serviceMenuId);
    if (m_settings->autoAdjustEnabled())
    {
        QTimer::singleShot(0, this, SLOT(slotAdjustToCurrentPopularity()));
    }
}

void QuickLauncher::updateStickyHighlightLayer()
{
    // Creates a transparent image which is used
    // to highlight those buttons which will never
    // be removed automatically from the launcher
    QPixmap areaPix(width(), height());
    QPainter areaPixPainter(&areaPix);
    areaPixPainter.fillRect(0, 0, width(), height(), QColor(255, 255, 255));
    QSize itemSize = m_manager->itemSize();
    QSize spaceSize = m_manager->spaceSize();
    for (uint n=0; n<m_buttons->size(); ++n)
    {
        QPoint pos = m_manager->pos(n);
        if ((*m_buttons)[n]->sticky() == false)
        {
            areaPixPainter.fillRect(pos.x()-(spaceSize.width()+1)/2,
                                    pos.y()-(spaceSize.height()+1)/2,
                                    itemSize.width()+spaceSize.width()+1,
                                    itemSize.height()+spaceSize.height()+1,
                                    QColor(0, 0, 0));
        }
    }
    QImage areaLayer = areaPix.toImage();
    m_stickyHighlightLayer = QImage(width(), height(), 32);
    m_stickyHighlightLayer.setAlphaBuffer(true);
    int pix, tlPix, brPix, w(width()), h(height());
    QRgb transparent(qRgba(0, 0, 0, 0));
    for (int y = h-1; y >= 0; --y)
    {
        for (int x = w-1; x >= 0; --x)
        {
            pix = qRed(areaLayer.pixel(x, y));
            if (pix == 0)
            {
                tlPix = (y>0 && x>0) ? qRed(areaLayer.pixel(x-1,y-1)) : 255;
                brPix = (y<h-1 && x<w-1) ? qRed(areaLayer.pixel(x+1,y+1)) : 255;
                int c = tlPix-brPix < 0 ? 255 : 0;
                int alpha = abs(tlPix-brPix)/2;
                m_stickyHighlightLayer.setPixel(x, y, qRgba(c, c, c, alpha));
            }
            else
            {
                m_stickyHighlightLayer.setPixel(x, y, transparent);
            }
        }
    }
    repaint();
}

void QuickLauncher::paintEvent(QPaintEvent* e)
{
    KPanelApplet::paintEvent(e);

    if (m_settings->autoAdjustEnabled() &&
        m_settings->showVolatileButtonIndicator())
    {
        QPainter p(this);
        p.drawImage(0, 0, m_stickyHighlightLayer);
    }
}

void QuickLauncher::slotStickyToggled()
{
    updateStickyHighlightLayer();
    saveConfig();
}

void QuickLauncher::positionChange(Plasma::Position)
{
    for (int n=0; n<int(m_buttons->size()); ++n)
    {
        (*m_buttons)[n]->setPopupDirection(popupDirection());
    }
}


#include "quicklauncher.moc"
