import sys
try: BatchPlacer.show()
except:
    Dir = 'D:/allarjealdepot/Engine/Extras/ProductivityPlugin/Maya'
    if Dir not in sys.path:
        sys.path.append(Dir)
    try: reload(BatchPlacer)
    except: import BatchPlacer
    BatchPlacer.main()