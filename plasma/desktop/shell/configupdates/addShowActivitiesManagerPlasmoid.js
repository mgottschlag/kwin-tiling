
function findWidgetsIn(containment, type)
{
    var widgetIds = containment.widgetIds
    for (id in widgetIds) {
        var widget = containment.widgetById(widgetIds[id])
        if (widget.type == type) {
             return widget;
        }
    }
    
    return false;
}

this.findWidgets = function(type)
{
    for (i in panelIds) {
        if (findWidgetsIn(panelById(panelIds[i]), type)) {
            return true;
        }
    }

    for (i in activityIds) {
        if (findWidgetsIn(activityById(activityIds[i]), type)) {
             return true;
	}
    }
    
    return false;
}

if (!findWidgets('org.kde.showActivityManager')) {
    var panels = panels()
    var found = false
    for (i in panels) {
	found = findWidgetsIn(panels[i], 'launcher')
	if (found) {
	  var showWidget = panels[i].addWidget("org.kde.showActivityManager")
	  showWidget.index = found.index + 1;
	  break;
	}
    }
    
    if (!found && panels.length > 0) {
      var showWidget = panels[i].addWidget("org.kde.showActivityManager")
      showWidget.index = 0;
    }
}
