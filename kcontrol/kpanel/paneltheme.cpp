/*
 * KPanel theme configuration.
 *
 * (C) Daniel M. Duley 1999
 */

#include "paneltheme.h"
#include <qlayout.h>
#include <qgroupbox.h>
#include <kapp.h>
#include <klocale.h>
#include <kconfigbase.h>
#include <kwm.h>
#include <kglobal.h>
#include <kstddirs.h>

extern KConfigBase *config;

void KPanelTheme::loadSettings()
{
    config->setGroup("kpanel");
    QColor defColor = colorGroup().background();

    canvas->colors[WidgetCanvas::C_Panel] =
        config->readColorEntry("PanelBackground", &defColor);
    canvas->colors[WidgetCanvas::C_Icon] =
        config->readColorEntry("IconBackground", &defColor);
    canvas->colors[WidgetCanvas::C_TBar] =
        config->readColorEntry("TaskbarFrameBackground", &defColor);
    canvas->colors[WidgetCanvas::C_TBtn] =
        config->readColorEntry("TaskbarBackground", &defColor);
    defColor = colorGroup().foreground();
    canvas->colors[WidgetCanvas::C_TText] =
        config->readColorEntry("TaskbarForeground", &defColor);

    pixNames[WidgetCanvas::C_Panel] =
        config->readEntry("BackgroundTexture", QString::null);
    pixNames[WidgetCanvas::C_Icon] =
        config->readEntry("IconTexture", QString::null);
    pixNames[WidgetCanvas::C_TBar] =
        config->readEntry("TaskbarFrameTexture", QString::null);
    pixNames[WidgetCanvas::C_TBtn] =
        config->readEntry("TaskbarTexture", QString::null);

    canvas->pixmaps[WidgetCanvas::C_Panel] =
        ldr->loadIcon(pixNames[WidgetCanvas::C_Panel]);
    canvas->pixmaps[WidgetCanvas::C_Icon] =
        ldr->loadIcon(pixNames[WidgetCanvas::C_Icon]);
    canvas->pixmaps[WidgetCanvas::C_TBar] =
        ldr->loadIcon(pixNames[WidgetCanvas::C_TBar]);
    canvas->pixmaps[WidgetCanvas::C_TBtn] =
        ldr->loadIcon(pixNames[WidgetCanvas::C_TBtn]);

    canvas->boxSize = config->readNumEntry("BoxWidth", 45);
}

void KPanelTheme::saveSettings()
{
    config->setGroup("kpanel");

    config->writeEntry("PanelBackground", canvas->colors[WidgetCanvas::C_Panel]);
    config->writeEntry("IconBackground",canvas->colors[WidgetCanvas::C_Icon]);
    config->writeEntry("TaskbarFrameBackground",canvas->colors[WidgetCanvas::C_TBar]);
    config->writeEntry("TaskbarBackground", canvas->colors[WidgetCanvas::C_TBtn]);
    config->writeEntry("TaskbarForeground",canvas->colors[WidgetCanvas::C_TText]);

    config->writeEntry("BackgroundTexture",
                       pixNames[WidgetCanvas::C_Panel]);
    config->writeEntry("IconTexture",
                       pixNames[WidgetCanvas::C_Icon]);
    config->writeEntry("TaskbarFrameTexture",
                       pixNames[WidgetCanvas::C_TBar]);
    config->writeEntry("TaskbarTexture",
                       pixNames[WidgetCanvas::C_TBtn]);

    config->sync();
}

void KPanelTheme::applySettings()
{
    saveSettings();
    KWM::sendKWMCommand("kpanel:restart");
}

void KPanelTheme::slotColor(const QColor &c)
{
    canvas->colors[wCombo->currentItem()] = c;
    canvas->drawSampleWidgets();
    canvas->repaint();
}

void KPanelTheme::slotColorDrop(int widget, const QColor &c)
{
    canvas->colors[widget] = c;
    canvas->drawSampleWidgets();
    canvas->repaint();
}

void KPanelTheme::slotPixmap(const QString &p)
{
    if(wCombo->currentItem() == WidgetCanvas::C_TText)
        return;
    pixNames[wCombo->currentItem()] = p;
    canvas->pixmaps[wCombo->currentItem()] = ldr->loadIcon(p);

    pixBtn->setIcon(p);
    pixBtn->setPixmap(ldr->loadIcon(p)); // hack
    canvas->drawSampleWidgets();
    canvas->repaint();
}

void KPanelTheme::slotWidgetClicked(int widget)
{
    wCombo->setCurrentItem(widget);
    colorBtn->setColor(canvas->colors[widget]);
    if(widget == WidgetCanvas::C_TText){
        pixBtn->setEnabled(false);
        pixLbl->setEnabled(false);
    }
    else{
        pixBtn->setEnabled(true);
        pixLbl->setEnabled(true);
        pixBtn->setIcon(pixNames[widget]);
        pixBtn->setPixmap(canvas->pixmaps[widget]);
    }
}

void KPanelTheme::slotResetWidget()
{
    int w = wCombo->currentItem();

    if(w == WidgetCanvas::C_TText){
        canvas->colors[w] = colorGroup().foreground();
    }
    else{
        canvas->pixmaps[w].resize(0,0);
        pixNames[w] = QString::null;
        canvas->colors[w] = colorGroup().background();
        pixBtn->setIcon(QString::null);
        pixBtn->setPixmap(canvas->pixmaps[w]);
    }
    colorBtn->setColor(canvas->colors[w]);
    canvas->drawSampleWidgets();
    canvas->repaint();
}

