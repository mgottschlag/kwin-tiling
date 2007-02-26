#include "kxkb_part.h"

KxkbPart::KxkbPart( QWidget* parentWidget,
               QObject* parent,
               const QStringList& args )
  : KParts::ReadWritePart(parent)
{
}
