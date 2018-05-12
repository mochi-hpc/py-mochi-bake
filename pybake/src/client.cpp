/*
 * (C) 2018 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#define BOOST_NO_AUTO_PTR
#include <boost/python.hpp>
#include <boost/python/return_opaque_pointer.hpp>
#include <boost/python/handle.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/return_value_policy.hpp>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <margo.h>
#include <bake.h>
#include <bake-client.h>
#if HAS_NUMPY
#include <boost/python/numpy.hpp>
namespace np = boost::python::numpy;
#endif

BOOST_PYTHON_OPAQUE_SPECIALIZED_TYPE_ID(margo_instance)
BOOST_PYTHON_OPAQUE_SPECIALIZED_TYPE_ID(bake_provider_handle)
BOOST_PYTHON_OPAQUE_SPECIALIZED_TYPE_ID(bake_client)
BOOST_PYTHON_OPAQUE_SPECIALIZED_TYPE_ID(hg_addr)

namespace bpl = boost::python;

static bake_client_t pybake_client_init(margo_instance_id mid) {
    bake_client_t result = BAKE_CLIENT_NULL;
    bake_client_init(mid, &result);
    return result;
}

static bake_provider_handle_t pybake_provider_handle_create(
        bake_client_t client,
        hg_addr_t addr,
        uint8_t provider_id) {

    bake_provider_handle_t providerHandle = BAKE_PROVIDER_HANDLE_NULL;
    bake_provider_handle_create(client, addr, provider_id, &providerHandle);
    return providerHandle;
}

static uint64_t pybake_get_eager_limit(
        bake_provider_handle_t ph)
{
    uint64_t limit;
    int ret = bake_provider_handle_get_eager_limit(ph, &limit);
    if(ret != 0) return 0;
    return limit;
}

static bpl::object pybake_probe(
        bake_provider_handle_t ph,
        uint64_t max_targets)
{
    bpl::list result;
    std::vector<bake_target_id_t> targets(max_targets);
    uint64_t num_targets;
    int ret = bake_probe(ph, max_targets, targets.data(), &num_targets);
    if(ret != 0) return bpl::object();
    for(uint64_t i=0; i < num_targets; i++) {
        result.append(bpl::object(targets[i]));
    }
    return result;
}

static bpl::object pybake_create(
        bake_provider_handle_t ph,
        bake_target_id_t bti,
        size_t region_size)
{
    bake_region_id_t rid;
    std::memset(&rid, 0, sizeof(rid));
    int ret = bake_create(ph, bti, region_size, &rid);
    if(ret != 0) return bpl::object();
    else return bpl::object(rid);
}

static bpl::object pybake_write(
        bake_provider_handle_t ph,
        const bake_region_id_t& rid,
        uint64_t offset,
        const std::string& data)
{
    int ret = bake_write(ph, rid, offset, (const void*)data.data(), data.size());
    if(ret == 0) return bpl::object(true);
    else return bpl::object(false);
}

#if HAS_NUMPY
static bpl::object pybake_write_numpy(
        bake_provider_handle_t ph,
        const bake_region_id_t& rid,
        uint64_t offset,
        const np::ndarray& data)
{
    if(!(data.get_flags() & np::ndarray::bitflag::V_CONTIGUOUS)) {
        std::cerr << "[pyBAKE error]: non-contiguous numpy arrays not yet supported" << std::endl;
        return bpl::object(false);
    }
    size_t size = data.get_dtype().get_itemsize();
    for(int i = 0; i < data.get_nd(); i++) {
        size *= data.shape(i);
    }
    void* buffer = data.get_data();
    int ret = bake_write(ph, rid, offset, buffer, size);
    if(ret != 0) return bpl::object(false);
    else return bpl::object(true);
}
#endif

static bpl::object pybake_persist(
        bake_provider_handle_t ph,
        const bake_region_id_t& rid)
{
    int ret = bake_persist(ph, rid);
    if(ret == 0) return bpl::object(true);
    else return bpl::object(false);
}

static bpl::object pybake_create_write_persist(
        bake_provider_handle_t ph,
        bake_target_id_t tid,
        const std::string& data)
{
    bake_region_id_t rid;
    int ret = bake_create_write_persist(ph, tid, 
            data.data(), data.size(), &rid);
    if(ret == 0) return bpl::object(rid);
    else return bpl::object();
}

#if HAS_NUMPY
static bpl::object pybake_create_write_persist_numpy(
        bake_provider_handle_t ph,
        bake_target_id_t tid,
        const np::ndarray& data)
{
    bake_region_id_t rid;
    if(!(data.get_flags() & np::ndarray::bitflag::V_CONTIGUOUS)) {
        std::cerr << "[pyBAKE error]: non-contiguous numpy arrays not yet supported" << std::endl;
        return bpl::object();
    }
    size_t size = data.get_dtype().get_itemsize();
    for(int i = 0; i < data.get_nd(); i++) {
        size *= data.shape(i);
    }
    void* buffer = data.get_data();
    int ret = bake_create_write_persist(ph, tid, 
            buffer, size, &rid);
    if(ret == 0) return bpl::object(rid);
    else return bpl::object();
}
#endif

static bpl::object pybake_get_size(
        bake_provider_handle_t ph,
        const bake_region_id_t& rid)
{
    uint64_t size;
    int ret = bake_get_size(ph, rid, &size);
    if(ret == 0) return bpl::object(size);
    else return bpl::object();
}

static bpl::object pybake_read(
        bake_provider_handle_t ph,
        const bake_region_id_t& rid,
        uint64_t offset,
        size_t size) 
{
    std::string result(size, '\0');
    uint64_t bytes_read;
    int ret = bake_read(ph, rid, offset, (void*)result.data(), size, &bytes_read);
    if(ret != 0) return bpl::object();
    result.resize(bytes_read);
    return bpl::object(result);
}

#if HAS_NUMPY
static bpl::object pybake_read_numpy(
        bake_provider_handle_t ph,
        const bake_region_id_t& rid,
        uint64_t offset,
        const bpl::tuple& shape,
        const np::dtype& dtype)
{
    np::ndarray result = np::empty(shape, dtype);
    size_t size = dtype.get_itemsize();
    for(int i=0; i < result.get_nd(); i++) 
        size *= result.shape(i);
    uint64_t bytes_read;
    int ret = bake_read(ph, rid, offset, (void*)result.get_data(), size, &bytes_read);
    if(ret != 0) return bpl::object();
    if(bytes_read != size) return bpl::object();
    else return result;
}
#endif

BOOST_PYTHON_MODULE(_pybakeclient)
{
#define ret_policy_opaque bpl::return_value_policy<bpl::return_opaque_pointer>()
#if HAS_NUMPY
    np::initialize();
#endif
    bpl::import("_pybaketarget");
    bpl::opaque<bake_client>();
    bpl::opaque<bake_provider_handle>();
    bpl::def("client_init", &pybake_client_init, ret_policy_opaque);
    bpl::def("client_finalize", &bake_client_finalize);
    bpl::def("provider_handle_create", &pybake_provider_handle_create, ret_policy_opaque);
    bpl::def("provider_handle_ref_incr", &bake_provider_handle_ref_incr);
    bpl::def("provider_handle_release", &bake_provider_handle_release);
    bpl::def("get_eager_limit", &pybake_get_eager_limit);
    bpl::def("set_eager_limit", &bake_provider_handle_set_eager_limit);
    bpl::def("probe", &pybake_probe);
    bpl::def("create", &pybake_create);
    bpl::def("write", &pybake_write);
    bpl::def("persist", &pybake_persist);
    bpl::def("create_write_persist", &pybake_create_write_persist);
    bpl::def("get_size", &pybake_get_size);
    bpl::def("read", &pybake_read);
    bpl::def("remove", &bake_remove);
    bpl::def("shutdown_service", &bake_shutdown_service);
#if HAS_NUMPY
    bpl::def("write_numpy", &pybake_write_numpy);
    bpl::def("create_write_persist_numpy", &pybake_create_write_persist_numpy);
    bpl::def("read_numpy", &pybake_read_numpy);
#endif

#undef ret_policy_opaque
}
