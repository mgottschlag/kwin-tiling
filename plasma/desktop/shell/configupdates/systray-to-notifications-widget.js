
var biggestId = 0;

for (var i in activityIds) {
    var activity = activityById(activityIds[i]);
    for (var j in activity.widgetIds) {
        if (j > biggestId) {
            biggestId = j
        }
    }
}

for (var i in panelIds) {
    var panel = panelById(panelIds[i]);
    for (var j in panel.widgetIds) {
        if (j > biggestId) {
            biggestId = j
        }
    }
}

for (var i in panelIds) {
    var panel = panelById(panelIds[i]);
    for (var j in panel.widgetIds) {
        var widget = panel.widgetById(panel.widgetIds[j]);
        if (widget.type == "systemtray") {
            widget.currentConfigGroup = new Array('Applets', biggestId+1);
            widget.writeConfig('plugin', 'notifications');
            widget.reloadConfig();
        }
    }
}