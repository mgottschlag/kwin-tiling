/*
 * $Id$
 *
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 *
 * Portions Copyright (C) 2000 TrollTech AS.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you wish to add, delete or rename slots use Qt Designer which will
 ** update this file, preserving your code. Create an init() slot in place of
 ** a constructor, and a destroy() slot in place of a destructor.
 *****************************************************************************/

#include <qsettings.h>
#include <qstylefactory.h>
#include <qobjectlist.h>
#include <qstyle.h>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kipc.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kglobalsettings.h>

#include <dcopclient.h>

#include "stylepreview.h"

void KCMStyle::init()
{
	load();
	initStyle();
	initEffects();
}

void KCMStyle::setDirty()
{
	emit changed(true);
}

void KCMStyle::defaults()
{
	defaultsStyle();
	defaultsEffects();
};

void KCMStyle::load()
{
	QSettings settings;

	// Page1 - Build up the Style ListBox
	loadStyle(settings);
	// Page2 and 3
	loadEffects(settings);
}

void KCMStyle::save()
{
	QSettings settings;

	// Page1
	saveStyle(settings);
	// Page2 and 3
	saveEffects(settings);

	// Is this equivalent to KConfig's sync()?!!
	// How darn stupid.
	//delete settings;

	// Propagate changes to all Qt applications.
	QApplication::x11_apply_settings();

	// Now allow KDE apps to reconfigure themselves.
	//KIPC::sendMessageAll(KIPC::StyleChanged);	// REDUNDANT - use Qt's method.
	KIPC::sendMessageAll(KIPC::ToolbarStyleChanged, 0);
	KIPC::sendMessageAll(KIPC::SettingsChanged);

	// Hmm, I'll soon fix KWin from flickering so000 much.. - Karol.
	// kapp->dcopClient()->send("kwin*", "", "reconfigure()", "");
}

void KCMStyle::destroy()
{
	delete appliedStyle;
}


// ----------------------------------------------------------------
// All the Style Switching / Preview stuff
// ----------------------------------------------------------------

void KCMStyle::initStyle()
{

	// ### Consider moving all the stuff over to QWorkspace and use 
	// kwin-styled MDI Windows for preview, clearly post 3.0 stuff
	// as this required a major rewrite of KWin's style engine (Daniel)

	appliedStyle = 0L;
	palette = QApplication::palette();
	pbConfigureStyle->setEnabled(false);

	// Connect all required stuff
	connect(lbStyle, SIGNAL(selectionChanged()), this, SLOT(setDirty()));
	connect(lbStyle, SIGNAL(highlighted(const QString&)), this, SLOT(updateStyleTimer(const QString&)));
	connect(&switchStyleTimer, SIGNAL(timeout()), this, SLOT(styleChanged()));
}

void KCMStyle::loadStyle(QSettings& settings)
{
	// Insert all the styles into the listbox.
	lbStyle->clear();
	QStringList styles = QStyleFactory::keys();
	lbStyle->insertStringList( styles );
	lbStyle->setSelectionMode( QListBox::Single );
	lbStyle->sort();

	// Find out which style is currently being used
	// This uses Qtconfig's method of style matching for compatibility
	QString currentStyle = settings.readEntry("/qt/style");
	if (currentStyle.isNull())
		currentStyle = QApplication::style().className();

	QStringList::iterator it = styles.begin();
	int styleNo = 0;
	while (it != styles.end())
	{
		if (*it == currentStyle)
			break;
		styleNo++;
		it++;
	}

	// Check if we found a match...
	if ( styleNo < lbStyle->count() )
		lbStyle->setCurrentItem(styleNo);
	else {
		// No match was found, so try to find the
		// closest match to the Style's className
		styleNo = 0;
		it = styles.begin();
		while (it != styles.end())
		{
			if (currentStyle.contains(*it))
				break;
			styleNo++;
			it++;
		}
		if (styleNo < lbStyle->count() )
			lbStyle->setCurrentItem(styleNo);
		else {
			lbStyle->insertItem(i18n("Unknown Style"));
			lbStyle->setCurrentItem( lbStyle->count() - 1 );
		}
	}
	currentStyle = lbStyle->currentText();
}

void KCMStyle::saveStyle(QSettings& settings)
{
	qWarning((QString("Changing Style to: ") + currentStyle).latin1());
	settings.writeEntry("/qt/style", currentStyle);

}

void KCMStyle::styleChanged()
{
	switchStyle( currentStyle );
}

