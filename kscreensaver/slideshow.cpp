/* Slide Show Screen Saver
 *  (C) 1999 Stefan Taferner <taferner@kde.org>
 *
 * This code is under GPL
 */


#define protected public
#include <qwidget.h>
#undef protected

#include <qdir.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfile.h>
#include <qcolor.h>
#include <qpaintdevicemetrics.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qslider.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kapp.h>
#include <klocale.h>
#include <kbuttonbox.h>
#include <kmessagebox.h>
#include <kimgio.h>
#include <kfiledialog.h>

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "slideshow.h"

// This refers to klock.po. If you want an extra dictionary,
// create an extra KLocale instance here.
extern KLocale *glocale;

static kSlideShowSaver *sSaver = NULL;


//=============================================================================
//  Class kSlideShowSaver
//=============================================================================
kSlideShowSaver::kSlideShowSaver(Drawable drawable): kScreenSaver(drawable)
{
  time_t t;

  time(&t);
  srand((unsigned int)t);

  kimgioRegister();

  mWidget.resize(1,1);
  mWidget.create((WId)drawable);
  mWidget.setBackgroundColor(black);
  mWidget.resize(800,600);
  blank();

  mEffect = NULL;
  mNumEffects = 0;
  mIntArray = NULL;
  registerEffects();

  readConfig();
  initNextScreen();

  mFileIdx = 0;
  mColorContext = QColor::enterAllocContext();

  mEffectRunning = false;
  loadNextImage();
  createNextScreen();

  mTimer.start(10, true);
  connect(&mTimer, SIGNAL(timeout()), SLOT(slotTimeout()));
}


//----------------------------------------------------------------------------
kSlideShowSaver::~kSlideShowSaver()
{
  mTimer.stop();
  QColor::leaveAllocContext();
  QColor::destroyAllocContext(mColorContext);
}


//-----------------------------------------------------------------------------
void kSlideShowSaver::initNextScreen()
{
  QPaintDeviceMetrics metric(&mWidget);
  int w, h;

  w = mWidget.width();
  h = mWidget.height();
  mNextScreen = QPixmap(w, h, metric.depth());
}


//-----------------------------------------------------------------------------
void kSlideShowSaver::readConfig()
{
  KConfig *config = KGlobal::config();
  config->setGroup("Settings");
  mShowRandom = config->readBoolEntry("ShowRandom", true);
  mZoomImages = config->readBoolEntry("ZoomImages", false);
  mPrintName = config->readBoolEntry("PrintName", true);
  mDirectory = config->readEntry("Directory", "~/.kde/share/apps/kslideshow/pics");
  mDelay = config->readNumEntry("Delay", 10) * 1000;

  loadDirectory();
  // loadFileList("slideshow.list");
}


//----------------------------------------------------------------------------
void kSlideShowSaver::registerEffects()
{
  int i = 0;

  mEffectList = new EffectMethod[64];
  mEffectList[i++] = &kSlideShowSaver::effectMeltdown;
  mEffectList[i++] = &kSlideShowSaver::effectSweep;
  mEffectList[i++] = &kSlideShowSaver::effectCircleOut;
  mEffectList[i++] = &kSlideShowSaver::effectBlobs;
  mEffectList[i++] = &kSlideShowSaver::effectIncomingEdges;
  mEffectList[i++] = &kSlideShowSaver::effectHorizLines;
  mEffectList[i++] = &kSlideShowSaver::effectVertLines;
  mEffectList[i++] = &kSlideShowSaver::effectRandom;
  mEffectList[i++] = &kSlideShowSaver::effectGrowing;

  mNumEffects = i;

//  mNumEffects = 1;  //...for testing
}


