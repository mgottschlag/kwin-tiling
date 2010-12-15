/*
 *   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
 *   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>
 *   Copyright 2009 by Artur Duque de Souza <morpheuz@gmail.com>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SEARCHLAUNCH_CONTAINMENT_H
#define SEARCHLAUNCH_CONTAINMENT_H

#include "models/krunnermodel.h"

#include <Plasma/Containment>
#include <Plasma/DataEngine>

#include <QGraphicsLinearLayout>

class QTimer;
class StripWidget;
class ItemView;
class LinearAppletOverlay;
class IconActionCollection;
class KRunnerModel;
class KServiceModel;
class KKeySequenceWidget;

namespace Plasma
{
    class IconWidget;
    class LineEdit;
    class Frame;
    class FrameSvg;
}

class SearchLaunch : public Plasma::Containment
{
    Q_OBJECT
public:
    SearchLaunch(QObject *parent, const QVariantList &args);
    ~SearchLaunch();
    void init();

    void setOrientation(Qt::Orientation orientation);

    void constraintsEvent(Plasma::Constraints constraints);

    void createConfigurationInterface(KConfigDialog *parent);

protected:
    void focusInEvent(QFocusEvent *event);
    void paintInterface(QPainter *, const QStyleOptionGraphicsItem *, const QRect &);
    void changeEvent(QEvent *event);
    void doSearch(const QString &query, const QString &runner = QString());

public Q_SLOTS:
    void configChanged();
    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);

private Q_SLOTS:
    void toggleImmutability();
    void layoutApplet(Plasma::Applet* applet, const QPointF &pos);
    void appletRemoved(Plasma::Applet* applet);
    void restoreStrip();
    void updateConfigurationMode(bool config);
    void overlayRequestedDrop(QGraphicsSceneDragDropEvent *event);
    void resultsViewRequestedDrag(QModelIndex index);
    void availableScreenRegionChanged();
    void launchPackageManager();
    void configDialogFinished();

    void delayedQuery();
    void query();
    void searchReturnPressed();
    void launch(QModelIndex index);
    void addFavourite(const QModelIndex &index);
    void reset();

    void saveFavourites();

private:
    /**
     * update the formfactor based on the location
     */
    void setFormFactorFromLocation(Plasma::Location loc);

    Plasma::FrameSvg *m_background;
    KRunnerModel *m_runnerModel;
    KServiceModel *m_serviceModel;
    Plasma::IconWidget *m_backButton;

    int m_queryCounter;
    int m_maxColumnWidth;

    QTimer *m_searchTimer;

    Plasma::LineEdit *m_searchField;

    ItemView *m_resultsView;
    StripWidget *m_stripWidget;

    Qt::Orientation m_orientation;
    QString m_lastQuery;
    Plasma::IconWidget *m_firstItem;

    KSharedPtr<KService> m_packageManagerService;

    QGraphicsLinearLayout *m_mainLayout;
    QGraphicsLinearLayout *m_resultsLayout;
    QGraphicsLinearLayout *m_appletsLayout;
    QPointF m_buttonDownMousePos;
    LinearAppletOverlay *m_appletOverlay;

    IconActionCollection *m_iconActionCollection;
    QWeakPointer<KKeySequenceWidget> m_shortcutEditor;

    bool m_stripUninitialized;
};


#endif // PLASMA_SEARCHLAUNCH_H
