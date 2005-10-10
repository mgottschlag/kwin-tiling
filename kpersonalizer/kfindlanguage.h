/***************************************************************************
                          kfindlanguage.h  -  description
                             -------------------
    begin                : Tue May 22 2001
    copyright            : (C) 2002 by Carsten Wolff
    email                : wolff@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KFINDLANGUAGE_H
#define KFINDLANGUAGE_H

class KFindLanguage {
public:
	KFindLanguage();
	~KFindLanguage();
	QStringList getLangList() const;
	QMap<QString,QString> getLangMap() const;
	QString getBestLang() const;
	QString getOldLang() const;
	QString getCountry() const;
private:
	QStringList m_langlist;          // stores tags like "en_US"
	QMap<QString,QString> m_langmap; // stores tag -> name pairs
	QString m_country;
	QString m_oldlang;
	QString m_bestlang;
};

#endif
