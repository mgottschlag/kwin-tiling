////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXftConfig
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 05/06/2001
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

#include "Misc.h"
#include "XftConfig.h"

#ifdef HAVE_XFT
#include <qstring.h>
#include <fstream.h>
#include <stdlib.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

using namespace std;

static CXftConfig *xft;  // Needed so C functions can access class instance...

extern "C"
{
#ifndef YYPARSE_RETURN_TYPE
#define YYPARSE_RETURN_TYPE int
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
YYPARSE_RETURN_TYPE
XftConfigparse (void *);
#else
YYPARSE_RETURN_TYPE
XftConfigparse (void);
#endif
#endif

#define _XftStrCmpIgnoreCase(A,B) CMisc::stricmp(A,B)

// The following have been copied from xftname.c

typedef struct _XftConstant {
    const char  *name;
    const char  *object;
    int         value;
} XftConstant;
 
static XftConstant XftConstants[] = {
    { "light",          "weight",   XFT_WEIGHT_LIGHT, },
    { "medium",         "weight",   XFT_WEIGHT_MEDIUM, },
    { "demibold",       "weight",   XFT_WEIGHT_DEMIBOLD, },
    { "bold",           "weight",   XFT_WEIGHT_BOLD, },
    { "black",          "weight",   XFT_WEIGHT_BLACK, },
 
    { "roman",          "slant",    XFT_SLANT_ROMAN, },
    { "italic",         "slant",    XFT_SLANT_ITALIC, },
    { "oblique",        "slant",    XFT_SLANT_OBLIQUE, },
 
    { "proportional",   "spacing",  XFT_PROPORTIONAL, },
    { "mono",           "spacing",  XFT_MONO, },
    { "charcell",       "spacing",  XFT_CHARCELL, },
 
    { "rgb",            "rgba",     XFT_RGBA_RGB, },
    { "bgr",            "rgba",     XFT_RGBA_BGR, },
};
 
#define NUM_XFT_CONSTANTS   (sizeof XftConstants/sizeof XftConstants[0])
 
static XftConstant *
_XftNameConstantLookup (char *string)
{
    unsigned int i;
 
    for (i = 0; i < NUM_XFT_CONSTANTS; i++)
        if (!_XftStrCmpIgnoreCase (string, XftConstants[i].name))
            return &XftConstants[i];
    return 0;
}
 
Bool
XftNameConstant (char *string, int *result)
{
    XftConstant *c;
 
    if ((c = _XftNameConstantLookup(string)))
    {
        *result = c->value;
        return True;
    }
    return False;
}

char *
_XftSaveString (const char *s)
{
    char    *r;
 
    r = (char *) malloc (strlen (s) + 1);
    if (!r)
        return 0;
    strcpy (r, s);
    return r;
}

// Re-defined functions...
Bool
XftConfigAddDir (char *)
{
    /* Ignore any dirs in the file... All dirs in X11 directory containing Type1 or TrueType fonts will be added */
    return True;
}

Bool
XftConfigAddEdit (XftTest *test, XftEdit *edit)
{
    xft->addEntry(test, edit);
 
    return True;
}

Bool
XftConfigPushInput (char *s, Bool complain)
{
    if(complain)
        xft->addInclude(s);
    else
        xft->addIncludeIf(s);

    return True;
}

// Output functions...

static const char * opToStr(XftOp op)
{
    switch(op)
    {
        case XftOpInteger:
            return "integer";
            break;
        case XftOpDouble:
            return "double";
            break;
        case XftOpString:
            return "string";
            break;
        case XftOpBool:
            return "bool";
            break;
        case XftOpNil:
            return "nil";
            break;
        case XftOpField:
            return "Field";
            break;
        case XftOpAssign:
            return "=";
            break;
        case XftOpPrepend:
            return "+=";
            break;
        case XftOpAppend:
            return "=+";
            break;
        case XftOpQuest:
            return "?";
            break;
        case XftOpOr:
            return "||";
            break;
        case XftOpAnd:
            return "&&";
            break;
        case XftOpEqual:
            return "==";
            break;
        case XftOpNotEqual:
            return "!=";
            break;
        case XftOpLess:
            return "<";
            break;
        case XftOpLessEqual:
            return "<=";
            break;
        case XftOpMore:
            return ">";
            break;
        case XftOpMoreEqual:
            return ">=";
            break;
        case XftOpPlus:
            return "+";
            break;
        case XftOpMinus:
            return "-";
            break;
        case XftOpTimes:
            return "*";
            break;
        case XftOpDivide:
            return "/";
            break;
        case XftOpNot:
            return "!";
            break;
        default:
            return "unk";
    }
}

static inline void printBool(QCString &str, Bool b)
{
    str+= (const char *)(b ? "true" : "false");
}

static void printInteger(QCString &str, char *field, int ival)
{
    unsigned int i;

    if(field)
        for(i=0; i<NUM_XFT_CONSTANTS; i++)
            if(!CMisc::stricmp(field, XftConstants[i].object) && XftConstants[i].value==ival)
            {
                str+=XftConstants[i].name;
                return;
            }

    QCString num;
    num.setNum(ival);
    str+=num;
}

static void printTest(QCString &str, XftTest *test)
{
    if(test)
    {
        str+="\n";
        str+="    ";
        str+=(const char *)(test->qual==XftQualAny ? "any" : "all");
        str+=" ";
        str+=test->field;
        str+=" ";
        str+=opToStr(test->op);
        str+=" ";
        switch(test->value.type)
        {
            case XftTypeVoid:
                str+="void";
                break;
            case XftTypeInteger:
                printInteger(str, test->field, test->value.u.i);
                break;
            case XftTypeDouble:
            {
                QCString num;
                num.setNum(test->value.u.d);
                str+=num;
                break;
            }
            case XftTypeString:
                str+="\"";
                str+=test->value.u.s;
                str+="\"";
                break;
            case XftTypeBool:
                printBool(str, test->value.u.b);
                break;
            default:
                str+="unk";
        }

        str+=" ";
        printTest(str, test->next);
    }
}

static void printExpr(QCString &str, XftExpr *expr, const char *field=NULL)
{
    if(expr)
    {
        QCString num;

        str+=" ";
        switch(expr->op)
        {
            case XftOpInteger:
                printInteger(str, (char *)field, expr->u.ival);
                break;
            case XftOpDouble:
                num.setNum(expr->u.dval);
                str+=num;
                break;
            case XftOpString:
                str+="\"";
                str+=expr->u.sval;
                str+="\"";
                break;
            case XftOpBool:
                printBool(str, expr->u.bval);
                break;
            case XftOpNil:
                str+="Nil";
                break;
            case XftOpField:
                str+="\"";
                str+=expr->u.field;
                str+="\"";
                break;
            case XftOpAssign:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpPrepend:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpAppend:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpQuest:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpOr:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpAnd:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpEqual:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpNotEqual:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpLess:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpLessEqual:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpMore:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpMoreEqual:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpPlus:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpMinus:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpTimes:
                printExpr(str, expr->u.tree.left);
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpDivide:
                printExpr(str, expr->u.tree.left);
                str+="/";
                printExpr(str, expr->u.tree.right);
                break;
            case XftOpNot:
                str+=opToStr(expr->op);
                printExpr(str, expr->u.tree.left);
            default:
                str+="Unk";
        }
    }
}

static void printEdit(QCString &str, XftEdit *edit)
{
    if(edit)
    {
        str+="\n";
        str+="    ";
        str+=edit->field;
        str+=" ";
        str+=opToStr(edit->op);
        printExpr(str, edit->expr, edit->field);
        str+="\n";
        printEdit(str, edit->next);
    }
}

}   // extern "C"

