////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXftConfigEditor
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 23/06/2001
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

#include "XftConfigEditor.h"

#ifdef HAVE_XFT
#include "FontEngine.h"
#include "KfiGlobal.h"
#include "Encodings.h"
#include "Misc.h"
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qvalidator.h>
#include <kmessagebox.h>
#include <stdlib.h>
#include <klocale.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

static const QString constOther ("Other...");

static XftOp strToOp(const QString &op)
{
    if(">"==op)
        return XftOpMore;
    else if("<"==op)
        return XftOpLess;
    else if("+="==op)
        return XftOpPrepend;
    else if("=+"==op)
        return XftOpAppend;
    else if("="==op)
        return XftOpAssign;
    else
        return XftOpEqual;
}

QValidator::State CXftConfigEditor::CStringValidator::validate(QString &input, int &) const
{
    if((itsAllowDash && (input.contains(QRegExp("[\"\t]")) || input.contains('-')>1)) || (!itsAllowDash && input.contains(QRegExp("[\"\t-]"))))
        return Invalid;
    else
        return Valid;
}

CXftConfigEditor::TValidators::TValidators(QLineEdit *lineedit, QLineEdit *other)
{
    lineeditStrNoDash=new CStringValidator(lineedit, false);
    otherStr=new CStringValidator(other);
    lineeditInt=new QIntValidator(lineedit);
    otherInt=new QIntValidator(other);
    lineeditDouble=new QDoubleValidator(lineedit);
}

CXftConfigEditor::CXftConfigEditor(QWidget *parent, const char *name)
                : CXftConfigEditorData(parent, name, true)
                , itsMatchMode(COMBO),
                  itsEditMode(COMBO),
                  itsMatchValidators(itsMatchString, itsMatchOther),
                  itsEditValidators(itsEditString, itsEditOther)
{
    itsMatchQualCombo->insertItem("all");
    itsMatchQualCombo->insertItem("any");
    itsMatchFieldNameCombo->insertItem("foundry");
    itsMatchFieldNameCombo->insertItem("encoding");
    itsMatchFieldNameCombo->insertItem("spacing");
    itsMatchFieldNameCombo->insertItem("bold");
    itsMatchFieldNameCombo->insertItem("italic");
    itsMatchFieldNameCombo->insertItem("antialias");
    itsMatchFieldNameCombo->insertItem("family");
    itsMatchFieldNameCombo->insertItem("size");
    itsMatchFieldNameCombo->insertItem("style");
    itsMatchFieldNameCombo->insertItem("slant");
    itsMatchFieldNameCombo->insertItem("weight");
    itsMatchFieldNameCombo->insertItem("outline");
    itsMatchCompareCombo->insertItem("==");
    itsMatchCompareCombo->insertItem(">");
    itsMatchCompareCombo->insertItem("<");
    itsEditFieldNameCombo->insertItem("foundry");
    itsEditFieldNameCombo->insertItem("encoding");
    itsEditFieldNameCombo->insertItem("spacing");
    itsEditFieldNameCombo->insertItem("bold");
    itsEditFieldNameCombo->insertItem("italic");
    itsEditFieldNameCombo->insertItem("antialias");
    itsEditFieldNameCombo->insertItem("family");
    itsEditFieldNameCombo->insertItem("size");
    itsEditFieldNameCombo->insertItem("style");
    itsEditFieldNameCombo->insertItem("slant");
    itsEditFieldNameCombo->insertItem("weight");
    itsEditFieldNameCombo->insertItem("outline");
    itsEditFieldNameCombo->insertItem("pixelsize");
    itsEditFieldNameCombo->insertItem("charspace");
    itsEditFieldNameCombo->insertItem("minspace");
    itsEditFieldNameCombo->insertItem("rgba");
    itsEditFieldNameCombo->insertItem("xlfd");
    itsEditFieldNameCombo->insertItem("file");
    itsEditFieldNameCombo->insertItem("core");
    itsEditFieldNameCombo->insertItem("render");
    itsEditFieldNameCombo->insertItem("index");
    itsEditFieldNameCombo->insertItem("scalable");
    itsEditFieldNameCombo->insertItem("scale");
    itsEditFieldNameCombo->insertItem("charwidth");
    itsEditFieldNameCombo->insertItem("charheight");
    itsEditFieldNameCombo->insertItem("matrix");
    itsEditAssignCombo->insertItem("=");
    itsEditAssignCombo->insertItem("+=");
    itsEditAssignCombo->insertItem("=+");
    itsBooleans.append("true");
    itsBooleans.append("false");
    itsRgbs.append("rgb");
    itsRgbs.append("bgr");
//    itsRgbs.append("vrgb");
//    itsRgbs.append("vbgr");
    itsRgbs.append(constOther);
    itsSlants.append("italic");
    itsSlants.append("oblique");
    itsSlants.append("roman");
    itsSpacings.append("proportional");
    itsSpacings.append("mono");
    itsSpacings.append("charcell");
    itsSpacings.append(constOther);
    itsWeights.append("light");
    itsWeights.append("medium");
    itsWeights.append("demibold");
    itsWeights.append("bold");
    itsWeights.append("black");

    itsWeights.append(constOther);

    itsMatchOther->setValidator(itsMatchValidators.otherInt);
    itsEditOther->setValidator(itsEditValidators.otherInt);
}

