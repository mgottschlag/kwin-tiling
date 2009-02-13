/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
  Copyright (c) 2008 by Petri Damsten <damu@iki.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef DESKTOPTHEMEDETAILS_H
#define DESKTOPTHEMEDETAILS_H

#include <kcmodule.h>
#include "ui_DesktopThemeDetails.h"

class ThemeModel;

class DesktopThemeDetails : public KCModule, public Ui::DesktopThemeItems
{
    Q_OBJECT
public:
    DesktopThemeDetails(QWidget* parent, const QVariantList &args);
    ~DesktopThemeDetails();

    void reloadConfig();

public slots:
    void replacementItemChanged();
    void resetThemeDetails();
    void toggleAdvancedVisible();
    void save();
    void removeTheme();
    void exportTheme();
    void newThemeInfoChanged();

private:
    void updateReplaceItemList(const int& item);
    void loadThemeItems();
    QString displayedItemText(int item);
    bool isCustomized(const QString& theme);
    void clearCustomized(const QString& themeRoot);

private slots:
    void getNewThemes();
    void cleanup();

private:
    ThemeModel* m_themeModel;

    QHash<QString, int> m_items; // theme items
    QHash<int, QString> m_itemPaths; // theme item paths
    QHash<int, QString> m_itemIcons; //theme item icons
    QHash<QString, int> m_themes; // installed themes
    QHash<int, QString> m_themeRoots; // installed themes root paths
    QHash<int, int> m_itemThemeReplacements; // source theme for item replacements 
    QHash<int, QString>m_itemFileReplacements; //non-theme source files for item replacements   

    bool m_themeCustomized;
    QString m_baseTheme;
};

#endif // DESKTOPTHEMEDETAILS_H
