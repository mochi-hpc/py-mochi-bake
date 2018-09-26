# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import sys
sys.path.append('.')
sys.path.append('build/lib.linux-x86_64-2.7')
from pymargo import MargoInstance
from pybake.target import BakeRegionID
from pybake.client import *

mid = MargoInstance('tcp')

server_addr = sys.argv[1]
mplex_id    = int(sys.argv[2])

client = BakeClient(mid)
addr = mid.lookup(server_addr)
ph = client.create_provider_handle(addr, mplex_id)

# Testing get_eager_limit
lim = ph.get_eager_limit()
print "Eager limit is: "+str(lim)

# probe the provider handle (for all targets)
targets = ph.probe()
print "Probe found the following targets:"
for t in targets:
    print "===== "+str(t)

target = targets[0]
# create a 32-bytes region in the first target
region = ph.create(target, 32)
print "Created region "+str(region)

regionstr = str(region)
region = BakeRegionID.from_str(regionstr)

# write into the region
ph.write(region, 0, 'A'*16)
ph.write(region, 16, 'B'*16)

# get size of region
s = ph.get_size(region)
print "Region size is "+str(s)

# persist region
ph.persist(region)

# read region
result = ph.read(region, 8, 16)
print "Reading region at offset 8, size 16 gives: "+str(result)

del ph
client.shutdown_service(addr)
del addr
client.finalize()
mid.finalize()
