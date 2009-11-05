/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_PLASMAAPPLETITEMMODEL_P_H
#define PLASMA_PLASMAAPPLETITEMMODEL_P_H

#include <kplugininfo.h>
#include <Plasma/Applet>
#include "kcategorizeditemsviewmodels_p.h"

class PlasmaAppletItemModel;

/**
 * Implementation of the KCategorizedItemsViewModels::AbstractItem
 */
class PlasmaAppletItem : public QObject, public KCategorizedItemsViewModels::AbstractItem
{
    Q_OBJECT

public:
    enum FilterFlag {
        NoFilter = 0,
        Favorite = 1,
        Used = 2
    };

    Q_DECLARE_FLAGS(FilterFlags, FilterFlag)

    PlasmaAppletItem(PlasmaAppletItemModel *model, const KPluginInfo& info, FilterFlags flags = NoFilter);

    QString pluginName() const;
    virtual QString name() const;
    virtual QString category() const;
    virtual QString description() const;
    virtual QString license() const;
    virtual QString website() const;
    virtual QString version() const;
    virtual QString author() const;
    virtual QString email() const;

    virtual int running() const;
    virtual bool used() const;
    virtual void setUsed(bool used);
    virtual void setFavorite(bool favorite);

    //set how many instances of this applet are running
    virtual void setRunning(int count);
    virtual bool passesFiltering(const KCategorizedItemsViewModels::Filter & filter) const;
    virtual QVariantList arguments() const;
    virtual QMimeData *mimeData() const;
    virtual QStringList mimeTypes() const;

private:
    PlasmaAppletItemModel * m_model;
};

class PlasmaAppletItemModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit PlasmaAppletItemModel(KConfigGroup configGroup, QObject * parent = 0);

    QStringList mimeTypes() const;
    QSet<QString> categories() const;

    QMimeData *mimeData(const QModelIndexList &indexes) const;

    void setFavorite(const QString &plugin, bool favorite);
    void setApplication(const QString &app);
    void setRunningApplets(const QHash<QString, int> &apps);
    void setRunningApplets(const QString &name, int count);

    QString &Application();

private:
    QString m_application;
    QStringList m_favorites;
    QStringList m_used;
    KConfigGroup m_configGroup;

private slots:
    void populateModel();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PlasmaAppletItem::FilterFlags)

#endif /*PLASMAAPPLETSMODEL_H_*/
