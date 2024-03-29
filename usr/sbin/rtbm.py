#! /usr/bin/env python
# -*- coding: utf-8 -*-
#/*
# *  This file is part of RTBM, Real-Time Bandwidth Monitor.
# *
# *  RTBM, Real-Time Bandwidth Monitor is free software: you can redistribute it and/or modify
# *  it under the terms of the GNU General Public License as published by
# *  the Free Software Foundation, either version 3 of the License, or
# *  (at your option) any later version.
# *
# *  RTBM, Real-Time Bandwidth Monitor is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *  GNU General Public License for more details.
# *
# *  You should have received a copy of the GNU General Public License
# *  along with RTBM, Real-Time Bandwidth Monitor.  If not, see <http://www.gnu.org/licenses/>.
# */

"""
"""

import pcap
import getopt, sys
import socket
import struct
import time
import threading
import thread
import copy
import cjson
import fcntl
import math
import signal
import os
import ConfigParser
import smtplib
from email.mime.text import MIMEText

#CSOM
import ctypes


lib = ctypes.cdll.LoadLibrary('/home/unixadmin/exclude/rtbm.som/usr/lib/libsom.so')
#lib.CSOM_getNodeWeights.restype = ctypes.POINTER(ctypes.c_double)
#lib.CSOM_setTrainingValues.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_double), ctypes.c_int, ctypes.c_int, ctypes.c_double]
lib.CSOM_query.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_double), ctypes.c_int]
lib.CSOM_query.restype = ctypes.c_double
lib.CSOM_importState.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
lib.CSOM_new.restype = ctypes.c_void_p

protocols={socket.IPPROTO_ICMP:'icmp',
	socket.IPPROTO_TCP:'tcp',
	socket.IPPROTO_UDP:'udp'}

BIDIRECTIONAL, INCOMING, OUTGOING = range(3)


#def handler(signum, frame):
	##global pc
	##nrecv, ndrop, nifdrop = pc.stats()
	##print '\n%d packets received by filter' % nrecv
	##print '%d packets dropped by kernel' % ndrop
	
	#sys.exit(0)

def sigmoid(value):
#	return 1.0/(1.0+math.exp(-float(value)))
	return value/(1.0+math.fabs(value))

#CSOM wrapper
class IPNetworkAnalyzer(object):
	def __init__(self):
		self.csom = lib.CSOM_new(15, 15, 300)
	
	def query(self, sample):
		size = len(sample)
		csample = (ctypes.c_double * size)()
		for node in range(0, size):
			csample[node] = sigmoid(sample[node])
		return lib.CSOM_query(self.csom, csample, size)

	def importState(self):
		lib.CSOM_importState(self.csom, ctypes.c_char_p("/home/unixadmin/exclude/rtbm.som/var/lib/rtbm/SOM_state.dat"))

	#def getWidth(self):
		#return lib.CSOM_getWidth(self.csom)

	#def getHeight(self):
		#return lib.CSOM_getHeight(self.csom)

	#def getNumInputNodes(self):
		#return lib.CSOM_getNumInputNodes(self.csom)

	#def getNodeWeights(self, x, y):
		#return lib.CSOM_getNodeWeights(self.csom, x, y)

	#def setTrainingValues(self, input, inputLenght, numIterations, initialLearningRate):
		#return lib.CSOM_setTrainingValues(self.csom, input, inputLenght, numIterations, initialLearningRate)

	#def trainNextIteration(self):
		#return lib.CSOM_trainNextIteration(self.csom)



class PacketDetails:
	def __init__(self, address, size):
		self.address = address
		self.size = size

	def getAddress(self):
		return self.address

	def getSize(self):
		return self.size

	def __str__(self):
		return '%s : %s' % (self.address, self.size)

class Counter:
	def __init__(self):
		self.lock=thread.allocate_lock()
		self.counter={}
		self.aggregate=0
	def addPacket(self, packetDetails):
		self.lock.acquire()
		if self.counter.has_key(packetDetails.getAddress()):
			self.counter[packetDetails.getAddress()] += packetDetails.getSize()
		else:
			self.counter[packetDetails.getAddress()] = packetDetails.getSize()
		self.aggregate += packetDetails.getSize()
		self.lock.release()
	def getCounter(self):
		self.lock.acquire()
		counter_copy = self.counter
		aggregate_copy = self.aggregate
		self.counter={}
		self.aggregate=0
		self.lock.release()
		return (counter_copy, aggregate_copy)
	def __str__(self):
		return str(self.counter)


#TODO: Add ports information:
#def decode_tcp_udp_packet(pkt,d):
	#d['source_port']=socket.ntohs(struct.unpack('H',pkt[0:2])[0])
	#d['destination_port']=socket.ntohs(struct.unpack('H',pkt[2:4])[0])

def decode_ip_packet(d,s):
	d['src']=socket.inet_ntoa(s[12:16])
	d['dst']=socket.inet_ntoa(s[16:20])
	return d

