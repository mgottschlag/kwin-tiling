/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef BACKGROUNDLISTMODEL_H
#define BACKGROUNDLISTMODEL_H

#include "backgroundpackage.h"

#include <QAbstractListModel>
#include <KDirWatch>

class Background;

class BackgroundContainer
{
public:
    virtual ~BackgroundContainer() {};
    virtual bool contains(const QString &path) const = 0;
};

class BackgroundListModel : public QAbstractListModel, public BackgroundContainer
{
public:
    BackgroundListModel(float ratio, QObject *listener);
    virtual ~BackgroundListModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Background* package(int index) const;

    void reload();
    void reload(const QStringList &selected);
    void addBackground(const QString &path);
    int indexOf(const QString &path) const;
    void removeBackground(const QString &path);
    virtual bool contains(const QString &bg) const;

    static QList<Background *> findAllBackgrounds(const BackgroundContainer *container,
                                                    const QString &path, float ratio);

    void setWallpaperSize(QSize size);
    void setResizeMethod(Background::ResizeMethod resizeMethod);

private:
    QObject *m_listener;
    QList<Background*> m_packages;
    float m_ratio;
    KDirWatch m_dirwatch;

    QSize m_size;
    Background::ResizeMethod m_resizeMethod;
};

#endif // BACKGROUNDLISTMODEL_H
