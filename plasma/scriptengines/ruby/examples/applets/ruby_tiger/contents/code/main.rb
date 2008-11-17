require 'plasma_applet'

module Tiger
  class Main < PlasmaScripting::Applet
    def initialize(parent, args = nil)
      super
    end

    def init
      @svg = Plasma::Svg.new(self)
      @svg.imagePath = 'widgets/tiger'
#      Qt::Internal::setDebug Qt::QtDebugChannel::QTDB_VIRTUAL
    end

    def paintInterface(painter, option, contentsRect)
      @svg.resize(size())
      @svg.paint(painter, 0, 0)
    end
  end
end
