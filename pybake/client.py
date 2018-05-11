# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import _pybakeclient
from pybake.target import *
import pymargo

class BakeClient():
    """
    The BakeClient class wraps a bake_client_t structure at C level.
    It registers the RPCs necessary to interact with a Bake provider.
    It can be used to create provider handles pointing to Bake providers.
    """

    def __init__(self, mid):
        """
        Constructor. Initializes a new BakeClient with a MargoInstance.

        Args:
            mid (MargoInstance): MargoInstance on which to register RPCs.
        """
        self._client = _pybakeclient.client_init(mid._mid)

    def create_provider_handle(self, addr, provider_id):
        """
        Creates a BakeProviderHandle object pointing to the given
        address and provider id.

        Args:
            addr (MargoAddress): Address of the Bake provider.
            provider_id (int): ID of the provider.
        """
        ph = _pybakeclient.provider_handle_create(self._client, addr.get_hg_addr(), provider_id)
        return BakeProviderHandle(ph)

    def shutdown_service(self, addr):
        """
        Shut down a MargoInstance running at a particular address.

        Args:
            addr (MargoAddress): Address of the MargoInstance to shut down.
        """
        _pybakeclient.shutdown_service(self._client, addr.get_hg_addr())
	
    def finalize(self):
        """
        Finalizes the underlying bake_client_t structure.
        """
        _pybakeclient.client_finalize(self._client)

class BakeProviderHandle():
    """
    The BakeProviderHandle class represents a handle to a remote Bake provider.
    Internally, this class wraps a bake_provider_handle_t C structure.
    """

    def __init__(self, ph):
        """
        Constructor. This is not supposed to be called by users.
        Users should create a BakeProviderHandle from a BakeClient clt
        by calling clt.create_provider_handle.
        """
        self._ph = ph
	
    def __del__(self):
        """
        Explicit destructor to call provider_handle_release on the underlying
        bake_provider_handle_t C structure.
        """
        _pybakeclient.provider_handle_release(self._ph)

    def get_eager_limit(self):
        """
        Get the threshold size bellow which this provider
        handle will embed data within the RPC arguments.
        """
        return _pybakeclient.get_eager_limit(self._ph)

    def set_eager_limit(self, limit):
        """
        Set the threshold size bellow which this provider handle
        will embed data within the RPC arguments.
        """ 
        return _pybakeclient.set_eager_limit(self._ph, limit)

    def probe(self, max_targets=0):
        """
        Get the list of BakeTargetIDs of targets located in
        this provider. If max_targets is not specified, this
        function will return all the target ids.
        """
        if(max_targets != 0):
            tgts = _pybakeclient.probe(self._ph, max_targets)
        else:
            num_targets = 32
            while(True):
                tgts = _pybakeclient.probe(self._ph, num_targets)
                if(len(tgts) == num_targets):
                    num_targets *= 2
                else:
                    break
        result = []
        for tgt in tgts:
            result.append(BakeTargetID(tgt))
        return result

    def create(self, bti, region_size):
        """
        Creates a region in the specified target, with a given size.

        Args:
            bti (BakeTargetID): ID of the bake target in which to create the region.
            region_size (int): size of the region to create.

        Returns:
            A BakeRegionID object representing the region.
        """
        rid = _pybakeclient.create(self._ph, bti._tid, region_size)
        return BakeRegionID(rid)

    def write(self, rid, offset, data):
        """
        Writes data in a region, at a specified offset.

        Args:
            rid (BakeRegionID): region in which to write.
            offset (int): offset at which to write.
            data (str): data to write.
        """
        return _pybakeclient.write(self._ph, rid._rid, offset, data)

    def persist(self, rid):
        """
        Make the changes to a given region persist.
        
        Args:
            rid (BakeRegionID): region to persist.
        """
        return _pybakeclient.persist(self._ph, rid._rid)

    def create_write_persist(self, bti, data):
        """
        Creates a new region, write data to it at a given offset,
        and persist the region.

        Args:
            bti (BakeTargetID): target id in which to create the region.
            size (int): size of the region to create.
            offset (int): offset at which to write data in the region.
            data (str): data to write.
        """
        rid = _pybakeclient.create_write_persist(self._ph, bti._tid, data)
        return BakeRegionID(rid)

    def get_size(self, rid):
        """
        Get the size of a given region.

        Args:
            rid (BakeRegionID): region id.

        Returns:
            The size (ind) of the provided region.
        """
        return _pybakeclient.get_size(self._ph, rid._rid)

    def read(self, rid, offset=0, size=-1):
        """
        Reads the data contained in a given region, at a given offset
        and with a given size. If the size is not provided,
        this function will first send an RPC to get the current region size
        and then read from the offset up to the end of the region. If the
        offset is not provided, the entire region is read.

        Args:
            rid (BakeRegionID): region id.
            offset (int): offset at which to read.
            size (int): size to read.

        Returns:
            The data read, in the form of a string, or None if an
            error occured.
        """
        if(size < 0):
            size = self.get_size(rid) - offset
        return _pybakeclient.read(self._ph, rid._rid, offset, size)
    
    def remove(self, rid):
        """
        Remove a region from its target.

        Args:
            rid (BakeRegionID): region to remove.
        """
        _pybakeclient.remove(self._ph, rid._rid)
