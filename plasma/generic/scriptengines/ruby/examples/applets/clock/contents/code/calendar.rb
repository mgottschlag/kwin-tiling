=begin
** Form generated from reading ui file 'calendar.ui'
**
** Created: Wed Aug 6 19:25:39 2008
**      by: Qt User Interface Compiler version 4.4.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
=end

class Ui_Calendar
    attr_reader :vboxLayout
    attr_reader :kdatepicker

    def setupUi(calendar)
    if calendar.objectName.nil?
        calendar.objectName = "calendar"
    end
    calendar.resize(276, 255)
    calendar.styleSheet = ""
    @vboxLayout = Qt::VBoxLayout.new(calendar)
    @vboxLayout.spacing = 0
    @vboxLayout.margin = 0
    @vboxLayout.objectName = "vboxLayout"
    @kdatepicker = KDE::DatePicker.new(calendar)
    @kdatepicker.objectName = "kdatepicker"
    @kdatepicker.autoFillBackground = true

    @vboxLayout.addWidget(@kdatepicker)


    retranslateUi(calendar)

    Qt::MetaObject.connectSlotsByName(calendar)
    end # setupUi

    def setup_ui(calendar)
        setupUi(calendar)
    end

    def retranslateUi(calendar)
    end # retranslateUi

    def retranslate_ui(calendar)
        retranslateUi(calendar)
    end

end

module Ui
    class Calendar < Ui_Calendar
    end
end  # module Ui

