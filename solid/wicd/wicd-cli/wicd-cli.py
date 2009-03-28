#!/usr/bin/python

#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the License, or
#       (at your option) any later version.
#       
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#       
#       You should have received a copy of the GNU General Public License
#       along with this program; if not, write to the Free Software
#       Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#       MA 02110-1301, USA.

import optparse
import dbus
import dbus.service
import sys
import wicd.misc as misc

misc.RenameProcess('wicd-cli')

if getattr(dbus, 'version', (0, 0, 0)) < (0, 80, 0):
    import dbus.glib
else:
    from dbus.mainloop.glib import DBusGMainLoop
    DBusGMainLoop(set_as_default=True)
    
bus = dbus.SystemBus()
try:
    proxy_obj = bus.get_object('org.wicd.daemon', '/org/wicd/daemon')
except dbus.DBusException:
	print 'Error: Could not connect to the daemon. Please make sure it is running.'
	sys.exit(3)

daemon = dbus.Interface(proxy_obj, 'org.wicd.daemon')
wireless = dbus.Interface(proxy_obj, 'org.wicd.daemon.wireless')
wired = dbus.Interface(proxy_obj, 'org.wicd.daemon.wired')
config = dbus.Interface(proxy_obj, 'org.wicd.daemon.config')

parser = optparse.OptionParser()

parser.add_option('--network', '-n', type='int', default=-1)
parser.add_option('--template', '-t', type='int', default=-1)
parser.add_option('--network-property', '-p')
parser.add_option('--set-to', '-s')
parser.add_option('--name', '-m')

parser.add_option('--scan', '-S', default=False, action='store_true')
parser.add_option('--save', '-w', default=False, action='store_true')
parser.add_option('--list-networks', '-l', default=False, action='store_true')
parser.add_option('--network-details', '-d', default=False, action='store_true')
parser.add_option('--disconnect', '-x', default=False, action='store_true')
parser.add_option('--connect', '-c', default=False, action='store_true')
parser.add_option('--list-encryption-types', '-e', default=False, action='store_true')
# short options for these two aren't great.
parser.add_option('--wireless', '-y', default=False, action='store_true')
parser.add_option('--wired', '-z', default=False, action='store_true')
parser.add_option('--load-profile', '-o', default=False, action='store_true')

options, arguments = parser.parse_args()

op_performed = False

if not (options.wireless or options.wired):
	print "Please use --wireless or --wired to specify " + \
	"the type of connection to operate on."

# functions
def is_valid_wireless_network_id(network_id):
	if not (network_id >= 0 \
			and network_id < wireless.GetNumberOfNetworks()):
		print 'Invalid wireless network identifier.'
		sys.exit(1)
		
def is_valid_wired_network_id(network_id):
	num = len(config.GetWiredProfileList())
	if not (network_id < num and \
			network_id >= 0):
		print 'Invalid wired network identifier.'
		sys.exit(4)

def is_valid_wired_network_profile(profile_name):
	if not profile_name in config.GetWiredProfileList():
		print 'Profile of that name does not exist.'
		sys.exit(5)

if options.scan and options.wireless:
	wireless.Scan()
	op_performed = True
	
if options.load_profile and options.wired:
	is_valid_wired_network_profile(options.name)
	config.ReadWiredNetworkProfile(options.name)
	op_performed = True

def print_network_information_headers():
	if options.wireless: print '#\tBSSID\t\t\tChannel\tESSID'

def print_network_information(network_id):
	if options.wireless:
		print '%s\t%s\t%s\t%s' % (network_id,
		wireless.GetWirelessProperty(network_id, 'bssid'),
		wireless.GetWirelessProperty(network_id, 'channel'),
		wireless.GetWirelessProperty(network_id, 'essid'))

if options.list_networks:
	if options.wireless:
		print_network_information_headers()
		for network_id in range(0, wireless.GetNumberOfNetworks()):
			print_network_information(network_id)
	elif options.wired:
		print '#\tProfile name'
		id = 0
		for profile in config.GetWiredProfileList():
			print '%s\t%s' % (id, profile)
			id += 1
	op_performed = True

if options.network_details:
	if options.wireless:
		is_valid_wireless_network_id(options.network)
		network_id = options.network
		print_network_information_headers()
		print_network_information(network_id)
	op_performed = True
	
# network properties

if options.network_property:
	if options.wireless:
		is_valid_wireless_network_id(options.network)
		if not options.set_to:
			print wireless.GetWirelessProperty(options.network, options.network_property)
		else:
			wireless.SetWirelessProperty(options.network, \
					options.network_property, options.set_to)
	elif options.wired:
		if not options.set_to:
			print wired.GetWiredProperty(options.network_property)
		else:
			wired.SetWiredProperty(options.network_property, options.set_to)
	op_performed = True

if options.disconnect:
	daemon.Disconnect()
	op_performed = True

if options.connect:
	if options.wireless and options.network > -1:
		is_valid_wireless_network_id(options.network)
		wireless.ConnectWireless(options.network)
	elif options.wired:
		wired.ConnectWired()
	op_performed = True

if options.wireless and options.list_encryption_types:
	et = misc.LoadEncryptionMethods()
	# print 'Installed encryption templates:'
	print '%s\t%-20s\t%s' % ('#', 'Name', 'Description')
	id = 0
	for type in et:
		print '%s\t%-20s\t%s' % (id, type[1], type[0])
		id += 1
	op_performed = True
	
if options.wireless and options.template > -1:
	et = misc.LoadEncryptionMethods()
	template_id = options.template
	if not (template_id >= 0 and template_id < len(et)):
		print "Invalid template identifier."
		sys.exit(6)
	print '%-20s\t%s' % ('Name','Description')
	for needed in et[template_id][2].iteritems():
		print '%-20s\t%s' % (needed[1][1],needed[1][0])
	op_performed = True

if options.save and options.network > -1:
	if options.wireless:
		is_valid_wireless_network_id(options.network)
		config.SaveWirelessNetworkProfile(options.network)
	elif options.wired:
		config.SaveWiredNetworkProfile(options.name)
	op_performed = True
	
if not op_performed:
	print "No operations performed."
