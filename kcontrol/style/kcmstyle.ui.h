/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <qsettings.h>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kipc.h>
#include <kmessagebox.h>
#include <kaboutdata.h>

#include <dcopclient.h>

void  KCMStyle::init()
{
	load();
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

void  KCMStyle::setDirty()
{
	emit changed(true);
}

void  KCMStyle::defaults()
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

void  KCMStyle::load()
{

	// qtconfig settings via QSetting

	QSettings settings;
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

void  KCMStyle::save()
{

	KMessageBox::information (this, i18n("Pure Qt applications need to be restarted in order "
			"to let the changes take effect there."), QString::null, "QtAppApplyMsg");

	// qtconfig settings via QSetting

	QSettings settings;
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

