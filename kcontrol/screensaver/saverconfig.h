#ifndef SAVERCONFIG_H
#define SAVERCONFIG_H

#include <QString>

class SaverConfig
{
public:
    SaverConfig();

    bool read(const QString &file);

    QString exec() const { return mExec; }
    QString setup() const { return mSetup; }
    QString saver() const { return mSaver; }
    QString name() const { return mName; }
    QString file() const { return mFile; }
    QString category() const { return mCategory; }

protected:
    QString mExec;
    QString mSetup;
    QString mSaver;
    QString mName;
    QString mFile;
    QString mCategory;
};

#endif
