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

#include "bindings.h"

#include <kdebug.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <klocalizedstring.h>
#include <kglobalaccel.h>

#include <QtCore/QList>

#include "x11_helper.h"
#include "flags.h"


static const char* actionName = I18N_NOOP("Switch to Next Keyboard Layout");
static const char* COMPONENT_NAME = "KDE Keyboard Layout Switcher";


KeyboardLayoutActionCollection::KeyboardLayoutActionCollection(QObject* parent, bool configAction_):
		KActionCollection(parent, KComponentData(COMPONENT_NAME)),
		configAction(configAction_)
{
	KAction* toggleAction = addAction( actionName );
	toggleAction->setText( i18n(actionName) );
	toggleAction->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_K));
	if( configAction ) {
	    toggleAction->setProperty("isConfigurationAction", true);
	}
	kDebug() << "Keyboard layout toggle shortcut" << toggleAction->globalShortcut().toString();
}

KeyboardLayoutActionCollection::~KeyboardLayoutActionCollection()
{
    clear();
}

KAction* KeyboardLayoutActionCollection::getToggeAction()
{
	return static_cast<KAction*>(action(0));
}

KAction* KeyboardLayoutActionCollection::createLayoutShortcutActon(const LayoutUnit& layoutUnit, const Rules* rules, bool autoload)
{
	QString longLayoutName = Flags::getLongText( layoutUnit, rules );
	QString actionName = "Switch keyboard layout to ";
	actionName += longLayoutName;
	KAction* action = addAction( actionName );
	action->setText( i18n("Switch keyboard layout to %1", longLayoutName) );
	KAction::GlobalShortcutLoading loading = autoload ? KAction::Autoloading : KAction::NoAutoloading;
	KShortcut shortcut = autoload ? KShortcut() : KShortcut(layoutUnit.getShortcut());
	action->setGlobalShortcut(shortcut, KAction::ShortcutTypes(KAction::ActiveShortcut /*| KAction::DefaultShortcut*/), loading);
	action->setData(layoutUnit.toString());
	if( configAction ) {
	    action->setProperty("isConfigurationAction", true);
	}
	kDebug() << "Registered layout shortcut" << action->globalShortcut(KAction::ActiveShortcut).primary().toString() << "for" << action->text() << "lu.shortcut" << layoutUnit.getShortcut().toString();
	return action;
}

void KeyboardLayoutActionCollection::setToggleShortcut(const QKeySequence& keySequence)
{
    KShortcut shortcut(keySequence);
    getToggeAction()->setGlobalShortcut(shortcut, KAction::ActiveShortcut, KAction::NoAutoloading);
    kDebug() << "Saving keyboard layout KDE shortcut" << shortcut.toString();
}

//KAction* KeyboardLayoutActionCollection::setShortcut(LayoutUnit& layoutUnit, const QKeySequence& keySequence, const Rules* rules)
//{
//	KAction* action = getAction(layoutUnit);
//	if( action == NULL && ! keySequence.isEmpty() ) {
//		action = createLayoutShortcutActon(layoutUnit, rules, false);
//	}
//	else if( action != NULL && keySequence.isEmpty() ){
////		action->setGlobalShortcut(KShortcut(keySequence), KAction::ActiveShortcut, KAction::NoAutoloading);	// do we need this?
//		removeAction(action);
//		action = NULL;
//	}
////	if( configAction ) {
////		layoutUnit.setShortcut(keySequence);	// shortcut was restored
////	}
//	return action;
//}

void KeyboardLayoutActionCollection::setLayoutShortcuts(QList<LayoutUnit>& layoutUnits, const Rules* rules)
{
	for (QList<LayoutUnit>::iterator i = layoutUnits.begin(); i != layoutUnits.end(); ++i) {
		LayoutUnit& layoutUnit = *i;
		if( ! layoutUnit.getShortcut().isEmpty() ) {
			createLayoutShortcutActon(layoutUnit, rules, false);
		}
	}
	kDebug() << "Cleaning component shortcuts on save" << KGlobalAccel::cleanComponent(COMPONENT_NAME);
}

void KeyboardLayoutActionCollection::loadLayoutShortcuts(QList<LayoutUnit>& layoutUnits, const Rules* rules)
{
	for (QList<LayoutUnit>::iterator i = layoutUnits.begin(); i != layoutUnits.end(); ++i) {
		LayoutUnit& layoutUnit = *i;
		KAction* action = createLayoutShortcutActon(layoutUnit, rules, true);
		QKeySequence shortcut = action->globalShortcut(KAction::ActiveShortcut).primary();	// shortcut was restored
		if( ! shortcut.isEmpty() ) {
			kDebug() << "Restored shortcut for" << layoutUnit.toString() << shortcut;
			layoutUnit.setShortcut(shortcut);
		}
		else {
			kDebug() << "Skipping empty shortcut for" << layoutUnit.toString();
			removeAction(action);
		}
	}
	kDebug() << "Cleaning component shortcuts on load" << KGlobalAccel::cleanComponent(COMPONENT_NAME);
}

//KAction* KeyboardLayoutActionCollection::getAction(const LayoutUnit& layoutUnit)
//{
//	for(int i=1; i<actions().size(); i++) {
//		if( action(i)->data() == layoutUnit.toString() )
//			return static_cast<KAction*>(action(i));
//	}
//	return NULL;
//}

void KeyboardLayoutActionCollection::resetLayoutShortcuts()
{
	for(int i=1; i<actions().size(); i++) {
		static_cast<KAction*>(action(i))->setGlobalShortcut(KShortcut(), KAction::ShortcutTypes(KAction::ActiveShortcut | KAction::DefaultShortcut), KAction::NoAutoloading);
	}
}
