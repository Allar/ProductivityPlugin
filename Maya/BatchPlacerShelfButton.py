import sys
try:
    BatchPlacer.batchPlaceSelected()
except:
    Dir = os.environ['UE4_PRODUCTIVITY'].replace("\\","/") + "/Maya"
    if Dir not in sys.path:
        sys.path.append(Dir)
    try: reload(BatchPlacer)
    except: import BatchPlacer
    BatchPlacer.main()