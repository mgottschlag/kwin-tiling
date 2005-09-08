#ifndef __RULES_H__
#define __RULES_H__


#include <qstring.h>
#include <q3dict.h>
#include <qmap.h>

class KeyRules
{
public:

  KeyRules();

  const Q3Dict<char> &models() const { return m_models; };
  const Q3Dict<char> &layouts() const { return m_layouts; };
  const Q3Dict<char> &options() const { return m_options; };

  void parseVariants(const QStringList& vars, Q3Dict<char>& variants, bool chkVars=true);
//  static QStringList rules(QString path = QString::null);

  QStringList getVariants(const QString& layout);
  unsigned int getGroup(const QString& layout, const char* baseGr);

  bool isXFree_v43() { return m_xfree43; }
  bool isSingleGroup(const QString& layout) { return m_xfree43 && !m_oldLayouts.contains(layout)
			     && !m_nonLatinLayouts.contains(layout); } 

protected:

  void loadRules(QString filename);
  void loadGroups(QString filename);
  void loadOldLayouts(QString filename);

private:

  Q3Dict<char> m_models;
  Q3Dict<char> m_layouts;
  Q3Dict<char> m_options;
  QMap<QString, unsigned int> m_initialGroups;
  Q3Dict<QStringList> m_varLists;
  QStringList m_oldLayouts;
  QStringList m_nonLatinLayouts;
  bool m_xfree43;
  
  QString X11_DIR;	// pseudo-constant
};


#endif
