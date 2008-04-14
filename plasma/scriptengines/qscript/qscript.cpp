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

#include <QScriptEngine>
#include <QFile>
#include <QUiLoader>

#include <KDebug>
#include <KLocale>
#include <KStandardDirs>

#include <plasma/applet.h>
#include <plasma/svg.h>
#include <plasma/uiloader.h>

using namespace Plasma;

#include "bind_dataengine.h"

#include "qscript.h"

Q_DECLARE_METATYPE(QPainter*)
Q_DECLARE_METATYPE(QStyleOptionGraphicsItem*)
Q_DECLARE_METATYPE(QScriptApplet*)
Q_DECLARE_METATYPE(Layout*)
Q_DECLARE_METATYPE(Applet*)

QScriptValue constructPainterClass(QScriptEngine *engine);
QScriptValue constructGraphicsItemClass(QScriptEngine *engine);
QScriptValue constructTimerClass(QScriptEngine *engine);
QScriptValue constructFontClass(QScriptEngine *engine);
QScriptValue constructQRectFClass(QScriptEngine *engine);
QScriptValue constructQPointClass(QScriptEngine *engine);
QScriptValue constructQSizeFClass(QScriptEngine *engine);

/*
 * Workaround the fact that QtScripts handling of variants seems a bit broken.
 */
QScriptValue variant2ScriptValue( QScriptEngine *engine, QVariant var )
{
    if( var.isNull() )
	return engine->nullValue();

    switch( var.type() )
    {
        case QVariant::Invalid:
	    return engine->nullValue();
        case QVariant::Bool:
	    return QScriptValue( engine, var.toBool() );
        case QVariant::Date:
	    return engine->newDate( var.toDateTime() );
        case QVariant::DateTime:
	    return engine->newDate( var.toDateTime() );
        case QVariant::Double:
	    return QScriptValue( engine, var.toDouble() );
        case QVariant::Int:
        case QVariant::LongLong:
	    return QScriptValue( engine, var.toInt() );
        case QVariant::String:
	    return QScriptValue( engine, var.toString() );
        case QVariant::Time:
	    return engine->newDate( var.toDateTime() );
        case QVariant::UInt:
	    return QScriptValue( engine, var.toUInt() );
	default:
	    break;
    }

    return qScriptValueFromValue(engine, var);
}

QScriptValue qScriptValueFromData( QScriptEngine *engine, const DataEngine::Data &data )
{
    DataEngine::Data::const_iterator begin = data.begin();
    DataEngine::Data::const_iterator end = data.end();
    DataEngine::Data::const_iterator it;

    QScriptValue obj = engine->newObject();

    for ( it = begin; it != end; ++it ) {
	obj.setProperty( it.key(), variant2ScriptValue( engine, it.value() ) );
    }

    return obj;
}


QScriptApplet::QScriptApplet( QObject *parent, const QVariantList &args )
    : Plasma::AppletScript( parent )
{
    kDebug() << "Script applet launched, args" << args;

    m_engine = new QScriptEngine( this );
    importExtensions();
    setupObjects();
}

QScriptApplet::~QScriptApplet()
{
}

void QScriptApplet::reportError()
{
    kDebug() << "Error: " << m_engine->uncaughtException().toString()
	     << " at line " << m_engine->uncaughtExceptionLineNumber() << endl;
    kDebug() << m_engine->uncaughtExceptionBacktrace();
}

void QScriptApplet::showConfigurationInterface()
{
    kDebug() << "Script: showConfigurationInterface";

    // Here we'll load a ui file...
    QScriptValue global = m_engine->globalObject();

    QScriptValue fun = m_self.property("showConfigurationInterface");
    if ( !fun.isFunction() ) {
	kDebug() << "Script: ShowConfiguratioInterface is not a function, " << fun.toString();
	return;
    }

    QScriptContext *ctx = m_engine->pushContext();
    ctx->setActivationObject( m_self );
    fun.call( m_self );
    m_engine->popContext();

    if ( m_engine->hasUncaughtException() ) {
	reportError();
    }
}

