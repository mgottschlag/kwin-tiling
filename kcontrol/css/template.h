#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__


#include <qstring.h>
#include <qasciidict.h>


class CSSTemplate
{
public:

  CSSTemplate(QString fname) : _filename(fname) {};

  bool expand(QString destname, QAsciiDict<char> dict);


protected:

  QString eval(QString expr, QAsciiDict<char> dict);


private:

  QString _filename;

};


#endif
