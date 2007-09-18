/* Preview widget for KDE Display color scheme setup module
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * eventFilter code Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "previewwidget.h"

#include <KGlobalSettings>
#include <KColorScheme>

#include "ui_preview.h"

PreviewWidget::PreviewWidget(QWidget *parent) : QFrame(parent)
{
    Ui::preview ui;
    ui.setupUi(this);

    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    // set correct colors on... lots of things
    ui.frame->setBackgroundRole(QPalette::Base);
    ui.viewWidget->setBackgroundRole(QPalette::Base);
    ui.labelView0->setBackgroundRole(QPalette::Base);
    ui.labelView3->setBackgroundRole(QPalette::Base);
    ui.labelView4->setBackgroundRole(QPalette::Base);
    ui.labelView2->setBackgroundRole(QPalette::Base);
    ui.labelView1->setBackgroundRole(QPalette::Base);
    ui.labelView5->setBackgroundRole(QPalette::Base);
    ui.labelView6->setBackgroundRole(QPalette::Base);
    ui.labelView7->setBackgroundRole(QPalette::Base);
    ui.selectionWidget->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection0->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection3->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection4->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection2->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection1->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection5->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection6->setBackgroundRole(QPalette::Highlight);
    ui.labelSelection7->setBackgroundRole(QPalette::Highlight);

    QList<QWidget*> widgets = findChildren<QWidget*>();
    foreach (QWidget* widget, widgets)
    {
        widget->installEventFilter(this);
        widget->setFocusPolicy(Qt::NoFocus);
    }

    // connect signals/slots
    // TODO

    // finally, add UI's to tab widget
}

PreviewWidget::~PreviewWidget()
{
}

bool PreviewWidget::eventFilter(QObject *, QEvent *ev)
{
    switch (ev->type())
    {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::Enter:
        case QEvent::Leave:
        case QEvent::Wheel:
        case QEvent::ContextMenu:
            return true; // ignore
        default:
            break;
    }
    return false;
}

inline void copyPaletteBrush(QPalette &palette, QPalette::ColorGroup state,
                             QPalette::ColorRole role)
{
    palette.setBrush(QPalette::Active, role, palette.brush(state, role));
}

void PreviewWidget::setPaletteRecursive(QWidget *widget,
                                        const QPalette &palette)
{
    widget->setPalette(palette);

    const QObjectList children = widget->children();
    foreach (QObject *child, children) {
        if (child->isWidgetType())
            setPaletteRecursive((QWidget*)child, palette);
    }
}

void PreviewWidget::setPalette(const KSharedConfigPtr &config,
                               QPalette::ColorGroup state)
{
    QPalette palette = KGlobalSettings::createApplicationPalette(config);

    if (state != QPalette::Active) {
        copyPaletteBrush(palette, state, QPalette::Base);
        copyPaletteBrush(palette, state, QPalette::Text);
        copyPaletteBrush(palette, state, QPalette::Window);
        copyPaletteBrush(palette, state, QPalette::WindowText);
        copyPaletteBrush(palette, state, QPalette::Button);
        copyPaletteBrush(palette, state, QPalette::ButtonText);
        copyPaletteBrush(palette, state, QPalette::Highlight);
        copyPaletteBrush(palette, state, QPalette::HighlightedText);
        copyPaletteBrush(palette, state, QPalette::AlternateBase);
        copyPaletteBrush(palette, state, QPalette::Link);
        copyPaletteBrush(palette, state, QPalette::LinkVisited);
        copyPaletteBrush(palette, state, QPalette::Light);
        copyPaletteBrush(palette, state, QPalette::Midlight);
        copyPaletteBrush(palette, state, QPalette::Mid);
        copyPaletteBrush(palette, state, QPalette::Dark);
        copyPaletteBrush(palette, state, QPalette::Shadow);
    }

    setPaletteRecursive(this, palette);
    // TODO set colors for "special" labels
}

#include "previewwidget.moc"
