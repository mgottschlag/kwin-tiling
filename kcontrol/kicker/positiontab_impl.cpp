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
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qtooltip.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>
#include <kpanelextension.h>
#include <kpixmap.h>
#include <kstandarddirs.h>
#include <kwin.h>

#include "main.h"
#include "../background/bgrender.h"

#include "positiontab_impl.h"
#include "positiontab_impl.moc"


// magic numbers for the preview widget layout
extern const int offsetX = 23;
extern const int offsetY = 14;
extern const int maxX = 150;
extern const int maxY = 114;
extern const int margin = 1;

PositionTab::PositionTab(QWidget *parent, const char* name)
  : PositionTabBase(parent, name),
    m_pretendPanel(0),
    m_desktopPreview(0),
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

    /*
     * set the tooltips on the buttons properly for RTL langs
     */
    if (kapp->reverseLayout())
    {
        locationTopRight->setToolTip(     i18n("Top left"));
        locationTop->setToolTip(          i18n("Top center"));
        locationTopLeft->setToolTip(      i18n("Top right" ) );
        locationRightTop->setToolTip(     i18n("Left top"));
        locationRight->setToolTip(        i18n("Left center"));
        locationRightBottom->setToolTip(  i18n("Left bottom"));
        locationBottomRight->setToolTip(  i18n("Bottom left"));
        locationBottom->setToolTip(       i18n("Bottom center"));
        locationBottomLeft->setToolTip(   i18n("Bottom right"));
        locationLeftTop->setToolTip(      i18n("Right top"));
        locationLeft->setToolTip(         i18n("Right center"));
        locationLeftBottom->setToolTip(   i18n("Right bottom"));
    }
    else
    {
        locationTopLeft->setToolTip(      i18n("Top left"));
        locationTop->setToolTip(          i18n("Top center"));
        locationTopRight->setToolTip(     i18n("Top right" ) );
        locationLeftTop->setToolTip(      i18n("Left top"));
        locationLeft->setToolTip(         i18n("Left center"));
        locationLeftBottom->setToolTip(   i18n("Left bottom"));
        locationBottomLeft->setToolTip(   i18n("Bottom left"));
        locationBottom->setToolTip(       i18n("Bottom center"));
        locationBottomRight->setToolTip(  i18n("Bottom right"));
        locationRightTop->setToolTip(     i18n("Right top"));
        locationRight->setToolTip(        i18n("Right center"));
        locationRightBottom->setToolTip(  i18n("Right bottom"));
    }

    // connections
    connect(m_locationGroup, SIGNAL(clicked(int)), SIGNAL(changed()));
    connect(m_xineramaScreenComboBox, SIGNAL(highlighted(int)), SIGNAL(changed()));

    connect(m_identifyButton,SIGNAL(clicked()),SLOT(showIdentify()));

    for(int s=0; s < QApplication::desktop()->numScreens(); s++)
    {   /* populate the combobox for the available screens */
        m_xineramaScreenComboBox->insertItem(QString::number(s+1));
    }
    m_xineramaScreenComboBox->insertItem(i18n("All Screens"));

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
    connect(m_desktopPreview, SIGNAL(imageDone(int)),
            SLOT(slotBGPreviewReady(int)));

    connect(KickerConfig::self(), SIGNAL(extensionInfoChanged()),
            SLOT(infoUpdated()));
    connect(KickerConfig::self(), SIGNAL(extensionAdded(ExtensionInfo*)),
            SLOT(extensionAdded(ExtensionInfo*)));
    connect(KickerConfig::self(), SIGNAL(extensionRemoved(ExtensionInfo*)),
            SLOT(extensionRemoved(ExtensionInfo*)));
    connect(KickerConfig::self(), SIGNAL(extensionChanged(const QString&)),
            SLOT(extensionChanged(const QString&)));
    connect(KickerConfig::self(), SIGNAL(extensionAboutToChange(const QString&)),
            SLOT(extensionAboutToChange(const QString&)));
    // position tab tells hiding tab about extension selections and vice versa
    connect(KickerConfig::self(), SIGNAL(hidingPanelChanged(int)),
            SLOT(jumpToPanel(int)));
    connect(m_panelList, SIGNAL(activated(int)),
            KickerConfig::self(), SIGNAL(positionPanelChanged(int)));

    connect(m_panelSize, SIGNAL(activated(int)),
            SLOT(sizeChanged(int)));
    connect(m_panelSize, SIGNAL(activated(int)),
            SIGNAL(changed()));
}