void QScriptApplet::configAccepted()
{
    QScriptValue fun = m_self.property("configAccepted");
    if ( !fun.isFunction() ) {
	kDebug() << "Script: configAccepted is not a function, " << fun.toString();
	return;
    }

    QScriptContext *ctx = m_engine->pushContext();
    ctx->setActivationObject( m_self );
    fun.call( m_self );
    m_engine->popContext();

    if ( m_engine->hasUncaughtException() ) {
	reportError();
    }
}

void QScriptApplet::dataUpdated( const QString &name, const DataEngine::Data &data )
{
    QScriptValue fun = m_self.property("dataUpdated");
    if ( !fun.isFunction() ) {
	kDebug() << "Script: dataUpdated is not a function, " << fun.toString();
	return;
    }

    QScriptValueList args;
    args << m_engine->toScriptValue( name ) << m_engine->toScriptValue( data );

    QScriptContext *ctx = m_engine->pushContext();
    ctx->setActivationObject( m_self );
    fun.call( m_self, args );
    m_engine->popContext();

    if ( m_engine->hasUncaughtException() ) {
	reportError();
    }
}

void QScriptApplet::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)

//    kDebug() << "paintInterface() (c++)";
    QScriptValue fun = m_self.property( "paintInterface" );
    if ( !fun.isFunction() ) {
	kDebug() << "Script: paintInterface is not a function, " << fun.toString();
	return;
    }

    QScriptValueList args;
    args << m_engine->toScriptValue( p );
    args << m_engine->toScriptValue( const_cast<QStyleOptionGraphicsItem*>(option) );
    args << m_engine->toScriptValue( contentsRect );

    QScriptContext *ctx = m_engine->pushContext();
    ctx->setActivationObject( m_self );
    fun.call( m_self, args );
    m_engine->popContext();

    if ( m_engine->hasUncaughtException() ) {
	reportError();
    }
}

bool QScriptApplet::init()
{
    kDebug() << "ScriptName:" << applet()->name();
    kDebug() << "ScriptCategory:" << applet()->category();

    applet()->resize(200, 200);
    QFile file(mainScript());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kWarning() << "Unable to load script file";
        return false;
    }

    QString script = file.readAll();
    kDebug() << "Script says" << script;

    m_engine->evaluate( script );
    if ( m_engine->hasUncaughtException() ) {
        reportError();
        return false;
    }

    return true;
}

void QScriptApplet::importExtensions()
{
    QStringList extensions;
    extensions << "qt.core" << "qt.gui" << "qt.svg" << "qt.xml" << "qt.plasma";
    for (int i = 0; i < extensions.size(); ++i) {
        QString ext = extensions.at(i);
        kDebug() << "importing " << ext << "...";
        QScriptValue ret = m_engine->importExtension(ext);
        if (ret.isError())
            kDebug() << "failed to import extension" << ext << ":" << ret.toString();
    }
    kDebug() << "done importing extensions.";
}

void QScriptApplet::setupObjects()
{
    QScriptValue global = m_engine->globalObject();

    // Expose an applet
    m_self = m_engine->newQObject( this );
    m_self.setScope( global );    

    global.setProperty("applet", m_self);
    // Add a global loadui method for ui files
    QScriptValue fun = m_engine->newFunction( QScriptApplet::loadui );
    global.setProperty("loadui", fun);

    // Work around bug in 4.3.0
    qMetaTypeId<QVariant>();

    // Add constructors
    global.setProperty("PlasmaSvg", m_engine->newFunction( QScriptApplet::newPlasmaSvg ) );

    // Add stuff from 4.4
    global.setProperty("QPainter", constructPainterClass(m_engine));
    global.setProperty("QGraphicsItem", constructGraphicsItemClass(m_engine));
    global.setProperty("QTimer", constructTimerClass(m_engine));
    global.setProperty("QFont", constructFontClass(m_engine));
    global.setProperty("QRectF", constructQRectFClass(m_engine));
    global.setProperty("QSizeF", constructQSizeFClass(m_engine));
    global.setProperty("QPoint", constructQPointClass(m_engine));

    // Bindings for data engine
    m_engine->setDefaultPrototype( qMetaTypeId<DataEngine*>(), m_engine->newQObject( new DataEngine() ) );
#if 0
    fun = m_engine->newFunction( QScriptApplet::dataEngine );
    m_self.setProperty("dataEngine", fun);
#endif
    
    qScriptRegisterMapMetaType<DataEngine::Dict>(m_engine);
//    qScriptRegisterMapMetaType<DataEngine::Data>(m_engine);
    qScriptRegisterMetaType<DataEngine::Data>( m_engine, qScriptValueFromData, 0, QScriptValue() );
}

