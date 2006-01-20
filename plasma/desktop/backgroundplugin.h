#ifndef BACKGROUNDPLUGIN_H
#define BACKGROUNDPLUGIN_H

namespace Plasma
{

class KDE_EXPORT BackgroundPlugin
{
    public:
        BackgroundPlugin();

    Q_SIGNALS:
        void update();

    protected Q_SLOTS:
        virtual void compose(QPainter *p, const QRect &dirtyRect) =0;
        virtual void loadFile(const QString &fileName);

};

}

#endif
