

var page = new Activity("newspaper")
page.screen = -1
page.wallpaperPlugin = 'image'
page.wallpaperMode = 'SingleImage'
page.name = templateName


page.addWidgetAt("news", 0, 0)
page.addWidgetAt("weather", 1, 0)
page.addWidgetAt("opendesktop", 0, 1)
page.addWidgetAt("knowledgebase", 1, 1)