QString QScriptApplet::findDataResource( const QString &filename )
{
    QString path("plasma-script/%1");
    return KGlobal::dirs()->findResource("data", path.arg(filename) );
}

void QScriptApplet::debug( const QString &msg )
{
    kDebug() << msg;
}

#if 0
QScriptValue QScriptApplet::dataEngine(QScriptContext *context, QScriptEngine *engine)
{
    if ( context->argumentCount() != 1 )
	return context->throwError("dataEngine takes one argument");

    QString dataEngine = context->argument(0).toString();

    Script *self = engine->fromScriptValue<Script*>( context->thisObject() );

    DataEngine *data = self->dataEngine( dataEngine );
    return engine->newQObject( data );
}
#endif

QScriptValue QScriptApplet::loadui(QScriptContext *context, QScriptEngine *engine)
{
    if ( context->argumentCount() != 1 )
	return context->throwError("loadui takes one argument");

    QUiLoader loader;
    QString filename = context->argument(0).toString();
    QFile f( filename );
    if ( !f.open(QIODevice::ReadOnly) )
	return context->throwError(QString("Unable to open '%1'").arg(filename) );

    QWidget *w = loader.load( &f );
    f.close();

    return engine->newQObject( w );
}

QScriptValue QScriptApplet::newPlasmaSvg(QScriptContext *context, QScriptEngine *engine)
{
    if ( context->argumentCount() == 0 )
	return context->throwError("Constructor takes at least 1 argument");

    QString filename = context->argument(0).toString();
    QObject *parent = 0;

    if ( context->argumentCount() == 2 )
	parent = qscriptvalue_cast<QObject *>(context->argument(1));

    Svg *svg = new Svg( filename, parent );
    return engine->newQObject( svg );
}

void QScriptApplet::installWidgets( QScriptEngine *engine )
{
    QScriptValue globalObject = engine->globalObject();
    UiLoader loader;

    QStringList widgets = loader.availableWidgets();
    for ( int i=0; i < widgets.size(); ++i ) {
	QScriptValue fun = engine->newFunction( createWidget );
	QScriptValue name = engine->toScriptValue(widgets[i]);
	fun.setProperty( QString("functionName"), name,
			 QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration );
	fun.setProperty( QString("prototype"), createPrototype( engine, name.toString() ) );

	globalObject.setProperty(widgets[i], fun);
    }

}

QScriptValue QScriptApplet::createWidget(QScriptContext *context, QScriptEngine *engine)
{
#if 0 // FIXME: Broken by WoC
    if ( context->argumentCount() > 1 )
	return context->throwError("Create widget takes one argument");

    Applet *parent = 0;
    if ( context->argumentCount() ) {
	parent = qscriptvalue_cast<Applet*>(context->argument(0));

	if ( !parent )
	    return context->throwError("The parent must be a Widget");
    }

    QString self = context->callee().property( "functionName" ).toString();
    UiLoader loader;
    Applet *w = loader.createWidget( self, parent );

    if (!w)
	return QScriptValue();

    QScriptValue fun = engine->newQObject( w );
    fun.setPrototype( context->callee().property("prototype") );

    return fun;
#endif
    return QScriptValue();
}

QScriptValue QScriptApplet::createPrototype( QScriptEngine *engine, const QString &name )
{
    Q_UNUSED(name)
    QScriptValue proto = engine->newObject();

    // Hook for adding extra properties/methods

    return proto;
}

#include "qscript.moc"


