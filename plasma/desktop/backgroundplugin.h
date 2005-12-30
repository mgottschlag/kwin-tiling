#ifndef BACKGROUNDPLUGIN_H
#define BACKGROUNDPLUGIN_H

namespace Plasma
{

class KDE_EXPORT BackgroundPlugin
{
    public:
        BackgroundPlugin();

    signals:
        void update();

    protected slots:
        virtual void compose(QPainter *p, const QRect &dirtyRect) =0;
        virtual void loadFile(const QString &fileName);

};

}

#endif
