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

#ifndef PLASMAWEBAPPLET_H
#define PLASMAWEBAPPLET_H

#include "webapplet.h"
#include "plasmajs.h"

#include <KTemporaryFile>

class PlasmaWebApplet : public WebApplet
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(uint id READ id)
    Q_PROPERTY(QString pluginName READ pluginName)
    Q_PROPERTY(QString icon READ icon)
    Q_PROPERTY(QString category READ category)
    Q_PROPERTY(int formFactor READ formFactor)
    Q_PROPERTY(int location READ location)

public:
    PlasmaWebApplet(QObject *parent, const QVariantList &args);
    virtual ~PlasmaWebApplet();

    virtual bool init();

    QString name() const;
    uint id() const;
    QString pluginName() const;
    QString icon() const;
    QString category() const;
    bool shouldConserveResources() const;
    int formFactor() const;
    int location() const;

public slots:
    QStringList listAllDataEngines();
    QObject* dataEngine(const QString& name);
    QObject* config();
    QObject* globalConfig();
    void setScrollBarPolicy(int orientation, int policy);
    QVariantList screenRect();
    void setConfigurationRequired(bool needsConfiguring, const QString &reason = QString());
    QVariantList getContentsMargins();
    void resize(qreal w, qreal h);
    QVariantList size();
    void setBackgroundHints(int hints);
    int backgroundHints();
    void setAspectRatioMode(int mode);
    int aspectRatioMode();
    void setMaximumSize(qreal w, qreal h);
    QVariantList maximumSize();
    void setMinimumSize(qreal w, qreal h);
    QVariantList minimumSize();
    void setPreferredSize(qreal w, qreal h);
    QVariantList preferredSize();
    void setGeometry(qreal x, qreal y, qreal w, qreal h);
    QVariantList geometry();
    void setPos(qreal x, qreal y);
    QVariantList pos();
    void setFailedToLaunch(bool failed, const QString &reason = QString());
    void update();
    bool isBusy() const;
    void setBusy(bool busy);

    QVariant arg(int index) const;
    QObject* objArg(int index) const;
    void dataUpdated(const QString& source, const Plasma::DataEngine::Data &data);
    void configChanged();
    void themeChanged();
    void makeStylesheet();

protected:
    QVariant callJsFunction(const QString &func, const QVariantList &args = QVariantList());
    void constraintsEvent(Plasma::Constraints constraints);

protected slots:
    virtual void loadFinished(bool success);
    virtual void initJsObjects();

private:
    QVariantList m_args;
    DataEngineDataWrapper m_dataEngineData;
    ConfigGroupWrapper m_config;
    ConfigGroupWrapper m_globalConfig;
    KTemporaryFile m_styleSheetFile;
    static QString s_jsConstants;
};

#endif
