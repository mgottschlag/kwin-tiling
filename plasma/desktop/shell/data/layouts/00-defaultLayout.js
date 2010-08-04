var topLeftScreen = 0
var rect = screenGeometry(0)

// find our "top left" screen to put the folderview on it
for (var i = 1; i < screenCount; ++i) {
    var g = screenGeometry(i)

    if (g.x <= rect.x && g.top >= rect.top) {
        rect = g
        topLeftScreen = i
    }
}

for (var i = 0; i < screenCount; ++i) {
    var desktop = new Activity
    desktop.screen = i
    desktop.wallpaperPlugin = 'image'
    desktop.wallpaperMode = 'SingleImage'

    print("for screen " + i)
    if (i == topLeftScreen) {
        var folderview = desktop.addWidget("folderview")
        folderview.writeConfig("url", "desktop:/")
        folderview.reloadConfig()
    }
}

loadTemplate("org.kde.plasma-desktop.defaultPanel")