//----------------------------------------------------------------------------
int kSlideShowSaver::effectMeltdown(bool aInit)
{
  int i, x, y;
  bool done;

  if (aInit)
  {
    if (mIntArray) delete mIntArray;
    mw = mWidget.width();
    mh = mWidget.height();
    mdx = 4;
    mdy = 16;
    mix = mw / mdx;
    mIntArray = new int[mix];
    for (i=mix-1; i>=0; i--)
      mIntArray[i] = 0;
  }

  done = true;
  for (i=0,x=0; i<mix; i++,x+=mdx)
  {
    y = mIntArray[i];
    if (y >= mh) continue;
    done = false;
    if ((rand()&15) < 6) continue;
    bitBlt(&mWidget, x, y+mdy, &mWidget, x, y, mdx, mh-y-mdy, CopyROP, true);
    bitBlt(&mWidget, x, y, &mNextScreen, x, y, mdx, mdy, CopyROP, true);
    mIntArray[i] += mdy;
  }

  if (done)
  {
    delete mIntArray;
    mIntArray = NULL;
    return -1;
  }

  return 15;
}


//----------------------------------------------------------------------------
int kSlideShowSaver::effectCircleOut(bool aInit)
{
  if (aInit)
  {
    startPainter();
    mw = mWidget.width();
    mh = mWidget.height();
    mi = 0;
    miy = 0;
  }

  if (mi >= 5760)
  {
    mPainter.end();
    showNextScreen();
    return -1;
  }

  miy = mi - 1;
  mi += 128;

  mPainter.drawPie(0, 0, mw, mh, miy, mi);

  return 20;
}


//----------------------------------------------------------------------------
int kSlideShowSaver::effectSweep(bool aInit)
{
  int w, h, x, y, i;

  if (aInit)
  {
    // subtype: 0=sweep right to left, 1=sweep left to right
    //          2=sweep bottom to top, 3=sweep top to bottom
    mSubType = rand() % 4;
    mw  = mWidget.width();
    mh  = mWidget.height();
    mdx = (mSubType==1 ? 16 : -16);
    mdy = (mSubType==3 ? 16 : -16);
    mx  = (mSubType==1 ? 0 : mw);
    my  = (mSubType==3 ? 0 : mh);
  }

  if (mSubType==0 || mSubType==1)
  {
    // horizontal sweep
    if ((mSubType==0 && mx < -64) ||
	(mSubType==1 && mx > mw+64))
    {
       return -1;
    }
    for (w=2,i=4,x=mx; i>0; i--, w<<=1, x-=mdx)
    {
      bitBlt(&mWidget, x, 0, &mNextScreen, x, 0, w, mh, CopyROP, true);
    }
    mx += mdx;
  }
  else
  {
    // vertical sweep
    if ((mSubType==2 && my < -64) ||
	(mSubType==3 && my > mh+64))
    {
      return -1;
    }
    for (h=2,i=4,y=my; i>0; i--, h<<=1, y-=mdy)
    {
      bitBlt(&mWidget, 0, y, &mNextScreen, 0, y, mw, h, CopyROP, true);
    }
    my += mdy;
  }

  return 20;
}


//----------------------------------------------------------------------------
int kSlideShowSaver::effectBlobs(bool aInit)
{
  int r;

  if (aInit)
  {
    startPainter();
    mAlpha = M_PI * 2;
    mw = mWidget.width();
    mh = mWidget.height();
    mi = 150;
  }

  if (mi <= 0)
  {
    mPainter.end();
    showNextScreen();
    return -1;
  }

  mx = rand() % mw;
  my = rand() % mh;
  r = (rand() % 200) + 50;

  mPainter.drawEllipse(mx-r, my-r, r, r);
  mi--;

  return 10;
}


//----------------------------------------------------------------------------
int kSlideShowSaver::effectRandom(bool /*aInit*/)
{
  int x, y, i, w, h, fact, sz;

  fact = (rand() % 3) + 1;

  w = mWidget.width() >> fact;
  h = mWidget.height() >> fact;
  sz = 1 << fact;

  for (i = (w*h)<<1; i > 0; i--)
  {
    x = (rand() % w) << fact;
    y = (rand() % h) << fact;
    bitBlt(&mWidget, x, y, &mNextScreen, x, y, sz, sz, CopyROP, true);
  }
  showNextScreen();

  return -1;
}


