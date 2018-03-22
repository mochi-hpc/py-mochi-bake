import _pybakeserver
import pymargo
from pybake.target import BakeStorageTarget

def make_pool(name, size, mode):
	_pybakeserver.make_pool(name, size, mode)

class BakeProvider(pymargo.Provider):

	def __init__(self, mid, mplex_id):
		super(BakeProvider, self).__init__(mid, mplex_id)
		self._bake_inst = _pybakeserver.register(mid._mid, mplex_id)

	def add_storage_target(self, name):
		tid = _pybakeserver.add_storage_target(self._bake_inst, name)
		return BakeStorageTarget(tid)

	def remove_storage_target(self, target):
		return _pybakeserver.remove_storage_target(self._bake_inst, target._tid)

	def remove_all_storage_targets(self):
		return _pybakeserver.remove_all_storage_targets(self._bake_inst)

	def count_storage_targets(self):
		return _pybakeserver.count_storage_targets(self._bake_inst)

	def list_storage_targets(self):
		return _pybakeserver.list_storage_targets(self._bake_inst)
