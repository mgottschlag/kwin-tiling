/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kcm_keyboard_widget.h"

#include <kactioncollection.h>
#include <kaction.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <klocale.h>

#include <QtGui/QMessageBox>
#include <QtGui/QWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QPixmap>
#include <QtGui/QVBoxLayout>

#include "keyboard_config.h"
#include "xkb_rules.h"
#include "flags.h"
#include "x11_helper.h"
#include "kcm_view_models.h"
#include "kcm_add_layout_dialog.h"

#include "kcmmisc.h"

#include "ui_kcm_add_layout_dialog.h"


static const QString GROUP_SWITCH_GROUP_NAME("grp");
static const QString LV3_SWITCH_GROUP_NAME("lv3");
static const char XKB_OPTION_GROUP_SEPARATOR = ':';
//static const QString RESET_XKB_OPTIONS("-option");

static const int TAB_HARDWARE = 0;
//static const int TAB_LAYOUTS = 1;
static const int TAB_ADVANCED = 2;

KCMKeyboardWidget::KCMKeyboardWidget(Rules* rules_, KeyboardConfig* keyboardConfig_, const KComponentData componentData_, QWidget* /*parent*/):
	componentData(componentData_),
	actionCollection(NULL),
	uiUpdating(false)
{
	flags = new Flags();
	rules = rules_;
	keyboardConfig = keyboardConfig_;

	uiWidget = new Ui::TabWidget;
	uiWidget->setupUi(this);

	kcmMiscWidget = new KCMiscKeyboardWidget(uiWidget->lowerHardwareWidget);
	connect(kcmMiscWidget, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
	//TODO: connect save/load

    initializeKeyboardModelUI();
    initializeXkbOptionsUI();
    initializeLayoutsUI();
}

KCMKeyboardWidget::~KCMKeyboardWidget()
{
	delete flags;
}

void KCMKeyboardWidget::updateUI()
{
	uiWidget->layoutsTableView->setModel(uiWidget->layoutsTableView->model());
	((LayoutsTableModel*)uiWidget->layoutsTableView->model())->refresh();

	uiUpdating = true;
	updateHardwareUI();
	updateXkbOptionsUI();
	updateSwitcingPolicyUI();
    updateLayoutsUI();
    updateShortcutsUI();
    uiUpdating = false;
}

void KCMKeyboardWidget::uiChanged()
{
	((LayoutsTableModel*)uiWidget->layoutsTableView->model())->refresh();

	if( uiUpdating )
		return;

	keyboardConfig->configureLayouts = uiWidget->configureLayoutsChk->isChecked();
	keyboardConfig->keyboardModel = uiWidget->keyboardModelComboBox->itemData(uiWidget->keyboardModelComboBox->currentIndex()).toString();
	keyboardConfig->showFlag = uiWidget->showFlagChk->isChecked();

	keyboardConfig->resetOldXkbOptions = uiWidget->configureKeyboardOptionsChk->isChecked();
//    if( keyboardConfig->resetOldXkbOptions ) {
//    	if( ! keyboardConfig->xkbOptions.contains(RESET_XKB_OPTIONS) ) {
//    		keyboardConfig->xkbOptions.insert(0, RESET_XKB_OPTIONS);
//    	}
//    }
//    else {
//    	keyboardConfig->xkbOptions.removeAll(RESET_XKB_OPTIONS);
//    }

	updateXkbShortcutsButtons();
	emit changed(true);
}

void KCMKeyboardWidget::initializeKeyboardModelUI()
{
    foreach(ModelInfo* modelInfo, rules->modelInfos) {
    	QString vendor = modelInfo->vendor;
    	if( vendor.isEmpty() ) {
    		vendor = i18nc("unknown keyboard model vendor", "Unknown");
    	}
		uiWidget->keyboardModelComboBox->addItem(i18nc("vendor | keyboard model", "%1 | %2", vendor, modelInfo->description), modelInfo->name);
	}
    uiWidget->keyboardModelComboBox->model()->sort(0);
	connect(uiWidget->keyboardModelComboBox, SIGNAL(activated(int)), this, SLOT(uiChanged()));
}

void KCMKeyboardWidget::addLayout()
{
	if( keyboardConfig->layouts.count() >= X11Helper::MAX_GROUP_COUNT ) {
		QMessageBox msgBox;
		msgBox.setText(i18n("Only up to %1 keyboard layouts is supported", X11Helper::MAX_GROUP_COUNT));
		// more information https://bugs.freedesktop.org/show_bug.cgi?id=19501
		msgBox.exec();
		return;
	}

    AddLayoutDialog dialog(rules, flags, this);
    dialog.setModal(true);
    if( dialog.exec() == QDialog::Accepted ) {
    	keyboardConfig->layouts.append( dialog.getSelectedLayoutConfig() );
    	layoutsTableModel->refresh();
    	uiChanged();
    }
}

void KCMKeyboardWidget::removeLayout()
{
	QModelIndexList selected = uiWidget->layoutsTableView->selectionModel()->selectedIndexes();
	foreach(const QModelIndex& idx, selected) {
		if( idx.column() == 0 ) {
			keyboardConfig->layouts.removeAt(idx.row());
		}
	}
	layoutsTableModel->refresh();
	uiChanged();
}

void KCMKeyboardWidget::layoutSelectionChanged()
{
	QModelIndexList selected = uiWidget->layoutsTableView->selectionModel()->selectedIndexes();
	uiWidget->removeLayoutBtn->setEnabled( ! selected.isEmpty() );
}

void KCMKeyboardWidget::initializeLayoutsUI()
{
	layoutsTableModel = new LayoutsTableModel(rules, flags, keyboardConfig, uiWidget->layoutsTableView);
	uiWidget->layoutsTableView->setModel(layoutsTableModel);
	connect(layoutsTableModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(uiChanged()));

//	connect(layoutsTableModel, SIGNAL(), this, SLOT(uiChanged()));
//    connect(uiWidget->layoutsTableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(layoutCellClicked(const QModelIndex &)));

    KIcon clearIcon = qApp->isLeftToRight() ? KIcon("edit-clear-locationbar-rtl") : KIcon("edit-clear-locationbar-ltr");
	uiWidget->xkbGrpClearBtn->setIcon(clearIcon);
	uiWidget->xkb3rdLevelClearBtn->setIcon(clearIcon);

	KIcon configIcon = KIcon("configure");
	uiWidget->xkbGrpShortcutBtn->setIcon(configIcon);
	uiWidget->xkb3rdLevelShortcutBtn->setIcon(configIcon);

    uiWidget->kdeKeySequence->setModifierlessAllowed(false);

	connect(uiWidget->addLayoutBtn, SIGNAL(clicked(bool)), this, SLOT(addLayout()));
	connect(uiWidget->removeLayoutBtn, SIGNAL(clicked(bool)), this, SLOT(removeLayout()));
//	connect(uiWidget->layoutsTable, SIGNAL(itemSelectionChanged ()), this, SLOT(layoutSelectionChanged()));
	connect(uiWidget->layoutsTableView->selectionModel(), SIGNAL(selectionChanged ( const QItemSelection &, const QItemSelection &)), this, SLOT(layoutSelectionChanged()));

	connect(uiWidget->showFlagChk, SIGNAL(clicked(bool)), this, SLOT(uiChanged()));

	connect(uiWidget->xkbGrpClearBtn, SIGNAL(clicked(bool)), this, SLOT(clearGroupShortcuts()));
	connect(uiWidget->xkb3rdLevelClearBtn, SIGNAL(clicked(bool)), this, SLOT(clear3rdLevelShortcuts()));

//	connect(uiWidget->xkbGrpClearBtn, SIGNAL(triggered(QAction*)), this, SLOT(uiChanged()));
//	connect(uiWidget->xkb3rdLevelClearBtn, SIGNAL(triggered(QAction*)), this, SLOT(uiChanged()));
	connect(uiWidget->kdeKeySequence, SIGNAL(keySequenceChanged (const QKeySequence &)), this, SLOT(uiChanged()));
	connect(uiWidget->switchingPolicyButtonGroup, SIGNAL(clicked(int)), this, SLOT(uiChanged()));

	connect(uiWidget->xkbGrpShortcutBtn, SIGNAL(clicked(bool)), this, SLOT(scrollToGroupShortcut()));
	connect(uiWidget->xkb3rdLevelShortcutBtn, SIGNAL(clicked(bool)), this, SLOT(scrollTo3rdLevelShortcut()));

	connect(uiWidget->configureLayoutsChk, SIGNAL(toggled(bool)), uiWidget->layoutsGroupBox, SLOT(setEnabled(bool)));
	connect(uiWidget->configureLayoutsChk, SIGNAL(toggled(bool)), uiWidget->shortcutsGroupBox, SLOT(setEnabled(bool)));
	connect(uiWidget->configureLayoutsChk, SIGNAL(toggled(bool)), uiWidget->switchingPolicyButtonGroup, SLOT(setEnabled(bool)));
	connect(uiWidget->configureLayoutsChk, SIGNAL(toggled(bool)), this, SLOT(uiChanged()));
}

void KCMKeyboardWidget::scrollToGroupShortcut()
{
    this->setCurrentIndex(TAB_ADVANCED);
	((XkbOptionsTreeModel*)uiWidget->xkbOptionsTreeView->model())->gotoGroup(GROUP_SWITCH_GROUP_NAME, uiWidget->xkbOptionsTreeView);
}

void KCMKeyboardWidget::scrollTo3rdLevelShortcut()
{
	this->setCurrentIndex(TAB_ADVANCED);
	((XkbOptionsTreeModel*)uiWidget->xkbOptionsTreeView->model())->gotoGroup(LV3_SWITCH_GROUP_NAME, uiWidget->xkbOptionsTreeView);
}

void KCMKeyboardWidget::clearGroupShortcuts()
{
	clearXkbGroup(GROUP_SWITCH_GROUP_NAME);
}

void KCMKeyboardWidget::clear3rdLevelShortcuts()
{
	clearXkbGroup(LV3_SWITCH_GROUP_NAME);
}

void KCMKeyboardWidget::clearXkbGroup(const QString& groupName)
{
	for(int ii=keyboardConfig->xkbOptions.count()-1; ii>=0; ii--) {
		if( keyboardConfig->xkbOptions[ii].startsWith(groupName+XKB_OPTION_GROUP_SEPARATOR) ) {
			keyboardConfig->xkbOptions.removeAt(ii);
		}
	}
	((XkbOptionsTreeModel*)uiWidget->xkbOptionsTreeView->model())->reset();
	uiWidget->xkbOptionsTreeView->update();
	updateXkbShortcutsButtons();
    emit changed(true);
}

void KCMKeyboardWidget::initializeXkbOptionsUI()
{
	XkbOptionsTreeModel* model = new XkbOptionsTreeModel(rules, keyboardConfig, uiWidget->xkbOptionsTreeView);
	uiWidget->xkbOptionsTreeView->setModel(model);
	connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(uiChanged()));

	connect(uiWidget->configureKeyboardOptionsChk, SIGNAL(toggled(bool)), uiWidget->xkbOptionsTreeView, SLOT(setEnabled(bool)));
	connect(uiWidget->configureKeyboardOptionsChk, SIGNAL(toggled(bool)), this, SLOT(uiChanged()));
}

