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
  : LookAndFeelTabBase (parent, name)
{
  connect(m_zoom_cb, SIGNAL(clicked()), SIGNAL(changed()));
  connect(m_showToolTips, SIGNAL(clicked()), SIGNAL(changed()));

  connect(m_kmenuTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_desktopTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_browserTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_urlTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_exeTile, SIGNAL(activated(int)), SIGNAL(changed()));
  connect(m_wlTile, SIGNAL(activated(int)), SIGNAL(changed()));

  connect(m_kmenuTile, SIGNAL(activated(int)), SLOT(kmenuTileChanged(int)));
  connect(m_desktopTile, SIGNAL(activated(int)), SLOT(desktopTileChanged(int)));
  connect(m_browserTile, SIGNAL(activated(int)), SLOT(browserTileChanged(int)));
  connect(m_urlTile, SIGNAL(activated(int)), SLOT(urlTileChanged(int)));
  connect(m_exeTile, SIGNAL(activated(int)), SLOT(exeTileChanged(int)));
  connect(m_wlTile, SIGNAL(activated(int)), SLOT(wlTileChanged(int)));

  connect(m_kmenuColor, SIGNAL(changed(const QColor&)), SIGNAL(changed()));
  connect(m_desktopColor, SIGNAL(changed(const QColor&)), SIGNAL(changed()));
  connect(m_browserColor, SIGNAL(changed(const QColor&)), SIGNAL(changed()));
  connect(m_urlColor, SIGNAL(changed(const QColor&)), SIGNAL(changed()));
  connect(m_exeColor, SIGNAL(changed(const QColor&)), SIGNAL(changed()));
  connect(m_wlColor, SIGNAL(changed(const QColor&)), SIGNAL(changed()));

  connect(m_backgroundImage, SIGNAL(clicked()), SIGNAL(changed()));
  connect(m_transparent, SIGNAL(clicked()), SIGNAL(changed()));
  connect(m_colorizeImage, SIGNAL(toggled(bool)), SIGNAL(changed()));

  connect(m_backgroundInput->lineEdit(), SIGNAL(lostFocus()), SLOT(browseTheme()));
  m_backgroundInput->fileDialog()->setFilter(KImageIO::pattern(KImageIO::Reading));
  m_backgroundInput->fileDialog()->setCaption(i18n("Select Image File"));

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
    advancedDialog* dialog = new advancedDialog(this, "advancedDialog");
    dialog->exec();
}

void LookAndFeelTab::enableTransparency( bool enable )
{
    bool b = m_backgroundImage->isChecked();

    m_backgroundImage->setDisabled( enable );
    m_backgroundInput->setDisabled( enable || !b );
    m_backgroundLabel->setDisabled( enable || !b );
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

void LookAndFeelTab::load()
{
  KConfig c(KickerConfig::configName(), false, false);

  c.setGroup("General");

  bool use_theme = c.readBoolEntry("UseBackgroundTheme", true);
  QString theme = c.readEntry("BackgroundTheme", "wallpapers/default.png").stripWhiteSpace();

  bool transparent = c.readBoolEntry( "Transparent", false );

  m_backgroundImage->setChecked(use_theme);
  m_backgroundInput->setEnabled(use_theme);
  m_backgroundLabel->setEnabled(use_theme);
  m_colorizeImage->setChecked(c.readBoolEntry("ColorizeBackground", true));
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

  bool zoom = c.readBoolEntry("EnableIconZoom", false);
  m_zoom_cb->setChecked(zoom);

  QString tile;
  c.setGroup("buttons");

  m_kmenuTile->setCurrentItem(0);
  m_desktopTile->setCurrentItem(0);
  m_urlTile->setCurrentItem(0);
  m_browserTile->setCurrentItem(0);
  m_exeTile->setCurrentItem(0);
  m_wlTile->setCurrentItem(0);
  m_kmenuColor->setEnabled(false);
  m_desktopColor->setEnabled(false);
  m_urlColor->setEnabled(false);
  m_browserColor->setEnabled(false);
  m_exeColor->setEnabled(false);
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

    if (c.readBoolEntry("EnableExeTiles", false))
    {
      tile = c.readEntry("ExeTile", "solid_red");
      m_exeTile->setCurrentItem(m_tilename.findIndex(tile));
      m_exeColor->setColor(c.readColorEntry("KMenuTileColor"));
      m_exeColor->setEnabled(tile == "Colorize");
    }

    if (c.readBoolEntry("EnableWindowListTiles", false))
    {
      tile = c.readEntry("WindowListTile", "solid_green");
      m_wlTile->setCurrentItem(m_tilename.findIndex(tile));
      m_wlColor->setColor(c.readColorEntry("KMenuTileColor"));
      m_wlColor->setEnabled(tile == "Colorize");
    }
  }
}

void LookAndFeelTab::save()
{
  KConfig c(KickerConfig::configName(), false, false);

  c.setGroup("General");
  c.writeEntry("UseBackgroundTheme", m_backgroundImage->isChecked());
  c.writeEntry("ColorizeBackground", m_colorizeImage->isChecked());
  c.writeEntry("Transparent", m_transparent->isChecked());
  c.writeEntry("BackgroundTheme", m_backgroundInput->url());
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

  tile = m_exeTile->currentItem();
  if (tile > 0)
  {
    enableTiles = true;
    c.writeEntry("EnableExeTiles", tile > 0);
    c.writeEntry("ExeTile", m_tilename[m_exeTile->currentItem()]);
    c.writeEntry("ExeTileColor", m_exeColor->color());
  }
  else
  {
    c.writeEntry("EnableExeTiles", false);
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
  m_exeTile->setCurrentItem(0);
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
  m_exeColor->setColor(QColor());
  m_exeColor->setEnabled(false);

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
  m_exeTile->clear();
  m_exeTile->insertItem(i18n("Default"));
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
    for (QStringList::iterator w = words.begin(); w != words.end(); w++)
      (*w)[0] = (*w)[0].upper();
    tile = i18n(words.join(" ").utf8());

    m_kmenuTile->insertItem(pix, tile);
    m_desktopTile->insertItem(pix, tile);
    m_urlTile->insertItem(pix, tile);
    m_browserTile->insertItem(pix, tile);
    m_exeTile->insertItem(pix, tile);
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
  m_exeTile->setMinimumHeight(minHeight);
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

void LookAndFeelTab::exeTileChanged(int i)
{
    m_exeColor->setEnabled(i == 1);
}

void LookAndFeelTab::wlTileChanged(int i)
{
    m_wlColor->setEnabled(i == 1);
}
