import maya.OpenMaya as om
import maya.cmds as cmds
import math
import socket
import time
import struct
import unicodedata

class BatchPlacer():
	def packText(self, text):
		return struct.pack('I'+str(len(text)+1)+"s", len(text)+1, text)
		
	def SendUnrealBatchPlaceData(self, sock, objName, locx, locy, locz, rotx, roty, rotz, scalex, scaley, scalez):
		output = "\x01" + self.packText(objName) + self.packText(locx) + self.packText(locy) + self.packText(locz) + self.packText(rotx) + self.packText(roty) + self.packText(rotz) + self.packText(scalex) + self.packText(scaley) + self.packText(scalez)
		padlen = 1024-len(output)
		output += "\x00"*padlen
		sock.sendall(output)

	def exportSelected(self, sock):
		selection = om.MSelectionList()
		om.MGlobal.getActiveSelectionList(selection)
		selection_iter = om.MItSelectionList(selection)
		while not selection_iter.isDone():
			obj = om.MObject()
			dagPath = om.MDagPath()
			selection_iter.getDependNode(obj)
			selection_iter.getDagPath(dagPath)
			node = om.MFnDependencyNode(obj)

			mt = om.MTransformationMatrix(dagPath.inclusiveMatrix())
			loc = mt.translation(om.MSpace.kWorld)
			rot = mt.rotation().asEulerRotation()
			scaleUtil = om.MScriptUtil()
			scaleUtil.createFromList([0,0,0],3)
			scaleVec = scaleUtil.asDoublePtr()
			mt.getScale(scaleVec, om.MSpace.kWorld)
			scale = [om.MScriptUtil.getDoubleArrayItem(scaleVec, i) for i in range(0,3)]
			if (cmds.upAxis(q=True,axis=True) == "y"):
				self.SendUnrealBatchPlaceData(sock, unicodedata.normalize('NFKD', node.name()).encode('ascii','ignore'), str(loc.x), str(loc.z), str(loc.y), str(math.degrees(rot.x)), str(math.degrees(rot.z)), str(math.degrees(rot.y)), str(scale[0]), str(scale[2]), str(scale[1]))
			else:
				self.SendUnrealBatchPlaceData(sock, unicodedata.normalize('NFKD', node.name()).encode('ascii','ignore'), str(loc.x), str(loc.y), str(loc.z), str(math.degrees(rot.x)), str(math.degrees(rot.y)), str(math.degrees(rot.z)), str(scale[0]), str(scale[1]), str(scale[2]))
			print "Sent mesh " + node.name()

			selection_iter.next()
		print "done sending"
		
def main():
	global placer
	global s
	placer = BatchPlacer()
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
	try:	
		s.connect(('127.0.0.1',51234)) 
	except Exception, e:
		cmds.error("Could not establish connection to UE4.")
		placer = None
		s.close()
		s = None
	
	
	placer.exportSelected(s)
	
def batchPlaceSelected():
	global placer
	global s
	if (placer != None and s != None):
		placer.exportSelected(s)
	else:
		main()
		
if __name__ == "__main__":
        main()
