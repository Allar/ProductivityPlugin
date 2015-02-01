# ProductivityPlugin
Some tools and scripts that extend the functionality of UE4

This repo is so unstable and nothing here is guaranteed to work, or to be documented.

# What's Included?
## UE4 Plugin
1. Static Mesh Actor to Instanced Mesh Collection and vice versa
2. Ability to recieve batch placements of static meshes from any networked app (i.e. Maya)

## Plugin Installation
To build this plugin with your copy of UE4, place the repo so that all files are in Engine/Plugins/ProductivityPlugin. You should be able to regenerate your project files and build successfully.

If you aren't going to be building this plugin from source code, download a binary version from the releases page and extract the ProductivityPlugin folder into Engine/Plugins/

After either option above, *it is very important* that you run the "Setup 3D Package Scripts.bat" file in Engine/Plugins/ProductivityPlugin in either case.

## 3D Package Scripts
To use Productivity Plugin to its max potential, you will need to install these scripts:

### Maya
  1. Make sure you ran the "Setup 3D Package Scripts.bat" file as mentioned above.
  2. Shove [this code](Maya/BatchPlacerShelfButton.py) into a Maya shelf button. This button, when clicked, will batch place selected objects into UE4.
  
### Max (Must be able to run Python, so 2014+)
  1. Make sure you ran the "Setup 3D Package Scripts.bat" file as mentioned above.
  2. Copy [LoadBatchPlacer.ms](Max/LoadBatchPlacer.ms) to your 3DS Max "Additional Startup Scripts" directory. This can be found by going to Customize->Configure System Paths
  3. Use Customize->Customize Interface to assign the "Batch place selected objects in UE4" command under the "Productivity" category to anywhere on your interface or keyboard shortcuts.