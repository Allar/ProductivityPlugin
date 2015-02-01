import math
import socket
import time
import struct
import unicodedata

global s
s = None

def packText(text):
	return struct.pack('I'+str(len(text)+1)+"s", len(text)+1, text)
		
def SendUnrealBatchPlaceData(objName, locx, locy, locz, rotx, roty, rotz, scalex, scaley, scalez):
	global s
	if (s is None):
		batchPlaceOpenSocket()
		
	if (s is not None):
		output = "\x01" + packText(objName) + packText(locx) + packText(locy) + packText(locz) + packText(rotx) + packText(roty) + packText(rotz) + packText(scalex) + packText(scaley) + packText(scalez)
		padlen = 1024-len(output)
		output += "\x00"*padlen
		s.sendall(output)
		print ('Sent ' + objName)

def batchPlaceOpenSocket():
	global s
	if (s is not None):
		batchPlaceCloseSocket()
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
	try:	
		s.connect(('127.0.0.1',51234)) 
	except Exception, e:
		print "Could not establish connection to UE4."
		s.close()
		s = None
			
def batchPlaceCloseSocket():
	global s
	if (s != None):
		s.shutdown(2)
		s.close()
		s = None