var panel = new Panel("netpanel")
if (panelIds.length == 1) {
    // we are the only panel, so set the location for the user
    panel.location = 'top'
}
panel.locked = false;
panel.height = 28
panel.addWidget("activitybar")
panel.addWidget("systemtray")
panel.addWidget("digital-clock")
panel.addWidget("currentappcontrol")
panel.locked = true;