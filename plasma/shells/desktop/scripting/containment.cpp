/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "containment.h"

#include <Plasma/Corona>
#include <Plasma/Containment>

#include "panelview.h"
#include "plasmaapp.h"
#include "scriptengine.h"
#include "widget.h"

Containment::Containment(Plasma::Containment *containment, QObject *parent)
    : QObject(parent),
      m_containment(containment),
      m_isPanel(m_containment ? ScriptEngine::isPanel(m_containment) : false)
{
}

Containment::~Containment()
{
}

QString Containment::location() const
{
    if (!m_containment) {
        return "floating";
    }

    switch (m_containment->location()) {
        case Plasma::Floating:
            return "floating";
            break;
        case Plasma::Desktop:
            return "desktop";
            break;
        case Plasma::FullScreen:
            return "fullscreen";
            break;
        case Plasma::TopEdge:
            return "top";
            break;
        case Plasma::BottomEdge:
            return "bottom";
            break;
        case Plasma::LeftEdge:
            return "left";
            break;
        case Plasma::RightEdge:
            return "right";
            break;
    }

    return "floating";
}

void Containment::setLocation(const QString &location)
{
    if (!m_containment) {
        return;
    }

    const QString lower = location.toLower();
    if (location == "floating") {
        m_containment->setLocation(Plasma::Floating);
    } else if (location == "desktop") {
        m_containment->setLocation(Plasma::Desktop);
    } else if (location == "fullscreen") {
        m_containment->setLocation(Plasma::FullScreen);
    } else if (location == "top") {
        m_containment->setLocation(Plasma::TopEdge);
    } else if (location == "bottom") {
        m_containment->setLocation(Plasma::BottomEdge);
    } else if (location == "left") {
        m_containment->setLocation(Plasma::LeftEdge);
    } else if (location == "right") {
        m_containment->setLocation(Plasma::RightEdge);
    }
}

int Containment::screen() const
{
    if (!m_containment) {
        return -1;
    }

    return m_containment->screen();
}

void Containment::setScreen(int screen)
{
    if (m_containment) {
        m_containment->setScreen(screen);
    }
}

int Containment::desktop() const
{
    if (!m_containment) {
        return -1;
    }

    return m_containment->desktop();
}

void Containment::setDesktop(int desktop)
{
    if (m_containment) {
        m_containment->setScreen(m_containment->screen(), desktop);
    }
}

QString Containment::formFactor() const
{
    if (!m_containment) {
        return "Planar";
    }

    switch (m_containment->formFactor()) {
        case Plasma::Planar:
            return "planar";
            break;
        case Plasma::MediaCenter:
            return "mediacenter";
            break;
        case Plasma::Horizontal:
            return "horizontal";
            break;
        case Plasma::Vertical:
            return "vertical";
            break;
    }

    return "Planar";
}

QList<int> Containment::widgetIds() const
{
    //FIXME: the ints could overflow since Applet::id() returns a uint,
    //       however QScript deals with QList<uint> very, very poory
    QList<int> w;

    if (m_containment) {
        foreach (const Plasma::Applet *applet, m_containment->applets()) {
            w.append(applet->id());
        }
    }

    return w;
}

QScriptValue Containment::widgetById(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("widgetById requires an id"));
    }

    const uint id = context->argument(0).toInt32();
    Containment *c = qobject_cast<Containment*>(context->thisObject().toQObject());

    if (!c) {
        return engine->undefinedValue();
    }

    if (c->m_containment) {
        foreach (Plasma::Applet *w, c->m_containment->applets()) {
            if (w->id() == id) {
                return ScriptEngine::wrap(w, engine);
            }
        }
    }

    return engine->undefinedValue();
}

QScriptValue Containment::addWidget(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("widgetById requires a name of a widget or a widget object"));
    }

    Containment *c = qobject_cast<Containment*>(context->thisObject().toQObject());

    if (!c || !c->m_containment) {
        return engine->undefinedValue();
    }

    QScriptValue v = context->argument(0);
    Plasma::Applet *applet = 0;
    if (v.isString()) {
        applet = c->m_containment->addApplet(v.toString());
        if (applet) {
            return ScriptEngine::wrap(applet, engine);
        }
    } else if (Widget *widget = qobject_cast<Widget*>(v.toQObject())) {
        applet = widget->applet();
        c->m_containment->addApplet(applet);
        return v;
    }

    return engine->undefinedValue();
}

uint Containment::id() const
{
    if (!m_containment) {
        return 0;
    }

    return m_containment->id();
}

QString Containment::name() const
{
    if (!m_containment) {
        return QString();
    }

    return m_containment->activity();
}

void Containment::setName(const QString &name)
{
    if (m_containment) {
        m_containment->setActivity(name);
    }
}

QString Containment::type() const
{
    if (!m_containment) {
        return QString();
    }

    return m_containment->pluginName();
}

void Containment::remove()
{
    m_isPanel = false;
    m_containment->destroy(false);
}

void Containment::showConfigurationInterface()
{
    if (m_containment) {
        QAction *configAction = m_containment->action("configure");
        if (configAction && configAction->isEnabled()) {
            configAction->trigger();
        }
    }
}

