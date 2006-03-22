/*****************************************************************

Copyright (c) 2005 Marc Cramdal
Copyright (c) 2005 Aaron Seigo <aseigo@kde.org>

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

#include <qapplication.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpalette.h>
#include <qtimer.h>
#include <q3tl.h>

#include <QMimeData>
#include <QMouseEvent>
#include <QPixmap>
#include <QVBoxLayout>
#include <QEvent>
#include <QCloseEvent>

#include <kiconloader.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>

#include "addapplet.h"
#include "addappletvisualfeedback.h"
#include "appletwidget.h"
#include "appletview.h"
#include "container_applet.h"
#include "container_extension.h"
#include "containerarea.h"
#include "kicker.h"
#include "kickerSettings.h"
#include "menuinfo.h"
#include "pluginmanager.h"

AppletWidget::AppletWidget(const AppletInfo& info, bool odd, QWidget *parent)
    : AppletItem(parent),
      m_appletInfo(info),
      m_odd(odd),
      m_selected(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setSelected(m_selected);

    itemTitle->setText("<h3>" + info.name() + "</h3>");
    itemTitle->installEventFilter(this);

    if (info.comment() != info.name())
    {
        itemDescription->setText(info.comment());
    }

    itemDescription->installEventFilter(this);

    KIconLoader * ldr = KGlobal::iconLoader();
    QPixmap icon = ldr->loadIcon(info.icon(), K3Icon::Panel, K3Icon::SizeLarge);
    itemPixmap->setPixmap(icon);
    itemPixmap->installEventFilter(this);
}

bool AppletWidget::eventFilter(QObject*, QEvent* e)
{
    if (e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        if (me->button() & Qt::LeftButton)
        {
            m_dragStart = me->pos();
        }
    }
    else if (m_dragStart.isNull())
    {
        return false;
    }

    if (e->type() == QEvent::MouseMove)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        if ((me->pos() - m_dragStart).manhattanLength() >
            KGlobalSettings::dndEventDelay())
        {
            QMimeData* data = new QMimeData;
            m_appletInfo.populateMimeData(data);

            QDrag* drag = new QDrag(this);
            drag->setMimeData(data);

            if (itemPixmap->pixmap())
            {
                drag->setPixmap(*itemPixmap->pixmap());
            }

            drag->start(Qt::CopyAction);
        }
    }
    else if (e->type() == QEvent::MouseButtonRelease)
    {
        m_dragStart = QPoint();
    }

    return false;
}

void AppletWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter ||
        e->key() == Qt::Key_Return)
    {
        emit doubleClicked(this);
    }
    else if (e->key() == Qt::Key_Up)
    {
        QKeyEvent fakedKeyPress(QEvent::KeyPress, Qt::Key_Backtab, 0, 0);
        QKeyEvent fakedKeyRelease(QEvent::KeyRelease, Qt::Key_Backtab, 0, 0);
        QApplication::sendEvent(this, &fakedKeyPress);
        QApplication::sendEvent(this, &fakedKeyRelease);
    }
    else if (e->key() == Qt::Key_Down)
    {
        QKeyEvent fakedKeyPress(QEvent::KeyPress, Qt::Key_Tab, 0, 0);
        QKeyEvent fakedKeyRelease(QEvent::KeyRelease, Qt::Key_Escape, 0, 0);
        QApplication::sendEvent(this, &fakedKeyPress);
        QApplication::sendEvent(this, &fakedKeyRelease);
    }
    else
    {
        AppletItem::keyPressEvent(e);
    }
}


void AppletWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        emit clicked(this);
        m_dragStart = e->pos();
    }
}

void AppletWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton &&
        !m_dragStart.isNull() &&
        (e->pos() - m_dragStart).manhattanLength() >
         KGlobalSettings::dndEventDelay())
    {
        QMimeData* data = new QMimeData;
        m_appletInfo.populateMimeData(data);

        QDrag* drag = new QDrag(this);
        drag->setMimeData(data);

        if (itemPixmap->pixmap())
        {
            drag->setPixmap(*itemPixmap->pixmap());
        }

        drag->start(Qt::CopyAction);
    }
}

void AppletWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_dragStart = QPoint();
}

void AppletWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (!e->button() == Qt::LeftButton)
    {
        AppletItem::mouseDoubleClickEvent(e);
        return;
    }

    emit doubleClicked(this);
}

void AppletWidget::setSelected(bool selected)
{
    m_selected = selected;

    // for now just used to switch colours around =)
    if (m_selected)
    {
        setPaletteBackgroundColor(KGlobalSettings::highlightColor());
        setPaletteForegroundColor(KGlobalSettings::highlightedTextColor());
    }
    else if (m_odd)
    {
        setPaletteBackgroundColor(KGlobalSettings::baseColor());
        setPaletteForegroundColor(KGlobalSettings::textColor());
    }
    else
    {
        setPaletteBackgroundColor(KGlobalSettings::alternateBackgroundColor());
        setPaletteForegroundColor(KGlobalSettings::textColor());
    }
}

void AppletWidget::setOdd(bool odd)
{
    m_odd = odd;
    setSelected(m_selected);
}

void AppletWidget::focusInEvent(QFocusEvent*)
{
    m_selected = true;
    setSelected(m_selected);
}

void AppletWidget::focusOutEvent(QFocusEvent*)
{
    m_selected = false;
    setSelected(m_selected);
}

AddAppletDialog::AddAppletDialog(ContainerArea* cArea,
                                 QWidget* parent,
                                 const char* name)
    : KDialog(parent, i18n("Add Applet"), 0),
      m_selectedApplet(0),
      m_containerArea(cArea),
      m_insertionPoint(Kicker::self()->insertionPoint()),
      m_closing(false)
{
    Q_UNUSED(name);
    setModal(false);

    m_mainWidget = new AppletView(this, "AddAppletDialog::m_mainWidget");
    m_mainWidget->appletScrollView->setWidgetResizable( true );
    setMainWidget(m_mainWidget);

    KConfig *cfg = KGlobal::config();
    cfg->setGroup("AddAppletDialog Settings");
    restoreDialogSize(cfg);

    centerOnScreen(this);

    KGuiItem addGuiItem = KStdGuiItem::add();
    addGuiItem.setText(m_mainWidget->appletInstall->text());
    m_mainWidget->appletInstall->setEnabled(false);
    m_mainWidget->appletInstall->setGuiItem(addGuiItem);
    m_mainWidget->closeButton->setGuiItem(KStdGuiItem::close());

    connect(m_mainWidget->appletSearch, SIGNAL(textChanged(const QString&)), this, SLOT(search(const QString&)));
    connect(m_mainWidget->appletFilter, SIGNAL(activated(int)), this, SLOT(filter(int)));
    connect(m_mainWidget->appletInstall, SIGNAL(clicked()), this, SLOT(addCurrentApplet()));
    connect(m_mainWidget->closeButton, SIGNAL(clicked()), this, SLOT(close()));

    m_selectedType = AppletInfo::Undefined;

    QTimer::singleShot(0, this, SLOT(populateApplets()));
}

void AddAppletDialog::updateInsertionPoint()
{
    m_insertionPoint = Kicker::self()->insertionPoint();
}


void AddAppletDialog::closeEvent(QCloseEvent* e)
{
    m_closing = true;

    KConfig *cfg = KGlobal::config();
    cfg->setGroup("AddAppletDialog Settings");
    KDialog::saveDialogSize(cfg);

    KDialog::closeEvent(e);
}

void AddAppletDialog::populateApplets()
{
    QWidget *appletBox = new QWidget(m_mainWidget->appletScrollView->viewport());
    appletBox->setPaletteBackgroundColor(KGlobalSettings::baseColor());
    m_mainWidget->appletScrollView->setWidget(appletBox);
    appletBox->show();

    QVBoxLayout* layout = new QVBoxLayout(appletBox);
    layout->addStretch(1);
    layout->setMargin(0);

    /* Three steps
     * - First we load the applets
     * - We load the special buttons
     * - Then we begin to populate the scrollview with the AppletWidget(s)
     */

    // Loading applets
    AppletInfo::List appletInfoList = PluginManager::applets(false, &appletInfoList);

    // Loading built in buttons
    appletInfoList = PluginManager::builtinButtons(false, &appletInfoList);

    // Loading special buttons
    appletInfoList = PluginManager::specialButtons(false, &appletInfoList);

    qHeapSort(appletInfoList);

    int i = 0;
    bool odd = true;
    QWidget* prevTabWidget = m_mainWidget->appletFilter;
    for (AppletInfo::List::iterator it = appletInfoList.begin();
         !m_closing && it != appletInfoList.end();
         ++i)
    {
        if ((*it).isHidden() || (*it).name().isEmpty() ||
            ((*it).isUniqueApplet() &&
             PluginManager::self()->hasInstance(*it)))
        {
            it = appletInfoList.erase(it);
            --i;
            continue;
        }

        AppletWidget *itemWidget = new AppletWidget(*it, odd, appletBox);

        if (m_mainWidget->appletSearch->text().isEmpty() ||
            appletMatchesSearch(itemWidget, m_mainWidget->appletSearch->text()))
        {
            itemWidget->show();
            odd = !odd;
        }
        else
        {
            itemWidget->hide();
        }

        layout->insertWidget(i, itemWidget);
        itemWidget->show();
        m_appletWidgetList.append(itemWidget);
        setTabOrder(prevTabWidget, itemWidget);
        prevTabWidget = itemWidget;

        connect(itemWidget, SIGNAL(clicked(AppletWidget*)),
                this, SLOT(selectApplet(AppletWidget*)));
        connect(itemWidget, SIGNAL(doubleClicked(AppletWidget*)),
                this, SLOT(addApplet(AppletWidget*)));

        // Avoid to freeze the inferface
        qApp->processEvents();

        if (m_closing)
        {
            return;
        }

        ++it;
    }

    m_mainWidget->closeButton->setEnabled(true);
}

