/*
  Copyright (c) 2000,2001 Matthias Elter <elter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <qpainter.h>
#include <qwhatsthis.h>
#include <qregexp.h>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kcursor.h>
#include <kglobalsettings.h>

#include "global.h"
#include "aboutwidget.h"
#include "aboutwidget.moc"
#include "modules.h"
#include "moduletreeview.h"

static const char kcc_text[] = I18N_NOOP("KDE Control Center");

static const char title_text[] = I18N_NOOP("Configure your desktop environment.");

static const char intro_text[] = I18N_NOOP("Welcome to the \"KDE Control Center\", "
                                "a central place to configure your "
                                "desktop environment. "
                                "Select an item from the index on the left "
                                "to load a configuration module.");

static const char kcc_infotext[] = I18N_NOOP("KDE Info Center");

static const char title_infotext[] = I18N_NOOP("Get system and desktop environment information");

static const char intro_infotext[] = I18N_NOOP("Welcome to the \"KDE Info Center\", "
                                "a central place to find information about your "
                                "computer system.");

static const char use_text[] = I18N_NOOP("Click on the \"Help\" tab on the left to view help "
                        "for the active "
                        "control module. Use the \"Search\" tab if you are unsure "
                        "where to look for "
                        "a particular configuration option.");

static const char version_text[] = I18N_NOOP("KDE version:");
static const char user_text[] = I18N_NOOP("User:");
static const char host_text[] = I18N_NOOP("Hostname:");
static const char system_text[] = I18N_NOOP("System:");
static const char release_text[] = I18N_NOOP("Release:");
static const char machine_text[] = I18N_NOOP("Machine:");

struct AboutWidget::ModuleLink
{
    ConfigModule *module;
    QRect linkArea;
};

QPixmap *AboutWidget::_part1 = 0L;
QPixmap *AboutWidget::_part2 = 0L;
QPixmap *AboutWidget::_part3 = 0L;
KPixmap *AboutWidget::_part3Effect = 0L;
QPixmap *AboutWidget::_part4TopRight = 0L;

AboutWidget::AboutWidget(QWidget *parent , const char *name, QListViewItem* category, const QString &caption)
   : QWidget(parent, name),
      _moduleList(false),
      _category(category),
      _activeLink(0),
      _caption(caption)
{
    if (_category)
      _moduleList = true;

    _moduleLinks.setAutoDelete(true);

    setMinimumSize(400, 400);

    // load images
    if( !_part1 )
    {
      kdDebug(1208) << "AboutWidget: pixmaps were not initialized! Please call initPixmaps() before the constructor and freePixmaps() after deleting the last instance!" << endl;
      _part1 = new QPixmap;
      _part2 = new QPixmap;
      _part3 = new QPixmap;
      _part4TopRight = new QPixmap;
      _part3Effect = new KPixmap;
    }

    // sanity check
    if(_part1->isNull() || _part2->isNull() || _part3->isNull() || _part4TopRight->isNull()) {
        kdError() << "AboutWidget::AboutWidget: Image loading error!" << endl;
        setBackgroundColor(QColor(49,121,172));
    }
    else
        setBackgroundMode(NoBackground); // no flicker

    // set qwhatsthis help
    QWhatsThis::add(this, i18n(intro_text));
}

void AboutWidget::setCategory( QListViewItem* category, const QString &caption )
{
  _caption = caption;
  _category = category;
  _activeLink = 0;
  if ( _category )
    _moduleList = true;
  else
    _moduleList = true;

  // Update the pixmap to be shown:
  updatePixmap();
  repaint();
}

void AboutWidget::initPixmaps()
{
  _part1 = new QPixmap( locate( "data", "kcontrol/pics/part1.png" ) );
  _part2 = new QPixmap( locate( "data", "kcontrol/pics/part2.png" ) );
  _part3 = new QPixmap( locate( "data", "kcontrol/pics/part3.png" ) );
  _part4TopRight = new QPixmap( locate( "data", "kcontrol/pics/top-right.png" ) );

  _part3Effect = new KPixmap( _part3->size() );

  QPainter pb;
  pb.begin( _part3Effect );
  pb.fillRect( 0, 0, _part3->width(), _part3->height(),
               QBrush( QColor( 49, 121, 172 ) ) );
  pb.drawPixmap( 0, 0, *_part3 );
  pb.end();

  KPixmapEffect::fade( *_part3Effect, 0.75, white );
}

void AboutWidget::freePixmaps()
{
  delete _part1;
  delete _part2;
  delete _part3;
  delete _part3Effect;
  delete _part4TopRight;
  _part1 = 0L;
  _part2 = 0L;
  _part3 = 0L;
  _part3Effect = 0L;
  _part4TopRight = 0L;
}

void AboutWidget::paintEvent(QPaintEvent* e)
{
    QPainter p (this);

    if(_buffer.isNull())
        p.fillRect(0, 0, width(), height(), QBrush(QColor(49,121,172)));
    else
    {
        p.drawPixmap(QPoint(e->rect().x(), e->rect().y()), _buffer, e->rect());
        if (_activeLink)
        {
            QRect src = e->rect() & _activeLink->linkArea;
            QPoint dest = src.topLeft();
            src.moveBy(-_linkArea.left(), -_linkArea.top());
            p.drawPixmap(dest, _linkBuffer, src);
        }
    }
}

void AboutWidget::resizeEvent(QResizeEvent*)
{
  updatePixmap();
}

void AboutWidget::updatePixmap()
{
    if(_part1->isNull() || _part2->isNull() || _part3->isNull() || _part4TopRight->isNull())
        return;

    _buffer.resize(width(), height());

    QPainter p(&_buffer);

    // draw part1
    p.drawPixmap(width() - _part4TopRight->width(), 0, *_part4TopRight);
    p.drawPixmap(0, 0, *_part1);

    int xoffset = _part1->width();
    int yoffset = _part1->height();

    // draw part2 tiled
    int xpos = xoffset;
    if(width() > xpos)
        p.drawTiledPixmap(xpos, 0, width() - xpos - _part4TopRight->width(), _part2->height(), *_part2);

    QFont f1 = font();
    QFont f2 = f1;
    QFont f3 = QFont(KGlobalSettings::generalFont().family(), 28, QFont::Bold, true);

    QString title, intro, caption;
    if (KCGlobal::isInfoCenter())
    {
       title = i18n(title_infotext);
       intro = i18n(intro_infotext);
       caption = i18n(kcc_infotext);
    }
    else
    {
       title = i18n(title_text);
       intro = i18n(intro_text);
       caption = i18n(kcc_text);
    }

    //draw the caption text
    p.setFont(f3);
    p.setPen(QColor(139, 163, 198));
    p.drawText(xpos + 13, 60, caption);
    p.setPen(black);
    p.drawText(xpos + 10, 57, caption);
    p.setFont(f1);

    const int hAlign = QApplication::reverseLayout() ? AlignRight : AlignLeft;


    // draw title text
    p.setPen(black);
    p.drawText(150, 84, width() - 160, 108 - 84, hAlign | AlignVCenter, title);

    // draw intro text
    p.setPen(black);
    p.drawText(28, 128, width() - 38, 184 - 128, hAlign | AlignVCenter | WordBreak, intro);

    // fill background
    p.fillRect(0, yoffset, width(), height() - yoffset, QBrush(QColor(49,121,172)));

    // draw part3
    if (height() <= 184) return;

    int part3EffectY = height() - _part3->height();
    int part3EffectX = (hAlign == AlignLeft) ? (width()  - _part3->width()) : 0;
    if ( part3EffectX < 0)
      part3EffectX = 0;
    if ( height() < 184 + _part3->height() )
      part3EffectY = 184;

    p.drawPixmap( part3EffectX, part3EffectY, *_part3 );

    // draw textbox
    if (height() <= 184 + 50) return;

    int bheight = height() - 184 - 50 - 40;
    int bwidth = width() - 50;

    if (bheight < 0) bheight = 0;
    if (bwidth < 0) bheight = 0;
    if (bheight > 400) bheight = 400;
    if (bwidth > 500) bwidth = 500;

    int boxX = (hAlign == AlignLeft) ? 25 : width()-bwidth-25;
    int boxY = 184 + 50;

    p.setClipRect(boxX, boxY, bwidth, bheight);
    p.fillRect( boxX, boxY, bwidth, bheight,
                QBrush( QColor( 204, 222, 234 ) ) );
    p.drawPixmap( part3EffectX, part3EffectY, *_part3Effect );

    p.setViewport( boxX, boxY, bwidth, bheight);
    p.setWindow(0, 0, bwidth, bheight);

    // draw info text
    xoffset = 10;
    yoffset = 30;

    int fheight = fontMetrics().height();

    f2.setBold(true);


    if (!_moduleList)
    {
        int xoffset = (hAlign == AlignLeft) ? 10 : bwidth-10-120;
        int xadd = (hAlign == AlignLeft) ? 120 : -xoffset+10;

        // kde version
        p.setFont(f1);
        p.drawText(xoffset, yoffset, 120, fheight, hAlign, i18n(version_text));
        p.setFont(f2);
        p.drawText(xoffset + xadd, yoffset, bwidth-130, fheight, hAlign, KCGlobal::kdeVersion());
        yoffset += fheight + 5;
        if(yoffset > bheight) return;

        // user name
        p.setFont(f1);
        p.drawText(xoffset, yoffset, 120, fheight, hAlign, i18n(user_text));
        p.setFont(f2);
        p.drawText(xoffset + xadd, yoffset, bwidth-130, fheight, hAlign, KCGlobal::userName());
        yoffset += fheight + 5;
        if(yoffset > bheight) return;

        // host name
        p.setFont(f1);
        p.drawText(xoffset, yoffset, 120, fheight, hAlign, i18n(host_text));
        p.setFont(f2);
        p.drawText(xoffset + xadd, yoffset, bwidth-130, fheight, hAlign, KCGlobal::hostName());
        yoffset += fheight + 5;
        if(yoffset > bheight) return;

        // system
        p.setFont(f1);
        p.drawText(xoffset, yoffset, 120, fheight, hAlign, i18n(system_text));
        p.setFont(f2);
        p.drawText(xoffset + xadd, yoffset, bwidth-130, fheight, hAlign, KCGlobal::systemName());
        yoffset += fheight + 5;
        if(yoffset > bheight) return;

        // release
        p.setFont(f1);
        p.drawText(xoffset, yoffset, 120, fheight, hAlign, i18n(release_text));
        p.setFont(f2);
        p.drawText(xoffset + xadd, yoffset, bwidth-130, fheight, hAlign, KCGlobal::systemRelease());
        yoffset += fheight + 5;
        if(yoffset > bheight) return;

        // machine
        p.setFont(f1);
        p.drawText(xoffset, yoffset, 120, fheight, hAlign, i18n(machine_text));
        p.setFont(f2);
        p.drawText(xoffset + xadd, yoffset, bwidth-130, fheight, hAlign, KCGlobal::systemMachine());
        if(yoffset > bheight) return;

        yoffset += 10;

        if(width() < 450 || height() < 450) return;

        // draw use text
        xoffset = 10;
        bheight = bheight - yoffset;
        bwidth = bwidth - (xoffset *2); // both left and right margin

        p.setFont(f1);

        QString ut = i18n(use_text);
        QRect r = p.boundingRect(0, 0, bwidth, bheight, hAlign | AlignVCenter | WordBreak, ut);
        if (bheight - r.height() < 10)
           return;

        p.drawText(xoffset, yoffset, bwidth, bheight, hAlign | AlignVCenter | WordBreak, ut);
    }
    else
    {
        // Need to set this here, not in the ctor. Otherwise Qt resets
        // it to false when this is reparented (malte)
        setMouseTracking(true);
        QFont headingFont = f2;
        int fs = headingFont.pointSize();
        if (fs == -1)
           fs = QFontInfo(headingFont).pointSize();
        headingFont.setPointSize(fs+5);
        QFont lf = f2;
        lf.setUnderline(true);

        const int alxadd = 200; // name-field width

 	const int nameoffset = (hAlign == AlignLeft) ? 10 : bwidth-alxadd;
	const int namewidth = alxadd -10;
 	const int commentoffset = (hAlign == AlignLeft) ? alxadd : 0;
 	const int commentwidth = bwidth-alxadd;

 	int yoffset = 15;

        p.setFont(headingFont);
        if (!_caption.isEmpty())
        {
           p.drawText(10, yoffset, bwidth-20, bheight - yoffset, hAlign | AlignTop, _caption );
           yoffset += fheight + 15;
        }

        // traverse the list
        _moduleLinks.clear();
        _linkBuffer.resize(namewidth, bheight);
	_linkArea = QRect(p.viewport().left()+nameoffset, p.viewport().top(),
	                  namewidth, p.viewport().height());
        QPainter lp(&_linkBuffer);
        lp.fillRect( 0, 0, namewidth, bheight,
                    QBrush( QColor( 204, 222, 234 ) ) );
        lp.drawPixmap( part3EffectX - boxX - nameoffset, part3EffectY - boxY, *_part3Effect );
        lp.setPen(QColor(0x19, 0x19, 0x70)); // same as about:konqueror
        lp.setFont(lf);
        QListViewItem* pEntry = _category;
        while (pEntry != NULL)
        {
            QString szName;
            QString szComment;
            ConfigModule *module = static_cast<ModuleTreeItem*>(pEntry)->module();
            if (module)
            {
                szName = module->moduleName();
                szComment = module->comment();
                p.setFont(f2);
                QRect bounds;
	        int height;
	        p.drawText(nameoffset, yoffset,
                           namewidth, bheight - yoffset,
                           hAlign | AlignTop | WordBreak, szName, -1, &bounds);
                lp.drawText(0, yoffset,
                            namewidth, bheight - yoffset,
                            hAlign | AlignTop | WordBreak, szName);
                height = bounds.height();
                p.setFont(f1);
                p.drawText(commentoffset, yoffset,
                           commentwidth, bheight - yoffset,
                           hAlign | AlignTop | WordBreak, szComment, -1, &bounds);

	        height = QMAX(height, bounds.height());

                ModuleLink *linkInfo = new ModuleLink;
                linkInfo->module = module;
                linkInfo->linkArea = QRect(nameoffset + p.viewport().left(),
                                           yoffset + p.viewport().top(),
                                           namewidth, height);
                _moduleLinks.append(linkInfo);
                yoffset += height + 5;
            }
            else
            {
                szName = static_cast<ModuleTreeItem*>(pEntry)->caption();
                p.setFont(f2);
                QRect bounds;
                p.drawText(nameoffset, yoffset, namewidth, bheight - yoffset,
                           hAlign | AlignTop | WordBreak, szName, -1, &bounds);
                lp.drawText(nameoffset, yoffset,
                            namewidth, bheight - yoffset,
                            hAlign | AlignTop | WordBreak, szName);
                yoffset += bounds.height() + 5;
            }

//          yoffset += fheight + 5;
            if(yoffset > bheight) return;

            pEntry = pEntry->nextSibling();
        }
    }
}

void AboutWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (!_moduleList)
        return;
    ModuleLink *newLink = 0;
    if (_linkArea.contains(e->pos()))
    {
        for (QPtrListIterator<ModuleLink> it(_moduleLinks); it.current(); ++it)
        {
            if (it.current()->linkArea.contains(e->pos()))
            {
                newLink = it.current();
                break;
            }
        }
    }
    if (newLink != _activeLink)
    {
        _activeLink = newLink;
        if (_activeLink)
            setCursor(KCursor::handCursor());
        else
            unsetCursor();
        repaint(_linkArea);
    }
}

void AboutWidget::mouseReleaseEvent(QMouseEvent*)
{
    if (_activeLink)
        emit moduleSelected(_activeLink->module);
}
