/*
 * KMatrix - a screensaver dedicated to the movie "The Matrix"
 *
 * Based on xmatrix, xscreensaver by Jamie Zawinski <jwz@jwz.org> Copyright (c) 1999
 * ported to KDE 1.1.2 by Dmitry DELTA Malykhanov <d.malykhanov@iname.com> in August 1999
 * ported to KDE 2.0 and setup dialog fixup by Thorsten Westheider
 * <thorsten.westheider@teleos-web.de> in March 2000
 *
 * Additional contributors:
 *
 * kblankscrn - Basic screen saver for KDE Copyright (c)  Martin R. Jones 1996
 * layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>
 *
 */

#include <stdio.h>
#include <stdlib.h>

#undef Below // Namespace collision

#include <qbuttongroup.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qslider.h>

#include <kapp.h>
#include <kdebug.h>
#include <kbuttonbox.h>
#include <kcolordlg.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <krandomsequence.h>
#include <kmessagebox.h>

#include "helpers.h"

#include <X11/xpm.h>

#ifdef DEBUG_MEM
#include <mcheck.h>
#endif

#include "matrix.h"

static KRandomSequence *rnd = 0;

static KMatrixSaver *saver = NULL;
const char progname[] = "KMatrix";
static const QString inserts[] = { "top", "bottom", "both" };


void KMatrixSaverCfg::readSettings() {
  QString str;

  KConfig *config = klock_config();
  config->setGroup("Settings");

  background = config->readColorEntry("BackgroundColor", &Qt::black);
  foreground = config->readColorEntry("ForegroundColor", &Qt::green);
  //str = config->readEntry("Mono");
  mono = False;
  density = config->readNumEntry("Density", 25);
  speed = config->readNumEntry("Speed", 100);
  insert = config->readEntry("Insert", "bottom");

  delete config;
}

void KMatrixSaverCfg::writeSettings() {
  KConfig *config = klock_config();
  config->setGroup( "Settings" );

  config->writeEntry("BackgroundColor", background);
  config->writeEntry("ForegroundColor", foreground);
  //config->writeEntry("Mono", str);
  config->writeEntry("Density", density);
  config->writeEntry("Speed", speed);
  config->writeEntry("Insert", insert);
  config->sync();
  delete config;
}

//--------------------------------------------------------------------
// standard screen saver interface functions
//
void startScreenSaver( Drawable d ) {
  if ( saver )
    return;
#ifdef DEBUG_MEM
  mtrace();
#endif
  saver = new KMatrixSaver( d );
}

void stopScreenSaver() {
  if ( saver )
    delete saver;
  saver = NULL;
}

int setupScreenSaver() {
  KMatrixSetup dlg;

  return dlg.exec();
}

QString getScreenSaverName() {
  return i18n("Kmatrix \"The Matrix\" movie style screen saver");
}

//--------------------------------------------------------------------
// KMatrixSaver code

KMatrixSaver::KMatrixSaver( Drawable drawable ) : kScreenSaver( drawable ) {
  rnd = new KRandomSequence();
  readSettings();

  blank();
  init_matrix();
  timer.start(cfg.speed);
  connect(&timer, SIGNAL(timeout()), SLOT(slotTimeout()));
}

void KMatrixSaver::slotTimeout() {
  draw_matrix();
  XSync(qt_xdisplay(), False);
}

KMatrixSaver::~KMatrixSaver() {
  //fprintf(stderr, "System shutdown... :)\n");
  timer.stop();
  shutdown_matrix();
  delete rnd; rnd = 0;
}

//--------------------------------------------------------------------
// configuration API
void KMatrixSaver::setBackgroundColor( const QColor &col ) {
  cfg.background = col;
  restart_matrix();
}

void KMatrixSaver::setForegroundColor(const QColor &col ) {
  cfg.foreground = col;
  restart_matrix();
}

void KMatrixSaver::setDensity(int den) {
  if( den>0 && den <=100 ) {
    cfg.density = den;
    restart_matrix();
  } else {
    fprintf(stderr, "%s: unsupported density %d\n", progname, den);
  }
}

void KMatrixSaver::setSpeed(int sp) {
  cfg.speed = sp;
  timer.stop();
  timer.start(cfg.speed);
}

void KMatrixSaver::setInsert(QString ins) {
  cfg.insert = ins;
  restart_matrix();
}

// read configuration settings from file
inline void KMatrixSaver::readSettings() {
  cfg.readSettings();
}

