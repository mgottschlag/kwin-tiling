/**
 * Copyright (c) 2000 Antonio Larrosa <larrosa@kde.org>
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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ICONTHEMES_H
#define ICONTHEMES_H

#include <kcmodule.h>
#include <qmap.h>
#include <klistview.h>

class QPushButton;
class DeviceManager;
class QCheckBox;
class QStringList;


class IconThemesConfig : public KCModule
{
  Q_OBJECT

public:
  IconThemesConfig(QWidget *parent = 0L, const char *name = 0L);
  virtual ~IconThemesConfig();

  void loadThemes();
  bool installThemes(const QStringList &themes, const QString &archiveName);
  QStringList findThemeDirs(const QString &archiveName);

  void updateRemoveButton();

  void load();
  void save();
  void defaults();

  int buttons();

protected slots:
  void themeSelected(QListViewItem *item);
  void installNewTheme();
  void removeSelectedTheme();

private:
  QListViewItem *iconThemeItem(const QString &name);

  KListView *m_iconThemes;
  QPushButton *m_removeButton;

  QLabel *m_previewExec;
  QLabel *m_previewFolder;
  QLabel *m_previewDocument;
  QListViewItem *m_defaultTheme;
  QMap <QString, QString>m_themeNames;
  bool m_bChanged;
};

#endif // ICONTHEMES_H

