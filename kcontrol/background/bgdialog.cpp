/* 
   This file is part of the KDE libraries

   Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
   Copyright (c) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
   Copyright (c) 1996 Martin R. Jones
   Copyright (c) 1997 Matthias Hoelzer
   Copyright (c) 1997 Mark Donohoe
   Copyright (c) 1998 Stephan Kulow
   Copyright (c) 1998 Matej Koss
   
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License 
   version 2 as published by the Free Software Foundation.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qcheckbox.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kpixmap.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kurlrequester.h>
#include <kwin.h>

#include "bgmonitor.h"
#include "bgwallpaper.h"
#include "bgadvanced.h"
#include "bgdialog.h"

#define NR_PREDEF_PATTERNS	6
#define SLIDE_SHOW		i18n("<Slide Show>")

BGDialog::BGDialog(QWidget* parent, KConfig* _config, bool _multidesktop)
  : BGDialog_UI(parent, "BGDialog")
{
   m_pGlobals = new KGlobalBackgroundSettings(_config);
   m_pDirs = KGlobal::dirs();
   m_multidesktop = _multidesktop;
   m_previewUpdates = true;
   m_copyAllDesktops = true;

   m_Max = m_multidesktop ? KWin::numberOfDesktops() : 1;
   m_Desk = m_multidesktop ? (KWin::currentDesktop() - 1) : 0;
   m_eDesk = m_pGlobals->commonBackground() ? 0 : m_Desk;

   m_comboWallpaperSpecial = -1;
   
   if (!m_multidesktop)
   {
      m_pDesktopLabel->hide();
      m_comboDesktop->hide();
      m_buttonSetupWallpapers->hide();
      m_groupIconText->hide();
   }

   m_monitorImage->setText(QString::null);
   m_monitorImage->setPixmap(locate("data", "kcontrol/pics/monitor.png"));
   m_monitorImage->setFixedSize(m_monitorImage->sizeHint());
   m_pMonitor = new BGMonitor(m_monitorImage, "preview monitor");
   m_pMonitor->setGeometry(23, 14, 151, 115);
   connect(m_pMonitor, SIGNAL(imageDropped(const QString &)), SLOT(slotImageDropped(const QString &)));

   connect(m_comboDesktop, SIGNAL(activated(int)),
           SLOT(slotSelectDesk(int)));

   connect(m_urlWallpaper->comboBox(), SIGNAL(activated(int)),
           SLOT(slotWallpaper(int)));
   connect(m_urlWallpaper, SIGNAL(urlSelected(const QString &)),
           SLOT(slotImageDropped(const QString &)));
   connect(m_comboWallpaperPos, SIGNAL(activated(int)),
           SLOT(slotWallpaperPos(int)));

   connect(m_buttonSetupWallpapers, SIGNAL(clicked()),
           SLOT(slotSetupMulti()));

   connect(m_colorPrimary, SIGNAL(changed(const QColor &)),
           SLOT(slotPrimaryColor(const QColor &)));
   connect(m_colorSecondary, SIGNAL(changed(const QColor &)),
           SLOT(slotSecondaryColor(const QColor &)));
   connect(m_comboPattern, SIGNAL(activated(int)),
           SLOT(slotPattern(int)));

   connect(m_colorText, SIGNAL(changed(const QColor &)),
           SLOT(slotTextColor()));
   connect(m_colorTextBackground, SIGNAL(changed(const QColor &)),
           SLOT(slotTextColor()));
   connect(m_cbSolidTextBackground, SIGNAL(toggled(bool)),
           SLOT(slotTextColor()));

   connect(m_buttonAdvanced, SIGNAL(clicked()),
           SLOT(slotAdvanced()));
           
   m_Renderer = QPtrVector<KBackgroundRenderer>( m_Max );
   m_Renderer.setAutoDelete(true);
   for (int i=0; i<m_Max; i++) 
   {
      m_Renderer.insert(i, new KBackgroundRenderer(i, _config));
      connect(m_Renderer[i], SIGNAL(imageDone(int)), SLOT(slotPreviewDone(int)));
   }

   m_slideShowRandom = m_Renderer[m_eDesk]->multiWallpaperMode();
   m_wallpaperPos = m_Renderer[m_eDesk]->wallpaperMode();
   if (m_wallpaperPos == KBackgroundSettings::NoWallpaper)
      m_wallpaperPos = KBackgroundSettings::Centred; // Default

   QWhatsThis::add( m_comboDesktop, i18n("Choose the desktop whose background"
	" you want to modify.") );

   QWhatsThis::add( m_pMonitor, i18n("In this monitor, you can preview how your settings will look like on a \"real\" desktop.") );

   QWhatsThis::add( m_colorPrimary, i18n("Click to choose the primary background color.") );
   QWhatsThis::add( m_colorSecondary, i18n("Click to choose the secondary background color. If no"
					" secondary color is used, this button is disabled.") );
   QWhatsThis::add( m_buttonSetupWallpapers, i18n("With this button you can choose"
					   " a set of images to be used as background picture. "
					   " One picture at a time will be shown for the specified time,"
					   " after which another image from the set is shown."
					   " Images can be shown at random or in the order you specified them.") );

   QWhatsThis::add( m_comboWallpaperPos, i18n("You can choose here how a background picture is shown on the desktop:"
					     " <ul><li><em>Centered:</em> Center the picture on the desktop.</li>"
					     " <li><em>Tiled:</em> Tile the picture beginning at the top left of the"
					     " desktop, so the desktop is totally covered up.</li>"
					     " <li><em>Center Tiled:</em> Center the picture on the desktop, and then"
					     " tile around it so that the background is totally covered up.</li>"
					     " <li><em>Centered Maxpect:</em> Magnify the picture without distorting it"
					     " until it fills either the width or height of the desktop, and then center"
					     " it on the desktop.</li>"
					     " <li><em>Scaled:</em> Magnify the picture, distorting it if necessary,"
					     " until the entire desktop is covered.</li>"
                                             " <li><em>Centered Auto Fit:</em> If the picture fits the desktop,"
                                             " this mode works like Centered. If the picture is larger"
                                             " than the desktop, it's scalled down to fit while keeping the aspect"
                                             " ratio.</li></ul>") );

   QWhatsThis::add( m_urlWallpaper->comboBox(), 
   		i18n("Click to choose from the list a graphic that you want"
		     " to use as background picture.") );

   QWhatsThis::add( m_urlWallpaper, 
   		i18n("Click here to freely choose a graphic or photo that you want"
		     " to use as background picture.") );

   initUI();
   updateUI();
}

BGDialog::~BGDialog()
{
   delete m_pGlobals;
}

void BGDialog::makeReadOnly()
{
    m_pMonitor->setEnabled( false );

    m_comboDesktop->setEnabled( false );
    m_colorPrimary->setEnabled( false );
    m_colorSecondary->setEnabled( false );
    m_comboPattern->setEnabled( false );

    m_urlWallpaper->setEnabled( false );
    m_comboWallpaperPos->setEnabled( false );
    m_buttonSetupWallpapers->setEnabled( false );
    m_buttonAdvanced->setEnabled( false );
}


void BGDialog::load()
{
   m_pGlobals->readSettings();
   m_eDesk = m_pGlobals->commonBackground() ? 0 : m_Desk;
   m_Renderer[m_eDesk]->load(m_eDesk);

   m_slideShowRandom = m_Renderer[m_eDesk]->multiWallpaperMode();
   m_wallpaperPos = m_Renderer[m_eDesk]->wallpaperMode();
   if (m_wallpaperPos == KBackgroundSettings::NoWallpaper)
      m_wallpaperPos = KBackgroundSettings::Centred; // Default

   updateUI();
   m_copyAllDesktops = true;
   emit changed(false);
}

void BGDialog::save()
{
   kdDebug() << "Saving stuff..." << endl;
   m_pGlobals->writeSettings();
   for (int i=0; i<m_Max; i++)
      m_Renderer[i]->writeSettings();

   emit changed(false);
}

void BGDialog::defaults()
{
   m_pGlobals->setCommonBackground(_defCommon);
   m_pGlobals->setLimitCache(_defLimitCache);
   m_pGlobals->setCacheSize(_defCacheSize);

   m_eDesk = _defCommon ? 0 : m_Desk;

   KBackgroundRenderer *r = m_Renderer[m_eDesk];

   if (r->isActive())
      r->stop();

   if (QPixmap::defaultDepth() > 8) 
   {
      r->setBackgroundMode(_defBackgroundMode);
   }
   else
   {
      r->setBackgroundMode(KBackgroundSettings::Flat);
   }

   r->setColorA(_defColorA);
   r->setColorB(_defColorB);
   r->setWallpaperMode(_defWallpaperMode);
   r->setMultiWallpaperMode(_defMultiMode);
   m_slideShowRandom = _defMultiMode;
   r->setBlendMode(_defBlendMode);
   r->setBlendBalance(_defBlendBalance);
   r->setReverseBlending(_defReverseBlending);

   updateUI();

   m_copyAllDesktops = true;
   emit changed(true);
}
               
QString BGDialog::quickHelp() const
{
   return i18n("<h1>Background</h1> This module allows you to control the"
      " appearance of the virtual desktops. KDE offers a variety of options"
      " for customization, including the ability to specify different settings"
      " for each virtual desktop, or a common background for all of them.<p>"
      " The appearance of the desktop results from the combination of its"
      " background colors and patterns, and optionally, wallpaper, which is"
      " based on the image from a graphic file.<p>"
      " The background can be made up of a single color, or a pair of colors"
      " which can be blended in a variety of patterns. Wallpaper is also"
      " customizable, with options for tiling and stretching images. The"
      " wallpaper can be overlaid opaquely, or blended in different ways with"
      " the background colors and patterns.<p>"
      " KDE allows you to have the wallpaper change automatically at specified"
      " intervals of time. You can also replace the background with a program"
      " that updates the desktop dynamically. For example, the \"kdeworld\""
      " program shows a day/night map of the world which is updated periodically.");
}

void BGDialog::initUI()
{
   int i;

   // Desktop names
   for (i=0; i<m_Max; i++)
      m_comboDesktop->insertItem(m_pGlobals->deskName(i));

   // Patterns
   m_comboPattern->insertItem(i18n("Single Color"));
   m_comboPattern->insertItem(i18n("Horizontal Gradient"));
   m_comboPattern->insertItem(i18n("Vertical Gradient"));
   m_comboPattern->insertItem(i18n("Pyramid Gradient"));
   m_comboPattern->insertItem(i18n("Pipecross Gradient"));
   m_comboPattern->insertItem(i18n("Elliptic Gradient"));

   m_Patterns = KBackgroundPattern::list();
   m_Patterns.sort(); // Defined order
   QStringList::Iterator it;
   for (it=m_Patterns.begin(); it != m_Patterns.end(); it++)
   {
      KBackgroundPattern pat(*it);
      m_comboPattern->insertItem(pat.comment());
   }

   // Wallpapers
   QStringList lst = m_pDirs->findAllResources("wallpaper", "*", false, true);
   lst.sort();
   KComboBox *comboWallpaper = m_urlWallpaper->comboBox();
   comboWallpaper->insertItem(i18n("<None>"));
   for (i=0; i<(int)lst.count(); i++) 
   {
      int n = lst[i].findRev('/');
      QString s = lst[i].mid(n+1);
      comboWallpaper->insertItem(s);
      m_Wallpaper[s] = i+1;
   }

   // Wallpaper tilings: again they must match the ones from bgrender.cc
   m_comboWallpaperPos->insertItem(i18n("Centered"));
   m_comboWallpaperPos->insertItem(i18n("Tiled"));
   m_comboWallpaperPos->insertItem(i18n("Center Tiled"));
   m_comboWallpaperPos->insertItem(i18n("Centered Maxpect"));
   m_comboWallpaperPos->insertItem(i18n("Tiled Maxpect"));
   m_comboWallpaperPos->insertItem(i18n("Scaled"));
   m_comboWallpaperPos->insertItem(i18n("Centered Auto Fit"));
}

void BGDialog::setWallpaper(const QString &s)
{
qWarning("setWallpaper(%s)", s.latin1());
   KComboBox *comboWallpaper = m_urlWallpaper->comboBox();
   comboWallpaper->blockSignals(true);
   if (s.isEmpty())
   {
      comboWallpaper->setCurrentItem(0);
qWarning("comboWallpaper->setCurrentItem(0) #1");
   }
   else if (s.startsWith("<"))
   {
      if (m_comboWallpaperSpecial == -1)
      {
         int i = comboWallpaper->count();
         comboWallpaper->insertItem(s);
         m_comboWallpaperSpecial = i;
      }
      else
      {
         comboWallpaper->changeItem(s, m_comboWallpaperSpecial);
      }
      comboWallpaper->setCurrentItem(m_comboWallpaperSpecial);
qWarning("comboWallpaper->setCurrentItem(%d) #2", m_comboWallpaperSpecial);
   }
   else
   {
      if (m_Wallpaper.find(s) == m_Wallpaper.end())
      {
         int i = comboWallpaper->count();
         if (comboWallpaper->text(i-1) == s)
         {
            i--;
            comboWallpaper->removeItem(i);
         }   
         comboWallpaper->insertItem(KStringHandler::lsqueeze(s, 45));
         m_Wallpaper[s] = i;
         comboWallpaper->setCurrentItem(i);
qWarning("comboWallpaper->setCurrentItem(%d) #3", i);
      }
      else
      {
         comboWallpaper->setCurrentItem(m_Wallpaper[s]);
qWarning("comboWallpaper->setCurrentItem(%d) #4", m_Wallpaper[s]);
      }
   }
   comboWallpaper->blockSignals(false);
}

void BGDialog::updateUI()
{
   KBackgroundRenderer *r = m_Renderer[m_eDesk];
   
   if (m_pGlobals->commonBackground())
      m_comboDesktop->setCurrentItem(0);
   else
      m_comboDesktop->setCurrentItem(m_Desk+1);
       
   m_colorPrimary->setColor(r->colorA());
   m_colorSecondary->setColor(r->colorB());
   
   if (r->backgroundMode() == KBackgroundSettings::Program)
   {
      m_colorPrimary->setEnabled(false);
      m_comboPattern->setEnabled(false);
   }
   else
   {
      m_colorPrimary->setEnabled(true);
      m_comboPattern->setEnabled(true);
   }

   int wallpaperMode = r->wallpaperMode();
   int multiMode = r->multiWallpaperMode();
qWarning("Wallpaper mode = %d %s", wallpaperMode, r->wallpaper().latin1());
   if (wallpaperMode == KBackgroundSettings::NoWallpaper )
   {
      m_comboWallpaperPos->setEnabled(false);
      m_lblWallpaperPos->setEnabled(false);
      setWallpaper(QString::null);
   }
   else if ((multiMode == KBackgroundSettings::NoMultiRandom) ||
            (multiMode == KBackgroundSettings::NoMulti))
   {
      m_comboWallpaperPos->setEnabled(true);
      m_lblWallpaperPos->setEnabled(true);
      setWallpaper(r->wallpaper());
   }
   else
   {
      m_comboWallpaperPos->setEnabled(true);
      m_lblWallpaperPos->setEnabled(true);
      setWallpaper(SLIDE_SHOW);
   }

   m_comboWallpaperPos->setCurrentItem(r->wallpaperMode()-1);

   bool bSecondaryEnabled = true;   
   m_comboPattern->blockSignals(true);
   switch (r->backgroundMode()) {
     case KBackgroundSettings::Flat:
        m_comboPattern->setCurrentItem(0);
        bSecondaryEnabled = false;
        break;

     case KBackgroundSettings::Pattern:
        {
           int i = m_Patterns.findIndex(r->KBackgroundPattern::name());
           if (i >= 0)
              m_comboPattern->setCurrentItem(NR_PREDEF_PATTERNS+i);
           else
              m_comboPattern->setCurrentItem(0);
        }
        break;

     case KBackgroundSettings::Program:
        m_comboPattern->setCurrentItem(0);
        bSecondaryEnabled = false;
        break;

     default: // Gradient
        m_comboPattern->setCurrentItem(1 + r->backgroundMode() - KBackgroundSettings::HorizontalGradient);
        break;
    }
    m_comboPattern->blockSignals(false);

    m_colorSecondary->setEnabled(bSecondaryEnabled);
    m_lblColorSecondary->setEnabled(bSecondaryEnabled);

    m_colorText->blockSignals(true);
    m_colorTextBackground->blockSignals(true);
    m_cbSolidTextBackground->blockSignals(true);
        
    m_colorText->setColor(r->textColor());
    if (r->textBackgroundColor().isValid())
    {
       m_cbSolidTextBackground->setChecked(true);
       m_colorTextBackground->setColor(r->textBackgroundColor());
       m_colorTextBackground->setEnabled(true);
       m_lblTextBackground->setEnabled(true);
    }
    else
    {
       m_cbSolidTextBackground->setChecked(false);
       m_colorTextBackground->setColor(Qt::white);
       m_colorTextBackground->setEnabled(false);
       m_lblTextBackground->setEnabled(false);
    }

    m_colorText->blockSignals(false);
    m_colorTextBackground->blockSignals(false);
    m_cbSolidTextBackground->blockSignals(false);


    // Start preview render
    r->setPreview(m_pMonitor->size());
    r->start();
}


void BGDialog::slotPreviewDone(int desk_done)
{
   kdDebug() << "Preview for desktop " << desk_done << " done" << endl;

   if (m_eDesk != desk_done)
      return;
      
   if (!m_previewUpdates)
      return;
      
   KBackgroundRenderer *r = m_Renderer[m_eDesk];

   KPixmap pm;
   if (QPixmap::defaultDepth() < 15)
      pm.convertFromImage(*r->image(), KPixmap::LowColor);
   else
      pm.convertFromImage(*r->image());

   m_pMonitor->setBackgroundPixmap(pm);
}

void BGDialog::slotImageDropped(const QString &uri)
{
   KBackgroundRenderer *r = m_Renderer[m_eDesk];

   m_comboWallpaperPos->setEnabled(true);
   m_lblWallpaperPos->setEnabled(true);
   r->setMultiWallpaperMode(KBackgroundSettings::NoMulti);
   r->setWallpaperMode(m_wallpaperPos);

   setWallpaper(uri);

   r->stop();
   r->setWallpaper(uri);
   r->start();

   m_copyAllDesktops = true;
   emit changed(true);
}

void BGDialog::slotWallpaper(int i)
{
   KBackgroundRenderer *r = m_Renderer[m_eDesk];

   r->stop();
   if (i == 0)
   {
      m_comboWallpaperPos->setEnabled(false);
      m_lblWallpaperPos->setEnabled(false);
      r->setWallpaperMode(KBackgroundSettings::NoWallpaper);
   }
   else if (i == m_comboWallpaperSpecial)
   {

      if (m_urlWallpaper->comboBox()->currentText() == SLIDE_SHOW)
      {
         m_comboWallpaperPos->setEnabled(true);
         m_lblWallpaperPos->setEnabled(true);
         m_comboWallpaperPos->blockSignals(true);
         m_comboWallpaperPos->setCurrentItem(m_wallpaperPos-1);
         m_comboWallpaperPos->blockSignals(false);

         r->setMultiWallpaperMode(m_slideShowRandom);
         r->setWallpaperMode(m_wallpaperPos);
      }
   }
   else
   {
      m_comboWallpaperPos->setEnabled(true);
      m_lblWallpaperPos->setEnabled(true);
      m_comboWallpaperPos->blockSignals(true);
      m_comboWallpaperPos->setCurrentItem(m_wallpaperPos-1);
      m_comboWallpaperPos->blockSignals(false);
      QString uri;
      for(QMap<QString,int>::ConstIterator it = m_Wallpaper.begin();
          it != m_Wallpaper.end();
          ++it)
      {
         if (it.data() == i)
         {
            uri = it.key();
            break;
         }
      }
      
      r->setMultiWallpaperMode(KBackgroundSettings::NoMulti);
      r->setWallpaperMode(m_wallpaperPos);
      r->setWallpaper(uri);
   }

   r->start();
   m_copyAllDesktops = true;
   emit changed(true);
}

void BGDialog::slotWallpaperPos(int mode)
{
   KBackgroundRenderer *r = m_Renderer[m_eDesk];

   mode++;
   m_wallpaperPos = mode;

   if (mode == r->wallpaperMode())
      return;

   r->stop();
   r->setWallpaperMode(mode);
   r->start();
   m_copyAllDesktops = true;
   emit changed(true);
}

void BGDialog::slotSetupMulti()
{
    KBackgroundRenderer *r = m_Renderer[m_eDesk];

    BGMultiWallpaperDialog dlg(r, topLevelWidget());
    if (dlg.exec() == QDialog::Accepted) {
        r->stop();
        m_slideShowRandom = r->multiWallpaperMode();
        r->setWallpaperMode(m_wallpaperPos);
        r->start();
        updateUI();
        m_copyAllDesktops = true;
        emit changed(true);
    }
}


void BGDialog::slotPrimaryColor(const QColor &color)
{
   KBackgroundRenderer *r = m_Renderer[m_eDesk];

   if (color == r->colorA())
      return;

   r->stop();
   r->setColorA(color);
   r->start();
   m_copyAllDesktops = true;
   emit changed(true);
}

void BGDialog::slotSecondaryColor(const QColor &color)
{
   KBackgroundRenderer *r = m_Renderer[m_eDesk];

   if (color == r->colorB())
      return;

   r->stop();
   r->setColorB(color);
   r->start();
   m_copyAllDesktops = true;
   emit changed(true);
}

void BGDialog::slotTextColor()
{
   KBackgroundRenderer *r = m_Renderer[m_eDesk];
   r->setTextColor(m_colorText->color());
   if (m_cbSolidTextBackground->isChecked())
   {
      r->setTextBackgroundColor(m_colorTextBackground->color());
      m_lblTextBackground->setEnabled(true);
      m_colorTextBackground->setEnabled(true);
   }
   else
   {
      r->setTextBackgroundColor(QColor());
      m_lblTextBackground->setEnabled(false);
      m_colorTextBackground->setEnabled(false);
   }
   m_copyAllDesktops = true;
   emit changed(true);
}

void BGDialog::slotPattern(int pattern)
{
   KBackgroundRenderer *r = m_Renderer[m_eDesk];
   r->stop();
   bool bSecondaryEnabled = true;
   if (pattern < NR_PREDEF_PATTERNS)
   {
      if (pattern == 0)
      {
        r->setBackgroundMode(KBackgroundSettings::Flat);
        bSecondaryEnabled = false;
      }
      else
      {
        r->setBackgroundMode(pattern - 1 + KBackgroundSettings::HorizontalGradient);
      }
   }
   else
   {
      r->setBackgroundMode(KBackgroundSettings::Pattern);
      r->setPatternName(m_Patterns[pattern - NR_PREDEF_PATTERNS]);
   }
   r->start();
   m_colorSecondary->setEnabled(bSecondaryEnabled);
   m_lblColorSecondary->setEnabled(bSecondaryEnabled);
   
   m_copyAllDesktops = true;
   emit changed(true);
}

void BGDialog::slotSelectDesk(int desk)
{
   // Copy the settings from "All desktops" to all the other desktops
   // at a suitable point.
   if (m_pGlobals->commonBackground() && (desk > 0) && m_copyAllDesktops)
   {
      // Copy stuff
      for (int i=1; i<m_Max; i++) 
      {
         m_Renderer[i]->copyConfig(m_Renderer[0]);
      }
      
   }
   m_copyAllDesktops = false;

   if (desk == 0)
   {
      if (m_pGlobals->commonBackground())
         return; // Nothing to do
       
      m_pGlobals->setCommonBackground(true);
      m_eDesk = m_Desk = 0;
   }
   else
   {
      desk--;
      if (desk == m_Desk && !m_pGlobals->commonBackground())
         return; // Nothing to do

      if (m_Renderer[m_Desk]->isActive())
         m_Renderer[m_Desk]->stop();
      m_pGlobals->setCommonBackground(false);
      m_eDesk = m_Desk = desk;
   }
   updateUI();
}

void BGDialog::slotAdvanced()
{
    KBackgroundRenderer *r = m_Renderer[m_eDesk];

    m_previewUpdates = false;
    BGAdvancedDialog dlg(r, topLevelWidget(), m_multidesktop);
    
    if (m_pGlobals->limitCache())
       dlg.setCacheSize( m_pGlobals->cacheSize() );
    else
       dlg.setCacheSize( 0 );
    
    dlg.exec();

    int cacheSize = dlg.cacheSize();
    if (cacheSize)
    {
       m_pGlobals->setCacheSize(cacheSize);
       m_pGlobals->setLimitCache(true);
    }
    else
    {
       m_pGlobals->setLimitCache(false);
    }
    
    r->stop();
    m_previewUpdates = true;
    
    r->start();
    updateUI();
    m_copyAllDesktops = true;
    emit changed(true);
}

#include "bgdialog.moc"