CXftConfig::TEntry * CXftConfigEditor::display(CXftConfig::TEntry *entry)
{
    const CEncodings::T8Bit  *enc8;
    const CEncodings::T16Bit *enc16;

    itsEncodings.clear();
    itsEncodings.append("iso10646-1");
    for(enc8=CKfiGlobal::enc().first8Bit(); NULL!=enc8; enc8=CKfiGlobal::enc().next8Bit())
        itsEncodings.append(enc8->name);
 
    for(enc16=CKfiGlobal::enc().first16Bit(); NULL!=enc16; enc16=CKfiGlobal::enc().next16Bit())
        itsEncodings.append(enc16->name);

    itsEncodings.append(CXftConfig::constSymbolEncoding);
    itsEncodings.sort();
    itsEncodings.append(constOther);

    itsMatchList->clear();
    itsMatchRemoveButton->setEnabled(false);

    if(entry)
    {
        int index;

        //
        // Set up "edit" widgets...
        if(entry->edit && entry->edit->field)
        {
            QString str(entry->edit->field);

            index=CMisc::findIndex(itsEditFieldNameCombo, entry->edit->field);
            itsEditFieldNameCombo->setCurrentItem(index>-1 ? index : 0);
            editFieldSelected(entry->edit->field);

            switch(entry->edit->op)
            {
                case XftOpPrepend:
                    index=CMisc::findIndex(itsEditAssignCombo, "+=");
                    break;
                case XftOpAppend:
                    index=CMisc::findIndex(itsEditAssignCombo, "=+");
                    break;
                case XftOpAssign:
                    index=CMisc::findIndex(itsEditAssignCombo, "=");
                default:
                    break;
            }

            itsEditAssignCombo->setCurrentItem(index>-1 ? index : 0);

            if(entry->edit->expr)
            {
                if(("foundry"==str || "family"==str || "style"==str) && XftOpString==entry->edit->expr->op)
                    itsEditString->setText(entry->edit->expr->u.sval);
                else if("encoding"==str && XftOpString==entry->edit->expr->op)
                {
                    index=CMisc::findIndex(itsEditCombo, entry->edit->expr->u.sval);
                    if(index<0)
                    {
                        index=CMisc::findIndex(itsEditCombo, constOther);
                        itsEditOther->setText(entry->edit->expr->u.sval);
                        itsEditOther->setEnabled(true);
                    }
                    itsEditCombo->setCurrentItem(index>-1 ? index : 0);
                }
                else if( ("bold"==str || "italic"==str || "antialias"==str || "outline"==str ||
                          "core"==str || "render"==str || "scalable"==str) && XftOpBool==entry->edit->expr->op)
                {
                    index=CMisc::findIndex(itsEditCombo, entry->edit->expr->u.bval ? "true" : "false");
                    itsEditCombo->setCurrentItem(index>-1 ? index : 0);
                }
                else if("size"==str && XftOpDouble==entry->edit->expr->op)
                {
                    QString num;

                    num.setNum(entry->edit->expr->u.dval);
                    itsEditString->setText(num);
                }
                else if(("rgba"==str || "slant"==str || "spacing"==str || "weight"==str) && (XftOpInteger==entry->edit->expr->op))
                {
                    bool doString=false;

                    if ("rgba"==str)
                        switch(entry->edit->expr->u.ival)
                        {
                            case XFT_RGBA_NONE:
                                index=CMisc::findIndex(itsEditCombo, "none");
                                break;
                            case XFT_RGBA_RGB:
                                index=CMisc::findIndex(itsEditCombo, "rgb");
                                break;
                            case XFT_RGBA_BGR:
                                index=CMisc::findIndex(itsEditCombo, "bgr");
                                break;
                            default:
                                doString=true;
                        }
                    else if("spacing"==str)
                        switch(entry->edit->expr->u.ival)
                        {
                            case XFT_PROPORTIONAL:
                                index=CMisc::findIndex(itsEditCombo, "proportional");
                                break;
                            case XFT_MONO:
                                index=CMisc::findIndex(itsEditCombo, "mono");
                                break;
                            case XFT_CHARCELL:
                                index=CMisc::findIndex(itsEditCombo, "charcell");
                                break;
                            default:
                                doString=true;
                        }
                    else if("weight"==str)
                        switch(entry->edit->expr->u.ival)
                        {
                            case XFT_WEIGHT_LIGHT:
                                index=CMisc::findIndex(itsEditCombo, "light");
                                break;
                            case XFT_WEIGHT_MEDIUM:
                                index=CMisc::findIndex(itsEditCombo, "medium");
                                break;
                            case XFT_WEIGHT_DEMIBOLD:
                                index=CMisc::findIndex(itsEditCombo, "demibold");
                                break;
                            case XFT_WEIGHT_BOLD:
                                index=CMisc::findIndex(itsEditCombo, "bold");
                                break;
                            case XFT_WEIGHT_BLACK:
                                index=CMisc::findIndex(itsEditCombo, "black");
                                break;
                            default:
                                doString=true;
                        }
                    else if("slant"==str)
                        switch(entry->edit->expr->u.ival)
                        {
                            case XFT_SLANT_ROMAN:
                                index=CMisc::findIndex(itsEditCombo, "roman");
                                break;
                            case XFT_SLANT_ITALIC:
                                index=CMisc::findIndex(itsEditCombo, "italic");
                                break;
                            case XFT_SLANT_OBLIQUE:
                                index=CMisc::findIndex(itsEditCombo, "oblique");
                                break;
                            default:
                                doString=true;
                        }
                    else
                        doString=true;

                    if(doString)
                    {
                        QString num;

                        num.setNum(entry->edit->expr->u.ival);
                        itsEditOther->setText(num);
                        itsEditOther->setEnabled(true);
                        index=CMisc::findIndex(itsEditCombo, constOther);
                    }
                    itsEditCombo->setCurrentItem(index>-1 ? index : 0);
                }
                else if( ("index"==str || "pixelsize"==str || "charspace"==str || "minspace"==str || "scale"==str || "charwidth"==str || "charheight"==str)
                         && XftOpInteger==entry->edit->expr->op)
                {
                    QString num;
 
                    num.setNum(entry->edit->expr->u.ival);
                    itsEditString->setText(num);
                }
                else if("matrix"==str)
                {
                    QString str;

                    switch(entry->edit->expr->op)
                    {
                        case XftOpString:
                            str=entry->edit->expr->u.sval;
                            break;
                        case XftOpDouble:
                            str.setNum(entry->edit->expr->u.dval);
                            break;
                        case XftOpInteger:
                            str.setNum(entry->edit->expr->u.ival);
                            break;
                        case XftOpBool:
                            str=entry->edit->expr->u.bval ? "true" : "false";
                            break;
                        default:
                            break;
                    }
                 
                    itsEditString->setText(str); 
                }
            }
        }
        else
           editFieldSelected(itsEditFieldNameCombo->text(0));

        //
        // Setup "match" list...
        if(entry->test && entry->test->field)
        {
            //
            // Easist way is to get string, and convert each line to an entry in the list...

            QCString str=entry->testStr();
            int      start=1,
                     end=0;

            while(-1!=(end=str.find('\n', start)) || -1!=(end=str.find('\0', start)))
            {
                QString match=str.mid(start, end-start);

                itsMatchList->insertItem(match.simplifyWhiteSpace());
                start=end+1;
            }

            index=CMisc::findIndex(itsMatchFieldNameCombo, entry->test->field);
        }

        itsMatchFieldNameCombo->setCurrentItem(0);
        matchFieldSelected(itsMatchFieldNameCombo->text(0));
    }
    else
    {
        itsMatchFieldNameCombo->setCurrentItem(0);
        matchFieldSelected(itsMatchFieldNameCombo->text(0));
        itsEditFieldNameCombo->setCurrentItem(0);
        editFieldSelected(itsEditFieldNameCombo->text(0));
    }

    if(QDialog::Accepted==exec() && (LINEEDIT!=itsEditMode ||
                                    (LINEEDIT==itsEditMode && itsEditString->text().length())))
    {
        //
        // "OK" has been selected, copy data from widgets into a CXftConfig::TEntry

        //
        // First do the "match" items...
        unsigned int match;
        CXftConfig::TEntry *retVal=new CXftConfig::TEntry;
        XftTest            **tst=&(retVal->test);
        XftValue           val;

        for(match=0; match<itsMatchList->count(); ++match)
        {
            int start=0,
                end=0;

            if(-1!=(end=itsMatchList->text(match).find(' ', start)))
            {
                XftQual qual=("any"==itsMatchList->text(match).mid(start, end-start)) ? XftQualAny : XftQualAll;

                start=end+1;
                if(-1!=(end=itsMatchList->text(match).find(' ', start)))
                {
                    QString field(itsMatchList->text(match).mid(start, end-start));
                    start=end+1;

                    if(-1!=(end=itsMatchList->text(match).find(' ', start)))
                    {
                        QString  op(itsMatchList->text(match).mid(start, end-start)),
                                 value=itsMatchList->text(match).mid(end+1);

                        if(getValue(val, field, value, false))
                        {
                            *tst=XftTestCreate(qual, field.local8Bit(), strToOp(op), val);
                            tst=&((*tst)->next);
                        }
                    }
                }
            }
        }

        //
        // Now do the "edit" stuff...
        QString strVal;

        if(LINEEDIT==itsEditMode)
            strVal=itsEditString->text();
        else
            if(constOther==itsEditCombo->currentText())
                strVal=itsEditOther->text();
            else
                strVal=itsEditCombo->currentText();

        if(getValue(val, itsEditFieldNameCombo->currentText(), strVal, true))
        {
            XftExpr *expr=new XftExpr;

            switch(val.type)
            {
                case XftTypeInteger:
                    expr->op=XftOpInteger;
                    expr->u.ival=val.u.i;
                    break;
                case XftTypeDouble:
                    expr->op=XftOpDouble;
                    expr->u.dval=val.u.d;
                    break;
                case XftTypeString:
                    expr->op=XftOpString;
                    expr->u.sval=val.u.s;
                    break;
                case XftTypeBool:
                default:
                    expr->op=XftOpBool;
                    expr->u.bval=val.u.b;
                    break;
            }

            retVal->edit=XftEditCreate(itsEditFieldNameCombo->currentText().local8Bit(), strToOp(itsEditAssignCombo->currentText()), expr);
        }

        return retVal;
    }
    else
        return NULL;
}

