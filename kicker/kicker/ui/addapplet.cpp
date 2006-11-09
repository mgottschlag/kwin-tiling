/*****************************************************************

Copyright (c) 2006 Rafael Fernández López <ereslibre@gmail.com>
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

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPalette>
#include <QTimer>
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
#include "container_applet.h"
#include "container_extension.h"
#include "containerarea.h"
#include "kicker.h"
#include "kickerSettings.h"
#include "menuinfo.h"
#include "pluginmanager.h"

AddAppletDialog::AddAppletDialog(ContainerArea *cArea,
                                 QWidget *parent,
                                 const char *name)
	: KDialog(parent)
	, m_mainWidgetView(new Ui::AppletView())
	, m_containerArea(cArea)
	, m_insertionPoint(Kicker::self()->insertionPoint())
{
	setCaption(i18n("Add Applet"));
	Q_UNUSED(name);
	setModal(false);

	setButtons(KDialog::User1 | KDialog::Close);
	setButtonGuiItem(User1, KGuiItem(i18n("Load Applet"), "ok"));
	enableButton(KDialog::User1, false);

	KConfig *cfg = KGlobal::config();
	cfg->setGroup("AddAppletDialog Settings");
	restoreDialogSize(cfg);

	centerOnScreen(this);

	m_mainWidget = new QWidget(this);
	m_mainWidgetView->setupUi(m_mainWidget);
	setMainWidget(m_mainWidget);

	connect(m_mainWidgetView->appletSearch, SIGNAL(textChanged(const QString&)), this, SLOT(search(const QString&)));
	connect(m_mainWidgetView->appletFilter, SIGNAL(activated(int)), this, SLOT(filter(int)));
	connect(m_mainWidgetView->appletListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(selectApplet(const QModelIndex&)));
	connect(m_mainWidgetView->appletListView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(addCurrentApplet(const QModelIndex&)));
	connect(this, SIGNAL(user1Clicked()), this, SLOT(slotUser1Clicked()));

	m_selectedType = AppletInfo::Undefined;

    m_mainWidgetView->appletListView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	QTimer::singleShot(0, this, SLOT(populateApplets()));
}

void AddAppletDialog::updateInsertionPoint()
{
	m_insertionPoint = Kicker::self()->insertionPoint();
}

void AddAppletDialog::closeEvent(QCloseEvent *e)
{
	KConfig *cfg = KGlobal::config();
	cfg->setGroup("AddAppletDialog Settings");
	KDialog::saveDialogSize(cfg);

	KDialog::closeEvent(e);
}

void AddAppletDialog::populateApplets()
{
	// Loading applets
	m_applets = PluginManager::applets(false, &m_applets);

	// Loading built in buttons
	m_applets = PluginManager::builtinButtons(false, &m_applets);

	// Loading special buttons
	m_applets = PluginManager::specialButtons(false, &m_applets);

	qHeapSort(m_applets);

	int i = 0;
	for (AppletInfo::List::iterator it = m_applets.begin();
		  it != m_applets.end();
		  ++i)
	{
		if ((*it).isHidden() || (*it).name().isEmpty() ||
			((*it).isUniqueApplet() &&
			 PluginManager::self()->hasInstance(*it)))
		{
			it = m_applets.erase(it);
			--i;
			continue;
		}

		++it;
	}

	 m_listModel = new AppletListModel(m_applets, this);
	 m_mainWidgetView->appletListView->setModel(m_listModel);

	 AppletItemDelegate *appletItemDelegate = new AppletItemDelegate(this);
	 m_mainWidgetView->appletListView->setItemDelegate(appletItemDelegate);
}

void AddAppletDialog::selectApplet(const QModelIndex &applet)
{
	selectedApplet = applet;

	if (!isButtonEnabled(KDialog::User1))
		enableButton(KDialog::User1, true);
}

void AddAppletDialog::addCurrentApplet(const QModelIndex &selectedApplet)
{
	this->selectedApplet = selectedApplet;
	AppletInfo applet(m_applets[selectedApplet.row()]);

	QPoint prevInsertionPoint = Kicker::self()->insertionPoint();
	Kicker::self()->setInsertionPoint(m_insertionPoint);

	const QWidget* appletContainer = 0;

	if (applet.type() == AppletInfo::Applet)
	{
		appletContainer = m_containerArea->addApplet(applet);
	}
	else if (applet.type() & AppletInfo::Button)
	{
		appletContainer = m_containerArea->addButton(applet);
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

		new AddAppletVisualFeedback(selectedApplet,
                                    m_mainWidgetView->appletListView,
                                    appletContainer,
                                    m_containerArea->popupDirection());
	}

	if (applet.isUniqueApplet() &&
	    PluginManager::self()->hasInstance(applet))
	{
		m_mainWidgetView->appletListView->setRowHidden(selectedApplet.row(), true);
		m_mainWidgetView->appletListView->clearSelection();
		enableButton(KDialog::User1, false);
	}

	Kicker::self()->setInsertionPoint(prevInsertionPoint);
}

bool AddAppletDialog::appletMatchesSearch(const AppletInfo *i, const QString &s)
{
	if (i->type() == AppletInfo::Applet &&
		 i->isUniqueApplet() &&
		 PluginManager::self()->hasInstance(*i))
	{
		return false;
	}

	return (m_selectedType == AppletInfo::Undefined ||
			  i->type() & m_selectedType) &&
			 (i->name().contains(s, Qt::CaseInsensitive) ||
			  i->comment().contains(s, Qt::CaseInsensitive));
}

void AddAppletDialog::search(const QString &s)
{
	AppletInfo *appletInfo;
	for (int i = 0; i < m_listModel->rowCount(); i++)
	{
		appletInfo = static_cast<AppletInfo*>(m_listModel->index(i).internalPointer());
		m_mainWidgetView->appletListView->setRowHidden(i, !appletMatchesSearch(appletInfo, s) ||
													   (appletInfo->isUniqueApplet() &&
														PluginManager::self()->hasInstance(*appletInfo)));
	}

	/**
	  * If our selection gets hidden because of searching, we deselect it and
	  * disable the "Add Applet" button.
	  */
	if ((selectedApplet.isValid() &&
		 (m_mainWidgetView->appletListView->isRowHidden(selectedApplet.row()))) ||
		 (!selectedApplet.isValid()))
	{
		m_mainWidgetView->appletListView->clearSelection();
		enableButton(KDialog::User1, false);
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

	AppletInfo *appletInfo;
	QString searchString = m_mainWidgetView->appletSearch->text();
	for (int j = 0; j < m_listModel->rowCount(); j++)
	{
		appletInfo = static_cast<AppletInfo*>(m_listModel->index(j).internalPointer());
		m_mainWidgetView->appletListView->setRowHidden(j, !appletMatchesSearch(appletInfo, searchString) ||
													   (appletInfo->isUniqueApplet() &&
													    PluginManager::self()->hasInstance(*appletInfo)));
	}

	/**
	  * If our selection gets hidden because of filtering, we deselect it and
	  * disable the "Add Applet" button.
	  */
	if ((selectedApplet.isValid() &&
		 (m_mainWidgetView->appletListView->isRowHidden(selectedApplet.row()))) ||
		 (!selectedApplet.isValid()))
	{
		m_mainWidgetView->appletListView->clearSelection();
		enableButton(KDialog::User1, false);
	}
}

void AddAppletDialog::slotUser1Clicked()
{
	if (selectedApplet.isValid())
	{
		addCurrentApplet(selectedApplet);
	}
}

#include "addapplet.moc"