void AddAppletDialog::selectApplet(AppletWidget *applet)
{
    m_mainWidget->appletInstall->setEnabled(true);

    if (m_selectedApplet)
    {
        m_selectedApplet->setSelected(false);
    }

    m_selectedApplet = applet;

    if (m_selectedApplet)
    {
        m_selectedApplet->setSelected(true);

        if (m_selectedApplet->info().isUniqueApplet() &&
            PluginManager::self()->hasInstance(m_selectedApplet->info()))
        {
            m_mainWidget->appletInstall->setEnabled(false);
        }
    }
}

void AddAppletDialog::addCurrentApplet()
{
    addApplet(m_selectedApplet);
}

void AddAppletDialog::addApplet(AppletWidget* applet)
{
    if (!applet)
    {
        return;
    }

    QPoint prevInsertionPoint = Kicker::self()->insertionPoint();
    Kicker::self()->setInsertionPoint(m_insertionPoint);

    const QWidget* appletContainer = 0;

    if (applet->info().type() == AppletInfo::Applet)
    {
        appletContainer = m_containerArea->addApplet(applet->info());

        if (applet->info().isUniqueApplet() &&
            PluginManager::self()->hasInstance(applet->info()))
        {
            applet->hide();

            // reset the odd/even colouring from this item on down in the list
            bool odd = applet->odd();
            AppletWidget::List::const_iterator it = m_appletWidgetList.find(applet);
            for (; it != m_appletWidgetList.constEnd(); ++it)
            {
                if ((*it)->isHidden())
                {
                    continue;
                }

                (*it)->setOdd(odd);
                odd = !odd;
            }
        }
    }
    else if (applet->info().type() & AppletInfo::Button)
    {
        appletContainer = m_containerArea->addButton(applet->info());
    }

    if (appletContainer)
    {
        ExtensionContainer* ec =
           dynamic_cast<ExtensionContainer*>(m_containerArea->topLevelWidget());

        if (ec)
        {
            // unhide the panel and keep it unhidden for at least the time the
            // helper tip will be there
            ec->unhideIfHidden(KickerSettings::mouseOversSpeed() + 2500);
        }

        new AddAppletVisualFeedback(applet, appletContainer,
                                    m_containerArea->popupDirection());
    }

    Kicker::self()->setInsertionPoint(prevInsertionPoint);
}

