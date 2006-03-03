/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __extensioninfo_h__
#define __extensioninfo_h__

#include <QMap>
#include <QVector>

#include <kdemacros.h>

class KDE_EXPORT ExtensionInfo
{
    public:
        typedef QVector<ExtensionInfo> List;
        typedef QMap<QObject*, ExtensionInfo*> Dict;
        enum ExtensionType { Undefined, Panel, Desktop };

        ExtensionInfo(const QString& desktopFile = QString(),
                      const QString& configFile = QString(),
                      const ExtensionType type = Undefined);

        virtual ~ExtensionInfo();
        ExtensionInfo(const ExtensionInfo& copy);
        ExtensionInfo &operator=(const ExtensionInfo& rhs);

        QString name() const;
        QString comment() const;
        QString icon() const;
	QString id() const;
	
        ExtensionType type() const;

        QString library() const;
        QString desktopFilePath() const;
        QString desktopFile() const;
        QString configFile() const;

        bool isUniqueExtension() const;
        bool isHidden() const;

        void setConfigFile(const QString &cf);

        bool operator<(const ExtensionInfo& rhs) const;
        bool operator>(const ExtensionInfo& rhs) const;
        bool operator<=(const ExtensionInfo& rhs) const;
        bool operator!=(const ExtensionInfo& rhs) const;
        bool operator==(const ExtensionInfo& rhs) const;

        void setType(ExtensionType type);

    protected:
        void setName(const QString &name);
        void setComment(const QString &comment);
        void setIcon(const QString &icon);
	void setId(const QString &id);
	void setLibrary(const QString &lib);
        void setIsUnique(bool u);

    private:
        class Private;
        Private *d;
};

#endif
