/*  This file is part of the KDE project

    Current maintainer 
    Copyright (C) 2002 Dawit Alemayehu <adawit@kde.org>

    Advanced web shortcuts 
    Copyright (C) 2001 Andreas Hochsteger <e9625392@student.tuwien.ac.at>
    
    Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
    Copyright (C) 1999 Yves Arrouye <yves@realnames.com>
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* $Id$ */

#ifndef __KURISearchFILTERENGINE_H__
#define __KURISearchFILTERENGINE_H__

#include <qvaluelist.h>
#include <qstringlist.h>

#include <kservice.h>

class KURL;


class KURISearchFilterEngine
{
public:
  typedef QMap <QString, QString> SubstMap;
  
  KURISearchFilterEngine();
  ~KURISearchFilterEngine() {};

  QCString name() const;
  
  QString webShortcutQuery (const KURL&) const;
  
  QString autoWebSearchQuery (const KURL&) const;
  
  bool verbose() const { return m_bVerbose; }

  void loadConfig();
  
  static KURISearchFilterEngine *self();

protected:
  QString formatResult (const QString& url, const QString& cset1, const QString& cset2,
                        const QString& query, bool isMalformed) const;
  
  QString formatResult (const QString& url, const QString& cset1, const QString& cset2,
                        const QString& query, bool isMalformed, SubstMap& map) const;

private:
  QStringList modifySubstitutionMap (SubstMap& map, const QString& query) const;  
  
  QString substituteQuery (const QString& url, SubstMap &map, 
                           const QString& userquery, const int encodingMib) const;
  
  bool m_bVerbose;  
  bool m_bWebShortcutsEnabled;
  
  QString m_defaultSearchEngine;
  static KURISearchFilterEngine *s_pSelf;
};

#endif
