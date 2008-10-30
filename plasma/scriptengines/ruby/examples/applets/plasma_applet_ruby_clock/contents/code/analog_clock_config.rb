=begin
** Form generated from reading ui file 'clockConfig.ui'
**
** Created: Wed Aug 6 19:25:39 2008
**      by: Qt User Interface Compiler version 4.4.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
=end

class Ui_AnalogClockConfig
    attr_reader :verticalLayout
    attr_reader :showSecondHandCheckBox
    attr_reader :showTimeStringCheckBox
    attr_reader :verticalSpacer

    def setupUi(clockConfig)
    if clockConfig.objectName.nil?
        clockConfig.objectName = "clockConfig"
    end
    clockConfig.resize(449, 300)
    clockConfig.minimumSize = Qt::Size.new(400, 300)
    @verticalLayout = Qt::VBoxLayout.new(clockConfig)
    @verticalLayout.objectName = "verticalLayout"
    @showSecondHandCheckBox = Qt::CheckBox.new(clockConfig)
    @showSecondHandCheckBox.objectName = "showSecondHandCheckBox"

    @verticalLayout.addWidget(@showSecondHandCheckBox)

    @showTimeStringCheckBox = Qt::CheckBox.new(clockConfig)
    @showTimeStringCheckBox.objectName = "showTimeStringCheckBox"

    @verticalLayout.addWidget(@showTimeStringCheckBox)

    @verticalSpacer = Qt::SpacerItem.new(20, 235, Qt::SizePolicy::Minimum, Qt::SizePolicy::Minimum)

    @verticalLayout.addItem(@verticalSpacer)


    retranslateUi(clockConfig)

    Qt::MetaObject.connectSlotsByName(clockConfig)
    end # setupUi

    def setup_ui(clockConfig)
        setupUi(clockConfig)
    end

    def retranslateUi(clockConfig)
    @showSecondHandCheckBox.toolTip = Qt::Application.translate("AnalogClockConfig", "Show the seconds", nil, Qt::Application::UnicodeUTF8)
    @showSecondHandCheckBox.whatsThis = Qt::Application.translate("AnalogClockConfig", "Check this if you want to show the seconds.", nil, Qt::Application::UnicodeUTF8)
    @showSecondHandCheckBox.text = Qt::Application.translate("AnalogClockConfig", "Show &seconds", nil, Qt::Application::UnicodeUTF8)
    @showTimeStringCheckBox.toolTip = Qt::Application.translate("AnalogClockConfig", "Show the time in text", nil, Qt::Application::UnicodeUTF8)
    @showTimeStringCheckBox.whatsThis = Qt::Application.translate("AnalogClockConfig", "Check this if you want to display the time as text within the clock.", nil, Qt::Application::UnicodeUTF8)
    @showTimeStringCheckBox.text = Qt::Application.translate("AnalogClockConfig", "Also show the time as text", nil, Qt::Application::UnicodeUTF8)
    end # retranslateUi

    def retranslate_ui(clockConfig)
        retranslateUi(clockConfig)
    end

end

module Ui
    class AnalogClockConfig < Ui_AnalogClockConfig
    end
end  # module Ui