//----------------------------------------------------------------------------
int kSlideShowSaver::effectGrowing(bool aInit)
{
  if (aInit)
  {
    mw = mWidget.width();
    mh = mWidget.height();
    mx = mw >> 1;
    my = mh >> 1;
    mfx = mx / 100.0;
    mfy = my / 100.0;
  }

  mx = (int)(mx - mfx);
  my = (int)(my - mfy);

  if (mx<0 || my<0)
  {
    showNextScreen();
    return -1;
  }

  bitBlt(&mWidget, mx, my, &mNextScreen, mx, my,
	 mw - (mx<<1), mh - (my<<1), CopyROP, true);

  return 20;
}


//----------------------------------------------------------------------------
int kSlideShowSaver::effectIncomingEdges(bool aInit)
{
  int x1, y1;

  if (aInit)
  {
    mw = mWidget.width();
    mh = mWidget.height();
    mix = mw >> 1;
    miy = mh >> 1;
    mfx = mix / 100.0;
    mfy = miy / 100.0;
    mx = 0;
    my = 0;
    mSubType = rand() & 1;
  }

  mx = (int)(mx + mfx);
  my = (int)(my + mfy);

  if (mx>mix || my>miy)
  {
    showNextScreen();
    return -1;
  }

  x1 = mw - mx;
  y1 = mh - my;

  if (mSubType)
  {
    // moving image edges
    bitBlt(&mWidget,  0,  0, &mNextScreen, mix-mx, miy-my, mx, my, CopyROP, true);
    bitBlt(&mWidget, x1,  0, &mNextScreen, mix, miy-my, mx, my, CopyROP, true);
    bitBlt(&mWidget,  0, y1, &mNextScreen, mix-mx, miy, mx, my, CopyROP, true);
    bitBlt(&mWidget, x1, y1, &mNextScreen, mix, miy, mx, my, CopyROP, true);
  }
  else
  {
    // fixed image edges
    bitBlt(&mWidget,  0,  0, &mNextScreen,  0,  0, mx, my, CopyROP, true);
    bitBlt(&mWidget, x1,  0, &mNextScreen, x1,  0, mx, my, CopyROP, true);
    bitBlt(&mWidget,  0, y1, &mNextScreen,  0, y1, mx, my, CopyROP, true);
    bitBlt(&mWidget, x1, y1, &mNextScreen, x1, y1, mx, my, CopyROP, true);
  }
  return 20;
}


//----------------------------------------------------------------------------
int kSlideShowSaver::effectHorizLines(bool aInit)
{
  static int iyPos[] = { 0, 4, 2, 6, 1, 5, 3, 7, -1 };
  int y;

  if (aInit)
  {
    mw = mWidget.width();
    mh = mWidget.height();
    mi = 0;
  }

  if (iyPos[mi] < 0) return -1;

  for (y=iyPos[mi]; y<mh; y+=8)
  {
    bitBlt(&mWidget, 0, y, &mNextScreen, 0, y, mw, 1, CopyROP, true);
  }

  mi++;
  if (iyPos[mi] >= 0) return 160;
  return -1;
}


//----------------------------------------------------------------------------
int kSlideShowSaver::effectVertLines(bool aInit)
{
  static int ixPos[] = { 0, 4, 2, 6, 1, 5, 3, 7, -1 };
  int x;

  if (aInit)
  {
    mw = mWidget.width();
    mh = mWidget.height();
    mi = 0;
  }

  if (ixPos[mi] < 0) return -1;

  for (x=ixPos[mi]; x<mw; x+=8)
  {
    bitBlt(&mWidget, x, 0, &mNextScreen, x, 0, 1, mh, CopyROP, true);
  }

  mi++;
  if (ixPos[mi] >= 0) return 160;
  return -1;
}


//-----------------------------------------------------------------------------
void kSlideShowSaver::startPainter(Qt::PenStyle aPen)
{
  QBrush brush;
  brush.setPixmap(mNextScreen);
  if (mPainter.isActive()) mPainter.end();
  mPainter.begin(&mWidget);
  mPainter.setBrush(brush);
  mPainter.setPen(aPen);
}


//-----------------------------------------------------------------------------
void kSlideShowSaver::restart()
{
  mEffectRunning = false;
  mEffect = NULL;
  blank();
  slotTimeout();
}


