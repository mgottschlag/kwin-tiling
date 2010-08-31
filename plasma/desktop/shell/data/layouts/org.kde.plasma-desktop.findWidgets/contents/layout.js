function findWidgetsIn(containment, type, callback)
{
    var widgetIds = containment.widgetIds
    for (id in widgetIds) {
        var widget = containment.widgetById(widgetIds[id])
            if (widget.type == type) {
                print("Found a " + type + " tray with id " + widget.id + " in " + containment.type + " with id " + containment.id)
                if (callback) {
                    callback(widget, containment)
                }
            }
    }
}

this.findWidgets = function(type, callback)
{
    for (i in panelIds) {
        findWidgetsIn(panelById(panelIds[i]), type, callback)
    }

    for (i in activityIds) {
        findWidgetsIn(activityById(activityIds[i]), type, callback)
    }
}