void KCMStyle::defaultsStyle()
{
	// Select default style
	QListBoxItem* item;
	if ( (item = lbStyle->findItem("Highcolor")) )
		lbStyle->setCurrentItem(item);
	else if ( (item = lbStyle->findItem("Default")) )
		lbStyle->setCurrentItem(item);
	else if ( (item = lbStyle->findItem("Windows")) )
		lbStyle->setCurrentItem(item);
	else if ( (item = lbStyle->findItem("Platinum")) )
		lbStyle->setCurrentItem(item);
	else if ( (item = lbStyle->findItem("Motif")) )
		lbStyle->setCurrentItem(item);
	else
		lbStyle->setCurrentItem((int)0);	// Use any available style

	// Other stuff...
}

void KCMStyle::updateStyleTimer(const QString& style)
{
    currentStyle = style;
    switchStyleTimer.start(500, TRUE);
}

void KCMStyle::switchStyle(const QString& styleName)
{
    // Create an instance of the new style...
    QStyle* style = QStyleFactory::create(styleName);
    if (!style)
        return;

    setStyleRecursive( stylePreview, style );

    // set "titlebar" color
    stylePreview->titleBar->setPaletteBackgroundColor(
            KGlobalSettings::activeTitleColor());

    stylePreview->titleBar->setPaletteForegroundColor(
            KGlobalSettings::activeTextColor());

    delete appliedStyle;
    appliedStyle = style;
}

void KCMStyle::setStyleRecursive(QWidget* w, QStyle* s)
{
    // Don't let broken styles kill the palette
    // for other styles being previewed. (e.g SGI style)
    w->setPalette(palette);

    // Apply the new style.
    w->setStyle(s);

    // Recursively update all children.
    const QObjectList *children = w->children();
    if (!children)
        return;

    // Apply the style to each child widget.
    QPtrListIterator<QObject> childit(*children);
    QObject *child;
    while ((child = childit.current()) != 0)
    {
        ++childit;
        if (child->isWidgetType())
            setStyleRecursive((QWidget *) child, s);
    }
}

// ----------------------------------------------------------------
// All the Effects stuff
// ----------------------------------------------------------------

void KCMStyle::initEffects()
{
	connect(cbToolbarIcons, SIGNAL(highlighted(int)), SLOT(setDirty()));
	connect(cboxHoverButtons, SIGNAL(toggled(bool)), SLOT(setDirty()));
	connect(cboxToolbarsHighlight, SIGNAL(toggled(bool)), SLOT(setDirty()));
	connect(cboxEnableGUIEffects, SIGNAL(toggled(bool)), SLOT(setDirty()));
	connect(cboxEnableTooltips, SIGNAL(toggled(bool)), SLOT(setDirty()));
	connect(cbMenuEffect, SIGNAL(highlighted(int)), SLOT(setDirty()));
	connect(cbComboEffect, SIGNAL(highlighted(int)), SLOT(setDirty()));
	connect(cbTooltipEffect, SIGNAL(highlighted(int)), SLOT(setDirty()));
	connect(cboxIconSupport, SIGNAL(toggled(bool)), SLOT(setDirty()));
	connect(cboxTearOffHandles, SIGNAL(toggled(bool)), SLOT(setDirty()));

	containerFrame->setEnabled(false);
}

void KCMStyle::loadEffects(QSettings& settings)
{
	QStringList effects = settings.readListEntry("/qt/GUIEffects");

	for (QStringList::Iterator it = effects.begin(); it != effects.end(); ++it )
	{

	if (*it == "general")
		cboxEnableGUIEffects->setChecked(true);
	if (*it == "animatemenu")
		cbMenuEffect->setCurrentItem(1);
	if (*it == "fademenu")
		cbMenuEffect->setCurrentItem(2);
	if (*it == "animatecombo")
		cbComboEffect->setCurrentItem(1);
	if (*it == "animatetooltip")
		cbTooltipEffect->setCurrentItem(1);
	if (*it == "fadetooltip")
		cbTooltipEffect->setCurrentItem(2);
    }

	// KDE's Part via KConfig

	KConfig config;
	config.setGroup("Toolbar style");
	cboxHoverButtons->setChecked(config.readBoolEntry("Highlighting", true));
	cboxToolbarsHighlight->setChecked(config.readBoolEntry("TransparentMoving", false));

	QString tbIcon = config.readEntry("IconText", "IconOnly");
	if (tbIcon == "IconOnly")
		cbToolbarIcons->setCurrentItem(0);
	else if (tbIcon == "TextOnly")
		cbToolbarIcons->setCurrentItem(1);
	else if (tbIcon == "IconTextRight")
		cbToolbarIcons->setCurrentItem(2);
	else if (tbIcon == "IconTextBottom")
		cbToolbarIcons->setCurrentItem(3);


	config.setGroup("KDE");
	cboxIconSupport->setChecked(config.readBoolEntry("ShowIconsOnPushButtons", true));
	cboxEnableTooltips->setChecked(!config.readBoolEntry("EffectNoTooltip", false));
	cboxTearOffHandles->setChecked(config.readBoolEntry("InsertTearOffHandle",true));

}