void CXftConfigEditor::matchFieldSelected(const QString &str)
{
    setWidgets(itsMatchCombo, itsMatchString, str, itsMatchMode, false, itsMatchValidators);

    itsMatchOther->setEnabled(false);
    if("encoding"==str)
        itsMatchOther->setValidator(itsMatchValidators.otherStr);
    else
        itsMatchOther->setValidator(itsMatchValidators.otherInt);
}

void CXftConfigEditor::matchCombo(const QString &str)
{
    itsMatchOther->setEnabled(constOther==str);
    if(constOther==str)
        itsMatchOther->setText("");
}

void CXftConfigEditor::editCombo(const QString &str)
{
    itsEditOther->setEnabled(constOther==str);
    if(constOther==str)
        itsEditOther->setText("");
}

void CXftConfigEditor::editFieldSelected(const QString &str)
{
    setWidgets(itsEditCombo, itsEditString, str, itsEditMode, true, itsEditValidators);

    itsEditOther->setEnabled(false);
    if("encoding"==str)
        itsEditOther->setValidator(itsEditValidators.otherStr);
    else
        itsEditOther->setValidator(itsEditValidators.otherInt);
}

void CXftConfigEditor::addMatch()
{
    if((LINEEDIT==itsMatchMode && 0==itsMatchString->text().length()) ||
       (itsMatchOther->isEnabled() && 0==itsMatchOther->text().length()))
        KMessageBox::error(this, i18n("String is empty!"), i18n("Format error"));
    else
    {
        QCString entry(itsMatchQualCombo->currentText().latin1());

        entry+=' ';
        entry+=itsMatchFieldNameCombo->currentText().latin1();
        entry+=' ';
        entry+=itsMatchCompareCombo->currentText().latin1();
        entry+=' ';

        if(LINEEDIT==itsMatchMode)                         // Get from "match" string...
        {
            entry+='\"';
            entry+=itsMatchString->text().latin1();
            entry+='\"';
        }
        else
        {
            bool enc="encoding"==itsMatchFieldNameCombo->currentText();

            if(enc)
                entry+='\"';

            if(constOther==itsMatchCombo->currentText())   // Get from "other" text field...
                entry+=itsMatchOther->text().latin1();
            else                                           // Get from combo box...
                entry+=itsMatchCombo->currentText().latin1();

            if(enc)
                entry+='\"';
        }

        if(NULL==itsMatchList->findItem(entry))
            itsMatchList->insertItem((QString)entry);
        else
            KMessageBox::error(this, i18n("Entry is already in list"), i18n("Duplicate"));
    }
}

