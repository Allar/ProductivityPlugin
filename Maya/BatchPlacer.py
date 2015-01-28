from PySide import QtGui, QtCore
import pysideuic
import xml.etree.ElementTree as xml
from cStringIO import StringIO
from pymel.core import *
import pymel.core as pm
from pymel import *
import maya.OpenMayaUI as apiUI
import maya.OpenMaya as om
import maya.cmds as cmds
from itertools import izip
import math

def pairwise(iterable):
    a = iter(iterable)
    return izip(a, a)

# Helper for loading Qt ui files from nathanhorne.com/?p=451
def loadUiType(uiFile):
    """
        Pyside lacks the "loadUiType" command, so we have to convert the ui file to py code in-memory first
        and then execute it in a special frame to retrieve the form_class.
        """
    parsed = xml.parse(uiFile)
    widget_class = parsed.find('widget').get('class')
    form_class = parsed.find('class').text

    with open(uiFile, 'r') as f:
        o = StringIO()
        frame = {}

        pysideuic.compileUi(f, o, indent=0)
        pyc = compile(o.getvalue(), '<string>', 'exec')
        exec pyc in frame

        #Fetch the base_class and form class based on their type in the xml from designer
        form_class = frame['Ui_%s'%form_class]
        base_class = eval('QtGui.%s'%widget_class)
    return form_class, base_class

# UE4 Static Mesh strings
ue4_static_header = "Begin Map\r\n   Begin Level\r\n"
ue4_static_footer = "   End Level\r\nBegin Surface\r\nEnd Map\r\n"
ue4_static_template_body = "      Begin Actor Class=StaticMeshActor Name=$MAYANAME$_1 Archetype=StaticMeshActor'/Script/Engine.Default__StaticMeshActor'\r\n         Begin Object Class=StaticMeshComponent Name=\"StaticMeshComponent0\" Archetype=StaticMeshComponent'Default__StaticMeshActor:StaticMeshComponent0'\r\n         End Object\r\n         Begin Object Name=\"StaticMeshComponent0\"\r\n            StaticMesh=$UNREALNAME$\r\n            RelativeLocation=(X=$LOC_X$,Y=$LOC_Y$,Z=$LOC_Z$)\r\n            RelativeScale3D=(X=$SCALE_X$,Y=$SCALE_Y$,Z=$SCALE_Z$)\r\n            BodyInstance=(Scale3D=(X=$SCALE_X$,Y=$SCALE_Y$,Z=$SCALE_Z$))\r\n            RelativeRotation=(Pitch=$ROT_Y$,Yaw=$ROT_Z$,Roll=$ROT_X$)\r\n         End Object\r\n         StaticMeshComponent=StaticMeshComponent0\r\n         RootComponent=StaticMeshComponent0\r\n         ActorLabel=\"$MAYANAME$\"\r\n      End Actor\r\n"

def GetTemplatedExportBody(MayaName, UnrealName, LocX, LocY, LocZ, RotX, RotY, RotZ, ScaleX, ScaleY, ScaleZ):
    body = ue4_static_template_body.replace("$MAYANAME$", MayaName)
    body = body.replace("$UNREALNAME$", UnrealName)
    body = body.replace("$LOC_X$", str(LocX))
    body = body.replace("$LOC_Y$", str(LocY))
    body = body.replace("$LOC_Z$", str(LocZ))
    body = body.replace("$ROT_X$", str(RotX))
    body = body.replace("$ROT_Y$", str(RotY))
    body = body.replace("$ROT_Z$", str(RotZ))
    body = body.replace("$SCALE_X$", str(ScaleX))
    body = body.replace("$SCALE_Y$", str(ScaleY))
    body = body.replace("$SCALE_Z$", str(ScaleZ))
    return body

# Paths to Files
homedir = os.environ['UE4_PRODUCTIVITY'].replace("\\","/") + "/"
usersettings = homedir + "Maya/batchplacer.config"
main_ui_filename = homedir + "Maya/batchplacer.ui"
main_form_class, main_base_class = loadUiType(main_ui_filename)
slot_ui_filename = homedir + "Maya/batchplacer_slotrow.ui"
slot_form_class, slot_base_class = loadUiType(slot_ui_filename)

# Slot Class
class BatchPlacerExportSlotUI(slot_form_class, slot_base_class):
    def __init__(self, mainui, parent=None):
        super(BatchPlacerExportSlotUI, self).__init__(parent)
        self.setupUi(self)
        self.connectInterface()
        self.parentui = mainui
        self.setLayout(self.horizontalLayout)

    def connectInterface(self):
        QtCore.QObject.connect(self.btnRemoveSlot, QtCore.SIGNAL("clicked()"), self.removeFromParent)
        QtCore.QObject.connect(self.btnMoveUpSlot, QtCore.SIGNAL("clicked()"), self.moveUpInParent)
        QtCore.QObject.connect(self.btnMoveDownSlot, QtCore.SIGNAL("clicked()"), self.moveDownInParent)
        QtCore.QObject.connect(self.lineObjName, QtCore.SIGNAL("textEdited(QString)"), self.saveAllSlots)
        QtCore.QObject.connect(self.lineUnrealName, QtCore.SIGNAL("textEdited(QString)"), self.saveAllSlots)

    def removeFromParent(self):
        for i in xrange(self.parentui.listWidget.count()):
            slotWidget = self.parentui.listWidget.itemWidget(self.parentui.listWidget.item(i))
            if (self == slotWidget):
                self.parentui.listWidget.takeItem(i)
                self.parentui.saveAllSlots()
                return

    def moveUpInParent(self):
        self.parentui.moveUpSlot(self)

    def moveDownInParent(self):
        self.parentui.moveDownSlot(self)

    def saveAllSlots(self, newtext):
        self.parentui.saveAllSlots()


