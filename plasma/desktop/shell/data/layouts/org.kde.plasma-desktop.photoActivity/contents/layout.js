var activity = new Activity
activity.name = i18n("Photos Activity")

var pictures = userDataPath("pictures")

var folderview = activity.addWidget("folderview")
folderview.writeConfig("url", pictures)

var frame = activity.addWidget("frame")
frame.writeConfig("slideshow", true)
frame.writeConfig("recursive slideshow", true)
frame.writeConfig("slideshow paths", pictures)
frame.writeConfig("slideshow time", 300)
frame.writeConfig("random", true)

