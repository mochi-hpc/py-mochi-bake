# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import _pybaketarget
import base64

class BakeTargetID():

    def __init__(self, tid):
        self._tid = tid

    def __str__(self):
        return str(self._tid)

    @staticmethod
    def from_str(string):
        tid = _pybaketarget.target_id_from_string(string)
        if(tid is None):
            return None
        else:
            return BakeTargetID(tid)

class BakeRegionID():

    def __init__(self, rid):
        self._rid = rid
    
    def __str__(self):
        return base64.b64encode(str(self._rid))

    @staticmethod
    def from_str(string):
        rid = _pybaketarget.region_id_from_string(base64.b64decode(string))
        if(rid is None):
            return None
        else:
            return BakeRegionID(rid)
