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
#include <stdlib.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qtooltip.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kpixmap.h>
#include <kstandarddirs.h>
#include <kwin.h>

#include "../background/bgrender.h"
#include "extensionInfo.h"
#include "positiontab_impl.h"
#include "positiontab_impl.moc"


extern int kickerconfig_screen_number;

// magic numbers for the preview widget layout
extern const int offsetX = 23;
extern const int offsetY = 14;
extern const int maxX = 150;
extern const int maxY = 114;
extern const int margin = 1;

PositionTab::PositionTab(KickerConfig *kcmKicker, const char* name)
  : PositionTabBase(kcmKicker, name),
    m_pretendPanel(0),
    m_desktopPreview(0),
    m_kcm(kcmKicker),
    m_panelInfo(0),
    m_panelPos(PosBottom),
    m_panelAlign(AlignLeft)
{
    QPixmap monitor(locate("data", "kcontrol/pics/monitor.png"));
    m_monitorImage->setPixmap(monitor);
    m_monitorImage->setFixedSize(m_monitorImage->sizeHint());

    m_pretendDesktop = new QWidget(m_monitorImage, "pretendBG");
    m_pretendDesktop->setGeometry(offsetX, offsetY, maxX, maxY);
    m_pretendPanel = new QFrame(m_monitorImage, "pretendPanel");
    m_pretendPanel->setGeometry(offsetX + margin, maxY + offsetY - 10, 
                                maxX - margin, 10 - margin);
    m_pretendPanel->setFrameShape(QFrame::MenuBarPanel);
    m_panelList->setSorting(-1);
    m_panelList->header()->hide();

    /*
     * set the tooltips on the buttons properly for RTL langs
     */
    if (kapp->reverseLayout())
    {
        QToolTip::add(locationTopRight,     i18n("Top left"));
        QToolTip::add(locationTop,          i18n("Top center"));
        QToolTip::add(locationTopLeft,      i18n("Top right" ) );
        QToolTip::add(locationRightTop,     i18n("Left top"));
        QToolTip::add(locationRight,        i18n("Left center"));
        QToolTip::add(locationRightBottom,  i18n("Left bottom"));
        QToolTip::add(locationBottomRight,  i18n("Bottom left"));
        QToolTip::add(locationBottom,       i18n("Bottom center"));
        QToolTip::add(locationBottomLeft,   i18n("Bottom right"));
        QToolTip::add(locationLeftTop,      i18n("Right top"));
        QToolTip::add(locationLeft,         i18n("Right center"));
        QToolTip::add(locationLeftBottom,   i18n("Right bottom"));
    }
    else
    {
        QToolTip::add(locationTopLeft,      i18n("Top left"));
        QToolTip::add(locationTop,          i18n("Top center"));
        QToolTip::add(locationTopRight,     i18n("Top right" ) );
        QToolTip::add(locationLeftTop,      i18n("Left top"));
        QToolTip::add(locationLeft,         i18n("Left center"));
        QToolTip::add(locationLeftBottom,   i18n("Left bottom"));
        QToolTip::add(locationBottomLeft,   i18n("Bottom left"));
        QToolTip::add(locationBottom,       i18n("Bottom center"));
        QToolTip::add(locationBottomRight,  i18n("Bottom right"));
        QToolTip::add(locationRightTop,     i18n("Right top"));
        QToolTip::add(locationRight,        i18n("Right center"));
        QToolTip::add(locationRightBottom,  i18n("Right bottom"));
    }
    
    // connections
    connect(m_locationGroup, SIGNAL(clicked(int)), SIGNAL(changed()));
    connect(m_xineramaScreenComboBox, SIGNAL(highlighted(int)), SIGNAL(changed()));
    
    connect(m_identifyButton,SIGNAL(pressed()),SLOT(showIdentify()));
    connect(m_identifyButton,SIGNAL(released()),SIGNAL(hideIdentify()));

    for(int s=0; s < QApplication::desktop()->numScreens(); s++)
    {   /* populate the combobox for the available screens */
        m_xineramaScreenComboBox->insertItem(QString::number(s));
    }
    
    // hide the xinerama chooser widgets if there is no need for them
    if (QApplication::desktop()->numScreens() < 2)
    {
        m_identifyButton->hide();
        m_xineramaScreenComboBox->hide();
        m_xineramaScreenLabel->hide();
    }
    
    connect(m_percentSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_percentSpinBox, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_expandCheckBox, SIGNAL(clicked()), SIGNAL(changed()));
    
    connect(m_sizeGroup, SIGNAL(clicked(int)), SIGNAL(changed()));
    connect(m_customSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_customSpinbox, SIGNAL(valueChanged(int)), SIGNAL(changed()));

    m_desktopPreview = new KBackgroundRenderer(0);
    connect(m_desktopPreview, SIGNAL(imageDone(int)), SLOT(slotBGPreviewReady(int)));
    
    connect(m_kcm, SIGNAL(extensionInfoChanged()), SLOT(infoUpdated()));
}