# Interface Class
class BatchPlacerUI(main_form_class, main_base_class):
    def __init__(self, parent=None):
        super(BatchPlacerUI, self).__init__(parent)
        self.setupUi(self)
        self.setObjectName('UE4BatchPlacer')
        self.connectInterface()

        lines = [line.strip() for line in open(usersettings)]

        for x, y in pairwise(lines):
                item_widget = self.addExportSlot()
                item_widget.lineObjName.setText(x)
                item_widget.lineUnrealName.setText(y)

    def connectInterface(self):
        QtCore.QObject.connect(self.btnExportSelected, QtCore.SIGNAL("clicked()"), self.exportSelected)
        QtCore.QObject.connect(self.btnAddExportSlot, QtCore.SIGNAL("clicked()"), self.addExportSlot)

    def getUnrealMesh(self, inputName):
        if inputName is None:
            return None;
        for i in xrange(self.listWidget.count()):
            slotWidget = self.listWidget.itemWidget(self.listWidget.item(i))
            if slotWidget.lineObjName.text() not in inputName: continue
            return slotWidget.lineUnrealName.text()

    def moveUpSlot(self, slotitem):
        taken = None
        foundId = 0
        mayaName = None
        unrealName = None
        for i in xrange(self.listWidget.count()):
            slotWidget = self.listWidget.itemWidget(self.listWidget.item(i))
            if (slotitem == slotWidget):
                if (i is not 0):
                    mayaName = slotitem.lineObjName.text()
                    unrealName = slotitem.lineUnrealName.text()
                    taken = self.listWidget.takeItem(i)
                    foundId = i
                break
        if (taken is not None):
            self.listWidget.insertItem(foundId-1,taken)
            item_widget = BatchPlacerExportSlotUI(self, self)
            item_widget.lineObjName.setText(mayaName)
            item_widget.lineUnrealName.setText(unrealName)
            taken.setSizeHint(item_widget.sizeHint())
            self.listWidget.setItemWidget(taken, item_widget)
        self.saveAllSlots()

    def moveDownSlot(self, slotitem):
        taken = None
        foundId = 0
        mayaName = None
        unrealName = None
        for i in xrange(self.listWidget.count()):
            slotWidget = self.listWidget.itemWidget(self.listWidget.item(i))
            if (slotitem == slotWidget):
                if (i is not self.listWidget.count()-1):
                    mayaName = slotitem.lineObjName.text()
                    unrealName = slotitem.lineUnrealName.text()
                    taken = self.listWidget.takeItem(i)
                    foundId = i
                break
        if (taken is not None):
            self.listWidget.insertItem(foundId+1,taken)
            item_widget = BatchPlacerExportSlotUI(self, self)
            item_widget.lineObjName.setText(mayaName)
            item_widget.lineUnrealName.setText(unrealName)
            taken.setSizeHint(item_widget.sizeHint())
            self.listWidget.setItemWidget(taken, item_widget)
        self.saveAllSlots()

    def addExportSlot(self):
        item = QtGui.QListWidgetItem(self.listWidget)
        item_widget = BatchPlacerExportSlotUI(self, self)
        item_widget.setLayout(item_widget.horizontalLayout)
        item.setSizeHint(item_widget.sizeHint())
        self.listWidget.addItem(item)
        self.listWidget.setItemWidget(item, item_widget)
        return item_widget

    def exportSelected(self):
        exportable = False
        export_body = ""
        selection = om.MSelectionList()
        om.MGlobal.getActiveSelectionList(selection)
        selection_iter = om.MItSelectionList(selection)
        while not selection_iter.isDone():
            obj = om.MObject()
            dagPath = om.MDagPath()
            selection_iter.getDependNode(obj)
            selection_iter.getDagPath(dagPath)
            node = om.MFnDependencyNode(obj)
            unrealName = self.getUnrealMesh(node.name())
            if (unrealName is not None):
                exportable = True
                mt = om.MTransformationMatrix(dagPath.inclusiveMatrix())
                loc = mt.translation(om.MSpace.kWorld)
                rot = mt.rotation().asEulerRotation()
                scaleUtil = om.MScriptUtil()
                scaleUtil.createFromList([0,0,0],3)
                scaleVec = scaleUtil.asDoublePtr()
                mt.getScale(scaleVec, om.MSpace.kWorld)
                scale = [om.MScriptUtil.getDoubleArrayItem(scaleVec, i) for i in range(0,3)]
                if (cmds.upAxis(q=True,axis=True) == "y"):
                        export_body += GetTemplatedExportBody(node.name(), unrealName, loc.x, loc.z, loc.y, math.degrees(rot.x), math.degrees(rot.z), math.degrees(rot.y), scale[0], scale[2], scale[1])
                else:
                        export_body += GetTemplatedExportBody(node.name(), unrealName, loc.x, loc.y, loc.z, math.degrees(rot.x), math.degrees(rot.y), math.degrees(rot.z), scale[0], scale[1], scale[2])
            selection_iter.next()
        if (exportable is True):
                export = ue4_static_header + export_body + ue4_static_footer
                clipboard = QtGui.QApplication.clipboard()
                clipboard.setText(export)


    def saveAllSlots(self):
        f = open(usersettings, 'w')
        for i in xrange(self.listWidget.count()):
            slotWidget = self.listWidget.itemWidget(self.listWidget.item(i))
            f.write(slotWidget.lineObjName.text() + '\n')
            f.write(slotWidget.lineUnrealName.text() + '\n')
        f.close()

# main
def main():
    global ui
    ui = BatchPlacerUI()
    ui.show()

def show():
    global ui
    if (ui != None):
        ui.show()
    else:
        main()

if __name__ == "__main__":
    main()
