#ifndef __FONT_THROUGHT_ANALYZER_H__
#define __FONT_THROUGHT_ANALYZER_H__
/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <strigi/streamthroughanalyzer.h>

class FontThroughAnalyzerFactory;
class QString;

class FontThroughAnalyzer : public Strigi::StreamThroughAnalyzer
{
    public:

    FontThroughAnalyzer(const FontThroughAnalyzerFactory *f);

    bool         checkHeader(const char *header, int32_t headersize) const;
    char         analyze(Strigi::AnalysisResult &idx, jstreams::InputStream *in);

    void                    setIndexable(Strigi::AnalysisResult* i) { analysisResult = i; }
    jstreams::InputStream * connectInputStream(jstreams::InputStream *in);
    bool                    isReadyWithStream()                     { return true; }

    private:

    void         add(const QString &family,  const QString &foundry, const QString &weight,
                     const QString &width,   const QString &spacing, const QString &slant,
                     const QString &version);

    private:

    const FontThroughAnalyzerFactory *factory;
    Strigi::AnalysisResult           *analysisResult;
};

#endif
