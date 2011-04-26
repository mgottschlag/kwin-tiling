
for (var i = 0; i < screenCount; ++i) {
    var desktop = new Activity
    desktop.name = i18n("Desktop")
    desktop.screen = i
    desktop.wallpaperPlugin = 'image'
    desktop.wallpaperMode = 'SingleImage'
}

loadTemplate("org.kde.plasma-desktop.defaultPanel")