PositionTab::~PositionTab()
{
    delete m_desktopPreview;
}

void PositionTab::load()
{
    m_panelInfo = 0;
    KickerConfig::self()->populateExtensionInfoList(m_panelList);
    m_panelsGroupBox->setHidden(m_panelList->count() < 2);

    switchPanel(KickerConfig::self()->currentPanelIndex());
    m_desktopPreview->setPreview(m_pretendDesktop->size());
    m_desktopPreview->start();
}

void PositionTab::extensionAdded(ExtensionInfo* info)
{
    m_panelList->insertItem(info->_name);
    m_panelsGroupBox->setHidden(m_panelList->count() < 2);
}

void PositionTab::save()
{
    storeInfo();
    KickerConfig::self()->saveExtentionInfo();
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

    m_panelSize->setCurrentItem(KPanelExtension::SizeNormal);

    // update the magic drawing
    lengthenPanel(-1);
    switchPanel(KickerConfig::self()->currentPanelIndex());
}

void PositionTab::sizeChanged(int which)
{
    bool custom = which == KPanelExtension::SizeCustom;
    m_customSlider->setEnabled(custom);
    m_customSpinbox->setEnabled(custom);
}

void PositionTab::movePanel(int whichButton)
{
    QPushButton* pushed = reinterpret_cast<QPushButton*>(m_locationGroup->find(whichButton));

    if (pushed == locationTopLeft)
    {
	if (!(m_panelInfo->_allowedPosition[PosTop])) 
	{
	   setPositionButtons();
	   return;
	}
        m_panelAlign = kapp->reverseLayout() ? AlignRight : AlignLeft;
        m_panelPos = PosTop;
    }
    else if (pushed == locationTop)
    {
	if (!(m_panelInfo->_allowedPosition[PosTop]))
	{
	   setPositionButtons();
	   return;
	}
        m_panelAlign = AlignCenter;
        m_panelPos = PosTop;
    }
    else if (pushed == locationTopRight)
    {
	if (!(m_panelInfo->_allowedPosition[PosTop]))
	{
	   setPositionButtons();
	   return;
	}
        m_panelAlign = kapp->reverseLayout() ? AlignLeft : AlignRight;
        m_panelPos = PosTop;
    }
    else if (pushed == locationLeftTop)
    {
	if (!(m_panelInfo->_allowedPosition[kapp->reverseLayout() ? PosRight : PosLeft]))
	{
	   setPositionButtons();
	   return;
	}
        m_panelAlign = AlignLeft;
        m_panelPos = kapp->reverseLayout() ? PosRight : PosLeft;
    }
    else if (pushed == locationLeft)
    {
	if (!(m_panelInfo->_allowedPosition[kapp->reverseLayout() ? PosRight : PosLeft]))
	{
	   setPositionButtons();
	   return;
	}
        m_panelAlign = AlignCenter;
        m_panelPos = kapp->reverseLayout() ? PosRight : PosLeft;
    }
    else if (pushed == locationLeftBottom)
    {
	if (!(m_panelInfo->_allowedPosition[kapp->reverseLayout() ? PosRight : PosLeft]))
	{
	   setPositionButtons();
	   return;
	}
        m_panelAlign = AlignRight;
        m_panelPos = kapp->reverseLayout() ? PosRight : PosLeft;
    }
    else if (pushed == locationBottomLeft)
    {
	if (!(m_panelInfo->_allowedPosition[PosBottom]))
	{
	   setPositionButtons();
	   return;
	}
        m_panelAlign = kapp->reverseLayout() ? AlignRight : AlignLeft;
        m_panelPos = PosBottom;
    }
    else if (pushed == locationBottom)
    {
	if (!(m_panelInfo->_allowedPosition[PosBottom]))
	{
	   setPositionButtons();
	   return;
	}
        m_panelAlign = AlignCenter;
        m_panelPos = PosBottom;
    }
    else if (pushed == locationBottomRight)
    {
	if (!(m_panelInfo->_allowedPosition[PosBottom]))
	{
	   setPositionButtons();
	   return;
	}
        m_panelAlign = kapp->reverseLayout() ? AlignLeft : AlignRight;
        m_panelPos = PosBottom;
    }
    else if (pushed == locationRightTop)
    {
	if (!(m_panelInfo->_allowedPosition[kapp->reverseLayout() ? PosLeft : PosRight]))
	{
	   setPositionButtons();
	   return;
	}
        m_panelAlign = AlignLeft;
        m_panelPos = kapp->reverseLayout() ? PosLeft : PosRight;
    }
    else if (pushed == locationRight)
    {
	if (!(m_panelInfo->_allowedPosition[kapp->reverseLayout() ? PosLeft : PosRight]))
	{
	   setPositionButtons();
	   return;
	}
        m_panelAlign = AlignCenter;
        m_panelPos = kapp->reverseLayout() ? PosLeft : PosRight;
    }
    else if (pushed == locationRightBottom)
    {
	if (!(m_panelInfo->_allowedPosition[kapp->reverseLayout() ? PosLeft : PosRight]))
	{
	   setPositionButtons();
	   return;
	}
        m_panelAlign = AlignRight;
        m_panelPos = kapp->reverseLayout() ? PosLeft : PosRight;
    }

    lengthenPanel(-1);
    emit panelPositionChanged(m_panelPos);
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

    switch (m_panelSize->currentItem())
    {
        case KPanelExtension::SizeTiny:
        case KPanelExtension::SizeSmall:
            panelSize = panelSize * 3 / 2;
            break;
        case KPanelExtension::SizeNormal:
            panelSize *= 2;
            break;
        case KPanelExtension::SizeLarge:
            panelSize = panelSize * 5 / 2;
            break;
        default:
            panelSize = panelSize * m_customSlider->value() / 24;
            break;
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

void PositionTab::switchPanel(int panelItem)
{
    blockSignals(true);
    ExtensionInfo* panelInfo = (KickerConfig::self()->extensionsInfo())[panelItem];

    if (!panelInfo)
    {
        m_panelList->setCurrentItem(0);
        panelInfo = (KickerConfig::self()->extensionsInfo())[panelItem];

        if (!panelInfo)
        {
            return;
        }
    }

    if (m_panelInfo)
    {
        storeInfo();
    }

    m_panelInfo = panelInfo;

    // because this changes when panels come and go, we have 
    // to be overly pedantic and remove the custom item every time and
    // decide to add it back again, or not
    m_panelSize->removeItem(KPanelExtension::SizeCustom);
    if (m_panelInfo->_customSizeMin != m_panelInfo->_customSizeMax)
    {
        m_panelSize->insertItem(i18n("Custom"), KPanelExtension::SizeCustom);
    }

    if (m_panelInfo->_size >= KPanelExtension::SizeCustom ||
        (!m_panelInfo->_useStdSizes &&
         m_panelInfo->_customSizeMin != m_panelInfo->_customSizeMax)) // compat
    {
        m_panelSize->setCurrentItem(KPanelExtension::SizeCustom);
        sizeChanged(KPanelExtension::SizeCustom);
    }
    else
    {
        m_panelSize->setCurrentItem(m_panelInfo->_size);
        sizeChanged(0);
    }
    m_panelSize->setEnabled(m_panelInfo->_useStdSizes);

    m_customSlider->setMinValue(m_panelInfo->_customSizeMin);
    m_customSlider->setMaxValue(m_panelInfo->_customSizeMax);
    m_customSlider->setTickInterval(m_panelInfo->_customSizeMax / 6);
    m_customSlider->setValue(m_panelInfo->_customSize);
    m_customSpinbox->setMinValue(m_panelInfo->_customSizeMin);
    m_customSpinbox->setMaxValue(m_panelInfo->_customSizeMax);
    m_customSpinbox->setValue(m_panelInfo->_customSize);
    m_sizeGroup->setEnabled(m_panelInfo->_resizeable);
    m_panelPos = m_panelInfo->_position;
    m_panelAlign = m_panelInfo->_alignment;
    if(m_panelInfo->_xineramaScreen >= 0 && m_panelInfo->_xineramaScreen < QApplication::desktop()->numScreens())
        m_xineramaScreenComboBox->setCurrentItem(m_panelInfo->_xineramaScreen);
    else if(m_panelInfo->_xineramaScreen == -2) /* the All Screens option: qt uses -1 for default, so -2 for all */
        m_xineramaScreenComboBox->setCurrentItem(m_xineramaScreenComboBox->count()-1);
    else
        m_xineramaScreenComboBox->setCurrentItem(QApplication::desktop()->primaryScreen());

    setPositionButtons();

    m_percentSlider->setValue(m_panelInfo->_sizePercentage);
    m_percentSpinBox->setValue(m_panelInfo->_sizePercentage);

    m_expandCheckBox->setChecked(m_panelInfo->_expandSize);

    lengthenPanel(m_panelInfo->_sizePercentage);
    blockSignals(false);
}


void PositionTab::setPositionButtons() {
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

}

void PositionTab::infoUpdated()
{
    switchPanel(0);
}

void PositionTab::extensionAboutToChange(const QString& configPath)
{
    ExtensionInfo* extension = (KickerConfig::self()->extensionsInfo())[m_panelList->currentItem()];
    if (extension && extension->_configPath == configPath)
    {
        storeInfo();
    }
}

void PositionTab::extensionChanged(const QString& configPath)
{
    ExtensionInfo* extension = (KickerConfig::self()->extensionsInfo())[m_panelList->currentItem()];
    if (extension && extension->_configPath == configPath)
    {
        m_panelInfo = 0;
        switchPanel(m_panelList->currentItem());
    }
}

void PositionTab::storeInfo()
{
    if (!m_panelInfo)
    {
        return;
    }

    // Magic numbers stolen from kdebase/kicker/core/global.cpp
    // PGlobal::sizeValue()
    if (m_panelSize->currentItem() < KPanelExtension::SizeCustom)
    {
        m_panelInfo->_size = m_panelSize->currentItem();
    }
    else
    {
        m_panelInfo->_size = KPanelExtension::SizeCustom;
        m_panelInfo->_customSize = m_customSlider->value();
    }

    m_panelInfo->_position = m_panelPos;
    m_panelInfo->_alignment = m_panelAlign;
    if(m_xineramaScreenComboBox->currentItem() == m_xineramaScreenComboBox->count()-1)
        m_panelInfo->_xineramaScreen = -2; /* all screens */
    else
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
        screenLabel->setNum(s + 1);
        // BUGLET: we should not allow the identification to be entered again
        //         until the timer fires.
        QTimer::singleShot(1500, screenLabel, SLOT(close()));

        QPoint screenCenter(QApplication::desktop()->screenGeometry(s).center());
        QRect targetGeometry(QPoint(0,0),screenLabel->sizeHint());
        targetGeometry.moveCenter(screenCenter);

        screenLabel->setGeometry(targetGeometry);

        screenLabel->show();
    }
}

void PositionTab::extensionRemoved(ExtensionInfo* info)
{
    int count = m_panelList->count();
    int extensionCount = KickerConfig::self()->extensionsInfo().count();
    int index = 0;
    for (; index < count && index < extensionCount; ++index)
    {
        if (KickerConfig::self()->extensionsInfo()[index] == info)
        {
            break;
        }
    }

    bool isCurrentlySelected = index == m_panelList->currentItem();
    m_panelList->removeItem(index);
    m_panelsGroupBox->setHidden(m_panelList->count() < 2);

    if (isCurrentlySelected)
    {
        m_panelList->setCurrentItem(0);
    }
}

void PositionTab::jumpToPanel(int index)
{
    m_panelList->setCurrentItem(index);
    switchPanel(index);
}
