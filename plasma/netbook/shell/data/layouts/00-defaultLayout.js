
var sal = new Activity("sal")
sal.screen = 0
sal.wallpaperPlugin = 'image'
sal.wallpaperMode = 'SingleImage'
//TODO: translate: how?
sal.name = 'Search and Launch'
sal.writeConfig("PackageManager", "kpackagekit")
sal.reloadConfig()

loadTemplate("org.kde.plasma-netbook.defaultPanel")
loadTemplate("org.kde.plasma-netbook.defaultPage")

