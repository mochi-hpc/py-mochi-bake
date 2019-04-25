# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import _pybaketarget
import _pybakeserver
import pymargo
from pybake.target import BakeTargetID

def make_pool(name, size, mode):
    _pybakeserver.make_pool(name, size, mode)

class BakeProvider(pymargo.Provider):
    """
    The BakeProvide class wraps a C-level bake_provider_t object.
    """

    def __init__(self, engine, provider_id):
        """
        Constructor. Initializes a provider with an Engine and provider_id.
        """
        super(BakeProvider, self).__init__(engine, provider_id)
        self._provider = _pybakeserver.register(mid._mid, provider_id)

    def add_storage_target(self, path):
        """
        Adds a storage target to the provider.
        Returns a BakeTargetID instance that can be used to access the storage target.
        """
        tid = _pybakeserver.add_storage_target(self._provider, path)
        return BakeTargetID(tid)

    def remove_storage_target(self, target):
        """
        Removes a storage target from the provider. This does not delete the underlying file.
        The target argument must be a BakeTargetID object.
        """
        _pybakeserver.remove_storage_target(self._provider, target._tid)

    def remove_all_storage_targets(self):
        """
        Removes all the storage targets managed by this provider.
        """
        _pybakeserver.remove_all_storage_targets(self._provider)

    def count_storage_targets(self):
        """
        Returns the number of storage targets that this provider manages.
        """
        return _pybakeserver.count_storage_targets(self._provider)

    def list_storage_targets(self):
        """
        Returns the list of storage targets (BakeTargetIDs) that this provider manages.
        """
        l = _pybakeserver.list_storage_targets(self._provider)
        if(l is None):
            return []
        else:
            return [ BakeTargetID(tid) for tid in l ]

#    def set_target_xfer_buffer(self, target, count, size):
#        """
#        Sets the number and size of intermediate buffers that can be used to
#        execute transfers to a specific target.
#        """
#        _pybakeserver.set_target_xfer_buffer(self._provider, target._tid, count, size)

#    def set_target_xfer_concurrency(self, target, num_threads):
#        """
#        Sets the number of ULTs that can be used to execute transfers concurrently.
#        """
#        _pybakeserver.set_target_xfer_concurrency(self._provider, target._tid, num_threads)
