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

private:
    void updateReplaceItemList(const QString& item);
    void loadThemeItems();
    bool isCustomized(const QString& theme);
    void clearCustomized();

private slots:
    void getNewThemes();
    void cleanup();

private:
    ThemeModel* m_themeModel;
    QHash<QString, QString> m_themeItems;
    QHash<QString, QString> m_themeReplacements;
    QHash<QString, QString> m_dropListFiles;
    bool m_themeCustomized;
    QString m_baseTheme;
};

#endif // DESKTOPTHEMEDETAILS_H