// Class based functions...

CXftConfig::~CXftConfig()
{
    CXftConfig::TEntry *entry;
 
    for(entry=itsList.first(); entry; entry=itsList.next())
        delete entry;
}

void CXftConfig::init()
{
    itsMadeChanges=false;
    itsList.clear();
}

bool CXftConfig::read(const QString &f)
{
    bool retVal=false;

    if(CMisc::fExists(f.latin1()))
    {
        init();
        xft=this;
        XftConfigLexFile((char *)f.latin1());
        retVal=XftConfigparse() ? false : true;
    }
    else
        if(CMisc::dWritable(CMisc::getDir(f)))
        {
            init();
            retVal=true;
        }
        else
            retVal=false;

    return retVal;
}

bool CXftConfig::save(const QString &f, const QStringList &dirs)
{
    ofstream of(f.latin1());

    if(of)
    {
        TEntry                *entry;
        QStringList::Iterator sIt;

        itsMadeChanges=false;

        of << "##############################" << endl
           << "# XRender configuration file #" << endl
           << "##############################" << endl;

        // Output every dir containing a TrueType or Type1 font...
        if(dirs.count())
        {
            of << endl
               << '#' << endl
               << "# Directories containing fonts to anti-alias" << endl
               << '#' << endl;

            for(sIt=((QStringList &)dirs).begin(); sIt!=((QStringList &)dirs).end(); ++sIt)
            {
                QString dir(*sIt);

                dir.remove(dir.length()-1, 1); // Remove trailing /
                of << "dir \"" << dir.latin1() << '\"' << endl;
            }
        }

        // Ouput any includes...
        if(itsIncludes.count())
        {
            of << endl
               << '#' << endl
               << "# Include other configuration files, and complain if missing" << endl
               << '#' << endl;
            for(sIt=itsIncludes.begin(); sIt!=itsIncludes.end(); ++sIt)
                of << "include \"" << (*sIt).latin1() << '\"' << endl;
        }

        // Ouput any includes...
        if(itsIncludeIfs.count())
        {
            of << endl
               << '#' << endl
               << "# Include other configuration files, but don't complain if missing" << endl
               << '#' << endl;
            for(sIt=itsIncludeIfs.begin(); sIt!=itsIncludeIfs.end(); ++sIt)
                of << "includeif \"" << (*sIt).latin1() << '\"' << endl;
        }

        // Output the entry details... 
        if(itsList.count())
        {
            of << endl
               << '#' << endl
               << "# Configuration patterns" << endl
               << '#' << endl;
            for(entry=itsList.first(); entry; entry=itsList.next())
                entry->output(of);
        }

        of.close();

        return true;
    }
    else
        return false;
}

