/*
 *  lookandfeeltab.cpp
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2000 Aaron J. Seigo <aseigo@olympusproject.org>
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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qregexp.h>

#include <kcolorbutton.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kimageio.h>
#include <kiconeffect.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klineedit.h>

#include "main.h"
#include "advancedDialog.h"
#include "lookandfeeltab_impl.h"
#include "lookandfeeltab_impl.moc"

#include <iostream>
using namespace std;

LookAndFeelTab::LookAndFeelTab( QWidget *parent, const char* name )
  : LookAndFeelTabBase (parent, name),
    m_advDialog(0)
{
  connect(m_zoom_cb, SIGNAL(clicked()), SIGNAL(changed()));
  connect(m_showToolTips, SIGNAL(clicked()), SIGNAL(changed()));

  connect(m_kmenuTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_desktopTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_browserTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_urlTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_wlTile, SIGNAL(activated(int)), SIGNAL(changed()));

  connect(m_kmenuTile, SIGNAL(activated(int)), SLOT(kmenuTileChanged(int)));
  connect(m_desktopTile, SIGNAL(activated(int)), SLOT(desktopTileChanged(int)));
  connect(m_browserTile, SIGNAL(activated(int)), SLOT(browserTileChanged(int)));
  connect(m_urlTile, SIGNAL(activated(int)), SLOT(urlTileChanged(int)));
  connect(m_wlTile, SIGNAL(activated(int)), SLOT(wlTileChanged(int)));

  connect(m_kmenuColor, SIGNAL(changed(const QColor&)), SIGNAL(changed()));
  connect(m_desktopColor, SIGNAL(changed(const QColor&)), SIGNAL(changed()));
  connect(m_browserColor, SIGNAL(changed(const QColor&)), SIGNAL(changed()));
  connect(m_urlColor, SIGNAL(changed(const QColor&)), SIGNAL(changed()));
  connect(m_wlColor, SIGNAL(changed(const QColor&)), SIGNAL(changed()));

  connect(m_transparent, SIGNAL(toggled(bool)), SIGNAL(changed()));
  connect(m_backgroundImage, SIGNAL(toggled(bool)), SIGNAL(changed()));
  connect(m_colorizeImage, SIGNAL(toggled(bool)), SIGNAL(changed()));
  connect(m_colorizeImage, SIGNAL(toggled(bool)), SLOT(browseTheme()));

  connect(m_backgroundInput->lineEdit(), SIGNAL(lostFocus()), SLOT(browseTheme()));
  m_backgroundInput->setFilter(KImageIO::pattern(KImageIO::Reading));
  m_backgroundInput->setCaption(i18n("Select Image File"));

  connect(m_showToolTips, SIGNAL(clicked()), SIGNAL(changed()));

  fillTileCombos();
}

void LookAndFeelTab::browseTheme()
{
    browseTheme(m_backgroundInput->url());
}

void LookAndFeelTab::browseTheme(const QString& newtheme)
{
    if (newtheme.isEmpty())
    {
        m_backgroundInput->clear();
        m_backgroundLabel->setPixmap(QPixmap());
        emit changed();
        return;
    }

    previewBackground(newtheme, true);
}

void LookAndFeelTab::launchAdvancedDialog()
{
    if (!m_advDialog)
    {
        m_advDialog = new advancedDialog(this, "advancedDialog");
        connect(m_advDialog, SIGNAL(finished()), this, SLOT(finishAdvancedDialog()));
        m_advDialog->show();
    }
    m_advDialog->setActiveWindow();
}

void LookAndFeelTab::finishAdvancedDialog()
{
    m_advDialog->delayedDestruct();
    m_advDialog = 0;
}
void LookAndFeelTab::enableTransparency( bool enable )
{
    bool b = m_backgroundImage->isChecked();

    m_backgroundImage->setDisabled( enable );
    m_backgroundInput->setDisabled( enable || !b );
    m_backgroundLabel->setDisabled( enable || !b );
    m_colorizeImage->setDisabled( enable || !b );
}

void LookAndFeelTab::previewBackground(const QString& themepath, bool isNew)
{
    QString theme = themepath;
    if (theme[0] != '/')
        theme = locate("data", "kicker/" + theme);

    QImage tmpImg(theme);
    if(!tmpImg.isNull())
    {
        tmpImg = tmpImg.smoothScale(m_backgroundLabel->contentsRect().width(),
                                    m_backgroundLabel->contentsRect().height());
        if (m_colorizeImage->isChecked())
            colorize(tmpImg);
        theme_preview.convertFromImage(tmpImg);
        if(!theme_preview.isNull()) {
            m_backgroundInput->lineEdit()->setText(theme);
            m_backgroundLabel->setPixmap(theme_preview);
            if (isNew)
                emit changed();
            return;
        }
    }

    KMessageBox::error(this,
                       i18n("Error loading theme image file.\n\n%1\n%2")
                            .arg(theme, themepath));
    m_backgroundInput->clear();
    m_backgroundLabel->setPixmap(QPixmap());
}

void LookAndFeelTab::colorize(QImage& image)
{
    // mercilessly ripped from the k menu side image colorizing
    KConfig *config = KGlobal::config();
    config->setGroup("WM");
    QColor color = palette().active().highlight();
    QColor activeTitle = config->readColorEntry("activeBackground", &color);
    QColor inactiveTitle = config->readColorEntry("inactiveBackground", &color);

    // figure out which color is most suitable for recoloring to
    int h1, s1, v1, h2, s2, v2, h3, s3, v3;
    activeTitle.hsv(&h1, &s1, &v1);
    inactiveTitle.hsv(&h2, &s2, &v2);
    palette().active().background().hsv(&h3, &s3, &v3);

    if ( (kAbs(h1-h3)+kAbs(s1-s3)+kAbs(v1-v3) < kAbs(h2-h3)+kAbs(s2-s3)+kAbs(v2-v3)) &&
        ((kAbs(h1-h3)+kAbs(s1-s3)+kAbs(v1-v3) < 32) || (s1 < 32)) && (s2 > s1))
        color = inactiveTitle;
    else
        color = activeTitle;

    // limit max/min brightness
    int r, g, b;
    color.rgb(&r, &g, &b);
    int gray = qGray(r, g, b);
    if (gray > 180) {
        r = (r - (gray - 180) < 0 ? 0 : r - (gray - 180));
        g = (g - (gray - 180) < 0 ? 0 : g - (gray - 180));
        b = (b - (gray - 180) < 0 ? 0 : b - (gray - 180));
    } else if (gray < 76) {
        r = (r + (76 - gray) > 255 ? 255 : r + (76 - gray));
        g = (g + (76 - gray) > 255 ? 255 : g + (76 - gray));
        b = (b + (76 - gray) > 255 ? 255 : b + (76 - gray));
    }
    color.setRgb(r, g, b);
    KIconEffect::colorize(image, color, 1.0);
}

void LookAndFeelTab::load()
{
  KConfig c(KickerConfig::configName(), false, false);

  c.setGroup("General");

  bool use_theme = c.readBoolEntry("UseBackgroundTheme", true);
  QString theme = c.readPathEntry("BackgroundTheme", "wallpapers/default.png").stripWhiteSpace();

  bool transparent = c.readBoolEntry( "Transparent", false );

  m_backgroundImage->setChecked(use_theme);
  m_backgroundInput->setEnabled(use_theme);
  m_backgroundLabel->setEnabled(use_theme);
  m_colorizeImage->setChecked(c.readBoolEntry("ColorizeBackground", false));
  m_colorizeImage->setEnabled(use_theme);
  m_backgroundInput->lineEdit()->setText( QString::null );
  m_transparent->setChecked( transparent );
  m_backgroundLabel->clear();
  if (theme.length() > 0)
  {
    previewBackground(theme, false);
  }

  m_showToolTips->setChecked( c.readBoolEntry( "ShowToolTips", true ) );

  c.setGroup("buttons");

  m_zoom_cb->setChecked(c.readBoolEntry("EnableIconZoom", true));

  QString tile;
  c.setGroup("buttons");

  m_kmenuTile->setCurrentItem(0);
  m_desktopTile->setCurrentItem(0);
  m_urlTile->setCurrentItem(0);
  m_browserTile->setCurrentItem(0);
  m_wlTile->setCurrentItem(0);
  m_kmenuColor->setEnabled(false);
  m_desktopColor->setEnabled(false);
  m_urlColor->setEnabled(false);
  m_browserColor->setEnabled(false);
  m_wlColor->setEnabled(false);
  if (c.readBoolEntry("EnableTileBackground", false))
  {
    c.setGroup("button_tiles");

    if (c.readBoolEntry("EnableKMenuTiles", false))
    {
      tile = c.readEntry("KMenuTile", "solid_blue");
      m_kmenuTile->setCurrentItem(m_tilename.findIndex(tile));
      m_kmenuColor->setColor(c.readColorEntry("KMenuTileColor"));
      m_kmenuColor->setEnabled(tile == "Colorize");
    }

    if (c.readBoolEntry("EnableDesktopButtonTiles", false))
    {
      tile = c.readEntry("DesktopButtonTile", "solid_orange");
      m_desktopTile->setCurrentItem(m_tilename.findIndex(tile));
      m_desktopColor->setColor(c.readColorEntry("KMenuTileColor"));
      m_desktopColor->setEnabled(tile == "Colorize");
    }

    if (c.readBoolEntry("EnableURLTiles", false))
    {
      tile = c.readEntry("URLTile", "solid_gray");
      m_urlTile->setCurrentItem(m_tilename.findIndex(tile));
      m_urlColor->setColor(c.readColorEntry("KMenuTileColor"));
      m_urlColor->setEnabled(tile == "Colorize");
    }

    if (c.readBoolEntry("EnableBrowserTiles", false))
    {
      tile = c.readEntry("BrowserTile", "solid_green");
      m_browserTile->setCurrentItem(m_tilename.findIndex(tile));
      m_browserColor->setColor(c.readColorEntry("KMenuTileColor"));
      m_browserColor->setEnabled(tile == "Colorize");
    }

    if (c.readBoolEntry("EnableWindowListTiles", false))
    {
      tile = c.readEntry("WindowListTile", "solid_green");
      m_wlTile->setCurrentItem(m_tilename.findIndex(tile));
      m_wlColor->setColor(c.readColorEntry("KMenuTileColor"));
      m_wlColor->setEnabled(tile == "Colorize");
    }
  }
  enableTransparency( transparent );
}

void LookAndFeelTab::save()
{
  KConfig c(KickerConfig::configName(), false, false);

  c.setGroup("General");
  c.writeEntry("UseBackgroundTheme", m_backgroundImage->isChecked());
  c.writeEntry("ColorizeBackground", m_colorizeImage->isChecked());
  c.writeEntry("Transparent", m_transparent->isChecked());
  c.writePathEntry("BackgroundTheme", m_backgroundInput->url());
  c.writeEntry( "ShowToolTips", m_showToolTips->isChecked() );

  c.setGroup("button_tiles");
  bool enableTiles = false;
  int tile = m_kmenuTile->currentItem();
  if (tile > 0)
  {
    enableTiles = true;
    c.writeEntry("EnableKMenuTiles", true);
    c.writeEntry("KMenuTile", m_tilename[m_kmenuTile->currentItem()]);
    c.writeEntry("KMenuTileColor", m_kmenuColor->color());
  }
  else
  {
    c.writeEntry("EnableKMenuTiles", false);
  }

  tile = m_desktopTile->currentItem();
  if (tile > 0)
  {
    enableTiles = true;
    c.writeEntry("EnableDesktopButtonTiles", true);
    c.writeEntry("DesktopButtonTile", m_tilename[m_desktopTile->currentItem()]);
    c.writeEntry("DesktopButtonTileColor", m_desktopColor->color());
  }
  else
  {
    c.writeEntry("EnableDesktopButtonTiles", false);
  }

  tile = m_urlTile->currentItem();
  if (tile > 0)
  {
    enableTiles = true;
    c.writeEntry("EnableURLTiles", tile > 0);
    c.writeEntry("URLTile", m_tilename[m_urlTile->currentItem()]);
    c.writeEntry("URLTileColor", m_urlColor->color());
  }
  else
  {
    c.writeEntry("EnableURLTiles", false);
  }

  tile = m_browserTile->currentItem();
  if (tile > 0)
  {
    enableTiles = true;
    c.writeEntry("EnableBrowserTiles", tile > 0);
    c.writeEntry("BrowserTile", m_tilename[m_browserTile->currentItem()]);
    c.writeEntry("BrowserTileColor", m_browserColor->color());
  }
  else
  {
    c.writeEntry("EnableBrowserTiles", false);
  }

  tile = m_wlTile->currentItem();
  if (tile > 0)
  {
    enableTiles = true;
    c.writeEntry("EnableWindowListTiles", tile > 0);
    c.writeEntry("WindowListTile", m_tilename[m_wlTile->currentItem()]);
    c.writeEntry("WindowListTileColor", m_wlColor->color());
  }
  else
  {
    c.writeEntry("EnableWindowListTiles", false);
  }

  c.setGroup("buttons");
  c.writeEntry("EnableTileBackground", enableTiles);
  c.writeEntry("EnableIconZoom", m_zoom_cb->isChecked());

  c.sync();
}

void LookAndFeelTab::defaults()
{
  m_zoom_cb->setChecked(false);
  m_showToolTips->setChecked(true);

  m_kmenuTile->setCurrentItem(0);
  m_urlTile->setCurrentItem(0);
  m_browserTile->setCurrentItem(0);
  m_wlTile->setCurrentItem(0);
  m_desktopTile->setCurrentItem(0);

  m_kmenuColor->setColor(QColor());
  m_kmenuColor->setEnabled(false);
  m_urlColor->setColor(QColor());
  m_urlColor->setEnabled(false);
  m_desktopColor->setColor(QColor());
  m_desktopColor->setEnabled(false);
  m_browserColor->setColor(QColor());
  m_browserColor->setEnabled(false);
  m_wlColor->setColor(QColor());
  m_wlColor->setEnabled(false);

  QString theme = "wallpapers/default.png";

  m_backgroundImage->setChecked(true);
  m_transparent->setChecked(false);
  m_backgroundInput->lineEdit()->setText(theme);
  m_backgroundLabel->clear();
  m_colorizeImage->setChecked(true);

  m_backgroundInput->setEnabled(true);
  m_backgroundLabel->setEnabled(true);
  m_colorizeImage->setEnabled(true);
  previewBackground(theme, false);
}

void LookAndFeelTab::fillTileCombos()
{
/*  m_kmenuTile->clear();
  m_kmenuTile->insertItem(i18n("Default"));
  m_desktopTile->clear();
  m_desktopTile->insertItem(i18n("Default"));
  m_urlTile->clear();
  m_urlTile->insertItem(i18n("Default"));
  m_browserTile->clear();
  m_browserTile->insertItem(i18n("Default"));
  m_wlTile->clear();
  m_wlTile->insertItem(i18n("Default"));*/
  m_tilename.clear();
  m_tilename << "" << "Colorize";

  QStringList list = KGlobal::dirs()->findAllResources("tiles","*_tiny_up.png");
  int minHeight = 0;

  for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
  {
    QString tile = (*it);
    QPixmap pix(tile);
    QFileInfo fi(tile);
    tile = fi.fileName();
    tile.truncate(tile.find("_tiny_up.png"));
    m_tilename << tile;

    // Transform tile to words with title case
    // The same is done when generating messages for translation
    QStringList words = QStringList::split(QRegExp("[_ ]"), tile);
    for (QStringList::iterator w = words.begin(); w != words.end(); ++w)
      (*w)[0] = (*w)[0].upper();
    tile = i18n(words.join(" ").utf8());

    m_kmenuTile->insertItem(pix, tile);
    m_desktopTile->insertItem(pix, tile);
    m_urlTile->insertItem(pix, tile);
    m_browserTile->insertItem(pix, tile);
    m_wlTile->insertItem(pix, tile);

    if (pix.height() > minHeight)
    {
        minHeight = pix.height();
    }
  }

  minHeight += 6;
  m_kmenuTile->setMinimumHeight(minHeight);
  m_desktopTile->setMinimumHeight(minHeight);
  m_urlTile->setMinimumHeight(minHeight);
  m_browserTile->setMinimumHeight(minHeight);
  m_wlTile->setMinimumHeight(minHeight);
}

void LookAndFeelTab::kmenuTileChanged(int i)
{
    m_kmenuColor->setEnabled(i == 1);
}

void LookAndFeelTab::desktopTileChanged(int i)
{
    m_desktopColor->setEnabled(i == 1);
}

void LookAndFeelTab::browserTileChanged(int i)
{
    m_browserColor->setEnabled(i == 1);
}

void LookAndFeelTab::urlTileChanged(int i)
{
    m_urlColor->setEnabled(i == 1);
}

void LookAndFeelTab::wlTileChanged(int i)
{
    m_wlColor->setEnabled(i == 1);
}
