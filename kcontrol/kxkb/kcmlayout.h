#ifndef __KCM_LAYOUT_H__
#define __KCM_LAYOUT_H__


#include <kcmodule.h>

#include <QHash>
#include <QString>
#include <qlistview.h>

#include "kxkbconfig.h"


class QWidget;
class OptionListItem;
class Q3ListViewItem;
class Ui_LayoutConfigWidget;
class XkbRules;

class LayoutConfig : public KCModule
{
  Q_OBJECT

public:
  LayoutConfig(QWidget *parent, const QStringList &args);
  virtual ~LayoutConfig();

  void load();
  void save();
  void defaults();
  void initUI();

protected:
  QString createOptionString();
  void updateIndicator(Q3ListViewItem* selLayout);

protected slots:
  void moveUp();
  void moveDown();
  void variantChanged();
  void displayNameChanged(const QString& name);
//  void latinChanged();
  void layoutSelChanged(Q3ListViewItem *);
  void loadRules();
  void updateLayoutCommand();
  void updateOptionsCommand();
  void add();
  void remove();

  void changed();

private:
  Ui_LayoutConfigWidget* widget;

  XkbRules *m_rules;
  KxkbConfig m_kxkbConfig;
  QHash<QString, OptionListItem*> m_optionGroups;

  QWidget* makeOptionsTab();
  void updateStickyLimit();
  void updateAddButton();
  void updateDisplayName();
  static LayoutUnit getLayoutUnitKey(Q3ListViewItem *sel);
};


#endif
