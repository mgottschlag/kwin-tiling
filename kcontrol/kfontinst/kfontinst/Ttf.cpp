////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CTtf
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst)
// Creation Date : 02/05/2001
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

#include "Ttf.h"
#include "FontEngine.h"
#include "KfiGlobal.h"
#include "Misc.h"
#include <fstream.h>
#include <netinet/in.h>
#include <string.h>
#include <klocale.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
 
CTtf::CTtf()
    : itsBuffer(NULL),
      itsBufferSize(0)
{
    ifstream ps(CMisc::locate("psnames").latin1());

    if(ps)
    {
        const int constMaxStrLen=256;

        char tmpStr[constMaxStrLen];

        itsPsNameList.setAutoDelete(true);

        do
        {
            ps.getline(tmpStr, constMaxStrLen);

            if(!ps.eof())
            { 
                TPsNameMap *map=new TPsNameMap;
                char       psName[constMaxStrLen];

                tmpStr[constMaxStrLen-1]='\0';

                if(sscanf(tmpStr, "%li %s", &(map->unicode), psName)==2)
                {
                    map->psName=psName;
                    itsPsNameList.append(map); 
                }
                else
                    delete map;
            }
        }
        while(!ps.eof());
    }
}

CTtf::~CTtf()
{
    if(itsBuffer)
        delete itsBuffer;
}

CTtf::EStatus CTtf::fixPsNames(const QString &nameAndPath)
{
    EStatus status=CONFIG_FILE_NOT_OPENED;

    if(itsPsNameList.count())
        if((status=readFile(nameAndPath))==SUCCESS)
            if(CKfiGlobal::fe().openFont(nameAndPath))
            {
                if(CKfiGlobal::fe().setCharmapUnicodeFt())
                {
                    bool         madeChange=false;
                    unsigned int gi;
                    EStatus      st;
                    TPsNameMap   *map;

                    for(map=itsPsNameList.first(); map!=NULL; map=itsPsNameList.next())
                        if((gi=CKfiGlobal::fe().getGlyphIndexFt(map->unicode)))
                            if((st=fixGlyphName(gi, map->psName.latin1()))==CHANGE_MADE)
                                madeChange=true;
                            else
                                if(st!=SUCCESS)
                                    break;

                    if(madeChange)
                    {
                         if((status=checksum())==SUCCESS);
                             status=writeFile(nameAndPath);
                    }
                    else
                        status=NO_REMAP_GLYPHS;
                }
                else
                    status=NO_CMAP;

                CKfiGlobal::fe().closeFont(); 
            }
            else
                status=FILE_OPEN_ERROR;

    return status;
}

QList<CTtf::TKerning> * CTtf::getKerningData(const QString &nameAndPath)
{
    QList<TKerning> *list=NULL;

    ifstream ttf(nameAndPath.latin1());

    if(ttf)
    {
        if(locateTable(ttf, "kern"))
        {
            TKern          kern;
            bool           error=false;
            unsigned short subTable;

            ttf.read(&kern, sizeof(TKern));

            if(0==ntohs(kern.version))
                for(subTable=0; subTable<ntohs(kern.numSubTables) && !error; ++subTable)
                    if(ttf.good())
                    {
                        TKernSubTable sub;
                        int           pos=ttf.tellg();

                        ttf.read(&sub, sizeof(TKernSubTable));

                        if(ttf.good())
                        {
                            if(0==(ntohs(sub.coverage)&0xFF00))  // Then it's format 0
                            {
                                TKernFmt0Header hdr;

                                ttf.read(&hdr, sizeof(hdr));

                                if(ttf.good())
                                {
                                    unsigned short pair;

                                    for(pair=0; pair<ntohs(hdr.numPairs) && !error; ++pair)
                                    {
                                        TKernFmt0 data;

                                        ttf.read(&data, sizeof(TKernFmt0));

                                        if(ttf.good())
                                        {
                                            if(NULL==list)
                                            {
                                                list=new QList<TKerning>;
                                                list->setAutoDelete(true);
                                            }
                                            list->append(new TKerning(ntohs(data.left), ntohs(data.right), ntohs(data.value)));
                                        }
                                        else
                                            error=true;
                                    }
                                }
                                else
                                    error=true;
                            }
                            if(!error)
                                ttf.seekg(pos+ntohs(sub.length), ios::beg);
                        }
                        else
                            error=true;
                    }
                    else
                        error=true;
        }
        ttf.close();
    }

    return list;   
}

