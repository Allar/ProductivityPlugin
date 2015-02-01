fn getProductivityEnvironmentPaths =
(
 dosCommand "echo %UE4_PRODUCTIVITY% >env.txt"
 in_text = openfile "env.txt"
 str = readLine in_text
 close in_text
 deleteFile "env.txt"
 trimRight str
)

ue4_path = getProductivityEnvironmentPaths()
batchPlacerPython = ue4_path + "\\Max\\"

r = batchPlacerPython.count
for i = 1 to r do
(
	if batchPlacerPython[i] == "\\" then
	(
		batchPlacerPython = replace batchPlacerPython i 1 "/"
	)
)

python.execute("sys.path.append('" + batchPlacerPython + "')")
python.execute("import BatchPlacer")

fn batchPlaceSelected =
(
	for s in selection do
	(
		locx = s.pos.x as string
		locy = s.pos.y as string
		locz = s.pos.z as string
		rotx = s.rotation.x_rotation as string
		roty = s.rotation.y_rotation as string
		rotz = s.rotation.z_rotation as string
		scalex = s.scale.x as string
		scaley = s.scale.y as string
		scalez = s.scale.z as string
		
		args = "'" + s.name + "','"+ locx + "','" + locy + "','" + locz + "','" + rotx + "','" + roty + "','" + rotz + "','" + scalex + "','" + scaley + "','" + scalez + "'"
		cmd = "BatchPlacer.SendUnrealBatchPlaceData(" + args + ")"
		
		python.execute cmd
	)
)

macroScript Batch_Place_Selected category:"Productivity" tooltip:"Batch place selected objects in UE4"
icon:#("standard", 1) -- use first icon in standard
(
	batchPlaceSelected()
)