void CXftConfig::addEntry(XftTest *test, XftEdit *edit)
{
    itsList.append(new TEntry(test, edit));
}

void CXftConfig::addInclude(const char *dir)
{
    if(itsIncludes.findIndex(dir)==-1)
        itsIncludes.append(dir);
}

void CXftConfig::addIncludeIf(const char *dir)
{
    if(itsIncludeIfs.findIndex(dir)==-1)
        itsIncludeIfs.append(dir);
}

bool CXftConfig::getExcludeRange(double &from, double &to)
{
    TEntry *entry=getExcludeRangeEntry();

    if(entry)
    {
        double first=XftTypeDouble==entry->test->value.type ? entry->test->value.u.d : entry->test->value.u.i,
               second=entry->test->value.type ? entry->test->next->value.u.d : entry->test->next->value.u.i;

        if(first<second)
        {
            from=first;
            to=second;
        }
        else
        {
            from=second;
            to=first;
        }
        return true;
    }
    else
        return false;
}

void CXftConfig::setExcludeRange(double from, double to)
{
    TEntry *entry=getExcludeRangeEntry();

    if(entry)
    {
        entry->test->value.type=XftTypeDouble;
        entry->test->value.u.d=from;
        entry->test->op=XftOpMore;
        entry->test->next->value.type=XftTypeDouble;
        entry->test->next->value.u.d=to;
        entry->test->next->op=XftOpLess;
    }
    else
    {
        XftValue value;

        entry=new TEntry;

        value.type=XftTypeDouble;
        value.u.d=from;
        entry->test=XftTestCreate(XftQualAny, "size", XftOpMore, value);
        value.type=XftTypeDouble;
        value.u.d=to;
        entry->test->next=XftTestCreate(XftQualAny, "size", XftOpLess, value);
        entry->edit=XftEditCreate("antialias", XftOpAssign, XftExprCreateBool(False));
        itsList.append(entry);
    }

    itsMadeChanges=true;
}

