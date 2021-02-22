# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import sys
sys.path.append('.')
sys.path.append('build/lib.linux-x86_64-3.6')
from pymargo import MargoInstance
import pybake
import pybake.server
from pybake.server import BakeProvider

mid = MargoInstance('tcp')
mid.enable_remote_shutdown()
provider_id = 42
print("Server running at address "+str(mid.addr())+" with provider_id="+str(provider_id))

provider = BakeProvider(mid, provider_id)
target = provider.create_target("/dev/shm/baketarget", 10*1024*1024)
print("target id is "+str(target))
print("number of targets: "+str(provider.count_storage_targets()))

print("storage targets: ")
targets = provider.list_targets()
for t in targets:
  print(str(t))

provider.detach_all_targets()
print "number of targets: "+str(provider.count_targets())

mid.wait_for_finalize()
