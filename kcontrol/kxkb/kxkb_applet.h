/*
 *  Copyright (C) 2007 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef KXKBAPPLET_H
#define KXKBAPPLET_H


#include <QMouseEvent>
#include <QPixmap>

#include <plasma/applet.h>
#include <plasma/widgets/icon.h>


class QSizeF;
class KxkbCore;

class KxkbApplet : public Plasma::Applet
{
  Q_OBJECT
public:
    explicit KxkbApplet(QObject *parent, const QVariantList &args);
    ~KxkbApplet();
    
//    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
//                                    const QRect& contentsRect);
    QSizeF contentSizeHint() const;

private:
    KxkbCore* m_kxkbCore;
//    KxkbWidget* m_kxkbWidget;
};


/*
    Flexible widget to show layouts
*/
class KxkbPlasmaWidget : public KxkbWidget
{
    Q_OBJECT

public:
    enum { ICON = 1, TEXT = 2 };

    KxkbPlasmaWidget(QGraphicsItem* parent=0, int controlType=MENU_FULL);
    virtual ~KxkbPlasmaWidget() { delete m_menu; } //delete m_indicatorWidget; }
//    Plasma::Icon* widget() { return m_indicatorWidget; }

protected:
    QMenu* contextMenu() { return m_menu; }
    void setToolTip(const QString& tip) { m_indicatorWidget->setToolTip(tip); }
    void setPixmap(const QPixmap& pixmap) { if( m_displayMode==ICON) m_indicatorWidget->setIcon(pixmap); }
    void setText(const QString& text) { if (m_displayMode==TEXT) m_indicatorWidget->setText(text); }
    void setVisible(bool visible) { m_indicatorWidget->setVisible(visible); }
	
private:
    const int m_displayMode;
    Plasma::Icon* m_indicatorWidget;
    QMenu* m_menu;
};


K_EXPORT_PLASMA_APPLET(kxkb, KxkbApplet)

#endif