void CXftConfig::removeExcludeRange()
{
    TEntry *entry=getExcludeRangeEntry();

    if(entry)
    {
        itsList.removeRef(entry);
        delete entry;
    }
}

void CXftConfig::setUseSubPixelHinting(bool use)
{
    TEntry *entry=getUseSubPixelHintingEntry();
 
    if(entry && !use)
    {
        itsList.removeRef(entry);
        delete entry;
    }
    else
        if(!entry && use)
        {
            entry=new TEntry;

            entry->test=NULL;
            entry->edit=XftEditCreate("rgba", XftOpAssign, XftExprCreateInteger(XFT_RGBA_BGR));
            itsList.append(entry);
        }

    itsMadeChanges=true;
}

void CXftConfig::setEntries(QList<TEntry> &list)
{
    //
    // The entries in 'list' may be duplicates of ones in itsList, or ones created by the editor...
    // ...so delete eny entries in 'list' that are not in itsList, then copy...
    CXftConfig::TEntry *entry;
 
    for(entry=itsList.first(); entry; entry=itsList.next())
        if(-1==list.findRef(entry))
            delete entry;

    itsList.clear();
    itsList=list;
    list.clear();
    itsMadeChanges=true;
}

CXftConfig::TEntry * CXftConfig::getExcludeRangeEntry()
{
    TEntry *entry=NULL;

    for(entry=itsList.first(); entry; entry=itsList.next())
        if(entry->test && entry->edit && entry->test->next && NULL==entry->test->next->next && NULL==entry->edit->next &&
           entry->edit->expr &&
           XftQualAny==entry->test->qual && XftQualAny==entry->test->next->qual &&
           ((XftOpMore==entry->test->op && XftOpLess==entry->test->next->op) || (XftOpLess==entry->test->op && XftOpMore==entry->test->next->op) )&&
           (XftTypeDouble==entry->test->value.type || XftTypeInteger==entry->test->value.type) &&
           (XftTypeDouble==entry->test->next->value.type || XftTypeInteger==entry->test->next->value.type) &&
           XftOpAssign==entry->edit->op && XftOpBool==entry->edit->expr->op &&
           CMisc::stricmp(entry->edit->field, "antialias")==0 &&
           CMisc::stricmp(entry->test->field, "size")==0 && CMisc::stricmp(entry->test->next->field, "size")==0)
            break;
 
    return entry;
}
 
CXftConfig::TEntry * CXftConfig::getUseSubPixelHintingEntry()
{
    TEntry *entry=NULL;

    for(entry=itsList.first(); entry; entry=itsList.next())
        if(NULL==entry->test && entry->edit && entry->edit->expr && NULL==entry->edit->next &&
           XftOpAssign==entry->edit->op && XftOpInteger==entry->edit->expr->op &&
           XFT_RGBA_BGR==entry->edit->expr->u.ival &&
           CMisc::stricmp(entry->edit->field, "rgba")==0)
            break;
 
    return entry;
}

static void XftTestDestroy(XftTest *test)
{
    if(test)
    {
        if(test->next)
            XftTestDestroy(test->next);

        if(test->field)
            free(test->field);

        if(XftTypeString==test->value.type)
            free(test->value.u.s);
    }
}

void CXftConfig::TEntry::clear()
{
    if(edit)
    {
        XftEditDestroy(edit);
        free(edit);
        edit=NULL;
    }

    if(test)
    {
        XftTestDestroy(test);
        free(test);
        test=NULL;
    }
}

void CXftConfig::TEntry::output(ofstream &of)
{
    of << "match";

    if(testStr().length())
        of << testStr();

    of << endl
       << "edit"
       << editStr()
       << ";\n\n";
}

QCString CXftConfig::TEntry::testStr()
{
    QCString str;

    printTest(str, test);

    return str;
}

QCString CXftConfig::TEntry::editStr()
{
    QCString str;
 
    printEdit(str, edit);

    return str;
}
#endif