void CXftConfigEditor::removeMatch()
{
    if(-1!=itsMatchList->currentItem())
        itsMatchList->removeItem(itsMatchList->currentItem());

    itsMatchRemoveButton->setEnabled(false);
}

void CXftConfigEditor::matchSelected(QListBoxItem *item)
{
    if(item)
        itsMatchRemoveButton->setEnabled(true);
}

void CXftConfigEditor::setWidgets(QComboBox *combo, QLineEdit *lineedit, const QString &str, EMode &mode, bool edit, TValidators &validators)
{
    combo->clear();
    combo->hide();
    lineedit->setText("");
    lineedit->setValidator(NULL);
    lineedit->hide();
    mode=COMBO;

    if("foundry"==str || "family"==str || "style"==str)
    {
        lineedit->show();
        lineedit->setValidator(validators.lineeditStrNoDash);
        mode=LINEEDIT;
    }
    else if("encoding"==str)
    {
        combo->insertStringList(itsEncodings);
        combo->show();
    }
    else if("spacing"==str)
    {
        combo->show();
        combo->insertStringList(itsSpacings);
    }
    else if("bold"==str || "italic"==str || "antialias"==str || "outline"==str ||
            (edit && ("core"==str || "render"==str || "scalable"==str)))
    {
        combo->show();
        combo->insertStringList(itsBooleans);
    }
    else if("size"==str)
    {
        lineedit->show();
        lineedit->setValidator(validators.lineeditDouble);
        mode=LINEEDIT;
    }
    else if("slant"==str)
    {
        combo->show();
        combo->insertStringList(itsSlants);
    }
    else if("weight"==str)
    {
        combo->show();
        combo->insertStringList(itsWeights);
    }
    else if(edit)
        if("index"==str || "pixelsize"==str || "charspace"==str || "minspace"==str || "scale"==str || "charwidth"==str || "charheight"==str)
        {
            lineedit->show();
            lineedit->setValidator(validators.lineeditInt);
            mode=LINEEDIT;
        }
        else if("matrix"==str)
        {
            lineedit->show();
            lineedit->setValidator(validators.lineeditStrNoDash);
            mode=LINEEDIT;
        }
        else if("rgba"==str)
        {
            combo->show();
            combo->insertStringList(itsRgbs);
        }
}