PositionTab::~PositionTab()
{
    delete m_desktopPreview;
}

void PositionTab::load()
{
    m_panelList->clear();
    m_kcm->populateExtensionInfoList(m_panelList);

    if (m_kcm->extensionsInfo().count() == 1)
    {
        m_panelList->hide();
    }

    switchPanel(0);
    m_desktopPreview->setPreview(m_pretendDesktop->size());
    m_desktopPreview->start();
}

void PositionTab::save()
{
    storeInfo();
    m_kcm->saveExtentionInfo();
}

void PositionTab::defaults()
{
    m_panelPos= PosBottom; // bottom of the screen
    m_percentSlider->setValue( 100 ); // use all space available
    m_percentSpinBox->setValue( 100 ); // use all space available
    m_expandCheckBox->setChecked( true ); // expand as required
    m_xineramaScreenComboBox->setCurrentItem(QApplication::desktop()->primaryScreen());
    
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
    QPushButton* pushed = reinterpret_cast<QPushButton*>(m_locationGroup->find(whichButton));

    if (pushed == locationTopLeft)
    {
        m_panelAlign = kapp->reverseLayout() ? AlignRight : AlignLeft;
        m_panelPos = PosTop;
    }
    else if (pushed == locationTop)
    {
        m_panelAlign = AlignCenter;
        m_panelPos = PosTop;
    }
    else if (pushed == locationTopRight)
    {
        m_panelAlign = kapp->reverseLayout() ? AlignLeft : AlignRight;
        m_panelPos = PosTop;
    }
    else if (pushed == locationLeftTop)
    {
        m_panelAlign = AlignLeft;
        m_panelPos = kapp->reverseLayout() ? PosRight : PosLeft;
    }
    else if (pushed == locationLeft)
    {
        m_panelAlign = AlignCenter;
        m_panelPos = kapp->reverseLayout() ? PosRight : PosLeft;
    }
    else if (pushed == locationLeftBottom)
    {
        m_panelAlign = AlignRight;
        m_panelPos = kapp->reverseLayout() ? PosRight : PosLeft;
    }
    else if (pushed == locationBottomLeft)
    {
        m_panelAlign = kapp->reverseLayout() ? AlignRight : AlignLeft;
        m_panelPos = PosBottom;
    }
    else if (pushed == locationBottom)
    {
        m_panelAlign = AlignCenter;
        m_panelPos = PosBottom;
    }
    else if (pushed == locationBottomRight)
    {
        m_panelAlign = AlignRight;
        m_panelPos = PosBottom;
    }
    else if (pushed == locationRightTop)
    {
        m_panelAlign = AlignLeft;
        m_panelPos = kapp->reverseLayout() ? PosLeft : PosRight;
    }
    else if (pushed == locationRight)
    {
        m_panelAlign = AlignCenter;
        m_panelPos = kapp->reverseLayout() ? PosLeft : PosRight;
    }
    else if (pushed == locationRightBottom)
    {
        m_panelAlign = AlignRight;
        m_panelPos = kapp->reverseLayout() ? PosLeft : PosRight;
    }

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
    
    if (x2 < 3)
    {
        x2 = 3;
    }

    if (y2 < 3)
    {
        y2 = 3;
    }

    m_pretendPanel->setGeometry(x, y, x2, y2);
}

