#ifndef __PRINT_DIALOG_H__
#define __PRINT_DIALOG_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CPrintDialog
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 12/05/2005
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
// (C) Craig Drummond, 2005
////////////////////////////////////////////////////////////////////////////////

#include <kdialogbase.h>
#include <qcombobox.h>

namespace KFI
{

class CPrintDialog : public KDialogBase
{
    public:

    CPrintDialog(QWidget *parent);

    bool exec(bool select, int size);
    int  chosenSize() { return itsSize->currentItem(); }
    bool outputAll()  { return 0==itsOutput->currentItem(); }

    private:

    QComboBox *itsOutput,
              *itsSize;
};

}

#endif
