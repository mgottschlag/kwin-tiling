
#ifndef __KXSITEM_H__
#define __KXSITEM_H__

#include <kconfig.h>

class KXSConfigItem
{
public:
  KXSConfigItem(const QString &name, KConfig &config);
  virtual ~KXSConfigItem() {}

  virtual QString command() = 0;
  virtual void save(KConfig &confg) = 0;

protected:
  QString mName;
  QString mLabel;
};

class KXSRangeItem : public KXSConfigItem
{
public:
  KXSRangeItem(const QString &name, KConfig &config);

  virtual QString command();
  virtual void save(KConfig &config);

protected:
  QString mSwitch;
  int mMinimum;
  int mMaximum;
  int mValue;
};

class KXSBoolItem : public KXSConfigItem
{
public:
  KXSBoolItem(const QString &name, KConfig &config);

  virtual QString command();
  virtual void save(KConfig &config);

protected:
  QString mSwitchOn;
  QString mSwitchOff;
  bool    mValue;
};

class KXSSelectItem : public KXSConfigItem
{
public:
  KXSSelectItem(const QString &name, KConfig &config);

  virtual QString command();
  virtual void save(KConfig &config);

protected:
  QStringList mOptions;
  QStringList mSwitches;
  int         mValue;
};

#endif

