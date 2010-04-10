print("creating the layout")
for (var i = 0; i < screenCount; ++i) {
    var desktop = new Activity
    desktop.screen = i
    desktop.wallpaperPlugin = 'image'
    desktop.wallpaperMode = 'SingleImage'

    print("for screen " + i)
    if (i == 0) {
        var folderview = desktop.addWidget("folderview")
        folderview.writeConfig("url", "desktop:/")
    }
}

loadTemplate("org.kde.plasma-desktop.defaultPanel")

