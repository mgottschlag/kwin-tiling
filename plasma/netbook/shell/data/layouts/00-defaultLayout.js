
var sal = new Activity("sal")
sal.screen = 0
sal.wallpaperPlugin = 'image'
sal.wallpaperMode = 'SingleImage'
sal.writeConfig("PackageManager", "kpackagekit")
sal.reloadConfig()

loadTemplate("org.kde.plasma-netbook.defaultPanel")
loadTemplate("org.kde.plasma-netbook.defaultPage")