void KMatrixSaver::blank()
{
  XSetWindowBackground( qt_xdisplay(), mDrawable, cfg.background.pixel() );
  XClearWindow( qt_xdisplay(), mDrawable );
}

//---------------------------------------------------------------------
/* restart matrix *
   this function is not a part of original xtarix code */
inline void KMatrixSaver::restart_matrix() {
  shutdown_matrix();
  init_matrix();
}

/* shutdown matrix *
   this function is not a part of original xmatrix code */
void KMatrixSaver::shutdown_matrix() {
  if( state!=NULL ) {
    XFreeGC(state->dpy, state->draw_gc);
    XFreeGC(state->dpy, state->erase_gc);
    if( state->cells!=NULL ) {
      free(state->cells); state->cells = NULL;
    }
    if( state->feeders!=NULL ) {
      free(state->feeders); state->feeders = NULL;
    }
    free(state); state = NULL;
  }
}

/* xscreensaver, Copyright (c) 1999 Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * Matrix -- simulate the text scrolls from the movie "The Matrix".
 *
 * The movie people distribute their own Windows/Mac screensaver that does
 * a similar thing, so I wrote one for Unix.  However, that version (the
 * Windows/Mac version at http://www.whatisthematrix.com/) doesn't match my
 * memory of what the screens in the movie looked like, so my `xmatrix'
 * does things differently.
 */

#include "pixmaps/matrix_font.xpm"

//#define CHAR_HEIGHT 31
/* I'm using smaller font. Delta */
#define CHAR_HEIGHT 16

/* load images */
void KMatrixSaver::load_images () {
  if (!cfg.mono && state->xgwa.depth > 1)
    {
      XpmAttributes xpmattrs;
      int result;
      xpmattrs.valuemask = 0;

# ifdef XpmCloseness
      xpmattrs.valuemask |= XpmCloseness;
      xpmattrs.closeness = 40000;
# endif
# ifdef XpmVisual
      xpmattrs.valuemask |= XpmVisual;
      xpmattrs.visual = state->xgwa.visual;
# endif
# ifdef XpmDepth
      xpmattrs.valuemask |= XpmDepth;
      xpmattrs.depth = state->xgwa.depth;
# endif
# ifdef XpmColormap
      xpmattrs.valuemask |= XpmColormap;
      xpmattrs.colormap = state->xgwa.colormap;
# endif

      result = XpmCreatePixmapFromData (state->dpy, state->window, (char **)matrix_font,
                                        &state->images, 0 /* mask */,
                                        &xpmattrs);
      if (!state->images || (result != XpmSuccess && result != XpmColorError))
        state->images = 0;

      state->image_width = xpmattrs.width;
      state->image_height = xpmattrs.height;
      state->nglyphs = state->image_height / CHAR_HEIGHT;
      //fprintf(stderr, "nglyphs = %d\n", state->nglyphs);
    }
}

/* initialize matrix */
void KMatrixSaver::init_matrix () {
  XGCValues gcv;
  state = (m_state *) calloc (sizeof(*state), 1);
  state->dpy = qt_xdisplay();
  state->window = mDrawable;

  XGetWindowAttributes (state->dpy, state->window, &state->xgwa);
  load_images();

  gcv.foreground = cfg.foreground.pixel();
  gcv.background = cfg.background.pixel();
  state->draw_gc = XCreateGC (state->dpy, state->window,
                              GCForeground|GCBackground, &gcv);
  gcv.foreground = gcv.background;
  state->erase_gc = XCreateGC (state->dpy, state->window,
                               GCForeground|GCBackground, &gcv);

  state->char_width = state->image_width / 2;
  state->char_height = CHAR_HEIGHT;

  state->grid_width  = state->xgwa.width  / state->char_width;
  state->grid_height = state->xgwa.height / state->char_height;
  state->grid_width++;
  state->grid_height++;

  state->cells = (m_cell *)
    calloc (sizeof(m_cell), state->grid_width * state->grid_height);
  state->feeders = (m_feeder *) calloc (sizeof(m_feeder), state->grid_width);

  state->density = cfg.density;

  QString insert = cfg.insert;
  if (insert == "top")
    {
      state->insert_top_p = True;
      state->insert_bottom_p = False;
    }
  else if (insert == "bottom")
    {
      state->insert_top_p = False;
      state->insert_bottom_p = True;
    }
  else if (insert == "both")
    {
      state->insert_top_p = True;
      state->insert_bottom_p = True;
    }
  else
    {
      if (!insert.isEmpty())
        kdError() << "`insert' must be `top', `bottom', or `both', not " << insert << endl;
      state->insert_top_p = False;
      state->insert_bottom_p = True;
    }

  /* return state; */
}