bool CXftConfigEditor::getValue(XftValue &val, const QString &field, const QString &strVal, bool edit)
{
    bool retVal=true;

    if("foundry"==field || "family"==field || "encoding"==field || "style"==field)
    {
        val.type=XftTypeString;
        val.u.s=(char *)malloc(edit ? strVal.length()+1 : strVal.length()-1);

        if(edit)
            strcpy(val.u.s, strVal.local8Bit());
        else
            strcpy(val.u.s, strVal.mid(1, strVal.length()-2).local8Bit());  // Need to remove quotes from each end of the string...
    }
    else if("spacing"==field)
    {
        int v=0;

        if(constOther!=strVal && -1!=itsSpacings.findIndex(strVal))
        {
            if("mono"==strVal)
                v=XFT_MONO;
            else if("proportional"==strVal)
                v=XFT_PROPORTIONAL;
            else if("charcell"==strVal)
                v=XFT_CHARCELL;
        }
        else
            if(constOther==strVal) // Its an integer...
                v=strVal.toInt();

        val.type=XftTypeInteger;
        val.u.i=v;
    }
    else if("bold"==field || "italic"==field || "antialias"==field || "outline"==field ||
            (edit && ("core"==field || "render"==field || "scalable"==field)))
    {
        val.type=XftTypeBool;
        val.u.b="true"==strVal ? True : False;
    }
    else if("size"==field)
    {
        val.type=XftTypeDouble;
        val.u.d=strVal.toDouble();
    }
    else if("slant"==field)
    {
        int v=0;
 
        if(constOther!=strVal && -1!=itsSlants.findIndex(strVal))
        {
            if("roman"==strVal)
                v=XFT_SLANT_ROMAN;
            else if("italic"==strVal)
                v=XFT_SLANT_ITALIC;
            else if("oblique"==strVal)
                v=XFT_SLANT_OBLIQUE;
        }
        else
            if(constOther==strVal) // Its an integer...
                v=strVal.toInt();
 
        val.type=XftTypeInteger;
        val.u.i=v;
    }
    else if("weight"==field)
    {
        int v=0;
 
        if(constOther!=strVal && -1!=itsWeights.findIndex(strVal))
        {
            if("light"==strVal)
                v=XFT_WEIGHT_LIGHT;
            else if("medium"==strVal)
                v=XFT_WEIGHT_MEDIUM;
            else if("demibold"==strVal)
                v=XFT_WEIGHT_DEMIBOLD;
            else if("bold"==strVal)
                v=XFT_WEIGHT_BOLD;
            else if("black"==strVal)
                v=XFT_WEIGHT_BLACK;
        }
        else
            if(constOther==strVal) // Its an integer...
                v=strVal.toInt();

        val.type=XftTypeInteger;
        val.u.i=v;
    }
    else if(edit)
        if("index"==field || "pixelsize"==field || "charspace"==field || "minspace"==field || "scale"==field || "charwidth"==field || "charheight"==field)
        {
            val.type=XftTypeInteger;
            val.u.i=strVal.toInt();
        }
        else if("matrix"==field)
        {
            // Not sure about this...
            val.type=XftTypeString;
            val.u.s=(char *)malloc(strVal.length()+1);
            strcpy(val.u.s, strVal.local8Bit());
        }
        else if("rgba"==field)
        {
            int v=0;
 
            if(constOther!=strVal)
            {
                if("none"==strVal)
                    v=XFT_RGBA_NONE;
                else if("rgb"==strVal)
                    v=XFT_RGBA_RGB;
                else if("bgr"==strVal)
                    v=XFT_RGBA_BGR;
            }
            else
                if(constOther==strVal) // Its an integer...
                    v=strVal.toInt();
 
            val.type=XftTypeInteger;
            val.u.i=v;
        }
        else
            retVal=false;

    return retVal;
}
#endif
