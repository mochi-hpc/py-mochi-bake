# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import sys
from pymargo import MargoInstance
from pybake.target import BakeRegionID
from pybake.client import *
import numpy as np

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

# write into a region
arr = np.random.randn(5,6)
print "Writing the following numpy array: "
print str(arr)
region = ph.create_write_persist_numpy(target, arr)

# get size of region
s = ph.get_size(region)
print "Region size is "+str(s)

# read region
result = ph.read_numpy(region, 0, shape=(5,6), dtype=arr.dtype)
# check for equalit
print "Reading region gave the following numpy array: "
print str(result)

if((result == arr).all()):
    print "The two arrays are equal"
else:
    print "The two arrays are NOT equal"

del ph
client.shutdown_service(addr)
del addr
client.finalize()
mid.finalize()