void KMatrixSaver::insert_glyph (int glyph, int x, int y) {
  Bool bottom_feeder_p = (y >= 0);
  m_cell *from, *to;

  if (y >= state->grid_height)
    return;

  if (bottom_feeder_p)
    {
      to = &state->cells[state->grid_width * y + x];
    }
  else
    {
      for (y = state->grid_height-1; y > 0; y--)
        {
          from = &state->cells[state->grid_width * (y-1) + x];
          to   = &state->cells[state->grid_width * y     + x];
          *to = *from;
          to->changed = True;
        }
      to = &state->cells[x];
    }

  to->glyph = glyph;
  to->changed = True;

  if (!to->glyph)
    ;
  else if (bottom_feeder_p)
    to->glow = 1 + rnd->getLong(2);
  else
    to->glow = 0;
}

void KMatrixSaver::feed_matrix () {
  int x;

  /* Update according to current feeders. */
  for (x = 0; x < state->grid_width; x++)
    {
      m_feeder *f = &state->feeders[x];

      if (f->throttle)		/* this is a delay tick, synced to frame. */
        {
          f->throttle--;
        }
      else if (f->remaining > 0)	/* how many items are in the pipe */
        {
          int g = rnd->getLong(state->nglyphs) + 1;
          insert_glyph (g, x, f->y);
          f->remaining--;
          if (f->y >= 0)  /* bottom_feeder_p */
            f->y++;
        }
      else				/* if pipe is empty, insert spaces */
        {
          insert_glyph (0, x, f->y);
          if (f->y >= 0)  /* bottom_feeder_p */
            f->y++;
        }

      if (rnd->getLong(10) == 0)		/* randomly change throttle speed */
        {
          f->throttle = rnd->getLong(5) + rnd->getLong(5);
        }
    }
}

int KMatrixSaver::densitizer () {
  /* Horrid kludge that converts percentages (density of screen coverage)
     to the parameter that actually controls this.  I got this mapping
     empirically, on a 1024x768 screen.  Sue me. */
  if      (state->density < 10) return 85;
  else if (state->density < 15) return 60;
  else if (state->density < 20) return 45;
  else if (state->density < 25) return 25;
  else if (state->density < 30) return 20;
  else if (state->density < 35) return 15;
  else if (state->density < 45) return 10;
  else if (state->density < 50) return 8;
  else if (state->density < 55) return 7;
  else if (state->density < 65) return 5;
  else if (state->density < 80) return 3;
  else if (state->density < 90) return 2;
  else return 1;
}

void KMatrixSaver::hack_matrix () {
  int x;

  /* Glow some characters. */
  if (!state->insert_bottom_p)
    {
      int i = rnd->getLong(state->grid_width / 2);
      while (--i > 0)
        {
          int x = rnd->getLong(state->grid_width);
          int y = rnd->getLong(state->grid_height);
          m_cell *cell = &state->cells[state->grid_width * y + x];
          if (cell->glyph && cell->glow == 0)
            {
              cell->glow = rnd->getLong(10);
              cell->changed = True;
            }
        }
    }

  /* Change some of the feeders. */
  for (x = 0; x < state->grid_width; x++)
    {
      m_feeder *f = &state->feeders[x];
      Bool bottom_feeder_p;

      if (f->remaining > 0)	/* never change if pipe isn't empty */
        continue;

      if (rnd->getLong(densitizer()) != 0) /* then change N% of the time */
        continue;

      f->remaining = 3 + rnd->getLong(state->grid_height);
      f->throttle = rnd->getLong(5) + rnd->getLong(5);

      if (rnd->getLong(4) != 0)
        f->remaining = 0;

      if (state->insert_top_p && state->insert_bottom_p)
        bottom_feeder_p = rnd->getLong(2);
      else
        bottom_feeder_p = state->insert_bottom_p;

      if (bottom_feeder_p)
        f->y = rnd->getLong(state->grid_height / 2);
      else
        f->y = -1;
    }
}

void KMatrixSaver::draw_matrix () {
  int x, y;
  int count = 0;

  feed_matrix ();
  hack_matrix ();

  for (y = 0; y < state->grid_height; y++)
    for (x = 0; x < state->grid_width; x++)
      {
        m_cell *cell = &state->cells[state->grid_width * y + x];

        if (cell->glyph)
          count++;

        if (!cell->changed)
          continue;

        if (cell->glyph == 0)
          XFillRectangle (state->dpy, state->window, state->erase_gc,
                          x * state->char_width,
                          y * state->char_height,
                          state->char_width,
                          state->char_height);
        else
          XCopyArea (state->dpy, state->images, state->window, state->draw_gc,
                     (cell->glow ? state->char_width : 0),
                     (cell->glyph - 1) * state->char_height,
                     state->char_width, state->char_height,
                     x * state->char_width,
                     y * state->char_height);

        cell->changed = False;

        if (cell->glow > 0)
          {
            cell->glow--;
            cell->changed = True;
          }
      }

}


