#ifndef __XFT_CONFIG_EDITOR_H__
#define __XFT_CONFIG_EDITOR_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXftConfigEditor
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 18/06/2001
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
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "XftConfigEditorData.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_XFT
#include <qvalidator.h>
#include <qstring.h>
#include "XftConfig.h"

class QWidget;
class QComboBox;
class QLineEdit;

class CXftConfigEditor : public CXftConfigEditorData
{
    private:

    enum EMode
    {
        COMBO,
        LINEEDIT
    };

    class CStringValidator : public QValidator
    {
        public:
 
        CStringValidator(QWidget *widget, bool allowDash=true) : QValidator(widget), itsAllowDash(allowDash) {}
        virtual ~CStringValidator()                                                                          {}
 
        State validate(QString &input, int &) const;
 
        private:
 
        bool itsAllowDash;
    };

    struct TValidators
    {
        TValidators(QLineEdit *lineedit, QLineEdit *other);

        CStringValidator *lineeditStrNoDash,
                         *otherStr;
        QIntValidator    *lineeditInt,
                         *otherInt;
        QDoubleValidator *lineeditDouble;
    };
   
    public:

    CXftConfigEditor(QWidget *parent, const char *name=NULL);
    virtual ~CXftConfigEditor() {}
  
    CXftConfig::TEntry * display(CXftConfig::TEntry *entry); 
    void                 matchFieldSelected(const QString &str);
    void                 matchCombo(const QString &str);
    void                 editCombo(const QString &str);
    void                 editFieldSelected(const QString &str);
    void                 addMatch();
    void                 removeMatch();
    void                 matchSelected(QListBoxItem *item);

    private:

    void                 setWidgets(QComboBox *combo, QLineEdit *lineedit, const QString &str, EMode &mode, bool edit, TValidators &validators);
    bool                 getValue(XftValue &val, const QString &field, const QString &strVal, bool edit);

    private:

    EMode       itsMatchMode,
                itsEditMode;
    QStringList itsBooleans,
                itsEncodings,
                itsRgbs,
                itsSlants,
                itsSpacings,
                itsWeights;
    TValidators itsMatchValidators,
                itsEditValidators;
    QString     itsOtherText;
};

#endif

#endif
