#! /usr/bin/env python
#
# Copyright David Faure <faure@kde.org>, License LGPL v2
#
# This script converts a kde3 screensaver to kde4, adapting it to the change
# of API in libkscreensaver.
#
# If the input file could be parsed correctly (i.e. it had extern "C" and kss_* symbols),
# then this script will *overwrite* it with the result of the conversion.
# Make backups first, if you don't use a version control system.
#
# Usage: change those two lines before running the script
#
filename = 'pendulum.cpp'
savername = 'KPendulumSaver'   # Interface is appended to this string.

import string,re,os
f = file(filename, 'r')
data = f.read()
lines = data.split('\n')
externCRe = re.compile('^\s*extern "C"')
appNameRe = re.compile('.*kss_applicationName\s*=\s*(".*")')
descriptionRe = re.compile('.*kss_description\s*=\s*(.*);')
versionRe = re.compile('.*kss_version\s*=\s*(.*);')
inExternC = 0
braceLevel = 0
aboutDataWritten = 0
appName = ''
description = ''
version = ''
outputlines = []
for line in lines:
	if (line[0:2] == '//'):
		outputlines.append(line)
		continue
	if not inExternC:
		if (externCRe.match(line)):
			inExternC = 1
			if (line.find('{') >= 0):
				braceLevel = braceLevel+1
	   		outputlines.append(line.replace('extern "C"', 'class ' + savername + 'Interface : public KScreenSaverInterface'))
		else:
			outputlines.append(line)
	else:
		if (line.find('{') >= 0):
			braceLevel = braceLevel+1
		if (line.find('}') >= 0):
	   		braceLevel = braceLevel-1
		match = appNameRe.match(line)
		if match:
			appName = match.group(1)
			line = ''
		match = descriptionRe.match(line)
		if match:
			description = match.group(1)
			line = ''
		match = versionRe.match(line)
		if match:
			version = match.group(1)
			line = ''
		if appName and description and version and not aboutDataWritten:
			outputlines.append( "public:" )
			outputlines.append( "    virtual KAboutData* aboutData() {" )
			outputlines.append( "        return new KAboutData( " + appName + ", " + description + ", " + version + ", " + description + " );" )
			outputlines.append( "    }" )
			aboutDataWritten = 1

		line = re.sub('KDE_EXPORT\s*','',line)
		line = re.sub('KScreenSaver\s*\*\s*kss_create','virtual KScreenSaver* create',line)
		line = re.sub('QDialog\s*\*\s*kss_setup','virtual QDialog* setup',line)
		if braceLevel == 0:
			outputlines.append( '};' )
			outputlines.append( '' )
			outputlines.append( 'int main( int argc, char *argv[] )' )
			outputlines.append( '{' )
			outputlines.append( '    ' + savername + 'Interface kss;' )
			outputlines.append( '    return kScreenSaverMain( argc, argv, kss );' )
			outputlines.append( '}' )
			inExternC = 0
		else:
			outputlines.append( line )

if not aboutDataWritten:
	print "PARSE ERROR"
	print 'appName=' + appName
	print 'description=' + description
	print 'version=' + version
else:
	open(filename,"w").write(string.join(outputlines, '\n'))
	os.system('svn di ' + filename)
