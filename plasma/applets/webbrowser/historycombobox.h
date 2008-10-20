/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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


#ifndef PLASMA_HISTORYCOMBOBOX_H
#define PLASMA_HISTORYCOMBOBOX_H

#include <QtGui/QGraphicsProxyWidget>

#include <KGlobalSettings>
#include <KCompletionBase>

class KHistoryComboBox;
class KPixmapProvider;

#include "webbrowser_export.h"

namespace Plasma
{

class WEBBROWSER_EXPORT HistoryComboBox : public QGraphicsProxyWidget
{
    Q_OBJECT

    Q_PROPERTY(QGraphicsWidget* parentWidget READ parentWidget)
    Q_PROPERTY(QString stylesheet READ stylesheet WRITE setStylesheet)
    Q_PROPERTY(KHistoryComboBox* nativeWidget READ nativeWidget)

public:
    explicit HistoryComboBox(QGraphicsWidget *parent = 0);
    ~HistoryComboBox();

    /**
     * Sets the style sheet used to control the visual display of this TextEdit
     *
     * @arg stylehseet a CSS string
     */
    void setStylesheet(const QString &stylesheet);

    /**
     * @return the stylesheet currently used with this widget
     */
    QString stylesheet();

    /**
     * @return the native widget wrapped by this TextEdit
     */
    KHistoryComboBox* nativeWidget() const;

    QStringList historyItems() const;
    bool removeFromHistory(const QString &item);
    void reset();
    void setHistoryItems(const QStringList &items);
    QString currentText() const;
    void insertUrl(int index, const KUrl &url);
    void setDuplicatesEnabled(bool enabled);

public Q_SLOTS:
    void addToHistory(const QString &item);

Q_SIGNALS:
    void cleared();
    void activated(int index);
    void returnPressed(const QString &);
    void textChanged(const QString &);
    void returnPressed();

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);

private:
    class Private;
    Private * const d;
};

} // namespace Plasma

#endif // multiple inclusion guard
