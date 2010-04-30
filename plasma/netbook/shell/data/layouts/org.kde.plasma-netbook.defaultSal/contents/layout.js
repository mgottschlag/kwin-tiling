
var sal = new Activity("sal")
sal.screen = 0
sal.wallpaperPlugin = 'image'
sal.wallpaperMode = 'SingleImage'
sal.name = templateName
sal.writeConfig("PackageManager", "kpackagekit")
sal.reloadConfig()
