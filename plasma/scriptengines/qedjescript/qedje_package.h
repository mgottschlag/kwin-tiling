#ifndef QEDJE_PACKAGE_H
#define QEDJE_PACKAGE_H

#include <Plasma/Package>
#include <Plasma/PackageStructure>
#include <Plasma/PackageMetadata>

class QEdjePackage : public Plasma::PackageStructure
{
    Q_OBJECT

public:
    QEdjePackage(QObject *parent, const QVariantList &args);

    virtual ~QEdjePackage();
    virtual bool installPackage(const QString &archivePath,
                                const QString &packageRoot);
};

#endif
