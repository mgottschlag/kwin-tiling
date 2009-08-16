#ifndef HOTKEYS_EXPORT_WIDGET_H
#define HOTKEYS_EXPORT_WIDGET_H
/**
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ui_hotkeys_export_widget.h"

#include "kdialog.h"
#include "kurl.h"

#include <QtGui/QWidget>


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KHotkeysExportWidget : public QWidget
    {
    Q_OBJECT

public:

    KHotkeysExportWidget(QWidget *parent);
    virtual ~KHotkeysExportWidget();

    // The layout
    Ui::KHotkeysExportWidget ui;

    }; // KHotkeysExportWidget

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KHotkeysExportDialog : public KDialog
    {
public:
    KHotkeysExportDialog(QWidget*);
    virtual ~KHotkeysExportDialog();

    QString importId() const;

    bool allowMerging() const;
    void setAllowMerging(bool);
    void setImportId(const QString &id);
    int state() const;
    KUrl url() const;

private:

    KHotkeysExportWidget *w;
    }; // KHotkeysExportDialog


#endif /* HOTKEYS_EXPORT_WIDGET_H */