class Capture( threading.Thread ):
	def __init__( self, direction ):
		threading.Thread.__init__( self )
		self.direction = direction
		self.pc = pcap.pcap(iface, 65535, False)
		self.pc.setdirection(direction);
		self.pc.setfilter("proto 6 or 17")
		self.counter = Counter()

	def run( self ):
		for ts, pkt in self.pc:
			if pkt is not None:
				d={}
				d['size']=len(pkt) #length of packet as caputred, in bytes
				#print d['size']
				decoded=decode_ip_packet(d, pkt[14:])
				# Assuming this is a TCP/UDP packet, from the filter.
				#decode_tcp_udp_packet(pkt[4*decoded['header_len']+14:(4*decoded['header_len'])+4+14], decoded)
				if self.direction == INCOMING:
					self.counter.addPacket(PacketDetails(decoded['src'], decoded['size']))
				elif self.direction == OUTGOING:
					self.counter.addPacket(PacketDetails(decoded['dst'], decoded['size']))
	
	def getStats( self ):
		return self.pc.stats()
		

class Report( threading.Thread ):
	def __init__( self ):
		threading.Thread.__init__( self )

	def run( self ):
		global cycle_time
		giaggregated = []
		goaggregated = []
		incoming = Capture(INCOMING)
		incoming.start()
		outgoing = Capture(OUTGOING)
		outgoing.start()
		
		analizer = IPNetworkAnalyzer()
		analizer.importState()
		
		
		
		while True:
			(icounter, iaggregated)=incoming.counter.getCounter()
			(ocounter, oaggregated)=outgoing.counter.getCounter()
			f = open(stat_file, mode='w')
			response={}
			nrecv, ndrop, nifdrop = incoming.getStats()
			response['nifdrop'] = nifdrop
			response['ndrop'] = ndrop
			response['nrecv'] = nrecv
			nrecv, ndrop, nifdrop = outgoing.getStats()
			response['nifdrop'] += nifdrop
			response['ndrop'] += ndrop
			response['nrecv'] += nrecv
			response['time'] = time.time()
			response['outgoing'] = ocounter
			response['incoming'] = icounter
			response['iface'] = iface
			f.write(cjson.encode(response))
			f.close()
			#Send the last 5 minutes of bandwidth to be analized by the IPNetworkAnalyzer
			giaggregated.insert(0, iaggregated)
			reso=1
			resi=1
			if len(giaggregated) > 300:
				giaggregated.pop()
				print response['time'], ";",
				resi = analizer.query(giaggregated)
				print ";",
			goaggregated.insert(0, oaggregated)
			if len(goaggregated) > 300:
				goaggregated.pop()
				reso = analizer.query(goaggregated)
				print "\n",
			
			if reso < 0.00001 or resi < 0.00001:
				notifications.notifyAdministrators()
				
			time.sleep(cycle_time)

class Notification():
	def __init__( self, server, port, encrypt, origin, username, password, destinations):
		self.server = server
		self.port = port
		self.encrypt = encrypt
		self.origin = origin
		self.username = username
		self.password = password
		self.destinations = destinations
	
	def notifyAdministrators(self):
		if self.encrypt == "smtps":
			s = smtplib.SMTP_SSL(self.server, self.port)
		else:
			s = smtplib.SMTP(self.server, self.port)
		#s.set_debuglevel(1)

		if self.encrypt == "starttls":
			s.starttls()
			
		if len(self.username) > 0:
			s.login(self.username, self.password)

		for destination in self.destinations.split(','):
			msg = MIMEText("Abuse or congestion on the system")
			msg['Subject'] = 'The system needs your attention'
			msg['From'] = self.origin
			msg['To'] = destination
			s.sendmail(self.origin, [destination], msg.as_string())
		s.quit()



def usage():
	print 'Usage: ' + sys.argv[0] + ' --config-file=<path to the file that holds the configuration for rtbm> --pid-file=<the file that will be used to hold the process id of the service>'

def main(argv):
	
	global pid_file
	pid_file = None
	global config_file
	config_file = None
	try:
		opts, args = getopt.getopt(argv, "c:p:h", ["config-file=", "pid-file=", "help"])
	except getopt.GetoptError:
		usage()
		sys.exit(-1)
	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
			sys.exit()
		elif opt in ("-p", "--pid-file"):
			pid_file = arg
		elif opt in ("-c", "--config-file"):
			config_file = arg

	if config_file is None or pid_file is None:
		usage()
		sys.exit(-1)


	#f = open(config_file, 'r')
	#config_file_data = f.read()

	#config = ConfigParser.RawConfigParser(allow_no_value=True)
	config = ConfigParser.RawConfigParser()
	config.read(config_file)
	#config.readfp(io.BytesIO(config_file_data))

	global stat_file
	global cycle_time
	global iface
	stat_file = config.get("general", "stat_file")
	iface = config.get("general", "iface")
	cycle_time = float(config.get("general", "cycle_time"))
	
	encrypt = config.get("notifications", "encrypt")
	server = config.get("notifications", "server")
	port = config.getint("notifications", "port")
	origin = config.get("notifications", "origin")
	username = config.get("notifications", "username")
	password = config.get("notifications", "password")
	destinations = config.get("notifications", "destination")
	
	notifications = Notification(server, port, encrypt, origin, username, password, destinations)

	f = open(pid_file, mode='w')
	f.write(str(os.getpid()))
	f.close()
		
	report = Report()
	report.start()



if __name__ == '__main__':
	#try: 
		#pid = os.fork() 
		#if pid > 0:
			## exit first parent
			#sys.exit(0) 
	#except OSError, e: 
		#print >>sys.stderr, "fork #1 failed: %d (%s)" % (e.errno, e.strerror) 
		#sys.exit(1)
	main(sys.argv[1:])
