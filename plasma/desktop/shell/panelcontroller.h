/*
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

#ifndef PANELCONTROLLER_H
#define PANELCONTROLLER_H

#include "controllerwindow.h"

class QLabel;

namespace Plasma
{
    class Dialog;
} // namespace Plasma

class PositioningRuler;
class ToolButton;

#include "panelview.h"

class PanelController : public ControllerWindow
{
    Q_OBJECT

public:
    enum DragElement {
        NoElement = 0,
        ResizeButtonElement,
        MoveButtonElement
    };

    PanelController(QWidget* parent = 0);
    ~PanelController();

    void setContainment(Plasma::Containment *containment);
    void resizePanel(const QSizeF newSize);

    void setLocation(const Plasma::Location &loc);

    void setOffset(int newOffset);
    int offset() const;

    void setAlignment(const Qt::Alignment &newAlignment);
    Qt::Alignment alignment() const;

    void setVisibilityMode(PanelView::VisibilityMode);
    PanelView::VisibilityMode panelVisibilityMode() const;

    void switchToController();

public Q_SLOTS:
    virtual void closeIfNotFocussed();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void resizeEvent(QResizeEvent *event);
    void moveEvent(QMoveEvent *event);
    void showEvent(QShowEvent *event);

Q_SIGNALS:
     void offsetChanged(int offset);
     void alignmentChanged(Qt::Alignment);
     void locationChanged(Plasma::Location);
     void panelVisibilityModeChanged(PanelView::VisibilityMode mode);
     void partialMove(const QPoint &delta);

private:
    void mouseMoveFilter(QMouseEvent *event);
    ToolButton *addTool(QAction *action, QWidget *parent, Qt::ToolButtonStyle style = Qt::ToolButtonTextBesideIcon);
    ToolButton *addTool(const QString iconName, const QString iconText, QWidget *parent, Qt::ToolButtonStyle style = Qt::ToolButtonTextBesideIcon, bool checkButton = false);
    void syncRuler();
    void resizeFrameHeight(const int newHeight);
    void syncToLocation();

private Q_SLOTS:
    void themeChanged();
    void switchToWidgetExplorer();
    void rulersMoved(int offset, int minLength, int maxLength);
    void alignToggled(bool toggle);
    void panelVisibilityModeChanged(bool toggle);
    void settingsPopup();
    void maximizePanel();
    void addSpace();

private:
    class ButtonGroup;

    QWidget *m_configWidget;
    QBoxLayout *m_extLayout;
    QBoxLayout *m_layout;
    QLabel *m_alignLabel;
    QLabel *m_modeLabel;
    DragElement m_dragging;
    QPoint m_startDragControllerPos;
    QPoint m_startDragMousePos;
    Plasma::Dialog *m_optionsDialog;
    QBoxLayout *m_optDialogLayout;
    ToolButton *m_settingsTool;
    Plasma::Svg *m_iconSvg;

    ToolButton *m_moveTool;
    ToolButton *m_sizeTool;

    //Alignment buttons
    ToolButton *m_leftAlignTool;
    ToolButton *m_centerAlignTool;
    ToolButton *m_rightAlignTool;

    //Panel mode buttons
    ToolButton *m_normalPanelTool;
    ToolButton *m_autoHideTool;
    ToolButton *m_underWindowsTool;
    ToolButton *m_overWindowsTool;

    ToolButton *m_closeControllerTool;

    //Widgets for actions
    QList<QWidget *> m_actionWidgets;

    PositioningRuler *m_ruler;

    bool m_drawMoveHint;

    QPoint m_lastPos;
    QRect m_controllerRect;

    ToolButton *m_expandTool;
};


#endif // multiple inclusion guard