PanelView *Containment::panel() const
{
    if (!m_isPanel || !m_containment) {
        return 0;
    }

    foreach (PanelView *v, PlasmaApp::self()->panelViews()) {
        if (v->containment() == m_containment) {
            return v;
        }
    }

    return 0;
}

QString Containment::alignment() const
{
    PanelView *v = panel();
    if (!v) {
        return "left";
    }

    switch (v->alignment()) {
        case Qt::AlignRight:
            return "right";
            break;
        case Qt::AlignCenter:
            return "center";
            break;
        default:
            return "left";
            break;
    }

    return "left";
}

void Containment::setAlignment(const QString &alignment)
{
    PanelView *v = panel();
    if (v) {
        bool success = false;

        if (alignment.compare("left", Qt::CaseInsensitive) == 0) {
            if (v->alignment() != Qt::AlignLeft) {
                success = true;
                v->setAlignment(Qt::AlignLeft);
            }
        } else if (alignment.compare("right", Qt::CaseInsensitive) == 0) {
            if (v->alignment() != Qt::AlignRight) {
                success = true;
                v->setAlignment(Qt::AlignRight);
            }
        } else if (alignment.compare("center", Qt::CaseInsensitive) == 0) {
            if (v->alignment() != Qt::AlignCenter) {
                success = true;
                v->setAlignment(Qt::AlignCenter);
            }
        }

        if (success) {
            v->setOffset(0);
        }
    }
}

int Containment::offset() const
{
    PanelView *v = panel();
    if (v) {
        return v->offset();
    }

    return 0;
}

void Containment::setOffset(int pixels)
{
    if (pixels < 0) {
        return;
    }

    PanelView *v = panel();
    if (v) {
        QRectF screen = m_containment->corona()->screenGeometry(v->screen());
        QSizeF size = m_containment->size();

        if (m_containment->formFactor() == Plasma::Vertical) {
            if (pixels > screen.height()) {
                return;
            }

            if (size.height() + pixels > screen.height()) {
                m_containment->resize(size.width(), screen.height() - pixels);
            }
        } else if (pixels > screen.width()) {
            return;
        } else if (size.width() + pixels > screen.width()) {
            size.setWidth(screen.width() - pixels);
            m_containment->resize(size);
            m_containment->setMinimumSize(size);
            m_containment->setMaximumSize(size);
        }

        v->setOffset(pixels);
    }
}

int Containment::length() const
{
    if (m_containment->formFactor() == Plasma::Vertical) {
        return m_containment->size().height();
    } else {
        return m_containment->size().width();
    }
}

void Containment::setLength(int pixels)
{
    if (pixels < 0) {
        return;
    }

    PanelView *v = panel();
    if (v) {
        QRectF screen = m_containment->corona()->screenGeometry(v->screen());
        QSizeF s = m_containment->size();
        if (m_containment->formFactor() == Plasma::Vertical) {
            if (pixels > screen.height() - v->offset()) {
                return;
            }

            s.setHeight(pixels);
        } else if (pixels > screen.width() - v->offset()) {
            return;
        } else {
            s.setWidth(pixels);
        }

        m_containment->resize(s);
        m_containment->setMinimumSize(s);
        m_containment->setMaximumSize(s);
    }
}

int Containment::height() const
{
    if (!m_isPanel || !m_containment) {
        return 0;
    }

    return m_containment->formFactor() == Plasma::Vertical ? m_containment->size().width() :
                                                             m_containment->size().height();
}

void Containment::setHeight(int height)
{
    if (height < 16) {
        return;
    }

    PanelView *v = panel();
    if (v) {
        QRect screen = m_containment->corona()->screenGeometry(v->screen());
        QSizeF size = m_containment->size();
        const int max = (m_containment->formFactor() == Plasma::Vertical ? screen.width() : screen.height()) / 3;
        height = qBound(16, height, max);

        if (m_containment->formFactor() == Plasma::Vertical) {
            size.setWidth(height);
        } else {
            size.setHeight(height);
        }

        m_containment->resize(size);
        m_containment->setMinimumSize(size);
        m_containment->setMaximumSize(size);
    }
}

QString Containment::hiding() const
{
    PanelView *v = panel();
    if (v) {
        switch (v->visibilityMode()) {
            case PanelView::NormalPanel:
                return "none";
                break;
            case PanelView::AutoHide:
                return "autohide";
                break;
            case PanelView::LetWindowsCover:
                return "windowscover";
                break;
            case PanelView::WindowsGoBelow:
                return "windowsbelow";
                break;
        }
    }

    return "none";
}

void Containment::setHiding(const QString &mode)
{
    PanelView *v = panel();
    if (v) {
        if (mode.compare("autohide", Qt::CaseInsensitive) == 0) {
            v->setVisibilityMode(PanelView::AutoHide);
        } else if (mode.compare("windowscover", Qt::CaseInsensitive) == 0) {
            v->setVisibilityMode(PanelView::LetWindowsCover);
        } else if (mode.compare("windowsbelow", Qt::CaseInsensitive) == 0) {
            v->setVisibilityMode(PanelView::WindowsGoBelow);
        } else {
            v->setVisibilityMode(PanelView::NormalPanel);
        }
    }
}

#include "containment.moc"