bool AddAppletDialog::appletMatchesSearch(const AppletWidget* w,
                                          const QString& s)
{
    if (w->info().type() == AppletInfo::Applet &&
        w->info().isUniqueApplet() &&
        PluginManager::self()->hasInstance(w->info()))
    {
        return false;
    }

    return (m_selectedType == AppletInfo::Undefined ||
            w->info().type() & m_selectedType) &&
           (w->info().name().contains(s, false) ||
            w->info().comment().contains(s, false));
}

void AddAppletDialog::search(const QString &s)
{
    QList<AppletWidget*>::const_iterator itEnd = m_appletWidgetList.constEnd();

    bool odd = true;
    for ( QList<AppletWidget*>::const_iterator it = m_appletWidgetList.constBegin();
          it != itEnd;
          ++it)
    {
        AppletWidget* w = *it;
        if (appletMatchesSearch(w, s))
        {
            w->setOdd(odd);
            w->show();
            odd = !odd;
        }
        else
        {
            w->hide();
        }
    }
}

void AddAppletDialog::filter(int i)
{
    m_selectedType = AppletInfo::Undefined;

    if (i == 1)
    {
        m_selectedType = AppletInfo::Applet;
    }
    else if (i == 2)
    {
        m_selectedType = AppletInfo::Button;
    }

    QList<AppletWidget*>::const_iterator itEnd = m_appletWidgetList.constEnd();

    bool odd = true;
    QString searchString = m_mainWidget->appletSearch->text();
    for (QList<AppletWidget*>::const_iterator it = m_appletWidgetList.constBegin();
         it != itEnd;
         ++it)
    {
        AppletWidget* w = *it;
        if (appletMatchesSearch(w, searchString))
        {
            w->setOdd(odd);
            w->show();
            odd = !odd;
        }
        else
        {
            w->hide();
        }
    }

}

#include "addapplet.moc"
#include "appletwidget.moc"

