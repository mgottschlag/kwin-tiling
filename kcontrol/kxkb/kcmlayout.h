#ifndef __KCM_LAYOUT_H__
#define __KCM_LAYOUT_H__


#include <kcmodule.h>
#include <kglobalaccel.h>

#include <QString>
#include <q3listview.h>
#include "rules.h"

class OptionListItem : public Q3CheckListItem
{
public:

  OptionListItem(  OptionListItem *parent, const QString &text, Type tt,
      const QString &optionName );
  OptionListItem(  Q3ListView *parent, const QString &text, Type tt,
      const QString &optionName );
  ~OptionListItem() {}

  QString optionName() const { return m_OptionName; }

  OptionListItem *findChildItem(  const QString& text );

protected:
  QString m_OptionName;
};

class LayoutConfigWidget;
class LayoutConfig : public KCModule
{
  Q_OBJECT

public:

  LayoutConfig(KInstance *inst,QWidget *parent = 0L);
  virtual ~LayoutConfig();

  void load();
  void save();
  void defaults();

  void itemStateChanged(); // must be slot?

protected:
  QString createOptionString();

protected Q_SLOTS:

  void moveUp();
  void moveDown();
  void variantChanged();
  void latinChanged();
  void layoutSelChanged(Q3ListViewItem *);
  void ruleChanged();
  void updateLayoutCommand();
  void updateOptionsCommand();
  void add();
  void remove();

  void changed();

private:

  enum SwitchMode { Global, Application, Window } switchMode;

  LayoutConfigWidget* widget;

  Q3Dict<OptionListItem> m_optionGroups;
  Q3Dict<char> m_variants;
  Q3Dict<char> m_includes;
//  QString m_rule;
  KeyRules *m_rules;

  QWidget* makeOptionsTab();
  void updateStickyLimit();
};


#endif