//-----------------------------------------------------------------------------
void kSlideShowSaver::slotTimeout()
{
  int tmout = -1;
  int i;

  if (mEffectRunning)
  {
    tmout = (this->*mEffect)(false);
  }
  else
  {
    loadNextImage();
    createNextScreen();

    if (mNumEffects > 1) i = rand() % mNumEffects;
    else i = 0;

    mEffect = mEffectList[i];
    mEffectRunning = true;
    tmout = (this->*mEffect)(true);
  }
  if (tmout <= 0)
  {
    tmout = mDelay;
    mEffectRunning = false;
  }
  mTimer.start(tmout, true);
}


//----------------------------------------------------------------------------
void kSlideShowSaver::showNextScreen()
{
  bitBlt(&mWidget, 0, 0, &mNextScreen, 0, 0,
	 mNextScreen.width(), mNextScreen.height(), CopyROP, true);
}


//----------------------------------------------------------------------------
void kSlideShowSaver::createNextScreen()
{
  QPainter p;
  int ww, wh, iw, ih, x, y;
  double fx, fy;

  if (mNextScreen.size() != mWidget.size())
    mNextScreen.resize(mWidget.size());

  mNextScreen.fill(black);

  ww = mNextScreen.width();
  wh = mNextScreen.height();
  iw = mImage.width();
  ih = mImage.height();

  p.begin(&mNextScreen);

  if (mFileList.isEmpty())
  {
    p.setPen(QColor("white"));
    p.drawText(20 + (rand() % (ww>>1)), 20 + (rand() % (wh>>1)),
	       glocale->translate("No images found"));
  }
  else
  {
    if (mZoomImages)
    {
      fx = (double)ww / iw;
      fy = (double)wh / ih;
      if (fx > fy) fx = fy;
      if (fx > 2) fx = 2;
      iw = (int)(iw * fx);
      ih = (int)(ih * fx);
      QImage scaledImg = mImage.smoothScale(iw, ih);

      x = (ww - iw) >> 1;
      y = (wh - ih) >> 1;

      p.drawImage(x, y, scaledImg);
    }
    else
    {
      x = (ww - iw) >> 1;
      y = (wh - ih) >> 1;

      // bitBlt(&mNextScreen, x, y, &mImage, 0, 0, iw, ih, CopyROP, false);
      p.drawImage(x, y, mImage);
    }

    if (mPrintName)
    {
      p.setPen(QColor("black"));
      for (x=9; x<=11; x++)
	for (y=21; y>=19; y--)
	  p.drawText(x, wh-y, mImageName);
      p.setPen(QColor("white"));
      p.drawText(10, wh-20, mImageName);
    }
  }

  p.end();
}


//----------------------------------------------------------------------------
void kSlideShowSaver::loadNextImage()
{
  QString fname, fpath;
  int num, i, j;

  if (mShowRandom)
  {
    num = mRandomList.count();
    if (num <= 0)
    {
      mRandomList = mFileList;
      num = mRandomList.count();
    }
    if (num <= 0) return;
    mFileIdx = rand() % num;
    fname = mRandomList[mFileIdx];
    mRandomList.remove(fname);
  }
  else
  {
    num = mFileList.count();
    if (mFileIdx >= num) mFileIdx = 0;
    fname = mFileList[mFileIdx];
  }

  if (!mDirectory.isEmpty()) fpath = mDirectory + '/' + fname;
  else fpath = fname;

  if (!mImage.load(fpath))
  {
    printf("Failed to load image '%s'\n", (const char*)fpath);
    mFileList.remove(fname);
    mRandomList.remove(fname);
    if (!mFileList.isEmpty()) loadNextImage();
    return;
  }
  mFileIdx++;

  i = fname.findRev('.');
  if (i < 0) i = 32767;
  j = fname.findRev('/') + 1;
  if (j < 0) j = 0;
  mImageName = fname.mid(j, i-j);
}


