# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
from pymargo import MargoInstance
import pybake
import pybake.server
from pybake.server import BakeProvider

mid = MargoInstance('tcp')
mid.enable_remote_shutdown()
mplex_id = 42
print "Server running at address "+str(mid.addr())+"with mplex_id="+str(mplex_id)

provider = BakeProvider(mid, mplex_id)
target   = provider.add_storage_target("/dev/shm/baketarget")
print "target id is "+str(target)
print "number of targets: "+str(provider.count_storage_targets())

pybake.server.make_pool("/dev/shm/baketarget2", 2*8388608, 0664)
target = provider.add_storage_target("/dev/shm/baketarget2")
print "target id is "+str(target)
print "number of targets: "+str(provider.count_storage_targets())

print "storage targets: "
targets = provider.list_storage_targets()
for t in targets:
  print str(t)

provider.remove_all_storage_targets()
print "number of targets: "+str(provider.count_storage_targets())

mid.wait_for_finalize()
