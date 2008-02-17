/*
*   Copyright 2007 by Alex Merry <huntedhacker@tiscali.co.uk>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "panel.h"

#include <QApplication>
#include <QPainter>
#include <QDesktopWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QAction>

#include <KDebug>
#include <KIcon>
#include <KDialog>
#include <KIntNumInput>

#include <plasma/corona.h>
#include <plasma/layouts/layout.h>
#include <plasma/svgpanel.h>
#include <plasma/theme.h>

using namespace Plasma;

Panel::Panel(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_cachedBackground(0),
      m_dialog(0),
      m_drawTop(true),
      m_drawLeft(true),
      m_drawRight(true),
      m_drawBottom(true),
      m_size(48)
{
    m_background = new Plasma::SvgPanel("widgets/panel-background", this);
    m_background->setBorderFlags(Plasma::SvgPanel::DrawAllBorders);
    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(backgroundChanged()));
    setZValue(150);
    setContainmentType(Containment::PanelContainment);

    connect(Plasma::Theme::self(), SIGNAL(changed()), this, SLOT(themeUpdated()));
    themeUpdated();
}

Panel::~Panel()
{
    delete m_dialog;
}

void Panel::init()
{
    KConfigGroup cg = config();
    m_size = qMax(16, cg.readEntry("size", m_size));

    Containment::init();
}

QList<QAction*> Panel::contextActions()
{
    if (m_actions.isEmpty()) {
        QAction *addWidgetsAction = new QAction(i18n("Add Widgets..."), this);
        addWidgetsAction->setIcon(KIcon("list-add"));
        connect(addWidgetsAction, SIGNAL(triggered()), this, SIGNAL(showAddWidgets()));

        QAction *configureAction = new QAction(i18n("Panel Settings"), this);
        configureAction->setIcon(KIcon("configure"));
        connect(configureAction, SIGNAL(triggered()), this, SLOT(configure()));

        m_actions << configureAction << addWidgetsAction;
    }

    return m_actions;
}

void Panel::backgroundChanged()
{
    constraintsUpdated(Plasma::LocationConstraint);
}

void Panel::constraintsUpdated(Plasma::Constraints constraints)
{
    //kDebug() << "constraints updated with" << constraints << "!!!!!!!!!!!!!!!!!";
    if (constraints & Plasma::SizeConstraint) {
        m_background->resize(size());
    }

    if (constraints & Plasma::LocationConstraint || constraints & Plasma::ScreenConstraint) {
        Plasma::Location loc = location();
        SvgPanel::BorderFlags bFlags = SvgPanel::DrawAllBorders;

        int s = screen();
        if (s < 0) {
            s = 0;
        }

        QRect r = QApplication::desktop()->screenGeometry(s);

        //kDebug() << "Setting location to" << loc << "on screen" << s << "with geom" << r;
        setMaximumSize(r.size());
        int x = r.left();
        int y = r.top();
        int width = 0;
        int height = 0;
        int topHeight = m_background->marginSize(Plasma::TopMargin);
        int bottomHeight = m_background->marginSize(Plasma::BottomMargin);
        int leftWidth = m_background->marginSize(Plasma::LeftMargin);
        int rightWidth = m_background->marginSize(Plasma::RightMargin);

        if (loc == BottomEdge || loc == TopEdge) {
            setFormFactor(Plasma::Horizontal);

            height = m_size;
            //FIXME: don't hardcode full width
            width = r.width();

            if (loc == BottomEdge) {
                bFlags ^= SvgPanel::DrawBottomBorder;
                bottomHeight = 0;
                height += topHeight;
                y = r.bottom() - height + 1;
            } else {
                bFlags ^= SvgPanel::DrawTopBorder;
                topHeight = 0;
                height += bottomHeight;
            }

            if (x <= r.x()) {
                bFlags ^= SvgPanel::DrawLeftBorder;
                leftWidth = 0;
            }

            if (x + width >= r.right()) {
                bFlags ^= SvgPanel::DrawRightBorder;
                rightWidth = 0;
            }
            //kDebug() << "top/bottom: Width:" << width << ", height:" << height;
        } else if (loc == LeftEdge || loc == RightEdge) {
            setFormFactor(Plasma::Vertical);

            width = m_size;
            //FIXME: don't hardcode full height
            height = r.height();

            if (loc == RightEdge) {
                bFlags ^= SvgPanel::DrawRightBorder;
                rightWidth = 0;
                width += leftWidth;
                x = r.right() - width + 1;
            } else {
                bFlags ^= SvgPanel::DrawLeftBorder;
                leftWidth = 0;
                width += rightWidth;
            }

            if (y <= r.y()) {
                bFlags ^= SvgPanel::DrawTopBorder;
                topHeight = 0;
            }

            if (y + height >= r.bottom()) {
                bFlags ^= SvgPanel::DrawBottomBorder;
                bottomHeight = 0;
            }
            //kDebug() << "left/right: Width:" << width << ", height:" << height;
        }

        // Lock the size so that stray applets don't cause the panel to grow
        // or the removal of applets to cause the panel to shrink
        // TODO: Update this when user-resizing is implemented
        setMinimumSize(QSizeF(width, height));
        setMaximumSize(QSizeF(width, height));

        QRectF geo = QRectF(x, y, width, height);

        //kDebug() << "Setting geometry to" << geo << "with margins" << leftWidth << topHeight << rightWidth << bottomHeight;
        setGeometry(geo);

        if (layout()) {
            layout()->setMargin(Plasma::TopMargin, topHeight);
            layout()->setMargin(Plasma::LeftMargin, leftWidth);
            layout()->setMargin(Plasma::RightMargin, rightWidth);
            layout()->setMargin(Plasma::BottomMargin, bottomHeight);
        }

        m_background->setBorderFlags(bFlags);

        if (corona()) {
            foreach (Containment *c, corona()->containments()) {
                if (c->type() != PanelContainment || c == this) {
                    continue;
                }

                if (c->geometry().intersects(geo)) {
                    //TODO: here is where we need to schedule a negotiation for where to show the
                    //      panel on the scene
                    //
                    //      we also probably need to direct whether to allow this containment to
                    //      be resized before moved, or moved only
                    kDebug() << "conflict!";
                }
                kDebug() << "panel containment with geometry of" << c->geometry() << "but really" << c->transform().map(geometry());
            }
        }
    }
}

void Panel::themeUpdated()
{
    //if the theme is changed all the calculations needs to be done again
    constraintsUpdated(Plasma::LocationConstraint);
}

Qt::Orientations Panel::expandingDirections() const
{
    if (formFactor() == Plasma::Horizontal) {
        return Qt::Horizontal;
    } else {
        return Qt::Vertical;
    }
}

void Panel::paintInterface(QPainter *painter,
                           const QStyleOptionGraphicsItem *,
                           const QRect& contentsRect)
{
    //FIXME: this background drawing is bad and ugly =)
    // draw the background untransformed (saves lots of per-pixel-math)
    painter->save();
    painter->resetTransform();

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->setRenderHint(QPainter::Antialiasing);

    m_background->paint(painter, contentsRect);

    // restore transformation and composition mode
    painter->restore();
}

void Panel::configure()
{
    if (! m_dialog) {
        m_dialog = new KDialog();
        m_dialog->setCaption( i18nc("@title:window","Configure Panel") );
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect(m_dialog, SIGNAL(applyClicked()), this, SLOT(applyConfig()));
        connect(m_dialog, SIGNAL(okClicked()), this, SLOT(applyConfig()));

        QWidget *p = m_dialog->mainWidget();
        QGridLayout *l = new QGridLayout(p);
        p->setLayout(l);

        QLabel *sizeLabel = new QLabel(i18n("Size:"), p);
        l->addWidget(sizeLabel, 0, 0);
        m_sizeCombo = new QComboBox(p);
        sizeLabel->setBuddy(m_sizeCombo);
        l->addWidget(m_sizeCombo, 0, 1);
        m_sizeCombo->addItem(i18n("Tiny"), QVariant(24));
        m_sizeCombo->addItem(i18n("Small"), QVariant(32));
        m_sizeCombo->addItem(i18n("Normal"), QVariant(48));
        m_sizeCombo->addItem(i18n("Large"), QVariant(64));
        m_sizeCombo->addItem(i18n("Custom"));
        m_sizeEdit = new KIntNumInput(p);
        m_sizeEdit->setRange(16, 256);
        l->addWidget(m_sizeEdit, 1, 1);
        l->setColumnStretch(1,1);
        connect(m_sizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(sizeComboChanged()));

        QLabel *locationLabel = new QLabel(i18n("Location:"), p);
        l->addWidget(locationLabel, 2, 0);
        m_locationCombo = new QComboBox(p);
        locationLabel->setBuddy(m_locationCombo);
        l->addWidget(m_locationCombo, 2, 1);
        m_locationCombo->addItem(i18n("Bottom"), Plasma::BottomEdge);
        m_locationCombo->addItem(i18n("Top"), Plasma::TopEdge);
        m_locationCombo->addItem(i18n("Right"), Plasma::RightEdge);
        m_locationCombo->addItem(i18n("Left"), Plasma::LeftEdge);
    }

    int idx = m_sizeCombo->count() - 1;
    for (int i = 0; i <= m_sizeCombo->count() - 2; ++i) {
        if (m_sizeCombo->itemData(i).toInt() == m_size) {
            idx = i;
            break;
        }
    }
    m_sizeCombo->setCurrentIndex(idx);
    m_sizeEdit->setValue(m_size);
    sizeComboChanged();
    idx = 0;
    for (int i = 0; i < m_locationCombo->count(); i++) {
        if (m_locationCombo->itemData(i).toInt() == location()) {
            idx = i;
            break;
        }
    }
    m_locationCombo->setCurrentIndex(idx);

    m_dialog->show();
}

void Panel::applyConfig()
{
    KConfigGroup cg = config();
    const int size = m_sizeCombo->itemData(m_sizeCombo->currentIndex()).toInt();
    m_size = size > 0 ? size : m_sizeEdit->value();
    cg.writeEntry("size", m_size);

    setLocation((Plasma::Location)(m_locationCombo->itemData(m_locationCombo->currentIndex()).toInt()));

    updateConstraints();
}

void Panel::sizeComboChanged()
{
    QVariant v = m_sizeCombo->itemData(m_sizeCombo->currentIndex());
    m_sizeEdit->setEnabled(v.isNull());
}

K_EXPORT_PLASMA_APPLET(panel, Panel)

#include "panel.moc"

