#ifndef __KFILE_FONT_H__
#define __KFILE_FONT_H__

//
// Need to use some files from KFontinst's main source dir, but don't want stuff that
// deals with global config...
#define KFI_METAINFO

#include <kfilemetainfo.h>
#include "FontEngine.h"

class KFileFontPlugin : public KFilePlugin
{
    public:

    KFileFontPlugin(QObject *parent, const char *name, const QStringList& args);
    virtual ~KFileFontPlugin();

    bool readInfo(KFileMetaInfo& info, uint what = KFileMetaInfo::Fastest);

    private:

    void addMimeType(const char *mime);

    private:

    CFontEngine itsFontEngine;
};

#endif
