[DEFAULT]
verbose = True

[project]
target = svn:target
start-revision = INITIAL
root-directory = /home/drf/Development/wicdtmp
state-file = tailor.state
source = git:source
subdir = .

[git:source]
repository = /home/drf/Development/kde4/wicd-solid

[svn:target]
module = solid-wicd
repository = svn+ssh://dafre@svn.kde.org/home/kde/trunk/playground/base/

