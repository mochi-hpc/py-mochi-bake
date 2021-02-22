# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import _pybaketarget
import _pybakeserver
import pymargo
from pybake.target import BakeTargetID

class BakeProvider(pymargo.Provider):
    """
    The BakeProvide class wraps a C-level bake_provider_t object.
    """

    def __init__(self, engine, provider_id):
        """
        Constructor. Initializes a provider with an Engine and provider_id.
        """
        super(BakeProvider, self).__init__(engine, provider_id)
        self._provider = _pybakeserver.register(engine._mid, provider_id)

    def create_target(self, path, size):
        """
        Create a storage target and attach it to the provider.
        Returns a BakeTargetID instance that can be used to access the storage target.
        """
        tid = _pybakeserver.create_target(self._provider, path, size)
        return BakeTargetID(tid)

    def attach_target(self, path):
        """
        Adds a storage target to the provider.
        Returns a BakeTargetID instance that can be used to access the storage target.
        """
        tid = _pybakeserver.attach_target(self._provider, path)
        return BakeTargetID(tid)

    def detach_target(self, target):
        """
        Removes a storage target from the provider. This does not delete the underlying file.
        The target argument must be a BakeTargetID object.
        """
        _pybakeserver.detach_target(self._provider, target._tid)

    def detach_all_targets(self):
        """
        Removes all the storage targets managed by this provider.
        """
        _pybakeserver.detach_all_targets(self._provider)

    def count_targets(self):
        """
        Returns the number of storage targets that this provider manages.
        """
        return _pybakeserver.count_targets(self._provider)

    def list_targets(self):
        """
        Returns the list of storage targets (BakeTargetIDs) that this provider manages.
        """
        l = _pybakeserver.list_targets(self._provider)
        if(l is None):
            return []
        else:
            return [ BakeTargetID(tid) for tid in l ]