void PositionTab::panelDimensionsChanged()
{
    lengthenPanel(-1);
}

void PositionTab::slotBGPreviewReady(int)
{
    KPixmap pm;
    if (QPixmap::defaultDepth() < 15)
    {
        pm.convertFromImage(*m_desktopPreview->image(), KPixmap::LowColor);
    }
    else
    {
        pm.convertFromImage(*m_desktopPreview->image());
    }

    m_pretendDesktop->setBackgroundPixmap(pm);
}

void PositionTab::switchPanel(QListViewItem* panelItem)
{
    blockSignals(true);
    extensionInfoItem* listItem = reinterpret_cast<extensionInfoItem*>(panelItem);

    if (!listItem)
    {
        m_panelList->setSelected(m_panelList->firstChild(), true);
        listItem = reinterpret_cast<extensionInfoItem*>(m_panelList->firstChild());
    }
    
    if (m_panelInfo)
    {
        storeInfo();
    }

    m_panelInfo = listItem->info();
    
    switch(m_panelInfo->_size) 
    {
        case 0: 
            m_sizeTiny->setChecked(true); 
            break;
        case 1: 
            m_sizeSmall->setChecked(true); 
            break;
        case 2: 
            m_sizeNormal->setChecked(true); 
            break;
        case 3: 
            m_sizeLarge->setChecked(true); 
            break;
        default:
            m_sizeCustom->setChecked(true);
        break;
    }

    if (!m_panelInfo->_useStdSizes &&
        m_panelInfo->_customSizeMin != m_panelInfo->_customSizeMax)
    {
        m_sizeCustom->setChecked(true);
    }
    
    m_customSlider->setMinValue(m_panelInfo->_customSizeMin);
    m_customSlider->setMaxValue(m_panelInfo->_customSizeMax);
    m_customSlider->setTickInterval(m_panelInfo->_customSizeMax / 6);
    m_customSlider->setValue(m_panelInfo->_customSize);
    m_customSpinbox->setMinValue(m_panelInfo->_customSizeMin);
    m_customSpinbox->setMaxValue(m_panelInfo->_customSizeMax);
    m_customSpinbox->setValue(m_panelInfo->_customSize);

    m_sizeTiny->setEnabled(m_panelInfo->_useStdSizes);         
    m_sizeSmall->setEnabled(m_panelInfo->_useStdSizes);
    m_sizeNormal->setEnabled(m_panelInfo->_useStdSizes);
    m_sizeLarge->setEnabled(m_panelInfo->_useStdSizes);
    m_sizeCustom->setEnabled(m_panelInfo->_customSizeMin != m_panelInfo->_customSizeMax);
    m_sizeGroup->setEnabled(m_panelInfo->_resizeable);

    
    m_panelPos = m_panelInfo->_position;
    m_panelAlign = m_panelInfo->_alignment;
    if(m_panelInfo->_xineramaScreen >= 0 && m_panelInfo->_xineramaScreen < QApplication::desktop()->numScreens())
        m_xineramaScreenComboBox->setCurrentItem(m_panelInfo->_xineramaScreen);
    else
        m_xineramaScreenComboBox->setCurrentItem(QApplication::desktop()->primaryScreen());

    if (m_panelPos == PosTop)
    {
        if (m_panelAlign == AlignLeft)
            kapp->reverseLayout() ? locationTopRight->setOn(true) : 
                                    locationTopLeft->setOn(true);
        else if (m_panelAlign == AlignCenter)
            locationTop->setOn(true);
        else // if (m_panelAlign == AlignRight
            kapp->reverseLayout() ? locationTopLeft->setOn(true) : 
                                    locationTopRight->setOn(true);
    }
    else if (m_panelPos == PosRight)
    {
        if (m_panelAlign == AlignLeft)
            kapp->reverseLayout() ? locationLeftTop->setOn(true) : 
                                    locationRightTop->setOn(true);
        else if (m_panelAlign == AlignCenter)
            kapp->reverseLayout() ? locationLeft->setOn(true) :
                                    locationRight->setOn(true);
        else // if (m_panelAlign == AlignRight
            kapp->reverseLayout() ? locationLeftBottom->setOn(true) :
                                    locationRightBottom->setOn(true);
    }
    else if (m_panelPos == PosBottom)
    {
        if (m_panelAlign == AlignLeft)
            kapp->reverseLayout() ? locationBottomRight->setOn(true) :
                                    locationBottomLeft->setOn(true);
        else if (m_panelAlign == AlignCenter)
            locationBottom->setOn(true);
        else // if (m_panelAlign == AlignRight
            kapp->reverseLayout() ? locationBottomLeft->setOn(true) :
                                    locationBottomRight->setOn(true);
    }
    else // if (m_panelPos == PosLeft
    {
        if (m_panelAlign == AlignLeft)
            kapp->reverseLayout() ? locationRightTop->setOn(true) :
                                    locationLeftTop->setOn(true);
        else if (m_panelAlign == AlignCenter)
            kapp->reverseLayout() ? locationRight->setOn(true) : 
                                    locationLeft->setOn(true);
        else // if (m_panelAlign == AlignRight
            kapp->reverseLayout() ? locationRightBottom->setOn(true) : 
                                    locationLeftBottom->setOn(true);
    }

    m_percentSlider->setValue(m_panelInfo->_sizePercentage);
    m_percentSpinBox->setValue(m_panelInfo->_sizePercentage);

    m_expandCheckBox->setChecked(m_panelInfo->_expandSize);
    
    lengthenPanel(m_panelInfo->_sizePercentage);
    blockSignals(false);
}

