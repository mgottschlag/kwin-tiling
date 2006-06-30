#ifndef __RULES_H__
#define __RULES_H__

#include <qstring.h>
#include <qhash.h>
#include <qmap.h>


class XkbRules
{
public:

  XkbRules(bool layoutsOnly=false);

  const QHash<QString, QString> &models() const { return m_models; };
  const QHash<QString, QString> &layouts() const { return m_layouts; };
  const QHash<QString, QString> &options() const { return m_options; };
  
  QStringList getAvailableVariants(const QString& layout);
  unsigned int getDefaultGroup(const QString& layout, const QString& includeGroup);

  bool isSingleGroup(const QString& layout);

protected:

  void loadRules(QString filename, bool layoutsOnly=false);
  void loadGroups(QString filename);
  void loadOldLayouts(QString filename);

private:

  QHash<QString, QString> m_models;
  QHash<QString, QString> m_layouts;
  QHash<QString, QString> m_options;
  QMap<QString, unsigned int> m_initialGroups;
  QHash<QString, QStringList*> m_varLists;
  QStringList m_oldLayouts;
  QStringList m_nonLatinLayouts;
  
  QString X11_DIR;	// pseudo-constant
  
//  void fixLayouts();
};


#endif
