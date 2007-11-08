/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef FONTTHROUGHANALYZER_H
#define FONTTHROUGHANALYZER_H

#include <strigi/streamthroughanalyzer.h>

class FontThroughAnalyzerFactory;
class QString;

class FontThroughAnalyzer : public Strigi::StreamThroughAnalyzer
{
    public:

    FontThroughAnalyzer(const FontThroughAnalyzerFactory *f);

    char                  analyze(Strigi::AnalysisResult &idx, Strigi::InputStream *in);
    void                  setIndexable(Strigi::AnalysisResult *i) { analysisResult = i; }
    Strigi::InputStream * connectInputStream(Strigi::InputStream *in);
    bool                  isReadyWithStream()                     { return true; }

    private:

    void                  result(const QString &family,  const QString &foundry, const QString &weight,
                                 const QString &width,   const QString &spacing, const QString &slant,
                                 const QString &version, const QString &mime);
    const char * name() const { return "FontThroughAnalyzer"; }

    private:

    const FontThroughAnalyzerFactory *factory;
    Strigi::AnalysisResult           *analysisResult;
};

#endif // FONTTHROUGHANALYZER_H
