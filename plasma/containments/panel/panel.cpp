/*
*   Copyright 2007 by Alex Merry <huntedhacker@tiscali.co.uk>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
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

#include <limits>

#include <QApplication>
#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QBitmap>
#include <QDesktopWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QAction>
#include <QGraphicsLayout>


#include <KDebug>
#include <KIcon>
#include <KDialog>
#include <KIntNumInput>
#include <KMessageBox>

#include <plasma/corona.h>
#include <plasma/panelsvg.h>
#include <plasma/theme.h>
#include <plasma/view.h>

using namespace Plasma;

Panel::Panel(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_dialog(0),
      m_appletBrowserAction(0),
      m_configureAction(0),
      m_removeAction(0),
      m_currentSize(QSize(QApplication::desktop()->screenGeometry(screen()).width(), 56)),
      m_lastViewGeom()
{
    m_background = new Plasma::PanelSvg(this);
    m_background->setImagePath("widgets/panel-background");
    m_background->setEnabledBorders(Plasma::PanelSvg::AllBorders);
    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(backgroundChanged()));
    setZValue(150);
    setContainmentType(Containment::PanelContainment);
    resize(m_currentSize);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
    connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));
}

Panel::~Panel()
{
    delete m_dialog;
}

void Panel::init()
{
    Containment::init();
    setFlag(ItemClipsChildrenToShape, true);
}

QList<QAction*> Panel::contextualActions()
{
    if (!m_appletBrowserAction) {
        m_appletBrowserAction = new QAction(i18n("Add Widgets..."), this);
        m_appletBrowserAction->setIcon(KIcon("list-add"));

        bool locked = immutability() != NotImmutable;
        m_appletBrowserAction->setVisible(!locked);
        connect(m_appletBrowserAction, SIGNAL(triggered()), this, SLOT(showAddWidgets()));

        m_configureAction = new QAction(i18n("Panel Settings"), this);
        m_configureAction->setIcon(KIcon("configure"));
        connect(m_configureAction, SIGNAL(triggered()), this, SLOT(configure()));

        m_removeAction = new QAction(i18n("Remove this Panel"), this);
        m_removeAction->setIcon(KIcon("edit-delete"));
        connect(m_removeAction, SIGNAL(triggered()), this, SLOT(remove()));
        m_removeAction->setVisible(!locked);
    }

    QList<QAction*> actions;
    actions << m_configureAction << m_appletBrowserAction << m_removeAction;
    return actions;
}

void Panel::backgroundChanged()
{
    constraintsEvent(Plasma::LocationConstraint);
}

void Panel::layoutApplet(Plasma::Applet* applet, const QPointF &pos)
{
    // this gets called whenever an applet is added, and we add it to our layout
    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout*>(layout());

    if (!lay) {
        return;
    }

    Plasma::FormFactor f = formFactor();
    int insertIndex = -1;

    //if pos is (-1,-1) insert at the end of the panel
    if (pos != QPoint(-1, -1)) {
        for (int i = 0; i < lay->count(); ++i) {
            QRectF siblingGeometry = lay->itemAt(i)->geometry();
            if (f == Plasma::Horizontal) {
                qreal middle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
                if (pos.x() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.x() <= siblingGeometry.right()) {
                    insertIndex = i + 1;
                    break;
                }
            } else { // Plasma::Vertical
                qreal middle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
                if (pos.y() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.y() <= siblingGeometry.bottom()) {
                    insertIndex = i + 1;
                    break;
                }
            }
        }
    }

    if (insertIndex == -1) {
        lay->addItem(applet);
    } else {
        lay->insertItem(insertIndex, applet);
    }
}

void Panel::updateBorders(const QRect &geom)
{
    Plasma::Location loc = location();
    PanelSvg::EnabledBorders enabledBorders = PanelSvg::AllBorders;

    int s = screen();
    //kDebug() << loc << s << formFactor() << geometry();

    qreal topHeight = m_background->marginSize(Plasma::TopMargin);
    qreal bottomHeight = m_background->marginSize(Plasma::BottomMargin);
    qreal leftWidth = m_background->marginSize(Plasma::LeftMargin);
    qreal rightWidth = m_background->marginSize(Plasma::RightMargin);

    //remove unwanted borders
    if (s < 0) {
        // do nothing in this case, we want all the borders
    } else if (loc == BottomEdge || loc == TopEdge) {
        QRect r = QApplication::desktop()->screenGeometry(s);

        if (loc == BottomEdge) {
            enabledBorders ^= PanelSvg::BottomBorder;
            bottomHeight = 0;
        } else {
            enabledBorders ^= PanelSvg::TopBorder;
            topHeight = 0;
        }

        if (geom.x() <= r.x()) {
            enabledBorders ^= PanelSvg::LeftBorder;
            leftWidth = 0;
        }
        if (geom.right() >= r.right()) {
            enabledBorders ^= PanelSvg::RightBorder;
            rightWidth = 0;
        }

        //hardcoded extra margin for the toolbox right now
        rightWidth += 20;
        //kDebug() << "top/bottom: Width:" << width << ", height:" << height;
    } else if (loc == LeftEdge || loc == RightEdge) {
        QRect r = QApplication::desktop()->screenGeometry(s);

        if (loc == RightEdge) {
            enabledBorders ^= PanelSvg::RightBorder;
            rightWidth = 0;
        } else {
            enabledBorders ^= PanelSvg::LeftBorder;
            leftWidth = 0;
        }
        if (geom.y() <= r.y()) {
            enabledBorders ^= PanelSvg::TopBorder;
            topHeight = 0;
        }
        if (geom.bottom() >= r.bottom()) {
            enabledBorders ^= PanelSvg::BottomBorder;
            bottomHeight = 0;
        }

        //hardcoded extra margin for the toolbox right now
        bottomHeight += 20;
        //kDebug() << "left/right: Width:" << width << ", height:" << height;
    } else {
        kDebug() << "no location!?";
    }

    //invalidate the layout and set again
    if (layout()) {
        layout()->setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);
        layout()->invalidate();
    }

    m_background->setEnabledBorders(enabledBorders);
    update();
}

void Panel::constraintsEvent(Plasma::Constraints constraints)
{
    kDebug() << "constraints updated with" << constraints << "!!!!!!";

    if (constraints & Plasma::FormFactorConstraint) {
        Plasma::FormFactor form = formFactor();

        // create our layout!
        if (form == Plasma::Horizontal || Plasma::Vertical) {
            if (layout()) {
                QGraphicsLayout *lay = layout();
                QGraphicsLinearLayout * linearLay = dynamic_cast<QGraphicsLinearLayout *>(lay);
                if (linearLay) {
                    linearLay->setOrientation(form == Plasma::Horizontal ? Qt::Horizontal :
                                                                           Qt::Vertical);
                }
            } else {
                QGraphicsLinearLayout *lay = new QGraphicsLinearLayout();
                lay->setOrientation(form == Plasma::Horizontal ? Qt::Horizontal : Qt::Vertical);
                lay->setContentsMargins(0, 0, 0, 0);
                lay->setSpacing(4);
                lay->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
                setLayout(lay);

                foreach (Applet *applet, applets()) {
                    lay->addItem(applet);
                }
            }
        }
    }

    //we need to know if the width or height is 100%
    bool fullSize = false;
    if (constraints & Plasma::LocationConstraint || constraints & Plasma::SizeConstraint) {
        m_currentSize = geometry().size().toSize();
        QRectF screenRect = screen() >= 0 ? QApplication::desktop()->screenGeometry(screen()) :
            geometry();

        if ((formFactor() == Horizontal && m_currentSize.width() >= screenRect.width()) || 
                (formFactor() == Vertical && m_currentSize.height() >= screenRect.height())) {
            fullSize = true;
        } else {
            fullSize = false;
        }
    }

    if (constraints & Plasma::LocationConstraint) {
        setFormFactorFromLocation(location());
        if (fullSize) {
            m_background->setElementPrefix(location());
        } else {
            m_background->setElementPrefix(QString());
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        m_background->resizePanel(m_currentSize);
    }

    if (constraints & Plasma::ImmutableConstraint && m_appletBrowserAction) {
        // we need to update the menu items that have already been created
        bool locked = immutability() != NotImmutable;
        m_appletBrowserAction->setVisible(!locked);
        m_removeAction->setVisible(!locked);
    }
}

void Panel::themeUpdated()
{
    //if the theme is changed all the calculations needs to be done again
    //TODO resize based on the change in theme bordersize
    constraintsEvent(Plasma::LocationConstraint);
}

void Panel::paintInterface(QPainter *painter,
                           const QStyleOptionGraphicsItem *option,
                           const QRect& contentsRect)
{
    //FIXME: this background drawing is bad and ugly =)
    // draw the background untransformed (saves lots of per-pixel-math)
    painter->save();
    painter->resetTransform();

    const Containment::StyleOption *containmentOpt = qstyleoption_cast<const Containment::StyleOption *>(option);
    
    QRect viewGeom;
    if (containmentOpt) {
        viewGeom = containmentOpt->view->geometry();
    }

    if (viewGeom != m_lastViewGeom) {
        m_lastViewGeom = viewGeom;
        updateBorders(viewGeom);
    }

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->setRenderHint(QPainter::Antialiasing);

    m_background->paintPanel(painter, contentsRect);

    if (containmentOpt && containmentOpt->view) {
        containmentOpt->view->setMask(m_background->mask());
    }

    // restore transformation and composition mode
    painter->restore();
}

void Panel::showAddWidgets()
{
    emit showAddWidgetsInterface(QPointF());
}

void Panel::configure()
{
    if (! m_dialog) {
        m_dialog = new KDialog();
        m_dialog->setCaption( i18nc("@title:window","Panel Settings") );
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
        m_sizeCombo->addItem(i18n("Tiny"), QVariant(32));
        m_sizeCombo->addItem(i18n("Small"), QVariant(40));
        m_sizeCombo->addItem(i18n("Normal"), QVariant(56));
        m_sizeCombo->addItem(i18n("Large"), QVariant(72));
        m_sizeCombo->addItem(i18n("Custom"));
        m_sizeEdit = new KIntNumInput(p);
        m_sizeEdit->setRange(16, 256);
        l->addWidget(m_sizeEdit, 1, 1);
        l->setColumnStretch(1,1);
        connect(m_sizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(sizeComboChanged()));

        QLabel *lengthLabel = new QLabel(i18n("Length:"), p);
        l->addWidget(lengthLabel, 2, 0);
        m_lengthEdit = new KIntNumInput(p);
        QRect screenRect = screen() >= 0 ? QApplication::desktop()->screenGeometry(screen()) : geometry().toRect();
        int screenlength = 0;
        int currentlength = 0;
        switch (location()) {
            case BottomEdge:
            case TopEdge:
            case Floating:
                screenlength = screenRect.width();
                currentlength = m_currentSize.width();
                break;
            case RightEdge:
            case LeftEdge:
                screenlength = screenRect.height();
                currentlength = m_currentSize.height();
                break;
            default:
                kDebug() << "shouldn't happen!" << location();
            return;
        }
        m_lengthEdit->setRange(0, screenlength);
        m_lengthEdit->setValue(currentlength);
        l->addWidget(m_lengthEdit, 2, 1);

        QLabel *locationLabel = new QLabel(i18n("Location:"), p);
        l->addWidget(locationLabel, 3, 0);
        m_locationCombo = new QComboBox(p);
        locationLabel->setBuddy(m_locationCombo);
        l->addWidget(m_locationCombo, 3, 1);
        m_locationCombo->addItem(i18n("Bottom"), Plasma::BottomEdge);
        m_locationCombo->addItem(i18n("Top"), Plasma::TopEdge);
        m_locationCombo->addItem(i18n("Right"), Plasma::RightEdge);
        m_locationCombo->addItem(i18n("Left"), Plasma::LeftEdge);
    }

    int panelSize = (formFactor() == Plasma::Horizontal) ? size().toSize().height() : size().toSize().width();
    int idx = m_sizeCombo->count() - 1;
    for (int i = 0; i <= m_sizeCombo->count() - 2; ++i) {
        if (m_sizeCombo->itemData(i).toInt() == panelSize) {
            idx = i;
            break;
        }
    }
    m_sizeCombo->setCurrentIndex(idx);
    m_sizeEdit->setValue(panelSize);
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

void Panel::remove()
{
    if (KMessageBox::warningContinueCancel(0, i18n( "Do you really want to remove this panel?"),
                     i18n("Remove Panel"), KStandardGuiItem::remove()) == KMessageBox::Continue ) {
         clearApplets();
         corona()->destroyContainment(this);
         delete this;
    }
}

void Panel::applyConfig()
{
    QSize newSize = QSize(m_lengthEdit->value(), m_sizeCombo->itemData(m_sizeCombo->currentIndex()).toInt());
    if (newSize.height() == 0) {
        newSize = QSize(m_lengthEdit->value(), m_sizeEdit->value());
    }
    Plasma::Location newLoc = (Plasma::Location)(m_locationCombo->itemData(m_locationCombo->currentIndex()).toInt());

    //swap width and height if the panel is vertical
    if (newLoc == LeftEdge || newLoc == RightEdge) {
        newSize = QSize(newSize.height(), newSize.width());
    }

    if (newLoc != location()) {
        m_currentSize = newSize;
        setFormFactorFromLocation(newLoc);
        setLocation(newLoc);
    } else if (newSize != m_currentSize) {
        updateSize(newSize);
    }

}

void Panel::setFormFactorFromLocation(Plasma::Location loc) {
    switch (loc) {
        case BottomEdge:
        case TopEdge:
            //kDebug() << "setting horizontal form factor";
            setFormFactor(Plasma::Horizontal);
            break;
        case RightEdge:
        case LeftEdge:
            //kDebug() << "setting vertical form factor";
            setFormFactor(Plasma::Vertical);
            break;
        case Floating:
            //TODO: implement a form factor for floating panels
            kDebug() << "Floating is unimplemented.";
            break;
        default:
            kDebug() << "invalid location!!";
    }
}

//assumes location is an edge and screen is valid
//TODO handle floating location too
void Panel::updateSize(const QSize &newSize)
{
    resize(newSize);
    setMinimumSize(newSize);
    setMaximumSize(newSize);
    //kDebug( )<< "geometry is now" << geometry() << sceneBoundingRect();
}

void Panel::sizeComboChanged()
{
    QVariant v = m_sizeCombo->itemData(m_sizeCombo->currentIndex());
    m_sizeEdit->setEnabled(v.isNull());
}

K_EXPORT_PLASMA_APPLET(panel, Panel)

#include "panel.moc"

