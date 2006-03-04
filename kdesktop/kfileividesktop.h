/* This file is proposed to be part of the KDE base.
 * Copyright (C) 2003 Laur Ivan <laurivan@eircom.net>
 *
 * Many thanks to:
 *  - Bernardo Hung <deciare@gta.igs.net> for the enhanced shadow
 *    algorithm (currently used)
 *  - Tim Jansen <tim@tjansen.de> for the API updates and fixes.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KFILE_IVI_DESKTOP
#define KFILE_IVI_DESKTOP

#include <qcolor.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qstring.h>
#include <kfileivi.h>

/*
 * The dither flags used to display the shadow image
 */
#define DITHER_FLAGS (Qt::DiffuseAlphaDither | Qt::ColorOnly | Qt::AvoidDither)


class QFont;
class KShadowEngine;

/**
 * This class replaces KFileIVI in the desktop only.
 * If the shadow object is NULL, then the class should behave almost identical
 * to its parent.
 */
class KFileIVIDesktop : public KFileIVI
{
 public:
    /**
     * Constructor. It replicates the KFileIVI constructor and adds an
     * optional shadow object.
     * @param iconview the parent (iconview)
     * @param fileitem the item theis object is supposed to draw
     * @param size the default size of the drawn object
     * @param shadow reference to the shadow object
     */
    KFileIVIDesktop(KonqIconViewWidget *iconview, KFileItem* fileitem, int
		     size, KShadowEngine *shadow = 0L);

    /**
     * Default destructor. Doesn't really do anything.
     */
    ~KFileIVIDesktop();

 protected:
    /**
     * Reimplements KIconView::calcRect to take the shadow metrics
     * into account
     */
     virtual void calcRect( const QString& _text );

    /**
     * Paints this item. Takes care of using the normal or alpha
     * blending methods depending on the configuration.
     * @param p the painter for drawing the item
     * @param cg the base color group
     */
    virtual void paintItem(QPainter *p, const QColorGroup &cg);

    /**
     * Reimplements QIconView::paintFocus() to take the shadow
     * metrics into account();
     */
    virtual void paintFocus( QPainter *p, const QColorGroup &cg );

    /**
     * Draws the shadow text.
     * @param p the painter for drawing the item
     * @param cg the base color group
     */
    virtual void drawShadowedText(QPainter *p, const QColorGroup &cg);

    /**
     * Builds the shadow. As the algorithm is pretty slow (at pixel level),
     * This method is triggered only if the configuration has changed.
     * @param p the painter for drawing the item
     * @param align the shadow alignment
     * @param shadowColor the shadow color
     */
    virtual QImage *buildShadow(QPainter *p, const int align, QColor &shadowColor);

 protected:
    void setNormalImage(QImage *newImage) { delete m_normalImage; m_normalImage = newImage; };
    void setSelectedImage(QImage *newImage) { delete m_selectedImage; m_selectedImage = newImage; };

    QImage *normalImage() { return m_normalImage; };
    QImage *selectedImage() { return m_selectedImage; };

 private:
    bool shouldUpdateShadow(bool selected);
    int shadowThickness() const;

    KShadowEngine *m_shadow;

    QImage  *m_selectedImage;
    QImage  *m_normalImage;

    QString oldText;

    unsigned long _selectedUID;
    unsigned long _normalUID;

};

#endif
