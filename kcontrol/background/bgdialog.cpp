/*
   kate: space-indent on; indent-width 3; indent-mode cstyle;
   
   This file is part of the KDE libraries

   Copyright (c) 2005 David Saxton <david@bluehaze.org>
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <qapplication.h>
#include <QPixmap>
#include <QDesktopWidget>

#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kfilemetainfo.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <klocale.h>
#include <kpixmap.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kurlrequester.h>
#include <kwin.h>
#include <kimagefilepreview.h>
#include <knewstuff/downloaddialog.h>

#include <stdlib.h>

#include "bgmonitor.h"
#include "bgwallpaper.h"
#include "bgadvanced.h"
#include "bgdialog.h"

#define NR_PREDEF_PATTERNS 6

BGDialog::BGDialog(QWidget* parent, KConfig* _config, bool _multidesktop)
  : BGDialog_UI(parent, "BGDialog")
{
   m_pGlobals = new KGlobalBackgroundSettings(_config);
   m_pDirs = KGlobal::dirs();
   m_multidesktop = _multidesktop;
   m_previewUpdates = true;
   
   m_numDesks = m_multidesktop ? KWin::numberOfDesktops() : 1;
   m_numScreens = QApplication::desktop()->numScreens();
   
   QString multiHead = getenv("KDE_MULTIHEAD");
   if (multiHead.lower() == "true") 
   {
      m_numScreens = 1;
   }

   m_desk = m_multidesktop ? KWin::currentDesktop() : 1;
   m_screen = QApplication::desktop()->screenNumber(this);
   if (m_screen >= (int)m_numScreens)
      m_screen = m_numScreens-1;
   
   m_eDesk = m_pGlobals->commonDeskBackground() ? 0 : m_desk;
   getEScreen();
   m_copyAllDesktops = true;
   m_copyAllScreens = true;

   if (!m_multidesktop)
   {
      m_pDesktopLabel->hide();
      m_comboDesktop->hide();
   }

   if (m_numScreens < 2)
   {
      m_comboScreen->hide();
      m_buttonIdentifyScreens->hide();
      m_screen = 0;
      m_eScreen = 0;
   }
   
   connect(m_buttonIdentifyScreens, SIGNAL(clicked()), SLOT(slotIdentifyScreens()));

   // preview monitor
   m_pMonitorArrangement = new BGMonitorArrangement(m_screenArrangement, "monitor arrangement");
   connect(m_pMonitorArrangement, SIGNAL(imageDropped(const QString &)), SLOT(slotImageDropped(const QString &)));
   if( m_multidesktop)
     {
       // desktop
       connect(m_comboDesktop, SIGNAL(activated(int)),
	       SLOT(slotSelectDesk(int)));
     }
   if (m_numScreens > 1)
   {
       connect(m_comboScreen, SIGNAL(activated(int)),
               SLOT(slotSelectScreen(int)));
   }

   // background image settings
   QIcon iconSet = SmallIconSet(QLatin1String("fileopen"));
   QPixmap pixMap = iconSet.pixmap( QIcon::Small, QIcon::Normal );
   m_urlWallpaperButton->setIconSet( iconSet );
   m_urlWallpaperButton->setFixedSize( pixMap.width()+8, pixMap.height()+8 );
   m_urlWallpaperButton->setToolTip( i18n("Open file dialog"));

   connect(m_buttonGroupBackground, SIGNAL(clicked(int)),
           SLOT(slotWallpaperTypeChanged(int)));
   connect(m_urlWallpaperBox, SIGNAL(activated(int)),
           SLOT(slotWallpaper(int)));
   connect(m_urlWallpaperButton, SIGNAL(clicked()),
           SLOT(slotWallpaperSelection()));
   connect(m_comboWallpaperPos, SIGNAL(activated(int)),
           SLOT(slotWallpaperPos(int)));
   connect(m_buttonSetupWallpapers, SIGNAL(clicked()),
           SLOT(slotSetupMulti()));

   // set up the background colour stuff
   connect(m_colorPrimary, SIGNAL(changed(const QColor &)),
           SLOT(slotPrimaryColor(const QColor &)));
   connect(m_colorSecondary, SIGNAL(changed(const QColor &)),
           SLOT(slotSecondaryColor(const QColor &)));
   connect(m_comboPattern, SIGNAL(activated(int)),
           SLOT(slotPattern(int)));

   // blend
   connect(m_comboBlend, SIGNAL(activated(int)), SLOT(slotBlendMode(int)));
   connect(m_sliderBlend, SIGNAL(valueChanged(int)),
           SLOT(slotBlendBalance(int)));
   connect(m_cbBlendReverse, SIGNAL(toggled(bool)),
           SLOT(slotBlendReverse(bool)));

   // advanced options
   connect(m_buttonAdvanced, SIGNAL(clicked()),
           SLOT(slotAdvanced()));

   connect(m_buttonGetNew, SIGNAL(clicked()),
           SLOT(slotGetNewStuff()));

   // renderers
   m_renderer.resize(m_numDesks+1);
   
   if (m_numScreens > 1)
   {
      for (unsigned i = 0; i < m_numDesks+1; ++i)
      {
         m_renderer[i].resize(m_numScreens+2);
         m_renderer[i].setAutoDelete(true);
         
         int eDesk = i>0 ? i-1 : 0;
         
         // Setup the merged-screen renderer
         KBackgroundRenderer * r = new KBackgroundRenderer(eDesk, 0, false, _config);
         m_renderer[i].insert( 0, r );
         connect( r, SIGNAL(imageDone(int,int)), SLOT(slotPreviewDone(int,int)) );
         
         // Setup the common-screen renderer
         r = new KBackgroundRenderer(eDesk, 0, true, _config);
         m_renderer[i].insert( 1, r );
         connect( r, SIGNAL(imageDone(int,int)), SLOT(slotPreviewDone(int,int)) );
         
         // Setup the remaining renderers for each screen
         for (unsigned j=0; j < m_numScreens; ++j )
         {
            r = new KBackgroundRenderer(eDesk, j, true, _config);
            m_renderer[i].insert( j+2, r );
            connect( r, SIGNAL(imageDone(int,int)), SLOT(slotPreviewDone(int,int)) );
         }
      }
   }
   else
   {
      for (unsigned i = 0; i < m_numDesks+1; ++i )
      {
         m_renderer[i].resize(1);
         m_renderer[i].setAutoDelete(true);
      }
      
      // set up the common desktop renderer
      KBackgroundRenderer * r = new KBackgroundRenderer(0, 0, false, _config);
      m_renderer[0].insert(0, r);
      connect(r, SIGNAL(imageDone(int,int)), SLOT(slotPreviewDone(int,int)));

      // set up all the other desktop renderers
      for (unsigned i = 0; i < m_numDesks; ++i)
      {
         r = new KBackgroundRenderer(i, 0, false, _config);
         m_renderer[i+1].insert(0, r);
         connect(r, SIGNAL(imageDone(int,int)), SLOT(slotPreviewDone(int,int)));
      }
   }

   // Random or InOrder
   m_slideShowRandom = eRenderer()->multiWallpaperMode();
   if (m_slideShowRandom == KBackgroundSettings::NoMultiRandom)
      m_slideShowRandom = KBackgroundSettings::Random;
   if (m_slideShowRandom == KBackgroundSettings::NoMulti)
      m_slideShowRandom = KBackgroundSettings::InOrder;

   // Wallpaper Position
   m_wallpaperPos = eRenderer()->wallpaperMode();
   if (m_wallpaperPos == KBackgroundSettings::NoWallpaper)
      m_wallpaperPos = KBackgroundSettings::Centred; // Default

   if (KGlobal::dirs()->isRestrictedResource("wallpaper"))
   {
      m_urlWallpaperButton->hide();
      m_buttonSetupWallpapers->hide();
      m_radioSlideShow->hide();
   }

   initUI();
   updateUI();

   connect( qApp->desktop(), SIGNAL( resized( int )), SLOT( desktopResized())); // RANDR support
}

BGDialog::~BGDialog()
{
   delete m_pGlobals;
}

KBackgroundRenderer * BGDialog::eRenderer()
{
   return m_renderer[m_eDesk][m_eScreen];
}

void BGDialog::getEScreen()
{
   if ( m_pGlobals->drawBackgroundPerScreen( m_eDesk>0 ? m_eDesk-1 : 0 ) )
      m_eScreen = m_pGlobals->commonScreenBackground() ? 1 : m_screen+2;
   else
      m_eScreen = 0;
   
   if ( m_numScreens == 1 )
      m_eScreen = 0;
   else if ( m_eScreen > int(m_numScreens+1) )
      m_eScreen = m_numScreens+1;
}

void BGDialog::makeReadOnly()
{
    m_pMonitorArrangement->setEnabled( false );
    m_comboScreen->setEnabled( false );
    m_comboDesktop->setEnabled( false );
    m_colorPrimary->setEnabled( false );
    m_colorSecondary->setEnabled( false );
    m_comboPattern->setEnabled( false );
    m_radioNoPicture->setEnabled( false );
    m_radioPicture->setEnabled( false );
    m_radioSlideShow->setEnabled( false );
    m_urlWallpaperBox->setEnabled( false );
    m_urlWallpaperButton->setEnabled( false );
    m_comboWallpaperPos->setEnabled( false );
    m_buttonSetupWallpapers->setEnabled( false );
    m_comboBlend->setEnabled( false );
    m_sliderBlend->setEnabled( false );
    m_cbBlendReverse->setEnabled( false );
    m_buttonAdvanced->setEnabled( false );
    m_buttonGetNew->setEnabled( false );
}

void BGDialog::load()
{
   m_pGlobals->readSettings();
   m_eDesk = m_pGlobals->commonDeskBackground() ? 0 : m_desk;
   getEScreen();
   
   for (unsigned desk = 0; desk < m_renderer.size(); ++desk)
   {
      unsigned eDesk = desk>0 ? desk-1 : 0;
      for (unsigned screen = 0; screen < m_renderer[desk].size(); ++screen)
      {
         unsigned eScreen = screen>1 ? screen-2 : 0;
//          m_renderer[desk][screen]->load( eDesk, eScreen, (screen>0) );
         m_renderer[desk][screen]->load( eDesk, eScreen, (screen>0), true );
      }
   }
   
   m_copyAllDesktops = true;
   m_copyAllScreens = true;

   // Random or InOrder
   m_slideShowRandom = eRenderer()->multiWallpaperMode();
   if (m_slideShowRandom == KBackgroundSettings::NoMultiRandom)
      m_slideShowRandom = KBackgroundSettings::Random;
   if (m_slideShowRandom == KBackgroundSettings::NoMulti)
      m_slideShowRandom = KBackgroundSettings::InOrder;

   // Wallpaper Position
   m_wallpaperPos = eRenderer()->wallpaperMode();
   if (m_wallpaperPos == KBackgroundSettings::NoWallpaper)
      m_wallpaperPos = KBackgroundSettings::Centred; // Default

   updateUI();
   emit changed(false);
}

void BGDialog::save()
{
   m_pGlobals->writeSettings();

   // write out the common desktop or the "Desktop 1" settings
   // depending on which are the real settings
   // they both share Desktop[0] in the config file
   // similar for screen...
   
   for (unsigned desk = 0; desk < m_renderer.size(); ++desk)
   {
      if (desk == 0 && !m_pGlobals->commonDeskBackground())
         continue;
      
      if (desk == 1 && m_pGlobals->commonDeskBackground())
         continue;
      
      for (unsigned screen = 0; screen < m_renderer[desk].size(); ++screen)
      {
         if (screen == 1 && !m_pGlobals->commonScreenBackground())
            continue;
         
         if (screen == 2 && m_pGlobals->commonScreenBackground())
            continue;
         
         m_renderer[desk][screen]->writeSettings();
      }
   }

   emit changed(false);
}

void BGDialog::defaults()
{
   m_pGlobals->setCommonScreenBackground(_defCommonScreen);
   m_pGlobals->setCommonDeskBackground(_defCommonDesk);
   m_pGlobals->setLimitCache(_defLimitCache);
   m_pGlobals->setCacheSize(_defCacheSize);
   m_comboWallpaperPos->setCurrentIndex( 0 );

   m_eDesk = _defCommonDesk ? 0 : m_desk;
   getEScreen();

   for ( unsigned i = 0; i < m_numDesks; ++i )
      m_pGlobals->setDrawBackgroundPerScreen( i, false );

   KBackgroundRenderer *r = eRenderer();

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
   if (m_slideShowRandom == KBackgroundSettings::NoMultiRandom)
      m_slideShowRandom = KBackgroundSettings::Random;
   if (m_slideShowRandom == KBackgroundSettings::NoMulti)
      m_slideShowRandom = KBackgroundSettings::InOrder;

   r->setBlendMode(_defBlendMode);
   r->setBlendBalance(_defBlendBalance);
   r->setReverseBlending(_defReverseBlending);

   updateUI();

   m_copyAllDesktops = true;
   m_copyAllScreens = true;
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

void BGDialog::slotIdentifyScreens()
{
   // Taken from PositionTab::showIdentify in kdebase/kcontrol/kicker/positiontab_impl.cpp
   for(unsigned s = 0; s < m_numScreens; s++)
   {
      QLabel *screenLabel = new QLabel(0,"Screen Identify", Qt::WStyle_StaysOnTop | Qt::WDestructiveClose | Qt::WStyle_Customize | Qt::WStyle_NoBorder);

      KWin::setState( screenLabel->winId(), NET::Modal | NET::Sticky | NET::StaysOnTop | NET::SkipTaskbar | NET::SkipPager );
      KWin::setType( screenLabel->winId(), NET::Override );

      QFont identifyFont(KGlobalSettings::generalFont());
      identifyFont.setPixelSize(100);
      screenLabel->setFont(identifyFont);

      screenLabel->setFrameStyle(QFrame::Panel);
      screenLabel->setFrameShadow(QFrame::Plain);

      screenLabel->setAlignment(Qt::AlignCenter);
      screenLabel->setNum(int(s + 1));
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

void BGDialog::initUI()
{
   // Desktop names
   for (unsigned i = 0; i < m_numDesks; ++i)
      m_comboDesktop->insertItem(m_pGlobals->deskName(i));
   
   // Screens
   for (unsigned i = 0; i < m_numScreens; ++i)
      m_comboScreen->insertItem( i18n("Screen %1").arg(QString::number(i+1)) );

   // Patterns
   m_comboPattern->insertItem(i18n("Single Color"));
   m_comboPattern->insertItem(i18n("Horizontal Gradient"));
   m_comboPattern->insertItem(i18n("Vertical Gradient"));
   m_comboPattern->insertItem(i18n("Pyramid Gradient"));
   m_comboPattern->insertItem(i18n("Pipecross Gradient"));
   m_comboPattern->insertItem(i18n("Elliptic Gradient"));

   m_patterns = KBackgroundPattern::list();
   m_patterns.sort(); // Defined order
   QStringList::Iterator it;
   for (it=m_patterns.begin(); it != m_patterns.end(); ++it)
   {
      KBackgroundPattern pat(*it);
      if (pat.isAvailable())
         m_comboPattern->insertItem(pat.comment());
   }

   loadWallpaperFilesList();

   // Wallpaper tilings: again they must match the ones from bgrender.cc
   m_comboWallpaperPos->insertItem(i18n("Centered"));
   m_comboWallpaperPos->insertItem(i18n("Tiled"));
   m_comboWallpaperPos->insertItem(i18n("Center Tiled"));
   m_comboWallpaperPos->insertItem(i18n("Centered Maxpect"));
   m_comboWallpaperPos->insertItem(i18n("Tiled Maxpect"));
   m_comboWallpaperPos->insertItem(i18n("Scaled"));
   m_comboWallpaperPos->insertItem(i18n("Centered Auto Fit"));
   m_comboWallpaperPos->insertItem(i18n("Scale & Crop"));

   // Blend modes: make sure these match with kdesktop/bgrender.cc !!
   m_comboBlend->insertItem(i18n("No Blending"));
   m_comboBlend->insertItem(i18n("Flat"));
   m_comboBlend->insertItem(i18n("Horizontal"));
   m_comboBlend->insertItem(i18n("Vertical"));
   m_comboBlend->insertItem(i18n("Pyramid"));
   m_comboBlend->insertItem(i18n("Pipecross"));
   m_comboBlend->insertItem(i18n("Elliptic"));
   m_comboBlend->insertItem(i18n("Intensity"));
   m_comboBlend->insertItem(i18n("Saturation"));
   m_comboBlend->insertItem(i18n("Contrast"));
   m_comboBlend->insertItem(i18n("Hue Shift"));
}

void BGDialog::loadWallpaperFilesList() {

   // Wallpapers
   // the following QMap is lower cased names mapped to cased names and URLs
   // this way we get case insensitive sorting
   QMap<QString, QPair<QString, QString> > papers;

   //search for .desktop files before searching for images without .desktop files
   QStringList lst = m_pDirs->findAllResources("wallpaper", "*desktop", false, true);
   QStringList files;
   for (QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it)
   {
      KSimpleConfig fileConfig(*it);
      fileConfig.setGroup("Wallpaper");

      QString imageCaption = fileConfig.readEntry("Name");
      QString fileName = fileConfig.readEntry("File");

      if (imageCaption.isEmpty())
      {
         imageCaption = fileName;
         imageCaption.replace('_', ' ');
         imageCaption = KStringHandler::capwords(imageCaption);
      }

      imageCaption = KStringHandler::rEmSqueeze(imageCaption, m_urlWallpaperBox->fontMetrics(), 11);

      // avoid name collisions
      QString rs = imageCaption;
      QString lrs = rs.toLower();
      for (int n = 1; papers.find(lrs) != papers.end(); ++n)
      {
         rs = imageCaption + " (" + QString::number(n) + ')';
         lrs = rs.toLower();
      }
      int slash = (*it).lastIndexOf('/') + 1;
      QString directory = (*it).left(slash);
      bool canLoadScaleable = false;
#ifdef HAVE_LIBART
      canLoadScaleable = true;
#endif
      if ( fileConfig.readEntry("ImageType") == "pixmap" || canLoadScaleable ) {
	      papers[lrs] = qMakePair(rs, directory + fileName);
	      files.append(directory + fileName);
      }
   }

   //now find any wallpapers that don't have a .desktop file
   lst = m_pDirs->findAllResources("wallpaper", "*", false, true);
   for (QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it)
   {
      if ( !(*it).endsWith(".desktop") && files.grep(*it).empty() ) {
         // First try to see if we have a comment describing the image.  If we do
         // just use the first line of said comment.
         KFileMetaInfo metaInfo(*it);
         QString imageCaption;

         if (metaInfo.isValid() && metaInfo.item("Comment").isValid())
            imageCaption = metaInfo.item("Comment").string().section('\n', 0, 0);

         if (imageCaption.isEmpty())
         {
            int slash = (*it).lastIndexOf('/') + 1;
            int endDot = (*it).lastIndexOf('.');

            // strip the extension if it exists
            if (endDot != -1 && endDot > slash)
               imageCaption = (*it).mid(slash, endDot - slash);
            else
               imageCaption = (*it).mid(slash);

            imageCaption.replace('_', ' ');
            imageCaption = KStringHandler::capwords(imageCaption);
         }

         imageCaption = KStringHandler::rEmSqueeze(imageCaption, m_urlWallpaperBox->fontMetrics(), 11);

         // avoid name collisions
         QString rs = imageCaption;
         QString lrs = rs.toLower();
         for (int n = 1; papers.find(lrs) != papers.end(); ++n)
         {
            rs = imageCaption + " (" + QString::number(n) + ')';
            lrs = rs.toLower();
         }
         papers[lrs] = qMakePair(rs, *it);
      }
   }

   KComboBox *comboWallpaper = m_urlWallpaperBox;
   comboWallpaper->clear();
   m_wallpaper.clear();
   int i = 0;
   for (QMap<QString, QPair<QString, QString> >::Iterator it = papers.begin();
        it != papers.end();
        ++it)
   {
      comboWallpaper->insertItem(it.value().first);
      m_wallpaper[it.value().second] = i;
      i++;
   }
}

void BGDialog::setWallpaper(const QString &s)
{
   KComboBox *comboWallpaper = m_urlWallpaperBox;
   comboWallpaper->blockSignals(true);

   if (m_wallpaper.find(s) == m_wallpaper.end())
   {
      int i = comboWallpaper->count();
      QString imageCaption;
      int slash = s.lastIndexOf('/') + 1;
      int endDot = s.lastIndexOf('.');

      // strip the extension if it exists
      if (endDot != -1 && endDot > slash)
         imageCaption = s.mid(slash, endDot - slash);
      else
         imageCaption = s.mid(slash);
      if (comboWallpaper->itemText(i-1) == imageCaption)
      {
         i--;
         comboWallpaper->removeItem(i);
      }
      comboWallpaper->insertItem(KStringHandler::rEmSqueeze(imageCaption, m_urlWallpaperBox->fontMetrics(), 11));
      m_wallpaper[s] = i;
      comboWallpaper->setCurrentIndex(i);
   }
   else
   {
      comboWallpaper->setCurrentIndex(m_wallpaper[s]);
   }
   comboWallpaper->blockSignals(false);
}

void BGDialog::slotWallpaperSelection()
{
   KFileDialog dlg( QString::null, QString::null, this);

   KImageFilePreview* previewWidget = new KImageFilePreview(&dlg);
   dlg.setPreviewWidget(previewWidget);

   QStringList mimeTypes = KImageIO::mimeTypes( KImageIO::Reading ); 
#ifdef HAVE_LIBART
   mimeTypes += "image/svg+xml";
#endif
   dlg.setFilter( mimeTypes.join( " " ) );
   dlg.setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
   dlg.setCaption( i18n("Select Wallpaper") );

   int j = m_urlWallpaperBox->currentIndex();
   QString uri;
   for(QMap<QString,int>::ConstIterator it = m_wallpaper.begin();
       it != m_wallpaper.end();
       ++it)
   {
      if (it.value() == j)
      {
         uri = it.key();
         break;
      }
   }

   if ( !uri.isEmpty() ) {
      dlg.setSelection( uri );
   }

   if ( dlg.exec() == QDialog::Accepted )
   {
      setWallpaper(dlg.selectedFile());

      int optionID = m_buttonGroupBackground->id(m_radioPicture);
      m_buttonGroupBackground->setButton( optionID );
      slotWallpaperTypeChanged( optionID );

      emit changed(true);
   }
}

void BGDialog::updateUI()
{
   KBackgroundRenderer *r = eRenderer();
   m_comboDesktop->setCurrentIndex(m_eDesk);
   m_comboScreen->setCurrentItem(m_eScreen);

   m_colorPrimary->setColor(r->colorA());
   m_colorSecondary->setColor(r->colorB());

   int wallpaperMode = r->wallpaperMode();
   int multiMode = r->multiWallpaperMode();

   if (r->backgroundMode() == KBackgroundSettings::Program &&
       wallpaperMode == KBackgroundSettings::NoWallpaper)
      groupBox3->setEnabled( false );
   else
      groupBox3->setEnabled( true );

   if ((multiMode == KBackgroundSettings::NoMultiRandom) ||
       (multiMode == KBackgroundSettings::NoMulti))
   {
      // No wallpaper
      if (wallpaperMode == KBackgroundSettings::NoWallpaper )
      {
         m_urlWallpaperBox->setEnabled(false);
         m_urlWallpaperButton->setEnabled(false);
         m_buttonSetupWallpapers->setEnabled(false);
         m_comboWallpaperPos->setEnabled(false);
         m_lblWallpaperPos->setEnabled(false);
         m_buttonGroupBackground->setButton(
         m_buttonGroupBackground->id(m_radioNoPicture) );
      }

      // 1 Picture
      else
      {
         m_urlWallpaperBox->setEnabled(true);
         m_urlWallpaperButton->setEnabled(true);
         m_buttonSetupWallpapers->setEnabled(false);
         m_comboWallpaperPos->setEnabled(true);
         m_lblWallpaperPos->setEnabled(true);
         setWallpaper(r->wallpaper());
         m_buttonGroupBackground->setButton(
         m_buttonGroupBackground->id(m_radioPicture) );
      }
   }

   // Slide show
   else
   {
      m_urlWallpaperBox->setEnabled(false);
      m_urlWallpaperButton->setEnabled(false);
      m_buttonSetupWallpapers->setEnabled(true);
      m_comboWallpaperPos->setEnabled(true);
      m_lblWallpaperPos->setEnabled(true);
      m_buttonGroupBackground->setButton(
      m_buttonGroupBackground->id(m_radioSlideShow) );
   }

   m_comboWallpaperPos->setCurrentIndex(r->wallpaperMode()-1);

   bool bSecondaryEnabled = true;
   m_comboPattern->blockSignals(true);
   switch (r->backgroundMode()) {
     case KBackgroundSettings::Flat:
        m_comboPattern->setCurrentIndex(0);
        bSecondaryEnabled = false;
        break;

     case KBackgroundSettings::Pattern:
        {
           int i = m_patterns.indexOf(r->KBackgroundPattern::name());
           if (i >= 0)
              m_comboPattern->setCurrentIndex(NR_PREDEF_PATTERNS+i);
           else
              m_comboPattern->setCurrentIndex(0);
        }
        break;

     case KBackgroundSettings::Program:
        m_comboPattern->setCurrentIndex(0);
        bSecondaryEnabled = false;
        break;

     default: // Gradient
        m_comboPattern->setCurrentIndex(
           1 + r->backgroundMode() - KBackgroundSettings::HorizontalGradient);
        break;
    }
    m_comboPattern->blockSignals(false);

    m_colorSecondary->setEnabled(bSecondaryEnabled);

    int mode = r->blendMode();

    m_comboBlend->blockSignals(true);
    m_sliderBlend->blockSignals(true);

    m_comboBlend->setCurrentIndex(mode);
    m_cbBlendReverse->setChecked(r->reverseBlending());
    m_sliderBlend->setValue( r->blendBalance() / 10 );

    m_comboBlend->blockSignals(false);
    m_sliderBlend->blockSignals(false);

    // turn it off if there is no background picture set!
    setBlendingEnabled(wallpaperMode != KBackgroundSettings::NoWallpaper);

    // Start preview renderer(s)
    if ( m_eScreen == 0 )
    {
       r->setPreview( m_pMonitorArrangement->combinedPreviewSize() );
       r->start(true);
    }
    else if ( m_eScreen == 1 )
    {
       r->setPreview( m_pMonitorArrangement->maxPreviewSize() );
       r->start(true);
    }
    else
    {
       for (unsigned j = 0; j < m_numScreens; ++j)
       {
          m_renderer[m_eDesk][j+2]->stop();
          m_renderer[m_eDesk][j+2]->setPreview( m_pMonitorArrangement->monitor(j)->size() );
          m_renderer[m_eDesk][j+2]->start(true);
       }
    }
}

void BGDialog::slotPreviewDone(int desk_done, int screen_done)
{
   int currentDesk = (m_eDesk > 0) ? m_eDesk-1 : 0;
   
   if ( desk_done != currentDesk )
      return;

   if (!m_previewUpdates)
      return;

   KBackgroundRenderer * r = m_renderer[m_eDesk][(m_eScreen>1) ? (screen_done+2) : m_eScreen];

   if (!r->image())
      return;

   KPixmap pm;
   if (QPixmap::defaultDepth() < 15)
      pm.convertFromImage(*r->image(), KPixmap::LowColor);
   else
      pm.convertFromImage(*r->image());

   if ( m_eScreen == 0 )
   {
      m_pMonitorArrangement->setPixmap(pm);
   }
   else if ( m_eScreen == 1 )
   {
      for (unsigned i = 0; i < m_pMonitorArrangement->numMonitors(); ++i)
         m_pMonitorArrangement->monitor(i)->setPixmap(pm);
   }
   else
   {
      m_pMonitorArrangement->monitor(screen_done)->setPixmap(pm);
   }
}

void BGDialog::slotImageDropped(const QString &uri)
{
   setWallpaper(uri);

   int optionID = m_buttonGroupBackground->id(m_radioPicture);
   m_buttonGroupBackground->setButton( optionID );
   slotWallpaperTypeChanged( optionID );
}

void BGDialog::slotWallpaperTypeChanged(int i)
{
   KBackgroundRenderer *r = eRenderer();
   r->stop();

   // No picture
   if (i == m_buttonGroupBackground->id(m_radioNoPicture))  //None
   {
      m_urlWallpaperBox->setEnabled(false);
      m_urlWallpaperButton->setEnabled(false);
      m_buttonSetupWallpapers->setEnabled(false);
      m_comboWallpaperPos->setEnabled(false);
      m_lblWallpaperPos->setEnabled(false);
      r->setWallpaperMode(KBackgroundSettings::NoWallpaper);

      if (m_slideShowRandom == KBackgroundSettings::InOrder)
         r->setMultiWallpaperMode(KBackgroundSettings::NoMulti);
      else
         r->setMultiWallpaperMode(KBackgroundSettings::NoMultiRandom);

      setBlendingEnabled(false);
   }

   // Slide show
   else if (i == m_buttonGroupBackground->id(m_radioSlideShow))
   {
      m_urlWallpaperBox->setEnabled(false);
      m_urlWallpaperButton->setEnabled(false);
      m_buttonSetupWallpapers->setEnabled(true);
      m_comboWallpaperPos->setEnabled(true);
      m_lblWallpaperPos->setEnabled(true);
      setBlendingEnabled(true);

      m_comboWallpaperPos->blockSignals(true);
      m_comboWallpaperPos->setCurrentIndex(m_wallpaperPos-1);
      m_comboWallpaperPos->blockSignals(false);

      if (r->wallpaperList().count() == 0)
         r->setWallpaperMode( KBackgroundSettings::NoWallpaper );
      else
         r->setWallpaperMode(m_wallpaperPos);

      r->setMultiWallpaperMode(m_slideShowRandom);
      setWallpaper(r->wallpaper());
      setBlendingEnabled(true);
   }

   // 1 Picture
   else if (i == m_buttonGroupBackground->id(m_radioPicture))
   {
      m_urlWallpaperBox->setEnabled(true);
      m_urlWallpaperButton->setEnabled(true);
      m_buttonSetupWallpapers->setEnabled(false);
      m_lblWallpaperPos->setEnabled(true);
      m_comboWallpaperPos->setEnabled(true);
      setBlendingEnabled(true);

      if (m_slideShowRandom == KBackgroundSettings::InOrder)
         r->setMultiWallpaperMode(KBackgroundSettings::NoMulti);
      else
         r->setMultiWallpaperMode(KBackgroundSettings::NoMultiRandom);

      int j = m_urlWallpaperBox->currentIndex();
      QString uri;
      for(QMap<QString,int>::ConstIterator it = m_wallpaper.begin();
          it != m_wallpaper.end();
          ++it)
      {
         if (it.value() == j)
         {
            uri = it.key();
            break;
         }
      }

      KFileMetaInfo metaInfo(uri);
      if (metaInfo.isValid() && metaInfo.item("Dimensions").isValid())
      {
         // If the image is greater than 800x600 default to using scaled mode,
         // otherwise default to tiled.

         QSize s = metaInfo.item("Dimensions").value().toSize();
         if (s.width() >= 800 && s.height() >= 600)
            m_wallpaperPos = KBackgroundSettings::Scaled;
         else
            m_wallpaperPos = KBackgroundSettings::Tiled;
      }

      r->setWallpaperMode(m_wallpaperPos);
      m_comboWallpaperPos->blockSignals(true);
      m_comboWallpaperPos->setCurrentIndex(m_wallpaperPos-1);
      m_comboWallpaperPos->blockSignals(false);

      r->setWallpaper(uri);
   }

   r->start(true);
   m_copyAllDesktops = true;
   m_copyAllScreens = true;
   emit changed(true);
}

void BGDialog::slotWallpaper(int)
{
   slotWallpaperTypeChanged(m_buttonGroupBackground->id(m_radioPicture));
   emit changed(true);
}

void BGDialog::setBlendingEnabled(bool enable)
{
   int mode = eRenderer()->blendMode();

   bool b = !(mode == KBackgroundSettings::NoBlending);
   m_lblBlending->setEnabled(enable);
   m_comboBlend->setEnabled(enable);
   m_lblBlendBalance->setEnabled(enable && b);
   m_sliderBlend->setEnabled(enable && b);

   b = !(mode < KBackgroundSettings::IntensityBlending);
   m_cbBlendReverse->setEnabled(enable && b);
}

void BGDialog::slotWallpaperPos(int mode)
{
   KBackgroundRenderer *r = eRenderer();

   mode++;
   m_wallpaperPos = mode;

   if (mode == r->wallpaperMode())
      return;

   r->stop();
   r->setWallpaperMode(mode);
   r->start(true);
   m_copyAllDesktops = true;
   m_copyAllScreens = true;
   emit changed(true);
}

void BGDialog::slotSetupMulti()
{
    KBackgroundRenderer *r = eRenderer();

    BGMultiWallpaperDialog dlg(r, topLevelWidget());
    if (dlg.exec() == QDialog::Accepted) {
        r->stop();
        m_slideShowRandom = r->multiWallpaperMode();
        r->setWallpaperMode(m_wallpaperPos);
        r->start(true);
        m_copyAllDesktops = true;
        m_copyAllScreens = true;
        emit changed(true);
    }
}

void BGDialog::slotPrimaryColor(const QColor &color)
{
   KBackgroundRenderer *r = eRenderer();

   if (color == r->colorA())
      return;

   r->stop();
   r->setColorA(color);
   r->start(true);
   m_copyAllDesktops = true;
   m_copyAllScreens = true;
   emit changed(true);
}

void BGDialog::slotSecondaryColor(const QColor &color)
{
   KBackgroundRenderer *r = eRenderer();

   if (color == r->colorB())
      return;

   r->stop();
   r->setColorB(color);
   r->start(true);
   m_copyAllDesktops = true;
   m_copyAllScreens = true;
   emit changed(true);
}

void BGDialog::slotPattern(int pattern)
{
   KBackgroundRenderer *r = eRenderer();
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
      r->setPatternName(m_patterns[pattern - NR_PREDEF_PATTERNS]);
   }
   r->start(true);
   m_colorSecondary->setEnabled(bSecondaryEnabled);

   m_copyAllDesktops = true;
   m_copyAllScreens = true;
   emit changed(true);
}

void BGDialog::slotSelectScreen(int screen)
{
   // Copy the settings from "All screens" to all the other screens
   // at a suitable point
   if (m_pGlobals->commonScreenBackground() && (screen >1) && m_copyAllScreens)
   {
      // Copy stuff
      for (unsigned desk = 0; desk < m_numDesks+1; ++desk )
      {
         KBackgroundRenderer *master = m_renderer[desk][1];
         for (unsigned screen = 0; screen < m_numScreens; ++screen)
         {
            m_renderer[desk][screen+2]->copyConfig(master);
         }
      }
   }
   
   if (screen == m_eScreen )
   {
      return; // Nothing to do
   }

   m_copyAllScreens = false;
   
   bool drawBackgroundPerScreen = screen > 0;
   bool commonScreenBackground = screen < 2;
   
   // Update drawBackgroundPerScreen
   if (m_eDesk == 0)
   {
      for (unsigned desk = 0; desk < m_numDesks; ++desk )
         m_pGlobals->setDrawBackgroundPerScreen(desk, drawBackgroundPerScreen);
   }
   else
   {
      m_pGlobals->setDrawBackgroundPerScreen(m_eDesk-1, drawBackgroundPerScreen);
   }
   
   m_pGlobals->setCommonScreenBackground(commonScreenBackground);
   
   if (screen < 2)
      emit changed(true);
   else
   {
      for (unsigned i = 0; i < m_renderer[m_eDesk].size(); ++i)
      {
         if ( m_renderer[m_eDesk][i]->isActive() )
            m_renderer[m_eDesk][i]->stop();
      }
   }

   m_eScreen = screen;
   updateUI();
}

void BGDialog::slotSelectDesk(int desk)
{
   // Copy the settings from "All desktops" to all the other desktops
   // at a suitable point.
   if (m_pGlobals->commonDeskBackground() && (desk > 0) && m_copyAllDesktops)
   {
      // Copy stuff
      for (unsigned screen = 0; screen < m_renderer[0].size(); ++screen )
      {
         KBackgroundRenderer *master = m_renderer[0][screen];
         for (unsigned desk = 0; desk < m_numDesks; ++desk )
         {
            m_renderer[desk+1][screen]->copyConfig(master);
         }
      }
   }

   if (desk == m_eDesk)
   {
      return; // Nothing to do
   }

   m_copyAllDesktops = false;
   if (desk == 0)
   {
      if (m_pGlobals->commonDeskBackground())
         return; // Nothing to do

      m_pGlobals->setCommonDeskBackground(true);
      emit changed(true);
   }
   else
   {
      for (unsigned i = 0; i < m_renderer[m_eDesk].size(); ++i)
      {
         if ( m_renderer[m_eDesk][i]->isActive() )
            m_renderer[m_eDesk][i]->stop();
      }
      m_pGlobals->setCommonDeskBackground(false);
   }

   m_eDesk = desk;
   getEScreen();
   updateUI();
}

void BGDialog::slotAdvanced()
{
    KBackgroundRenderer *r = eRenderer();

    m_previewUpdates = false;
    BGAdvancedDialog dlg(r, topLevelWidget(), m_multidesktop);

    if (!m_pMonitorArrangement->isEnabled()) {
       dlg.makeReadOnly();
       dlg.exec();
       return;
    }

    dlg.setTextColor(m_pGlobals->textColor());
    dlg.setTextBackgroundColor(m_pGlobals->textBackgroundColor());
    dlg.setShadowEnabled(m_pGlobals->shadowEnabled());
    dlg.setTextLines(m_pGlobals->textLines());
    dlg.setTextWidth(m_pGlobals->textWidth());

    if (m_pGlobals->limitCache())
       dlg.setCacheSize( m_pGlobals->cacheSize() );
    else
       dlg.setCacheSize( 0 );

    if( !dlg.exec())
    {
        m_previewUpdates = true;
        return;
    }
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

    m_pGlobals->setTextColor(dlg.textColor());
    m_pGlobals->setTextBackgroundColor(dlg.textBackgroundColor());
    m_pGlobals->setShadowEnabled(dlg.shadowEnabled());
    m_pGlobals->setTextLines(dlg.textLines());
    m_pGlobals->setTextWidth(dlg.textWidth());

    r->stop();
    m_previewUpdates = true;
    r->start(true);

    updateUI();
    m_copyAllDesktops = true;
    emit changed(true);
}

void BGDialog::slotGetNewStuff()
{
   //FIXME set this to a server when we get one
   //should really be in a .rc file but could be either
   //kcmshellrc or kcontrolrc
   KConfig* config = KGlobal::config();
   config->setGroup("KNewStuff");
   config->writeEntry( "ProvidersUrl", "http://download.kde.org/khotnewstuff/wallpaper-providers.xml" );
   config->writeEntry( "StandardResource", "wallpaper" );
   config->sync();

   KNS::DownloadDialog::open("wallpapers", i18n("Get New Wallpapers"));
   loadWallpaperFilesList();
}

void BGDialog::slotBlendMode(int mode)
{
   if (mode == eRenderer()->blendMode())
      return;

   bool b = !(mode == KBackgroundSettings::NoBlending);
   m_sliderBlend->setEnabled( b );
   m_lblBlendBalance->setEnabled( b );

   b = !(mode < KBackgroundSettings::IntensityBlending);
   m_cbBlendReverse->setEnabled(b);
   emit changed(true);

   eRenderer()->stop();
   eRenderer()->setBlendMode(mode);
   eRenderer()->start(true);
}

void BGDialog::slotBlendBalance(int value)
{
   value = value*10;
   if (value == eRenderer()->blendBalance())
      return;
   emit changed(true);

   eRenderer()->stop();
   eRenderer()->setBlendBalance(value);
   eRenderer()->start(true);
}

void BGDialog::slotBlendReverse(bool b)
{
   if (b == eRenderer()->reverseBlending())
      return;
   emit changed(true);

   eRenderer()->stop();
   eRenderer()->setReverseBlending(b);
   eRenderer()->start(true);
}

void BGDialog::desktopResized()
{
   for (unsigned i = 0; i < m_renderer.size(); ++i)
   {
      for (unsigned j = 0; j < m_renderer[i].size(); ++j )
      {
         KBackgroundRenderer * r = m_renderer[i][j];
         if( r->isActive())
             r->stop();
         r->desktopResized();
      }
   }
   eRenderer()->start(true);
}


#include "bgdialog.moc"
