/***************************************************************************
 *   Copyright (C) 2008 by Artur Duque de Souza <morpheuz@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


#ifndef QEDJEAPPLETSCRIPT_HEADER
#define QEDJEAPPLETSCRIPT_HEADER

#include "ui_qedjeConfig.h"

// Include qedje stuff
#include <qzion.h>
#include <qedje.h>

#include <Plasma/AppletScript>


// Define our plasma AppletScript
class QEdjeAppletScript: public Plasma::AppletScript
{
    Q_OBJECT
public:
    // Basic Create/Destroy
    QEdjeAppletScript(QObject *parent, const QVariantList &args);
    ~QEdjeAppletScript();

    virtual bool init();
    virtual void resizeAll(QSize size);

    // The paintInterface procedure paints the applet to screen
    virtual void paintInterface(QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                const QRect& contentsRect);

public Q_SLOTS:
    void showConfigurationInterface();
    void configChanged();
    void groupSelected(int index);

private:
    QEdje *world;
    QZionCanvas *canvas;
    QGraphicsProxyWidget *proxy;

    // everything needed by the fancy setup UI
    Ui::qedjeConfig ui;
    KDialog *dialog;
    QWidget *config_widget;
    QEdje *previewWorld;
    QZionCanvas *previewCanvas;

    QString m_edje_file;
    QString m_edje_group;
    QStringList m_groups_list;
    int currentIndex;

    void setup_canvas();
};

#endif
