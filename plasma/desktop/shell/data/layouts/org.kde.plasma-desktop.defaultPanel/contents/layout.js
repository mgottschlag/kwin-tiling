var panel = new Panel
if (panelIds.length == 1) {
    // we are the only panel, so set the location for the user
    panel.location = 'bottom'
}

panel.height = 27
panel.addWidget("launcher")
panel.addWidget("pager")
panel.addWidget("tasks")
panel.addWidget("systemtray")
panel.addWidget("digital-clock")