void KPanelTheme::slotResetAll()
{
    int w;
    for(w=0; w < CANVAS_ITEMS; ++w){
        if(w == WidgetCanvas::C_TText){
            canvas->colors[w] = colorGroup().foreground();
        }
        else{
            canvas->pixmaps[w].resize(0,0);
            pixNames[w] = QString::null;
            canvas->colors[w] = colorGroup().background();
            pixBtn->setIcon(QString::null);
            pixBtn->setPixmap(canvas->pixmaps[w]);
        }
    }
    colorBtn->setColor(canvas->colors[wCombo->currentItem()]);
    canvas->drawSampleWidgets();
    canvas->repaint();
}

KPanelTheme::KPanelTheme(QWidget *parent, const char *name)
    :KConfigWidget(parent, name)
{
    ldr = new KIconLoader();
    ldr->setIconType("kpanel_pics");
    KGlobal::dirs()->addResourceType("kpanel_pics", KStandardDirs::kde_default("data") + "kpanel/pics");
  
    QGroupBox *optionBox = new QGroupBox(i18n("Options"), this);
    colorBtn = new KColorButton(optionBox);
    colorBtn->setMinimumSize(QSize(64,64));
    pixBtn = new KIconLoaderButton(ldr, optionBox);
    pixBtn->setMinimumSize(colorBtn->size());
    pixBtn->iconLoaderDialog()->changeDirs(KGlobal::dirs()->getResourceDirs("kpanel_pics"));
    wCombo = new QComboBox(optionBox);
    wCombo->insertItem(i18n("Panel"), WidgetCanvas::C_Panel);
    wCombo->insertItem(i18n("Icon Background"), WidgetCanvas::C_Icon);
    wCombo->insertItem(i18n("Taskbar"), WidgetCanvas::C_TBar);
    wCombo->insertItem(i18n("Taskbar Button"), WidgetCanvas::C_TBtn);
    wCombo->insertItem(i18n("Taskbar Text"), WidgetCanvas::C_TText);
    wCombo->setMinimumSize(wCombo->sizeHint());
    QPushButton *resetBtn = new
        QPushButton(i18n("Reset this item"), optionBox);
    QPushButton *resetAllBtn =
        new QPushButton(i18n("Reset all items"), optionBox);
    pixLbl = new QLabel(i18n("Pixmap:"), optionBox);
    QLabel *colorLbl = new QLabel(i18n("Color:"), optionBox);
    QLabel *wLbl = new QLabel(i18n("KPanel Item:"), optionBox);

    // This layout is acting wacky :P
    QGridLayout *optionLayout = new QGridLayout(optionBox, 9, 5, 10);
    optionLayout->addRowSpacing(0, 15);
    optionLayout->addMultiCellWidget(wLbl, 1, 1, 0, 3);
    optionLayout->addMultiCellWidget(wCombo, 2, 2, 0, 1);
    optionLayout->addMultiCellWidget(colorLbl, 3, 3, 0, 1);
    optionLayout->addMultiCellWidget(pixLbl, 3, 3, 2, 3);
    optionLayout->addWidget(colorBtn, 4, 0);
    optionLayout->addWidget(pixBtn, 4, 2);
    optionLayout->addRowSpacing(5, 20); //gridlayout is getting spacing wrong
    optionLayout->addMultiCellWidget(resetBtn, 6, 6, 0, 2);
    optionLayout->addMultiCellWidget(resetAllBtn, 7, 7, 0, 2);
    optionLayout->setColStretch(4, 1);
    optionLayout->setRowStretch(8, 1);
    
    QGroupBox *canvasBox = new QGroupBox(i18n("KPanel Preview"), this);
    canvas = new WidgetCanvas(canvasBox);
    canvas->goPix = BarIcon("go");
    canvas->appPix = BarIcon("mini-go");
    loadSettings();
    canvas->drawSampleWidgets();
    canvas->setMinimumSize(canvas->sizeHint());
    QVBoxLayout *canvasLayout = new QVBoxLayout(canvasBox, 5);
    canvasLayout->addSpacing(15);
    canvasLayout->addWidget(canvas, 1);
    
    QVBoxLayout *layout = new QVBoxLayout(this, 5);
    layout->addWidget(canvasBox);
    layout->addWidget(optionBox, 1);
    layout->activate();

    connect(wCombo, SIGNAL(activated(int)), SLOT(slotWidgetClicked(int)));
    connect(colorBtn, SIGNAL(changed(const QColor &)),
            SLOT(slotColor(const QColor &)));
    connect(pixBtn, SIGNAL(iconChanged(const QString &)),
            SLOT(slotPixmap(const QString &)));
    connect(resetBtn, SIGNAL(clicked()), SLOT(slotResetWidget()));
    connect(resetAllBtn, SIGNAL(clicked()), SLOT(slotResetAll()));
    connect(canvas, SIGNAL(widgetSelected(int)), SLOT(slotWidgetClicked(int)));
    connect(canvas, SIGNAL(colorDropped(int, const QColor &)),
            SLOT(slotColorDrop(int, const QColor &)));
}

KPanelTheme::~KPanelTheme()
{
    ;
}

#include "paneltheme.moc"