//----------------------------------------------------------------------------
void kSlideShowSaver::loadDirectory()
{
  QDir dir(mDirectory);

  mFileIdx = 0;
  mFileList.clear();
  mFileList = dir.entryList();
  mFileList.remove(".");
  mFileList.remove("..");
}


//----------------------------------------------------------------------------
void kSlideShowSaver::loadFileList(const QCString& aFileName)
{
  QFile file(aFileName);
  QString fname;

  mFileList.clear();
  mFileIdx = 0;

  if (!file.open(IO_ReadOnly)) return;

  while (!file.atEnd())
  {
    if (file.readLine(fname, 256) > 0)
    {
      fname = fname.stripWhiteSpace();
      if (fname.isEmpty() || fname[0] == '#') continue;
      mFileList.append(fname);
    }
  }
  file.close();
}


//-----------------------------------------------------------------------------
void kSlideShowSaver::blank()
{
   mWidget.erase();
}


//=============================================================================
//  Class kSlideShowSetup
//=============================================================================
kSlideShowSetup::kSlideShowSetup(QWidget *aParent, const char *aName):
  QDialog(aParent, aName, TRUE )
{
  setCaption(glocale->translate("Setup Slide Show"));

  QLabel *label;
  QPushButton *button;

  mSaver = NULL;

  QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);
  QHBoxLayout *tl1 = new QHBoxLayout;
  tl->addLayout(tl1);
  QVBoxLayout *tl11 = new QVBoxLayout(5);
  tl1->addLayout(tl11);

  mCboDir = new QComboBox(false, this);
  mCboDir->insertItem(glocale->translate("Select..."));
  mCboDir->setMaxCount(21);
  mCboDir->setSizeLimit(20);
  minSize(mCboDir);
  mCboDir->setMaximumWidth(32767);
  connect(mCboDir, SIGNAL(activated(int)), SLOT(slotDirSelected(int)));
  tl11->addWidget(mCboDir);
  tl11->addSpacing(5);

  mCbxZoom = new QCheckBox(glocale->translate("Zoom pictures"), this);
  connect(mCbxZoom, SIGNAL(clicked()), SLOT(writeSettings()));
  minSize(mCbxZoom);
  tl11->addWidget(mCbxZoom);

  mCbxRandom = new QCheckBox(glocale->translate("Random play"), this);
  connect(mCbxRandom, SIGNAL(clicked()), SLOT(writeSettings()));
  minSize(mCbxRandom);
  tl11->addWidget(mCbxRandom);

  mCbxShowName = new QCheckBox(glocale->translate("Show names"), this);
  connect(mCbxShowName, SIGNAL(clicked()), SLOT(writeSettings()));
  minSize(mCbxShowName);
  tl11->addWidget(mCbxShowName);
  tl11->addSpacing(5);

  label = new QLabel(glocale->translate("Delay:"), this);
  minSize(label);
  tl11->addWidget(label);

  mDelay = new QSlider(1, 60, 10, 1, QSlider::Horizontal, this);
  mDelay->setMinimumSize(90, 20);
  mDelay->setTracking(false);
  mDelay->setTickmarks(QSlider::Right);
  mDelay->setTickInterval(10);
  connect(mDelay, SIGNAL(valueChanged(int)), SLOT(slotDelay(int)));
  tl11->addWidget(mDelay);
  tl11->addSpacing(5);

  tl11->addStretch(1000);

  mPreview = new QWidget(this);
  mPreview->setFixedSize(220, 170);
  mPreview->setBackgroundColor(black);
  mPreview->show();    // otherwise saver does not get correct size
  mSaver = new kSlideShowSaver(mPreview->winId());
  tl1->addWidget(mPreview);

  KButtonBox *bbox = new KButtonBox(this);	
  button = bbox->addButton(glocale->translate("About"));
  connect(button, SIGNAL(clicked()), SLOT(slotAbout()));
  bbox->addStretch(1);

  button = bbox->addButton(glocale->translate("OK"));	
  connect(button, SIGNAL(clicked()), SLOT(slotOkPressed()));

  button = bbox->addButton(glocale->translate("Cancel"));
  connect(button, SIGNAL(clicked()), SLOT(reject()));
  bbox->layout();
  tl->addWidget(bbox);

  tl->freeze();

  readSettings();
}


