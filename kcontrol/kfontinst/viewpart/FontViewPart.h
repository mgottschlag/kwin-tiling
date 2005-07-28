#ifndef __FONT_VIEW_PART_H__
#define __FONT_VIEW_PART_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CFontViewPart
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 03/08/2002
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2002, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include <kparts/part.h>
#include <q3valuevector.h>
//Added by qt3to4:
#include <QLabel>
#include <Q3Frame>

class QPushButton;
class Q3Frame;
class QLabel;
class QStringList;
class KIntNumInput;
class KAction;
class KURL;

namespace KFI
{

class CFontPreview;
class CFcEngine;

class CFontViewPart : public KParts::ReadOnlyPart
{
    Q_OBJECT

    public:

    CFontViewPart(QWidget *parent=0, const char *name=0);
    virtual ~CFontViewPart() {}

    bool openURL(const KURL &url);

    protected:

    bool openFile();

    private slots:

    void previewStatus(bool st);
    void install();
    void changeText();
    void print();

    private:

    CFontPreview  *itsPreview;
    QPushButton   *itsInstallButton;
    Q3Frame        *itsFrame,
                  *itsToolsFrame;
    QLabel        *itsFaceLabel;
    KIntNumInput  *itsFaceSelector;
    KAction       *itsChangeTextAction,
                  *itsPrintAction;
    bool          itsShowInstallButton;
    int           itsFace;
};

}

#endif
