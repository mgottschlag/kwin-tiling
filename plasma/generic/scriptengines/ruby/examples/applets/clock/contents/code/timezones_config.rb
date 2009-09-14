=begin
** Form generated from reading ui file 'timezonesConfig.ui'
**
** Created: Wed Aug 6 19:25:39 2008
**      by: Qt User Interface Compiler version 4.4.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
=end

class Ui_TimezonesConfig
    attr_reader :vboxLayout
    attr_reader :localTimeZone
    attr_reader :timeZones

    def setupUi(timezonesConfig)
    if timezonesConfig.objectName.nil?
        timezonesConfig.objectName = "timezonesConfig"
    end
    timezonesConfig.resize(308, 227)
    @vboxLayout = Qt::VBoxLayout.new(timezonesConfig)
    @vboxLayout.objectName = "vboxLayout"
    @localTimeZone = Qt::CheckBox.new(timezonesConfig)
    @localTimeZone.objectName = "localTimeZone"

    @vboxLayout.addWidget(@localTimeZone)

    @timeZones = KDE::TimeZoneWidget.new(timezonesConfig)
    @timeZones.objectName = "timeZones"
    @timeZones.minimumSize = Qt::Size.new(300, 150)

    @vboxLayout.addWidget(@timeZones)


    retranslateUi(timezonesConfig)
    Qt::Object.connect(@localTimeZone, SIGNAL('toggled(bool)'), @timeZones, SLOT('setDisabled(bool)'))

    Qt::MetaObject.connectSlotsByName(timezonesConfig)
    end # setupUi

    def setup_ui(timezonesConfig)
        setupUi(timezonesConfig)
    end

    def retranslateUi(timezonesConfig)
    @localTimeZone.text = Qt::Application.translate("timezonesConfig", "Use &local time zone", nil, Qt::Application::UnicodeUTF8)
    @timeZones.headerItem.setText(0, Qt::Application.translate("timezonesConfig", "Area", nil, Qt::Application::UnicodeUTF8))
    @timeZones.headerItem.setText(1, Qt::Application.translate("timezonesConfig", "Region", nil, Qt::Application::UnicodeUTF8))
    @timeZones.headerItem.setText(2, Qt::Application.translate("timezonesConfig", "Comment", nil, Qt::Application::UnicodeUTF8))
    end # retranslateUi

    def retranslate_ui(timezonesConfig)
        retranslateUi(timezonesConfig)
    end

end

module Ui
    class TimezonesConfig < Ui_TimezonesConfig
    end
end  # module Ui