bool CTtf::locateTable(ifstream &ttf, const char *table)
{
    bool status=false;

    if(ttf)
    {
        TDirectory dir;
        TDirEntry  entry;
        int        e;

        ttf.seekg(0, ios::beg);
        ttf.read(&dir, sizeof(TDirectory));

        if(ttf.good())
            for(e=0; e<ntohs(dir.numTables); ++e)
            {
                ttf.read(&entry, sizeof(TDirEntry));
                if(ttf.good())
                {
                    if(memcmp(entry.tag, table, 4)==0)
                    {
                        ttf.seekg(ntohl(entry.offset), ios::beg);
                        if(ttf.good())
                            status=true;
                        break;
                    }
                }
                else
                    break;
            }
    }

    return status;
}

CTtf::EStatus CTtf::readFile(const QString &nameAndPath)
{
    EStatus status=SUCCESS;

    fstream ff(nameAndPath.latin1(), ios::in|ios::binary);

    if(ff)
    {
        ff.seekg(0, ios::end);

        itsBufferSize=ff.tellg();

        if(itsBuffer)
            delete itsBuffer;
 
        itsBuffer=new char [itsBufferSize];
        ff.seekg(0, ios::beg);
        ff.read(itsBuffer, itsBufferSize);
        ff.close();
    }
    else
        status=FILE_OPEN_ERROR;

    return status;
}

CTtf::EStatus CTtf::writeFile(const QString &nameAndPath)
{
    EStatus status=SUCCESS;

    fstream ff(nameAndPath.latin1(), ios::out|ios::binary);

    if(ff)
    {
        ff.write(itsBuffer, itsBufferSize);
        ff.close();
    }
    else
        status=FILE_WRITE_ERROR;

    return status;
}

CTtf::EStatus CTtf::fixGlyphName(int index, const char *newName)
{
    EStatus    status=SUCCESS;
    TDirectory *ttfDir=(TDirectory *)itsBuffer;
    TDirEntry  *ttfDirEntry=&(ttfDir->list);
    int        table;

    ttfDirEntry=&(ttfDir->list);

    for(table=0; table<ntohs(ttfDir->numTables); ++table)
    {
        if(memcmp(ttfDirEntry->tag, "post", 4)==0)
            break;
        ttfDirEntry++;
    }

    if(table!=ntohs(ttfDir->numTables))
    {
        enum EPsTableFormat
        {
            FORMAT_1   = 0x00010000,
            FORMAT_2   = 0x00020000,
            FORMAT_2_5 = 0x00028000,
            FORMAT_3   = 0x00030000,
            FORMAT_4   = 0x00040000
        };

        TPostHead *ttfPostHead=(TPostHead *)(itsBuffer+ntohl(ttfDirEntry->offset));

        switch(htonl(ttfPostHead->formatType))
        {
            case FORMAT_2:
                if(index < 258 || index > ntohs(ttfPostHead->numGlyphs))
                   status=FILE_FORMAT_ERROR;
                else
                {
                    unsigned short *nameIndex=&(ttfPostHead->glyphNameIndex);
                    int            glyph=ntohs(nameIndex[index]);


                    // Names are stored as PASCAL strings, these have the format...
                    //   1st byte=num chars in string
                    //   rest = string (NOT!) null terminated
                    char *name=(char *)(((char *)(&ttfPostHead->glyphNameIndex))+(ntohs(ttfPostHead->numGlyphs)*2));

                    for(int g=0; g<glyph-258; ++g)
                        name+=1+(*name);

                    int  numChars=*name;
                    char nameStr[256];

                    strncpy(nameStr, ++name, numChars);
                    nameStr[numChars]='\0';

                    if(strcmp(nameStr, newName))
                    {
                        int diff=strlen(newName)-strlen(nameStr);

                        if(diff==0)
                            memcpy(name, newName, strlen(newName));
                        else
                        {
                            int remainderSize=itsBufferSize-((name-itsBuffer)+strlen(nameStr));

                            // As font size has now changed, may need to later the 'offset' values in the TTF file

                            TDirEntry *psTtfEntry=ttfDirEntry;

                            ttfDirEntry=&(ttfDir->list);

                            for(table=0; table<ntohs(ttfDir->numTables); ++table)
                            {
                                if(ntohl(ttfDirEntry->offset)>ntohl(psTtfEntry->offset))
                                    ttfDirEntry->offset=htonl(ntohl(ttfDirEntry->offset)+diff);
                                ttfDirEntry++;
                            }

                            // Also, 'post' table's size has changed...
                            psTtfEntry->length=htonl(ntohl(psTtfEntry->length)+diff);

                            if(diff<0)
                            {
                                char *tmpBuffer=new char[remainderSize];

                                *(name-1)=strlen(newName);
                                memcpy(name, newName, strlen(newName));
                                memcpy(tmpBuffer, name+strlen(nameStr), remainderSize);
                                memcpy(name+strlen(newName), tmpBuffer, remainderSize);
                                delete tmpBuffer;
                            }
                            else
                            {
                                char *newBuffer=new char[itsBufferSize+diff];

                                memcpy(newBuffer, itsBuffer, (name-itsBuffer)-1); // Copy start block
                                newBuffer[(name-itsBuffer)-1]=strlen(newName);    // Set string size
                                memcpy(&newBuffer[name-itsBuffer], newName, strlen(newName));  // copy string
                                memcpy(&newBuffer[(name-itsBuffer)+strlen(newName)], &itsBuffer[(name-itsBuffer)+strlen(nameStr)], 
                                       remainderSize);         // copy remainder 

                                delete itsBuffer; 
                                itsBuffer=newBuffer;
                            }
                            itsBufferSize=itsBufferSize+diff;
                         }

                        status=CHANGE_MADE;
                    }
                }
                break;
            case FORMAT_1:
            case FORMAT_2_5:
                status=USES_MAC_STANDARD;
                break;
            case FORMAT_3:
                case FORMAT_4:
            status=NO_SUITABLE_TABLE;
                break;
        }
    }
    else
        status=NO_POST;

    return status;
}

