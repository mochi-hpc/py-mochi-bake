# (C) 2018 The University of Chicago
# See COPYRIGHT in top-level directory.
import sys
sys.path.append('.')
sys.path.append('build/lib.linux-x86_64-3.7')
from pymargo.core import Engine
from pybake.target import BakeRegionID
from pybake.client import *

mid = Engine('ofi+tcp')

def test():

    server_addr = sys.argv[1]
    mplex_id    = int(sys.argv[2])

    client = BakeClient(mid)
    addr = mid.lookup(server_addr)
    ph = client.create_provider_handle(addr, mplex_id)

    # Testing get_eager_limit
    lim = ph.get_eager_limit()
    print("Eager limit is: "+str(lim))

    # probe the provider handle (for all targets)
    targets = ph.probe()
    print("Probe found the following targets:")
    for t in targets:
        print("===== "+str(t))

    target = targets[0]
    # create a 32-bytes region in the first target
    region = ph.create(target, 32)
    print("Created region "+str(region))

    regionstr = str(region)
    region = BakeRegionID.from_str(regionstr)
    print("reconverting region to string: "+str(region))
    # write into the region
    ph.write(target, region, 0, 'A'*16)
    ph.write(target, region, 16, 'B'*16)

    # get size of region
    try:
        s = ph.get_size(target, region)
        print("Region size is "+str(s))
    except:
        print("Getting region size is not supported")
    # persist region
    ph.persist(target, region, size=32)

    # read region
    result = ph.read(target, region, 8, 16)
    print("Reading region at offset 8, size 16 gives: "+str(result))

    del ph
    client.shutdown_service(addr)
    del addr
    client.finalize()

test()
mid.finalize()
