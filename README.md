# ProductivityPlugin
Some tools and scripts that extend the functionality of UE4

This repo is so unstable and nothing here is guaranteed to work, or to be documented.

# What's Included?
## UE4 Plugin
1. Static Mesh Actor to Instanced Mesh Collection and vice versa

## 3D Package Scripts
1. [Batch Placer (Maya 2015 Only For Now)](https://www.youtube.com/watch?v=TcbSW4icYV4)
  1. Install by running Setup 3D Package Scripts.bat
  2. Shove this code into a Maya shelf button

```
import sys
try:
    BatchPlacer.show()
    BatchPlacer.activateWindow()
    BatchPlacer.raise_()
except:
    Dir = os.environ['UE4_PRODUCTIVITY'].replace("\\","/") + "/Maya"
    if Dir not in sys.path:
        sys.path.append(Dir)
    try: reload(BatchPlacer)
    except: import BatchPlacer
    BatchPlacer.main()
```