void KCMKeyboardWidget::updateSwitcingPolicyUI()
{
    switch (keyboardConfig->switchingPolicy){
        case KeyboardConfig::SWITCH_POLICY_DESKTOP:
            uiWidget->switchByDesktopRadioBtn->setChecked(true);
            break;
        case KeyboardConfig::SWITCH_POLICY_APPLICATION:
            uiWidget->switchByApplicationRadioBtn->setChecked(true);
            break;
        case KeyboardConfig::SWITCH_POLICY_WINDOW:
            uiWidget->switchByWindowRadioBtn->setChecked(true);
            break;
        default:
        case KeyboardConfig::SWITCH_POLICY_GLOBAL:
            uiWidget->switchByGlobalRadioBtn->setChecked(true);
    }
}

void KCMKeyboardWidget::updateXkbShortcutButton(const QString& groupName, QPushButton* button)
{
	QStringList grpOptions = keyboardConfig->xkbOptions.filter(QRegExp("^"+groupName+XKB_OPTION_GROUP_SEPARATOR));
	switch( grpOptions.size() ) {
	case 0:
		button->setText(i18nc("no shourtcuts defined", "None"));
	break;
	case 1: {
		const OptionGroupInfo* optionGroupInfo = rules->getOptionGroupInfo(groupName);
		const OptionInfo* optionInfo = optionGroupInfo->getOptionInfo(grpOptions.first());
		button->setText(optionInfo->description);
	}
	break;
	default:
		button->setText(i18n("%1 shortcuts", grpOptions.size()));
	}
}

