
#include "kxsconfig.h"
#include <klocale.h>

//===========================================================================
KXSConfigItem::KXSConfigItem(const QString &name, KConfig &config)
  : mName(name)
{
  config.setGroup(name);
  mLabel = i18n(config.readEntry("Label").utf8());
}

//===========================================================================
KXSRangeItem::KXSRangeItem(const QString &name, KConfig &config)
  : KXSConfigItem(name, config)
{
  mMinimum = config.readNumEntry("Minimum");
  mMaximum = config.readNumEntry("Maximum");
  mValue   = config.readNumEntry("Value");
  mSwitch  = config.readEntry("Switch");
}

QString KXSRangeItem::command()
{
  return mSwitch.arg(mValue);
}

void KXSRangeItem::save(KConfig &config)
{
  config.setGroup(mName);
  config.writeEntry("Value", mValue);
}

//===========================================================================
KXSBoolItem::KXSBoolItem(const QString &name, KConfig &config)
  : KXSConfigItem(name, config)
{
  mValue = config.readBoolEntry("Value");
  mSwitchOn  = config.readEntry("SwitchOn");
  mSwitchOff = config.readEntry("SwitchOff");
}

QString KXSBoolItem::command()
{
  return mValue ? mSwitchOn : mSwitchOff;
}

void KXSBoolItem::save(KConfig &config)
{
  config.setGroup(mName);
  config.writeEntry("Value", mValue);
}

//===========================================================================
KXSSelectItem::KXSSelectItem(const QString &name, KConfig &config)
  : KXSConfigItem(name, config)
{
  mOptions = config.readListEntry("Options");
  mSwitches = config.readListEntry("Switches");
  mValue = config.readNumEntry("Value");
}

QString KXSSelectItem::command()
{
  QStringList::Iterator it = mSwitches.at(mValue);
  return (*it);
}

void KXSSelectItem::save(KConfig &config)
{
  config.setGroup(mName);
  config.writeEntry("Value", mValue);
}


