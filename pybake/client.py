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
            A BakeRegionID object representing the region. None if an error occured.
        """
        rid = _pybakeclient.create(self._ph, bti._tid, region_size)
        if(rid == None):
            return None
        return BakeRegionID(rid)

    def write(self, rid, offset, data):
        """
        Writes data in a region, at a specified offset.

        Args:
            rid (BakeRegionID): region in which to write.
            offset (int): offset at which to write.
            data (str): data to write.
        Returns:
            True if the data was correctly written, False otherwise.
        """
        return _pybakeclient.write(self._ph, rid._rid, offset, data)

    def write_numpy(self, rid, offset, array):
        """
        Writes a numpy array in a region at a specified offset.

        Args:
            rid (BakeRegionID): region in which to write.
            offset (int): offset at which to write.
            data (numpy.ndarray): numpy array to write.
        Returns:
            True if the data was correctly written, False otherwise.
        """
        return _pybakeclient.write_numpy(self._ph, rid._rid, offset, array)

    def persist(self, rid):
        """
        Make the changes to a given region persist.
        
        Args:
            rid (BakeRegionID): region to persist.
        Returns:
            True if the region was correctly persisted, False otherwise.
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
        Returns:
            True if the region was correctly written and persisted, False otherwise.
        """
        rid = _pybakeclient.create_write_persist(self._ph, bti._tid, data)
        if(rid is None):
            return None
        return BakeRegionID(rid)

    def create_write_persist_numpy(self, bti, array):
        """
        Creates a new region, write the numpy array to it at a given offset,
        and persist the region.

        Args:
            bti (BakeTargetID): target id in which to create the region.
            size (int): size of the region to create.
            offset (int): offset at which to write data in the region.
            array (numpy.ndarray): numpy array to write.
        Returns:
            True if the region was correctly written and persisted, False otherwise.
        """
        rid = _pybakeclient.create_write_persist_numpy(self._ph, bti._tid, array)
        if(rid is None):
            return None
        return BakeRegionID(rid)

    def get_size(self, rid):
        """
        Get the size of a given region.

        Args:
            rid (BakeRegionID): region id.

        Returns:
            The size (int) of the provided region. None in case of error.
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
    
    def read_numpy(self, rid, offset, shape, dtype):
        """
        Reads the data contained in a given region, at a given offset,
        and interpret it as a numpy array of a given shape and datatype.
        This function will fail if the full array cannot be loaded
        (e.g. the size of the region from the provided offset is too small
         compared with the size of the numpy that should result from the call)

        Args:
            rid (BakeRegionID): region id.
            offset (int): offset at which to read.
            shape (tuple): shape of the resulting array.
            dtype (numpy.dtype): datatype of the resuling array.

        Returns:
            A numpy array or None if it could not be read.
        """
        return _pybakeclient.read_numpy(self._ph, rid._rid, offset, tuple(shape), dtype)

    def remove(self, rid):
        """
        Remove a region from its target.

        Args:
            rid (BakeRegionID): region to remove.
        """
        ret = _pybakeclient.remove(self._ph, rid._rid)
        return (ret == 0)

    def migrate_region(self, source_rid, dest_addr, dest_provider_id, dest_target, remove_source=True):
        """
        Migrates a give region from its source to a destination designated by
        an address, a provider id, and a target id. This function will also remove
        the original region if remove_source is set to True.

        Args:
            source_rid (BakeRegionID): region to remove.
            dest_addr (str): destination address.
            dest_provider_id (int): destination provider id.
            dest_target (BakeTargetID): destinatin target id.
            remove_source (bool): whether to remove the source region.
        Returns:
            The resulting BakeRegionID if successful, None otherwise.
        """
        ret = _pybakeclient.migrate_region(self._ph, source_rid._rid, remove_source,
                str(dest_addr), int(dest_provider_id), dest_target._tid)
        if(ret == None):
            return ret
        return BakeRegionID(ret)

    def migrate_target(self, source_tid, dest_addr, dest_provider_id, dest_root, remove_source=True):
        """
        Migrates a given target from its source to a destination designated by
        an address, and a provider id. This function will also remove the original region
        if remove_source is set to True.

        Args:
            source_tid (BakeTargetID): source target to move.
            dest_addr (str): destination address.
            dest_provider_id (int): destination provider id.
            dest_root (str): destination root where to migrate the target.
            remove_source (bool) whether to remove the source region or not.
        Returns:
            True if the target was correctly migrated.
        """
        return _pybakeclient.migrate_target(self._ph, source_tid._tid, remove_source,
                str(dest_addr), int(dest_provider_id), str(dest_root))
