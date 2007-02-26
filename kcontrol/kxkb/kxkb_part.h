#ifndef __kxkb_part_h
#define __kxkb_part_h

#include <kparts/part.h>

class QWidget;
class QString;

class KxkbPart : public KParts::ReadWritePart
{
  Q_OBJECT
 public:
  KxkbPart( QWidget* parentWidget,
               QObject* parent,
               const QStringList& args = QStringList() );
  virtual ~KxkbPart() {}

 protected slots:
  bool setLayout(const QString& layoutPair);

};

#endif