void PositionTab::infoUpdated()
{
    switchPanel(0);
}

void PositionTab::storeInfo()
{
    if (!m_panelInfo)
    {
        return;
            }
   
    // Magic numbers stolen from kdebase/kicker/core/global.cpp
    // PGlobal::sizeValue()
    if (m_sizeTiny->isChecked())
    {
        m_panelInfo->_size = 0;
    }
    else if (m_sizeSmall->isChecked())
    {
        m_panelInfo->_size = 1;
    }
    else if (m_sizeNormal->isChecked())
    {
        m_panelInfo->_size = 2;
    }
    else if (m_sizeLarge->isChecked())
    {
        m_panelInfo->_size = 3;
    }
    else // if (m_sizeCustom->isChecked())
    {
        m_panelInfo->_size = 4;
        m_panelInfo->_customSize = m_customSlider->value();
    }

    m_panelInfo->_position = m_panelPos;
    m_panelInfo->_alignment = m_panelAlign;
    m_panelInfo->_xineramaScreen = m_xineramaScreenComboBox->currentItem();

    m_panelInfo->_sizePercentage = m_percentSlider->value();
    m_panelInfo->_expandSize = m_expandCheckBox->isChecked();
}

void PositionTab::showIdentify()
{
    for(int s=0; s < QApplication::desktop()->numScreens();s++)
    {

        QLabel *screenLabel = new QLabel(0,"Screen Identify", WStyle_StaysOnTop | WDestructiveClose | WStyle_Customize | WStyle_NoBorder);

        KWin::setState( screenLabel->winId(), NET::Modal | NET::Sticky | NET::StaysOnTop | NET::SkipTaskbar | NET::SkipPager );
        KWin::setType( screenLabel->winId(), NET::Override );
    
        QFont identifyFont(KGlobalSettings::generalFont());
        identifyFont.setPixelSize(100);
        screenLabel->setFont(identifyFont);

        screenLabel->setFrameStyle(QFrame::Panel);
        screenLabel->setFrameShadow(QFrame::Plain);

        screenLabel->setAlignment(Qt::AlignCenter);
        screenLabel->setNum(s);
        connect(this,SIGNAL(hideIdentify()),screenLabel,SLOT(close()));

        QPoint screenCenter(QApplication::desktop()->screenGeometry(s).center());
        QRect targetGeometry(QPoint(0,0),screenLabel->sizeHint());
        targetGeometry.moveCenter(screenCenter);

        screenLabel->setGeometry(targetGeometry);
  
        screenLabel->show();
    }
}
