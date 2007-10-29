/*
 *  Copyright (C) 2003-2006 Andriy Rysin (rysin@kde.org)
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

#ifndef __RULES_H__
#define __RULES_H__


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

  QList<XkbVariant> getAvailableVariants(const QString& layout);
  unsigned int getDefaultGroup(const QString& layout, const QString& includeGroup);

  bool isSingleGroup(const QString& layout);

private:

  QHash<QString, QString> m_models;
  QHash<QString, QString> m_layouts;
  QHash<QString, XkbOptionGroup> m_optionGroups;
  QHash<QString, XkbOption> m_options;
  QHash<QString, QList<XkbVariant>*> m_varLists;

  QString X11_DIR;	// pseudo-constant

  static bool m_layoutsClean;

#ifdef HAVE_XKLAVIER
  void loadNewRules(bool layoutsOnly);
#else
  void loadRules(const QString &filename, bool layoutsOnly=false);
  void fixOptionGroups();
#endif
};

#endif
