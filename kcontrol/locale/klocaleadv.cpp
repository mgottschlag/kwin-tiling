/*
  klocaleadv.cpp - A KControl Application

  Copyright 2000 Hans Petter Bieker <bieker@kde.org>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */

#include <klocale.h>

#include "klocaleadv.h"

KLocaleAdvanced::KLocaleAdvanced( const QString& catalogue )
  : KLocale( catalogue )
{
  _time =   KLocale::time();
  _money =  KLocale::money();
  _number = KLocale::number();
}

KLocaleAdvanced::~KLocaleAdvanced()
{
}

void KLocaleAdvanced::setChset(const QString &chrset)
{
  chset = chrset;
}

void KLocaleAdvanced::setDateFormat(const QString &fmt)
{
  _datefmt = fmt;
}

void KLocaleAdvanced::setDateFormatShort(const QString &fmt)
{
  _datefmtshort = fmt;
}

void KLocaleAdvanced::setTimeFormat(const QString &fmt)
{
  _timefmt = fmt;
}

QString KLocaleAdvanced::dateFormat() const
{
  return _datefmt;
}

QString KLocaleAdvanced::dateFormatShort() const
{
  return _datefmtshort;
}

QString KLocaleAdvanced::timeFormat() const
{
  return _timefmt;
}

void KLocaleAdvanced::setDecimalSymbol(const QString &symb)
{
  _decimalSymbol = symb;
}

void KLocaleAdvanced::setThousandsSeparator(const QString &sep)
{
  _thousandsSeparator = sep;
}

void KLocaleAdvanced::setPositiveSign(const QString &sign)
{
  _positiveSign = sign;
}

void KLocaleAdvanced::setNegativeSign(const QString &sign)
{
  _negativeSign = sign;
}

void KLocaleAdvanced::setPositiveMonetarySignPosition(SignPosition signpos)
{
  _positiveMonetarySignPosition = signpos;
}

void KLocaleAdvanced::setNegativeMonetarySignPosition(SignPosition signpos)
{
  _negativeMonetarySignPosition = signpos;
}

void KLocaleAdvanced::setPositivePrefixCurrencySymbol(bool prefixcur)
{
  _positivePrefixCurrencySymbol = prefixcur;
}

void KLocaleAdvanced::setNegativePrefixCurrencySymbol(bool prefixcur)
{
  _negativePrefixCurrencySymbol = prefixcur;
}

void KLocaleAdvanced::setFracDigits(int digits)
{
  _fracDigits = digits;
}

void KLocaleAdvanced::setMonetaryThousandsSeparator(const QString &sep)
{
  _monetaryThousandsSeparator = sep;
}

void KLocaleAdvanced::setMonetaryDecimalSymbol(const QString &symbol)
{
  _monetaryDecimalSymbol = symbol;
}

void KLocaleAdvanced::setCurrencySymbol(const QString &symbol)
{
  _currencySymbol = symbol;
}

QString KLocaleAdvanced::money() const
{
    return _money;
}

QString KLocaleAdvanced::number() const
{
    return _number;
}

QString KLocaleAdvanced::time() const
{
    return _time;
}

void KLocaleAdvanced::setCountry(const QString &number, const QString &money, const QString &time)
{
  if (!number.isNull()) _number = number;
  if (!money.isNull()) _money = money;
  if (!time.isNull()) _time = time;
}

void KLocaleAdvanced::setCountry(const QString &country)
{
  setCountry(country, country, country);
}