//--------------------------------------------------------------------
// dialog to setup screen saver parameters
//
KMatrixSetup::KMatrixSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{
  QString tmps;
  cfg.readSettings();

  QLabel *label;
  QPushButton *button;

  setCaption( i18n("Setup KMatrix Screensaver") );

  QVBoxLayout *tl = new QVBoxLayout(this, 10);
  QHBoxLayout *tl1 = new QHBoxLayout;
  tl->addLayout(tl1);

  QVBoxLayout *tl11 = new QVBoxLayout(5);
  tl1->addLayout(tl11);

  // desnity controls (label, edit control, slider
  QHBoxLayout *dhb = new QHBoxLayout;
  tl11->addLayout(dhb);
  label = new QLabel( i18n("Density:"), this);
  min_size(label);
  dhb->addWidget(label);
  /*
  densityEd = new QLineEdit(this);
  fixed_size(densityEd);
  tmps.setNum(cfg.density);
  densityEd->setText(tmps);
  connect(densityEd, SIGNAL(textChanged(const QString &)),
	  SLOT(slotDensityEdit(const QString &)));
  dhb->addWidget(densityEd);
  */
#undef Below
  densitySld = new QSlider(0, 100, 1, cfg.density, QSlider::Horizontal,
			   this, "Density");
  densitySld->setTickmarks(QSlider::Below);
  densitySld->setTickInterval(10);
  densitySld->setFixedHeight(densitySld->sizeHint().height());
  connect( densitySld, SIGNAL(valueChanged(int)), SLOT(slotDensity(int)));
  tl11->addWidget(densitySld);

  // speed controls: label, edito control, slider
#define SPEED2SL(_s) (100-((_s)-10)/2)
#define SPEED2CFG(_s) (10 + (100-(_s))*2)
  QHBoxLayout *shb = new QHBoxLayout;
  tl11->addLayout(shb);
  label = new QLabel( i18n("Speed:"), this);
  min_size(label);
  shb->addWidget(label);
  /*
  speedEd = new QLineEdit(this);
  fixed_size(speedEd);
  tmps.setNum(SPEED2SL(cfg.speed));
  speedEd->setText(tmps);
  connect(speedEd, SIGNAL(textChanged(const QString &)),
	  SLOT(slotSpeedEdit(const QString &)));
  shb->addWidget(speedEd);
  */
  speedSld = new QSlider(0, 100, 1, SPEED2SL(cfg.speed), QSlider::Horizontal,
			 this, "Speed");
  speedSld->setTickmarks(QSlider::Below);
  speedSld->setTickInterval(10);
  speedSld->setFixedHeight(speedSld->sizeHint().height());
  connect( speedSld, SIGNAL(valueChanged(int)), SLOT(slotSpeed(int)));
  tl11->addWidget(speedSld);
  tl11->addSpacing(10);
  // insert controls: radio group
  QRadioButton *rb;
  int i = 0;
  insertRbGrp = new QButtonGroup(2, Qt::Horizontal, this, "insertGroup");
  insertRbGrp->setTitle(i18n("Insert from:"));
  QVBoxLayout *grpvbox = new QVBoxLayout(insertRbGrp, 10);
  QFontMetrics  metrics(insertRbGrp->font());
  grpvbox->addSpacing(metrics.height()/2+4);
  rb = new QRadioButton( insertRbGrp);
  rb->setText( i18n("Top"));
  min_size(rb);
  if( cfg.insert==inserts[i++] ) rb->setChecked(TRUE);
    else rb->setChecked(FALSE);
  grpvbox->addWidget(rb);
  rb = new QRadioButton( insertRbGrp);
  rb->setText( i18n("Bottom"));
  min_size(rb);
  if( cfg.insert == inserts[i++] ) rb->setChecked(TRUE);
    else rb->setChecked(FALSE);
  grpvbox->addWidget(rb);
  rb = new QRadioButton( insertRbGrp);
  rb->setText( i18n("Both"));
  min_size(rb);
  if( cfg.insert == inserts[i++] ) rb->setChecked(TRUE);
    else rb->setChecked(FALSE);
  grpvbox->addWidget(rb);
  connect(insertRbGrp, SIGNAL(clicked(int)), SLOT(slotInsert(int)));
  tl11->addWidget(insertRbGrp);
  tl11->addStretch(1);

  // preview panel
  QFrame *frame = new QFrame( this );
  frame->setFixedSize( 224, 174 );
  frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  frame->setLineWidth(2);
  preview = new QWidget( frame );
  preview->setGeometry( 2, 2, 220, 170 );
  preview->setBackgroundColor( black );
  preview->show();    // otherwise saver does not get correct size
  saver = new KMatrixSaver( preview->winId() );
  tl1->addWidget(frame);

  // -----------
  KButtonBox *bbox = new KButtonBox(this);
  button = bbox->addButton(i18n("About"));
  connect(button, SIGNAL(clicked()), SLOT(slotAbout()));
  bbox->addStretch(1);

  button = bbox->addButton( i18n("OK"));
  connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

  button = bbox->addButton(i18n("Cancel"));
  connect( button, SIGNAL( clicked() ), SLOT( reject() ) );
  bbox->layout();
  tl->addWidget(bbox);

  tl->freeze();
}

