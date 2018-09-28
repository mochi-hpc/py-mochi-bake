# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import _pybaketarget
import base64

class BakeTargetID():
    """
    The BakeTargetID class is a wrapper for a bake_target_id_t
    object at C level. It can be used by client and server
    interfaces. It can be converted into a string using the
    __str__ method, and be deserialized from a string using
    the BakeTargetID.from_str static method.
    """

    def __init__(self, tid):
        """
        Constructor. Not supposed to be called by users.
        """
        self._tid = tid

    def __str__(self):
        """
        Converts the BakeTargetID into a string.
        """
        if(self._tid is None):
            return str(None)
        byte_string = _pybaketarget.target_id_to_string(self._tid)
        return byte_string.decode()

    @staticmethod
    def from_str(byte_string):
        """
        Converts a string representation of the BakeTargetID into
        a concrete BakeTargetID object.
        """
        if(isinstance(byte_string,str)):
            byte_string = byte_string.encode()
        tid = _pybaketarget.target_id_from_string(byte_string)
        if(tid is None):
            return None
        else:
            return BakeTargetID(tid)

# ================================================================== #
# ================================================================== #
# ================================================================== #

class BakeRegionID():
    """
    The BakeRegionID is a wrapper for a bake_region_id_t object at
    C level. It provides functionalities to convert a BakeRegionID
    from/to a string representation, and can be used by the server
    and client API.
    """

    def __init__(self, rid):
        """
        Constructor. Not supposed to be called by users.
        """
        self._rid = rid
    
    def __str__(self):
        """
        Converts the BakeRegionID into a string.
        """
        if(self._rid is None):
            return str(None)
        a = _pybaketarget.region_id_to_string(self._rid)
        return base64.b64encode(a).decode()

    @staticmethod
    def from_str(byte_string):
        """
        Converts a string representing a BakeRegionID into a concrete
        BakeRegionID object.
        """
        if(isinstance(byte_string,str)):
            byte_string = byte_string.encode()
        rid = _pybaketarget.region_id_from_string(base64.b64decode(byte_string))
        if(rid is None):
            return None
        else:
            return BakeRegionID(rid)