void KCMStyle::saveEffects(QSettings& settings)
{
   // qtconfig settings via QSetting

   QStringList effects;
   if (cboxEnableGUIEffects->isChecked()) {
      effects << "general";

      switch (cbMenuEffect->currentItem()) {
      case 1: effects << "animatemenu"; break;
      case 2: effects << "fademenu"; break;
      }

      switch (cbComboEffect->currentItem()) {
      case 1: effects << "animatecombo"; break;
      }

      switch (cbTooltipEffect->currentItem()) {
      case 1: effects << "animatetooltip"; break;
      case 2: effects << "fadetooltip"; break;
      }
   } else
      effects << "none";

   settings.writeEntry("/qt/GUIEffects", effects);

   // KDE's Part via KConfig

   KConfig *config = kapp->config();

   config->setGroup("Toolbar style");
   config->writeEntry("Highlighting", cboxHoverButtons->isChecked(), true, true);
   config->writeEntry("TransparentMoving", cboxToolbarsHighlight->isChecked(), true, true);

   QString tbIcon;

   switch (cbToolbarIcons->currentItem()) {
   case 0:
      tbIcon = QString::fromLatin1("IconOnly");
      break;
   case 1:
      tbIcon = QString::fromLatin1("TextOnly");
      break;
   case 2:
      tbIcon = QString::fromLatin1("IconTextRight");
      break;
   case 3:
      tbIcon = QString::fromLatin1("IconTextBottom");
      break;
   default:
      tbIcon = QString::fromLatin1("IconOnly");
   }
   config->writeEntry("IconText", tbIcon, true, true);

   config->setGroup("KDE");
   config->writeEntry("ShowIconsOnPushButtons", cboxIconSupport->isChecked(), true, true);
   config->writeEntry("EffectNoTooltip", !cboxEnableTooltips->isChecked(), true, true);
   config->writeEntry("InsertTearOffHandle", cboxTearOffHandles->isChecked(),true, true);

   config->sync();

   // Notify all KApplications && KWin about the updated style stuff
   KIPC::sendMessageAll(KIPC::StyleChanged);
   KIPC::sendMessageAll(KIPC::ToolbarStyleChanged, 0);
   KIPC::sendMessageAll(KIPC::SettingsChanged);
   kapp->dcopClient()->send("kwin*", "", "reconfigure()", "");

   QApplication::syncX();
}


void KCMStyle::defaultsEffects()
{
	cbToolbarIcons->setCurrentItem(0);
	cboxHoverButtons->setChecked(false);
	cboxToolbarsHighlight->setChecked(false);
	cboxEnableGUIEffects->setChecked(false);
	cboxEnableTooltips->setChecked(true);
	cbMenuEffect->setCurrentItem(0);
	cbComboEffect->setCurrentItem(0);
	cbTooltipEffect->setCurrentItem(0);
	cboxIconSupport->setChecked(false);
	cboxTearOffHandles->setChecked(true);
}


// ----------------------------------------------------------------
// Obligatory KCModule methods
// ----------------------------------------------------------------


const KAboutData* KCMStyle::aboutData() const
{
	KAboutData *about =
	new KAboutData(I18N_NOOP("kcmstyle"), I18N_NOOP("KDE Style Module"),  0, 0, KAboutData::License_GPL,
						I18N_NOOP("(c) 2001 Daniel Molkentin, Karol Szwed"));

	about->addAuthor("Daniel Molkentin", 0, "molkentin@kde.org");
	about->addAuthor("Karol Szwed", 0, "gallium@kde.org");

	return about;
}

QString  KCMStyle::quickHelp() const
{
	return i18n("<h1>Style Settings</h1>This module allows you to define "
					"the behaviour for some widgets inside of KDE applications. "
					"This includes the toolbar general animation effects.<br>"
					"Note that some settings like the GUI Effects will also "
					"apply to Qt only apps."); // ### REVISE!
}

// vim: set noet ts=4:
