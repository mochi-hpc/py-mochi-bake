# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import _pybakeclient
import pymargo

class BakeClient():

	def __init__(self, mid):
		self._client = _pybakeclient.client_init(mid._mid)

	def create_provider_handle(self, addr, mplex_id):
		ph = _pybakeclient.provider_handle_create(self._client, addr.get_hg_addr(), mplex_id)
		return BakeProviderHandle(ph)

	def shutdown_service(self, addr):
		_pybakeclient.shutdown_service(self._client, addr.get_hg_addr())
	
	def __del__(self):
		_pybakeclient.client_finalize(self._client)

class BakeProviderHandle():

	def __init__(self, ph):
		self._ph = ph
	
	def __del__(self):
		_pybakeclient.provider_handle_release(self._ph)
