/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2002 Aaron Seigo <aseigo@olympusproject.org>
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
 */

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qslider.h>
#include <qspinbox.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kstandarddirs.h>

#include "positiontab_impl.h"
#include "positiontab_impl.moc"


extern int kickerconfig_screen_number;

// magic numbers for the preview widget layout
extern const int offsetX = 23;
extern const int offsetY = 14;
extern const int maxX = 151;
extern const int maxY = 115;
extern const int margin = 1;

PositionTab::PositionTab( QWidget *parent, const char* name )
  : PositionTabBase (parent, name),
    m_pretendPanel(0),
    m_panelPos(PosBottom),
    m_panelAlign(AlignLeft)
{
    // connections
    connect(m_locationGroup, SIGNAL(clicked(int)), SLOT(changed()));
    connect(m_percentSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_percentSpinBox, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_expandCheckBox, SIGNAL(clicked()), SIGNAL(changed()));
    
    connect(m_sizeGroup, SIGNAL(clicked(int)), SIGNAL(changed()));
    connect(m_customSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_customSpinbox, SIGNAL(valueChanged(int)), SIGNAL(changed()));

    QPixmap monitor(locate("data", "kcontrol/pics/monitor.png"));
    m_monitorImage->setPixmap(monitor);
    m_monitorImage->setFixedSize(m_monitorImage->sizeHint());

    m_pretendPanel = new QFrame(m_monitorImage, "pretendPanel");
    m_pretendPanel->setGeometry(offsetX + margin, maxY + offsetY - 10, 
                                maxX - margin, 10 - margin);
    m_pretendPanel->setFrameShape(QFrame::MenuBarPanel);

    load();
}

void PositionTab::load()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
        configname = "kickerrc";
    else
        configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig c(configname, false, false);

    c.setGroup("General");

    // Magic numbers stolen from kdebase/kicker/core/global.cpp
    // PGlobal::sizeValue()
    int panelSize = c.readNumEntry("Size", 30);
    switch(panelSize) 
    {
        case 24: 
            m_sizeTiny->setChecked(true); 
            break;
        case 30: 
            m_sizeSmall->setChecked(true); 
            break;
        case 46: 
            m_sizeNormal->setChecked(true); 
            break;
        case 58: 
            m_sizeLarge->setChecked(true); 
            break;
        default: 
            m_sizeCustom->setChecked(true);
            m_customSlider->setValue(panelSize);
            m_customSpinbox->setValue(panelSize);
        break;
    }

    m_panelPos = c.readNumEntry("Position", PosBottom);
    m_panelAlign = c.readNumEntry("Alignment", QApplication::reverseLayout() ? AlignRight : AlignLeft);

    /*
     * TODO: set the panel to the right place
     */

    int sizepercentage = c.readNumEntry( "SizePercentage", 100 );
    m_percentSlider->setValue( sizepercentage );
    m_percentSpinBox->setValue( sizepercentage );

    m_expandCheckBox->setChecked( c.readBoolEntry( "ExpandSize", true ) );
    
    lengthenPanel(sizepercentage);
}

void PositionTab::save()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
        configname = "kickerrc";
    else
        configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig c(configname, false, false);

    c.setGroup("General");

    // Magic numbers stolen from kdebase/kicker/core/global.cpp
    // PGlobal::sizeValue()
    if (m_sizeTiny->isChecked())
    {
        c.writeEntry("Size",24);
    }
    else if (m_sizeSmall->isChecked())
    {
        c.writeEntry("Size",30);
    }
    else if (m_sizeNormal->isChecked())
    {
        c.writeEntry("Size",46);
    }
    else if (m_sizeLarge->isChecked())
    {
        c.writeEntry("Size",58);
    }
    else // if (m_sizeCustom->isChecked())
    {
        c.writeEntry("Size", m_customSlider->value());
    }

    c.writeEntry("Position", m_panelPos);
    c.writeEntry("Alignment", m_panelAlign);

    c.writeEntry( "SizePercentage", m_percentSlider->value() );
    c.writeEntry( "ExpandSize", m_expandCheckBox->isChecked() );
    c.sync();
}

void PositionTab::defaults()
{
    m_panelPos= PosBottom; // bottom of the screen
    m_percentSlider->setValue( 100 ); // use all space available
    m_percentSpinBox->setValue( 100 ); // use all space available
    m_expandCheckBox->setChecked( true ); // expand as required
    
    if (QApplication::reverseLayout())
    {
        // RTL lang aligns right
        m_panelAlign = AlignRight;
    }
    else
    {
        // everyone else aligns left
        m_panelAlign = AlignLeft;
    }
    
    m_sizeSmall->setChecked(true); // small size
    
    // update the magic drawing
    lengthenPanel(-1);
}