//-----------------------------------------------------------------------------
void kSlideShowSetup::readSettings()
{
  int i, num, cur;
  QString key, value, curDir;
  KConfig *config = KGlobal::config();

  config->setGroup("Settings");
  mCbxRandom->setChecked(config->readBoolEntry("ShowRandom", true));
  mCbxZoom->setChecked(config->readBoolEntry("ZoomImages", false));
  mCbxShowName->setChecked(config->readBoolEntry("PrintName", true));
  mDelay->setValue(config->readNumEntry("Delay", 5));
  curDir = config->readEntry("Directory");

  config->setGroup("Slide Show");
  num = config->readNumEntry("Count", 0);
  for (i=0, cur=-1; i<num; i++)
  {
    key.sprintf("Dir%d", i);
    value = config->readEntry(key);
    if (!value.isEmpty())
    {
      if (cur < 0 && value == curDir) cur = i;
      if (!value.isEmpty()) mCboDir->insertItem(value);
    }
  }
  if (cur < 0 && !curDir.isEmpty())
  {
    mCboDir->insertItem(curDir);
    mCboDir->setCurrentItem(mCboDir->count());
  }
  else mCboDir->setCurrentItem(cur+1);
}


//-----------------------------------------------------------------------------
void kSlideShowSetup::writeSettings()
{
  int i, num;
  QString key, value;
  KConfig *config = KGlobal::config();

  config->setGroup("Settings");
  config->writeEntry("ShowRandom", mCbxRandom->isChecked());
  config->writeEntry("ZoomImages", mCbxZoom->isChecked());
  config->writeEntry("PrintName",  mCbxShowName->isChecked());
  config->writeEntry("Delay", mDelay->value());

  if (mCboDir->currentItem() >= 1)
    config->writeEntry("Directory", mCboDir->currentText());

  num = mCboDir->count() - 1;
  if (num > 20) num = 20;
  config->writeEntry("Count", num);

  config->setGroup("Slide Show");
  for (i=0; i<num; i++)
  {
    key.sprintf("Dir%d", i);
    value = mCboDir->text(i+1);
    config->writeEntry(key, value);
  }

  config->sync();

  if (mSaver)
  {
    mSaver->readConfig();
    mSaver->restart();
  }
}


//-----------------------------------------------------------------------------
void kSlideShowSetup::slotDelay(int)
{
  writeSettings();
}


//-----------------------------------------------------------------------------
void kSlideShowSetup::slotDirSelected(int aIdx)
{
  if (aIdx <= 0)
  {
    QString dirName;
    dirName = KFileDialog::getExistingDirectory(QDir::homeDirPath(), this,
		   glocale->translate("Choose Images Directory") );
    if (dirName.isEmpty()) return;
    mCboDir->insertItem(dirName, 1);
    mCboDir->setCurrentItem(1);
  }
  writeSettings();
}


//-----------------------------------------------------------------------------
void kSlideShowSetup::slotOkPressed()
{
  writeSettings();
  accept();
}


//-----------------------------------------------------------------------------
void kSlideShowSetup::slotAbout()
{
  KMessageBox::about(this,
     glocale->translate("SlideShow Version 1.1\n\n"
			"Copyright (c) 1999 by\n"
			"Stefan Taferner <taferner@kde.org>\n"));
}


//-----------------------------------------------------------------------------
void kSlideShowSetup::minSize(QWidget* aWidget)
{
  aWidget->setFixedSize(aWidget->sizeHint());
}


//=============================================================================
//  Kde Screen Saver Interface Functions
//=============================================================================
void startScreenSaver(Drawable d)
{
  if (sSaver) return;
  sSaver = new kSlideShowSaver(d);
}


//-----------------------------------------------------------------------------
void stopScreenSaver()
{
  if (sSaver)
  {
    delete sSaver;
    sSaver = NULL;
  }
}


//-----------------------------------------------------------------------------
int setupScreenSaver()
{
  kSlideShowSetup dlg;
  return dlg.exec();
}


#include "slideshow.moc"
