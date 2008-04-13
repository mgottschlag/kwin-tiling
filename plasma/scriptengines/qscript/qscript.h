/*
 *   Copyright 2007 Richard J. Moore <rich@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QScriptValue>

#include <plasma/scripting/appletscript.h>
#include <plasma/dataengine.h>

class QScriptEngine;
class QScriptContext;

class QScriptApplet : public Plasma::AppletScript
{
    Q_OBJECT

public:
    QScriptApplet( QObject *parent, const QVariantList &args );
    ~QScriptApplet();
    bool init();

    void reportError();

    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);

    Q_INVOKABLE QString findDataResource( const QString &filename );
    Q_INVOKABLE void debug( const QString &msg );
//    Q_INVOKABLE void update( const QRectF & rect = QRectF() ) { Applet::update(rect); }

public slots:
    void dataUpdated( const QString &name, const Plasma::DataEngine::Data &data );
    void showConfigurationInterface();
    void configAccepted();

private:
    void importExtensions();
    void setupObjects();

    //static QScriptValue dataEngine(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue loadui(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newPlasmaSvg(QScriptContext *context, QScriptEngine *engine);

    void installWidgets( QScriptEngine *engine );
    static QScriptValue createWidget(QScriptContext *context, QScriptEngine *engine);

    static QScriptValue createPrototype( QScriptEngine *engine, const QString &name );

private:
    QScriptEngine *m_engine;
    QScriptValue m_self;
};

K_EXPORT_PLASMA_APPLETSCRIPTENGINE(qscriptapplet, QScriptApplet)


#endif // SCRIPT_H