void PositionTab::movePanel(int whichButton)
{
    switch (whichButton)
    {
        case posTopLeft:
            m_panelAlign = AlignLeft;
            m_panelPos = PosTop;
            break;
        case posTop:
            m_panelAlign = AlignCenter;
            m_panelPos = PosTop;
            break;
        case posTopRight:
            m_panelAlign = AlignRight;
            m_panelPos = PosTop;
            break;
        case posLeftTop:
            m_panelAlign = AlignLeft;
            m_panelPos = PosLeft;
            break;
        case posLeft:
            m_panelAlign = AlignCenter;
            m_panelPos = PosLeft;
            break;
        case posLeftBottom:
            m_panelAlign = AlignRight;
            m_panelPos = PosLeft;
            break;
        case posBottomLeft:
            m_panelAlign = AlignLeft;
            m_panelPos = PosBottom;
            break;
        case posBottom:
            m_panelAlign = AlignCenter;
            m_panelPos = PosBottom;
            break;
        case posBottomRight:
            m_panelAlign = AlignRight;
            m_panelPos = PosBottom;
            break;
        case posRightTop:
            m_panelAlign = AlignLeft;
            m_panelPos = PosRight;
            break;
        case posRight:
            m_panelAlign = AlignCenter;
            m_panelPos = PosRight;
            break;
        case posRightBottom:
            m_panelAlign = AlignRight;
            m_panelPos = PosRight;
            break;
    }

    emit changed();
    lengthenPanel(-1);
}

void PositionTab::lengthenPanel(int sizePercent)
{
    if (sizePercent < 0)
    {
        sizePercent = m_percentSlider->value();
    }

    unsigned int x(0), y(0), x2(0), y2(0);
    unsigned int diff = 0;
    unsigned int panelSize = 4;
    
    if (m_sizeSmall->isChecked())
    {
        panelSize = panelSize * 3 / 2;
    }
    else if (m_sizeNormal->isChecked())
    {
        panelSize *= 2;
    }
    else if (m_sizeLarge->isChecked())
    {
        panelSize = panelSize * 5 / 2;
    }
    else if (m_sizeCustom->isChecked())
    {
        panelSize = panelSize * m_customSlider->value() / 24;
    }

    switch (m_panelPos)
    {
        case PosTop:
            x  = offsetX + margin;
            x2 = maxX - margin;
            y  = offsetY + margin;
            y2 = panelSize;
            
            diff =  x2 - ((x2 * sizePercent) / 100);
            if (m_panelAlign == AlignLeft)
            {
                x2  -= diff;
            }
            else if (m_panelAlign == AlignCenter)
            {
                x  += diff / 2;
                x2 -= diff;
            }
            else // m_panelAlign == AlignRight
            {
                x  += diff;
                x2 -= diff;
            }
            break;
        case PosLeft:
            x  = offsetX + margin;
            x2 = panelSize;
            y  = offsetY + margin;
            y2 = maxY - margin;
            
            diff =  y2 - ((y2 * sizePercent) / 100);
            if (m_panelAlign == AlignLeft)
            {
                y2  -= diff;
            }
            else if (m_panelAlign == AlignCenter)
            {
                y  += diff / 2;
                y2 -= diff;
            }
            else // m_panelAlign == AlignRight
            {
                y  += diff;
                y2 -= diff;
            }
            break;
        case PosBottom:
            x  = offsetX + margin;
            x2 = maxX - margin;
            y  = offsetY + maxY - panelSize;
            y2 = panelSize;
            
            diff =  x2 - ((x2 * sizePercent) / 100);
            if (m_panelAlign == AlignLeft)
            {
                x2  -= diff;
            }
            else if (m_panelAlign == AlignCenter)
            {
                x  += diff / 2;
                x2 -= diff;
            }
            else // m_panelAlign == AlignRight
            {
                x  += diff;
                x2 -= diff;
            }
            break;
        default: // case PosRight:
            x  = offsetX + maxX - panelSize;
            x2 = panelSize;
            y  = offsetY + margin;
            y2 = maxY - margin;
            
            diff =  y2 - ((y2 * sizePercent) / 100);
            if (m_panelAlign == AlignLeft)
            {
                y2  -= diff;
            }
            else if (m_panelAlign == AlignCenter)
            {
                y  += diff / 2;
                y2 -= diff;
            }
            else // m_panelAlign == AlignRight
            {
                y  += diff;
                y2 -= diff;
            }
            break;
    }

    m_pretendPanel->setGeometry(x, y, x2, y2);
}

void PositionTab::panelDimensionsChanged()
{
    lengthenPanel(-1);
}


