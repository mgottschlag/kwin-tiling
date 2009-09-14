#ifndef nitrogenshadowconfiguration_h
#define nitrogenshadowconfiguration_h

//////////////////////////////////////////////////////////////////////////////
// nitrogenshadowconfiguration.h
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include <KConfigGroup>
#include <QPalette>

namespace NitrogenConfig
{

  static const QString SHADOW_SIZE = "Size";
  static const QString SHADOW_HOFFSET= "HorizontalOffset";
  static const QString SHADOW_VOFFSET= "VerticalOffset";
  static const QString SHADOW_INNER_COLOR = "InnerColor";
  static const QString SHADOW_OUTER_COLOR = "OuterColor";
  static const QString SHADOW_USE_OUTER_COLOR = "UseOuterColor";
}

namespace Nitrogen
{

  class NitrogenShadowConfiguration
  {

    public:

    //! button size enumeration
    //! default constructor
    NitrogenShadowConfiguration( QPalette::ColorGroup );

    //! constructor from KConfig
    NitrogenShadowConfiguration( QPalette::ColorGroup, KConfigGroup );

    //! destructor
    virtual ~NitrogenShadowConfiguration( void )
    {}

    //! equal to operator
    bool operator == ( const NitrogenShadowConfiguration& ) const;

    //! write to kconfig group
    virtual void write( KConfigGroup& ) const;

    //! color group
    QPalette::ColorGroup colorGroup( void ) const
    { return colorGroup_; }

    //! shadow size
    qreal shadowSize( void ) const
    { return shadowSize_; }

    //! shadow size
    void setShadowSize( qreal value )
    { shadowSize_ = value; }

    //! horizontal offset
    qreal horizontalOffset( void ) const
    { return horizontalOffset_; }

    //! horizontal offset
    void setHorizontalOffset( qreal value )
    { horizontalOffset_ = value; }

    //! vertical offset
    qreal verticalOffset( void ) const
    { return verticalOffset_; }

    //! vertical offset
    void setVerticalOffset( qreal value )
    { verticalOffset_ = value; }

    //! inner color
    QColor innerColor( void ) const
    { return innerColor_; }

    //! inner color
    void setInnerColor( QColor );

    //! mid color
    QColor midColor( void ) const
    { return midColor_; }

    //! outer color
    QColor outerColor( void ) const
    { return useOuterColor() ? outerColor_ : outerColor2_; }

    //! outer color
    void setOuterColor( QColor );

    //! use outer color
    bool useOuterColor( void ) const
    { return useOuterColor_; }

    //! use outer color
    void setUseOuterColor( bool value )
    { useOuterColor_ = value; }

    protected:

    //! mid color
    void setMidColor( QColor );

    //! calculated outer color
    QColor outerColor2( void ) const
    { return outerColor2_; }

    //! calculated outer color
    void setOuterColor2( QColor );

    //! calculate mid color
    QColor calcMidColor( void ) const;

    //! calculate outer color
    QColor calcOuterColor( void ) const;

    private:

    //! color group
    QPalette::ColorGroup colorGroup_;

    //! shadow size
    qreal shadowSize_;

    //! horizontal offset
    qreal horizontalOffset_;

    //! vertical offset
    qreal verticalOffset_;

    //! inner color
    QColor innerColor_;

    //! mid color
    QColor midColor_;

    //! outer color
    QColor outerColor_;

    //! calculated outer color
    QColor outerColor2_;

    //! use outer color
    bool useOuterColor_;

  };

}

#endif