KMatrixSetup::~KMatrixSetup() {
  if( saver )
    delete saver;
}

// read settings from config file
inline void KMatrixSetup::readSettings() {
  cfg.readSettings();
}

void KMatrixSetup::slotBackgrColor() {
  if ( KColorDialog::getColor(cfg.background) == QDialog::Accepted ) {
    saver->setBackgroundColor(cfg.background);
    QColorGroup colgrp(black, cfg.background, cfg.background.light(),
		       cfg.background.dark(), cfg.background.dark(120),
		       black, white);
    backgrPush->setPalette( QPalette( colgrp, colgrp, colgrp ) );
  }
}

void KMatrixSetup::slotForegrColor() {
  if ( KColorDialog::getColor(cfg.foreground) == QDialog::Accepted ) {
    saver->setForegroundColor(cfg.foreground);
    QColorGroup colgrp(black, cfg.foreground, cfg.foreground.light(),
		       cfg.foreground.dark(), cfg.foreground.dark(120),
		       black, white);
    foregrPush->setPalette( QPalette( colgrp, colgrp, colgrp ) );
  }
}

void KMatrixSetup::slotDensity(int val) {
  setDensity(val);
  // update edit control
  // tmp.setNum(val);
  // densityEd->setText(tmp);
}

void KMatrixSetup::setDensity(int val) {
  cfg.density = val;
  //fprintf(stderr, "setDensity: val=%d, density=%d\n", val, cfg.density);
  saver->setDensity(cfg.density);
}

void KMatrixSetup::slotSpeed(int val) {
  setSpeed(val);
  // update edit control
  // tmp.setNum(val);
  // speedEd->setText(tmp);
}

void KMatrixSetup::setSpeed(int val) {
  cfg.speed = SPEED2CFG(val);
  //fprintf(stderr, "setSpeed: val=%d, speed=%d\n", val, cfg.speed);
  saver->setSpeed(cfg.speed);
}

void KMatrixSetup::slotInsert(int id) {
  cfg.insert = inserts[id];
  //fprintf(stderr, "radio button id %d (%s)\n", id, inserts[id]);
  saver->setInsert(cfg.insert);
}

void KMatrixSetup::slotDensityEdit(const QString &s) {
  int val = s.toInt();
  //fprintf(stderr, "slotDensityEdit: arg = %s\n", s);
  setDensity(val);
  densitySld->setValue(val);
}

void KMatrixSetup::slotSpeedEdit(const QString &s) {
  int val = s.toInt();
  //fprintf(stderr, "slotSpeedEdit: arg = %s\n", s);
  setSpeed(val);
  speedSld->setValue(val);
}

// Ok pressed - save settings and exit
void KMatrixSetup::slotOkPressed() {
  cfg.writeSettings();
  accept();
}

void KMatrixSetup::slotAbout() {
  KMessageBox::about(0,
		       i18n("KMatrix\n\n"
"based on xmatrix\n"
"Copyright (C) 1999 by Jamie Zawinski <jwz@jwz.org>\n"
"Ported to kscreensaver by Dmitry DELTA Malykhanov\n"
"<d.malykhanov@iname.com>\n"
"Ported to KDE 2.0 by Thorsten Westheider\n"
"<thorsten.westheider@teleos-web.de>"),
                       i18n("About KMatrix"));
}

#include "matrix.moc"