unsigned long CTtf::checksum(unsigned long *tbl, unsigned long numb)
{
    unsigned long sum=0,
                  nLongs=(numb+3)/4;
 
    while(nLongs--> 0)
        sum+=ntohl(*tbl++);
 
    return htonl(sum);
} 

CTtf::EStatus CTtf::checksum()
{
    EStatus    status=SUCCESS;
    TDirectory *ttfDir=(TDirectory *)itsBuffer;
    TDirEntry  *ttfDirEntry=&(ttfDir->list);
    int        table;
 
    for(table=0; table<ntohs(ttfDir->numTables); ++table)
    {
        if(memcmp(ttfDirEntry->tag, "head", 4)==0)
            break;
        ttfDirEntry++;
    }
 
    if(table!=ntohs(ttfDir->numTables))
    {
        THead *ttfHead=(THead *)(itsBuffer+ntohl(ttfDirEntry->offset));
 
        ttfHead->checksumAdjust=0;
        ttfDirEntry=&(ttfDir->list);
 
        for(table=0; table<ntohs(ttfDir->numTables); ++table)
        {
            ttfDirEntry->checksum=checksum((unsigned long *)(itsBuffer+ntohl(ttfDirEntry->offset)),
                                           ntohl(ttfDirEntry->length));
            ttfDirEntry++;
        }

        unsigned long wholeFontCs=checksum((unsigned long *)itsBuffer, itsBufferSize);

        ttfHead->checksumAdjust=htonl(0xB1B0AFBA-ntohl(wholeFontCs));
    }
    else
        status=NO_HEAD;
 
    return status; 
}

QString CTtf::toString(EStatus status)
{
    switch(status)
    {
        case SUCCESS:
            return i18n("Success");
        case FILE_OPEN_ERROR:
            return i18n("File open error");
        case FILE_WRITE_ERROR:
            return i18n("File write error");
        case NO_HEAD:
            return i18n("No TTF header");
        case NO_POST:
            return i18n("No 'postscript' table");
        case NO_CMAP:
            return i18n("No 'character map' table");
        case NO_SUITABLE_TABLE:
            return i18n("No suitable 'postcript' table");
        case USES_MAC_STANDARD:
            return i18n("Uses MAC standard table");
        case CONFIG_FILE_NOT_OPENED:
            return i18n("PS name maping file not found, or empty"); 
        case NO_REMAP_GLYPHS:
            return i18n("Font does not contain any glyphs to remap");
        case FILE_FORMAT_ERROR:
            return i18n("Error with TTF file format");
        default:
            return i18n("<ERROR>");
    }
}