void KCMKeyboardWidget::updateXkbShortcutsButtons()
{
	updateXkbShortcutButton(GROUP_SWITCH_GROUP_NAME, uiWidget->xkbGrpShortcutBtn);
	updateXkbShortcutButton(LV3_SWITCH_GROUP_NAME, uiWidget->xkb3rdLevelShortcutBtn);
}

void KCMKeyboardWidget::updateShortcutsUI()
{
	updateXkbShortcutsButtons();

	delete actionCollection;
    actionCollection = new KActionCollection(this, KComponentData("keyboard"));
    KAction *a = NULL;
#include "bindings.cpp"
    a->setProperty("isConfigurationAction", true);
    uiWidget->kdeKeySequence->setKeySequence(a->globalShortcut().primary());
    kDebug() << "getting shortcut" << a->globalShortcut().toString();
}

void KCMKeyboardWidget::updateXkbOptionsUI()
{
    uiWidget->configureKeyboardOptionsChk->setChecked(keyboardConfig->resetOldXkbOptions);
}

void KCMKeyboardWidget::updateLayoutsUI()
{
	uiWidget->configureLayoutsChk->setChecked(keyboardConfig->configureLayouts);
	uiWidget->showFlagChk->setChecked(keyboardConfig->showFlag);

//    int i = 0;
//    uiWidget->layoutsTable->setRowCount(keyboardConfig->layouts.size());
//    foreach(QString layout, keyboardConfig->layouts) {
//		qDebug() << "adding" << layout;
//
//		QStringList lv = layout.split(KeyboardConfig::LAYOUT_VARIANT_SEPARATOR, QString::SkipEmptyParts);
//		QTableWidgetItem *item = new QTableWidgetItem(lv[0]);
//		const QPixmap* pixmap = flags->getPixmap(lv[0]);
//		if( pixmap != NULL ) {
//			item->setIcon(*new QIcon(*pixmap));
//		}
//		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
//		uiWidget->layoutsTable->setItem(i, 0, item);
//
//		const LayoutInfo* layoutInfo = rules->getLayoutInfo(lv[0]);
//		//TODO: if layoutInfo == NULL
//		QTableWidgetItem *item2 = new QTableWidgetItem(layoutInfo->description);
//		item2->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
//		uiWidget->layoutsTable->setItem(i, 1, item2);
//
//		QString variantText;
//		if( lv.count() > 1 ) {
//			//TODO: if variantInfo == NULL
//			const VariantInfo* variantInfo = layoutInfo->getVariantInfo(lv[1]);
//			variantText = variantInfo != NULL ? variantInfo->description : lv[1];
//		}
//		QTableWidgetItem *item3 = new QTableWidgetItem(variantText);
//		item3->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
//		uiWidget->layoutsTable->setItem(i, 2, item3);
//		i++;
//	}
//
//    connect(uiWidget->layoutsTable, SIGNAL(cellClicked(int,int)), this, SLOT(layoutCellClicked(int,int)));
}

void KCMKeyboardWidget::updateHardwareUI()
{
	int idx = uiWidget->keyboardModelComboBox->findData(keyboardConfig->keyboardModel);
	if( idx != -1 ) {
		uiWidget->keyboardModelComboBox->setCurrentIndex(idx);
	}
}
