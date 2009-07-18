require 'plasma_applet'

module RubyExtenderTutorial
  class ExtenderTutorial < PlasmaScripting::PopupApplet
    slots 'sourceAdded(QString)'
    
    def initialize(parent, args = nil)
      super

      # We want to collapse into an icon when put into a panel.
      # If you don't call this function, you can display another 
      # widget, or draw something yourself.      
      setPopupIcon("extendertutorial")
    end

    def init
      # Calling extender() instantiates an extender for you if you
      # haven't already done so. Never instantiate an extender 
      # before init() since Extender needs access to applet->config()
      # to work.
 
      # The message to be shown when there are no ExtenderItems in
      # this extender.
      extender.emptyExtenderMessage = KDE::i18n("no running jobs...")
 
      # Notify ourself whenever a new job is created.
      connect(dataEngine("kuiserver"),  SIGNAL('sourceAdded(QString)'),
              self, SLOT('sourceAdded(QString)'))    
    end 

    def initExtenderItem(item)
      # Create a Meter widget and wrap it in the ExtenderItem
      meter = Plasma::Meter.new(item) do |m|
        m.meterType = Plasma::Meter::BarMeterHorizontal
        m.svg = "widgets/bar_meter_horizontal"
        m.maximum = 100
        m.value = 0
  
        m.minimumSize = Qt::SizeF.new(250, 45)
        m.preferredSize = Qt::SizeF(250, 45)
      end

      # often, you'll want to connect dataengines or set properties
      # depending on information contained in item->config().
      # In this situation that won't be necessary though.    
      item.widget = meter
  
      # Job names are not unique across plasma restarts (kuiserver
      # engine just starts with Job1 again), so avoid problems and
      # just don't give reinstantiated items a name.
      item.name = ""
  
      # Show a close button.
      item.showCloseButton
    end

    def sourceAdded(source)
      # Add a new ExtenderItem
      item = Plasma::ExtenderItem.new(extender)
      initExtenderItem(item)
  
      # We give this item a name, which we don't use in this
      # example, but allows us to look up extenderItems by calling
      # extenderItem(name). That function is useful to avoid 
      # duplicating detached ExtenderItems between session, because 
      # you can check if a certain item already exists.
      item.name = source
  
      # And we give this item a title. Titles, along with icons and
      # names are persistent between sessions.
      item.title = source
 
      # Connect a dataengine. If this applet would display data where 
      # datasources would have unique names, even between sessions, 
      # you should do this in initExtenderItem, so that after a plasma 
      # restart, datasources would still get connected to the 
      # appropriate sources. Kuiserver jobs are not persistent however, 
      # so we connect them here.
      dataEngine("kuiserver").connectSource(source, item.widget, 200)
  
      # Show the popup for 5 seconds if in panel, so the user notices
      # that there's a new job running.
      showPopup(5000)
    end
  end
end

# kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;
