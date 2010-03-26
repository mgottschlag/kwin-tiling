#include <QVector>

struct KDbusImageStruct {
    int width;
    int height;
    QByteArray data;
};

Q_DECLARE_METATYPE(KDbusImageStruct)

typedef QVector<KDbusImageStruct> KDbusImageVector;

Q_DECLARE_METATYPE(KDbusImageVector)

struct KDbusToolTipStruct {
    QString icon;
    KDbusImageVector image;
    QString title;
    QString subTitle;
};

Q_DECLARE_METATYPE(KDbusToolTipStruct)