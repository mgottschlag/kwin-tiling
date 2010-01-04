/*
Copyright (c) 2007 Zack Rusin <zack@kde.org>
Copyright (c) 2008 Petri Damstén <damu@iki.fi>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "plasmawebapplet.h"

#include <QAction>
#include <QWebPage>
#include <QWebFrame>

#include <KColorScheme>

#include <Plasma/DataEngineManager>
#include <Plasma/Applet>
#include <Plasma/Theme>
#include <Plasma/WebView>

#define JS_CONSTANTS_CONSTRAINT \
"var NoConstraint = %1;\n"\
"var FormFactorConstraint = %2;\n"\
"var LocationConstraint = %3;\n"\
"var ScreenConstraint = %4;\n"\
"var SizeConstraint = %5;\n"\
"var ImmutableConstraint = %6;\n"\
"var StartupCompletedConstraint = %7;\n"\
"var ContextConstraint = %8;\n"\
"var AllConstraints = %9;\n"

#define JS_CONSTANTS_BACKGROUND \
"var NoBackground = %1;\n"\
"var StandardBackground = %2;\n"\
"var TranslucentBackground = %3;\n"\
"var DefaultBackground = %5;\n"

#define JS_CONSTANTS_SCROLLBAR \
"var QtHorizontal = %1;\n"\
"var QtVertical = %2;\n"\
"var ScrollBarAsNeeded = %3;\n"\
"var ScrollBarAlwaysOff = %4;\n"\
"var ScrollBarAlwaysOn = %5;\n"\

#define JS_CONSTANTS_ASPECTRATIO \
"var InvalidAspectRatioMode = %1;\n"\
"var IgnoreAspectRatio = %2;\n"\
"var KeepAspectRatio = %3;\n"\
"var Square = %4;\n"\
"var ConstrainedSquare = %5;\n"\
"var FixedSize = %6;\n"

#define JS_CONSTANTS_FORMFACTOR \
"var Planar = %1;\n"\
"var MediaCenter = %2;\n"\
"var Horizontal = %3;\n"\
"var Vertical = %4;\n"\

#define JS_CONSTANTS_LOCATION \
"var Floating = %1;\n"\
"var Desktop = %2;\n"\
"var FullScreen = %3;\n"\
"var TopEdge = %4;\n"\
"var BottomEdge = %5;\n"\
"var LeftEdge = %6;\n"\
"var RightEdge = %7;\n"

#define JS_CONSTANTS_OTHER \
"var size_width = 0;\n"\
"var size_height = 1;\n"\
"var point_x = 0;\n"\
"var point_y = 1;\n"\
"var rect_x = 0;\n"\
"var rect_y = 1;\n"\
"var rect_width = 2;\n"\
"var rect_height = 3;\n"\
"var margin_left = 0;\n"\
"var margin_top = 1;\n"\
"var margin_right = 2;\n"\
"var margin_bottom = 3;\n"\

#define CSS "body { font-family: %3; font-size: %4pt; color:%1; background-color:%2 }\n"

QString PlasmaWebApplet::s_jsConstants;

PlasmaWebApplet::PlasmaWebApplet(QObject *parent, const QVariantList &args)
: WebApplet(parent, args)
{
    if (s_jsConstants.isEmpty()) {
        s_jsConstants = JS_CONSTANTS_OTHER;
        s_jsConstants += QString(JS_CONSTANTS_CONSTRAINT)
                .arg(Plasma::NoConstraint)
                .arg(Plasma::FormFactorConstraint)
                .arg(Plasma::LocationConstraint)
                .arg(Plasma::ScreenConstraint)
                .arg(Plasma::SizeConstraint)
                .arg(Plasma::ImmutableConstraint)
                .arg(Plasma::StartupCompletedConstraint)
                .arg(Plasma::ContextConstraint)
                .arg(Plasma::AllConstraints);
        s_jsConstants += QString(JS_CONSTANTS_BACKGROUND)
                .arg(Plasma::Applet::NoBackground)
                .arg(Plasma::Applet::StandardBackground)
                .arg(Plasma::Applet::TranslucentBackground)
                .arg(Plasma::Applet::DefaultBackground);
        s_jsConstants += QString(JS_CONSTANTS_SCROLLBAR)
                .arg(Qt::Horizontal)
                .arg(Qt::Vertical)
                .arg(Qt::ScrollBarAsNeeded)
                .arg(Qt::ScrollBarAlwaysOff)
                .arg(Qt::ScrollBarAlwaysOn);
        s_jsConstants += QString(JS_CONSTANTS_ASPECTRATIO)
                .arg(Plasma::InvalidAspectRatioMode)
                .arg(Plasma::IgnoreAspectRatio)
                .arg(Plasma::KeepAspectRatio)
                .arg(Plasma::Square)
                .arg(Plasma::ConstrainedSquare)
                .arg(Plasma::FixedSize);
        s_jsConstants += QString(JS_CONSTANTS_FORMFACTOR)
                .arg(Plasma::Planar)
                .arg(Plasma::MediaCenter)
                .arg(Plasma::Horizontal)
                .arg(Plasma::Vertical);
        s_jsConstants += QString(JS_CONSTANTS_LOCATION)
                .arg(Plasma::Floating)
                .arg(Plasma::Desktop)
                .arg(Plasma::FullScreen)
                .arg(Plasma::TopEdge)
                .arg(Plasma::BottomEdge)
                .arg(Plasma::LeftEdge)
                .arg(Plasma::RightEdge);
    }
}

PlasmaWebApplet::~PlasmaWebApplet()
{
}

bool PlasmaWebApplet::init()
{
    if (WebApplet::init()) {
        connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
                this, SLOT(themeChanged()));
        makeStylesheet();
        page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
        QAction *inspectAction = page()->action(QWebPage::InspectElement);
        inspectAction->setText(i18n("Inspect this widget"));
        return true;
    }

    return false;
}

void PlasmaWebApplet::makeStylesheet()
{
    if (!page()) {
        return;
    }

    // this temporary file contains the style sheet to be used when loading/reloading the
    // web content. we keep it around for the lifetime of the widget since it needs to be there
    // when reloaded

    //TODO perhaps share this file between all instances? perhaps even keep it persistent on disk
    // to limit disk write/deletes. that would be simple enough, the only trick would be to
    // ensure it updates (and only updates once) on Plasma theme changes so that it doesn't get
    // written to in a flury by every PlasmaWebApplet when the theme updates
    // probably a reference counted singleton would be the way to go here.
    if (!m_styleSheetFile.open()) {
        return;
    }

    KColorScheme plasmaColorTheme = KColorScheme(QPalette::Active, KColorScheme::View,
            Plasma::Theme::defaultTheme()->colorScheme());
    QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QColor backgroundColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);

    QString css = QString(CSS).arg(textColor.name())
                              .arg(backgroundColor.name())
                              .arg(font.family())
                              .arg(font.pointSize());
    m_styleSheetFile.write(css.toUtf8());
    page()->settings()->setUserStyleSheetUrl(QUrl(m_styleSheetFile.fileName()));
    m_styleSheetFile.close();
}

void PlasmaWebApplet::themeChanged()
{
    makeStylesheet();
    callJsFunction("themeChanged");
}

void PlasmaWebApplet::loadFinished(bool success)
{
    WebApplet::loadFinished(success);
    if (success) {
        page()->mainFrame()->evaluateJavaScript(s_jsConstants);
        callJsFunction("init");
    }
}

void PlasmaWebApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (page() && constraints & Plasma::SizeConstraint) {
        qreal left;
        qreal top;
        qreal right;
        qreal bottom;
        applet()->getContentsMargins(&left, &top, &right, &bottom);
        view()->setPos(QPointF(left, top));
        view()->resize(WebApplet::size() - QSizeF(left + right, top + bottom));
        //kDebug() << WebApplet::size() << left << right << top << bottom << page()->size();
    }
    callJsFunction("constraintsEvent", QVariantList() << (int)constraints);
}

QVariant PlasmaWebApplet::arg(int index) const
{
    return m_args[index];
}

QObject* PlasmaWebApplet::objArg(int index) const
{
    return m_args[index].value<QObject*>();
}

QString PlasmaWebApplet::name() const
{
    return applet()->name();
}

uint PlasmaWebApplet::id() const
{
    return applet()->id();
}

int PlasmaWebApplet::formFactor() const
{
    return (int)applet()->formFactor();
}

int PlasmaWebApplet::location() const
{
    return (int)applet()->location();
}

QString PlasmaWebApplet::pluginName() const
{
    return applet()->pluginName();
}

QString PlasmaWebApplet::icon() const
{
    return applet()->icon();
}

QString PlasmaWebApplet::category() const
{
    return applet()->category();
}

bool PlasmaWebApplet::shouldConserveResources() const
{
    return applet()->shouldConserveResources();
}

QStringList PlasmaWebApplet::listAllDataEngines()
{
    return Plasma::DataEngineManager::listAllEngines();
}

QObject* PlasmaWebApplet::dataEngine(const QString& name)
{
    QString id = QString("%1").arg(applet()->id());
    Plasma::DataEngine *de = applet()->dataEngine(name);
    DataEngineWrapper *wrapper = de->findChild<DataEngineWrapper*>(id);
    if (!wrapper) {
        wrapper = new DataEngineWrapper(de, this);
        wrapper->setObjectName(id);
    }
    return wrapper;
}

QObject* PlasmaWebApplet::config()
{
    m_config.setConfig(applet()->config());
    return &m_config;
}

QObject* PlasmaWebApplet::globalConfig()
{
    m_globalConfig.setConfig(applet()->globalConfig());
    return &m_globalConfig;
}

void PlasmaWebApplet::resize(qreal w, qreal h)
{
    applet()->resize(w, h);
}

void PlasmaWebApplet::setBackgroundHints(int hints)
{
    applet()->setBackgroundHints((Plasma::Applet::BackgroundHints)hints);
}

void PlasmaWebApplet::setScrollBarPolicy(int orientation, int policy)
{
    page()->mainFrame()->setScrollBarPolicy((Qt::Orientation)orientation,
                                            (Qt::ScrollBarPolicy)policy);
}

void PlasmaWebApplet::setAspectRatioMode(int mode)
{
    applet()->setAspectRatioMode((Plasma::AspectRatioMode)mode);
}

QVariant PlasmaWebApplet::callJsFunction(const QString& func, const QVariantList& args)
{
    if (loaded()) {
        m_args = args;
        QString cmd = "if (window." + func + ") { " + func + '(';
        for(int i = 0; i < args.count(); ++i) {
            if (i > 0) {
                cmd += ',';
            }
            if (args[i].canConvert<QObject*>()) {
                cmd += QString("window.plasmoid.objArg(%1)").arg(i);
            } else {
                cmd += QString("window.plasmoid.arg(%1)").arg(i);
            }
        }
        cmd += ") }";
        //kDebug() << cmd;
        return page()->mainFrame()->evaluateJavaScript(cmd);
    }
    return QVariant();
}

void PlasmaWebApplet::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    m_dataEngineData.setData(data);
    callJsFunction("dataUpdated",
                   QVariantList() << source << QVariant::fromValue((QObject*)&m_dataEngineData));
}

void PlasmaWebApplet::configChanged()
{
    callJsFunction("configChanged");
}

void PlasmaWebApplet::initJsObjects()
{
    QWebFrame *frame = qobject_cast<QWebFrame*>(sender());
    Q_ASSERT(frame);
    frame->addToJavaScriptWindowObject(QLatin1String("plasmoid"), this);
}

QVariantList PlasmaWebApplet::geometry()
{
    return QVariantList() << applet()->geometry().left() << applet()->geometry().top()
                          << applet()->geometry().width() << applet()->geometry().height();
}

QVariantList PlasmaWebApplet::screenRect()
{
    return QVariantList() << applet()->screenRect().left() << applet()->screenRect().top()
                          << applet()->screenRect().width() << applet()->screenRect().height();
}

int PlasmaWebApplet::backgroundHints()
{
    return applet()->backgroundHints();
}

int PlasmaWebApplet::aspectRatioMode()
{
    return applet()->aspectRatioMode();
}

void PlasmaWebApplet::setConfigurationRequired(bool needsConfiguring, const QString &reason)
{
    WebApplet::setConfigurationRequired(needsConfiguring, reason);
}

void PlasmaWebApplet::setMaximumSize(qreal w, qreal h)
{
    applet()->setMaximumSize(w, h);
}

void PlasmaWebApplet::setMinimumSize(qreal w, qreal h)
{
    applet()->setMinimumSize(w, h);
}

void PlasmaWebApplet::setPreferredSize(qreal w, qreal h)
{
    applet()->setPreferredSize(w, h);
}

QVariantList PlasmaWebApplet::maximumSize()
{
    return QVariantList() << applet()->maximumSize().width() << applet()->maximumSize().height();
}

QVariantList PlasmaWebApplet::minimumSize()
{
    return QVariantList() << applet()->minimumSize().width() << applet()->minimumSize().height();
}

QVariantList PlasmaWebApplet::preferredSize()
{
    return QVariantList() << applet()->preferredSize().width()
                          << applet()->preferredSize().height();
}

QVariantList PlasmaWebApplet::getContentsMargins()
{
    qreal left;
    qreal top;
    qreal right;
    qreal bottom;
    applet()->getContentsMargins(&left, &top, &right, &bottom);
    return QVariantList() << left << top << right << bottom;
}

void PlasmaWebApplet::setGeometry(qreal x, qreal y, qreal w, qreal h)
{
    applet()->setGeometry(x, y, w, h);
}

void PlasmaWebApplet::setPos(qreal x, qreal y)
{
    applet()->setPos(x, y);
}

QVariantList PlasmaWebApplet::pos()
{
    return QVariantList() << applet()->pos().x() << applet()->pos().y();
}

QVariantList PlasmaWebApplet::size()
{
    return QVariantList() << applet()->size().width() << applet()->size().height();
}

void PlasmaWebApplet::setFailedToLaunch(bool failed, const QString &reason)
{
    WebApplet::setFailedToLaunch(failed, reason);
}

void PlasmaWebApplet::update()
{
    applet()->update();
}

bool PlasmaWebApplet::isBusy() const
{
    return applet()->isBusy();
}

void PlasmaWebApplet::setBusy(bool busy)
{
    applet()->setBusy(busy);
}

#include "plasmawebapplet.moc"
