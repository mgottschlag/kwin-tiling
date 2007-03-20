#ifndef __RULES_H__
#define __RULES_H__

#include <QString>
#include <QHash>
#include <QMap>

#include "x11helper.h"

class XkbRules
{
public:

  XkbRules(bool layoutsOnly=false);

  const QHash<QString, QString> &models() const { return m_models; }
  const QHash<QString, QString> &layouts() const { return m_layouts; }
  const QHash<QString, XkbOption> &options() const { return m_options; }
  const QHash<QString, XkbOptionGroup> &optionGroups() const { return m_optionGroups; }

  QStringList getAvailableVariants(const QString& layout);
  unsigned int getDefaultGroup(const QString& layout, const QString& includeGroup);

  bool isSingleGroup(const QString& layout);

  static bool areLayoutsClean() { return m_layoutsClean; }

private:

  QHash<QString, QString> m_models;
  QHash<QString, QString> m_layouts;
  QHash<QString, XkbOptionGroup> m_optionGroups;
  QHash<QString, XkbOption> m_options;
  QMap<QString, unsigned int> m_initialGroups;
  QHash<QString, QStringList*> m_varLists;
  
  QString X11_DIR;	// pseudo-constant

  static bool m_layoutsClean;

#ifdef HAVE_XKLAVIER
  void loadNewRules(bool layoutsOnly);
#else
  QStringList m_oldLayouts;
  QStringList m_nonLatinLayouts;

  void loadRules(QString filename, bool layoutsOnly=false);
  void loadGroups(QString filename);
  void loadOldLayouts(QString filename);
  void fixOptionGroups();
#endif
};


#endif
