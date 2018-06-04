/*
 * (C) 2018 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <margo.h>
#include <bake.h>
#include <bake-client.h>

namespace py11 = pybind11;
namespace np = py11;

typedef py11::capsule pymargo_instance_id;
typedef py11::capsule pymargo_addr;
typedef py11::capsule pybake_client_t;
typedef py11::capsule pybake_provider_handle_t;

#define MID2CAPSULE(__mid)   py11::capsule((void*)(__mid), "margo_instance_id", nullptr)
#define ADDR2CAPSULE(__addr) py11::capsule((void*)(__addr), "hg_addr_t", nullptr)
#define BAKEPH2CAPSULE(__bph) py11::capsule((void*)(__bph), "bake_provider_handle_t", nullptr)
#define BAKECL2CAPSULE(__bcl) py11::capsule((void*)(__bcl), "bake_client_t", nullptr)

static pybake_client_t pybake_client_init(pymargo_instance_id mid) {
    bake_client_t result = BAKE_CLIENT_NULL;
    bake_client_init(mid, &result);
    return BAKECL2CAPSULE(result);
}

static pybake_provider_handle_t pybake_provider_handle_create(
        pybake_client_t client,
        pymargo_addr addr,
        uint8_t provider_id) {

    bake_provider_handle_t providerHandle = BAKE_PROVIDER_HANDLE_NULL;
    bake_provider_handle_create(client, addr, provider_id, &providerHandle);
    return BAKEPH2CAPSULE(providerHandle);
}

static uint64_t pybake_get_eager_limit(
    pybake_provider_handle_t ph)
{
    uint64_t limit;
    int ret = bake_provider_handle_get_eager_limit(ph, &limit);
    if(ret != 0) return 0;
    return limit;
}

static py11::object pybake_probe(
        pybake_provider_handle_t ph,
        uint64_t max_targets)
{
    py11::list result;
    std::vector<bake_target_id_t> targets(max_targets);
    uint64_t num_targets;
    int ret;
    Py_BEGIN_ALLOW_THREADS
    ret = bake_probe(ph, max_targets, targets.data(), &num_targets);
    Py_END_ALLOW_THREADS
    if(ret != 0) return py11::object();
    for(uint64_t i=0; i < num_targets; i++) {
        result.append(py11::cast(targets[i]));
    }
    return result;
}

static py11::object pybake_create(
        pybake_provider_handle_t ph,
        bake_target_id_t bti,
        size_t region_size)
{
    bake_region_id_t rid;
    std::memset(&rid, 0, sizeof(rid));
    int ret;
    Py_BEGIN_ALLOW_THREADS
    ret = bake_create(ph, bti, region_size, &rid);
    Py_END_ALLOW_THREADS
    if(ret != 0) return py11::none();
    else return py11::cast(rid);
}

static py11::object pybake_write(
        pybake_provider_handle_t ph,
        const bake_region_id_t& rid,
        uint64_t offset,
        const py11::bytes& bdata)
{
    int ret;
    Py_BEGIN_ALLOW_THREADS
    std::string data = (std::string)bdata;
    ret = bake_write(ph, rid, offset, (const void*)data.data(), data.size());
    Py_END_ALLOW_THREADS
    if(ret == 0) return py11::cast(true);
    else return py11::cast(false);
}

#if HAS_NUMPY
static py11::object pybake_write_numpy(
        pybake_provider_handle_t ph,
        const bake_region_id_t& rid,
        uint64_t offset,
        const np::array& data)
{
    if(!(data.flags() & 
            (np::array::f_style | np::array::c_style))) {
        std::cerr << "[pyBAKE error]: non-contiguous numpy arrays not yet supported" << std::endl;
        return py11::cast(false);
    }
    size_t size = data.dtype().itemsize();
    for(int i = 0; i < data.ndim(); i++) {
        size *= data.shape(i);
    }
    const void* buffer = data.data();
    int ret;
    Py_BEGIN_ALLOW_THREADS
    ret = bake_write(ph, rid, offset, buffer, size);
    Py_END_ALLOW_THREADS
    if(ret != 0) return py11::cast(false);
    else return py11::cast(true);
}
#endif

static py11::object pybake_persist(
        pybake_provider_handle_t ph,
        const bake_region_id_t& rid)
{
    int ret;
    Py_BEGIN_ALLOW_THREADS
    ret = bake_persist(ph, rid);
    Py_END_ALLOW_THREADS
    if(ret == 0) return py11::cast(true);
    else return py11::cast(false);
}

static py11::object pybake_create_write_persist(
        pybake_provider_handle_t ph,
        bake_target_id_t tid,
        const py11::bytes& bdata)
{
    bake_region_id_t rid;
    int ret;
    Py_BEGIN_ALLOW_THREADS
    std::string data = (std::string)bdata;
    ret = bake_create_write_persist(ph, tid, 
            data.data(), data.size(), &rid);
    Py_END_ALLOW_THREADS
    if(ret == 0) return py11::cast(rid);
    else return py11::none();
}

#if HAS_NUMPY
static py11::object pybake_create_write_persist_numpy(
        pybake_provider_handle_t ph,
        bake_target_id_t tid,
        const np::array& data)
{
    bake_region_id_t rid;
    if(!(data.flags() & (np::array::f_style | np::array::c_style))) {
        std::cerr << "[pyBAKE error]: non-contiguous numpy arrays not yet supported" << std::endl;
        return py11::none();
    }
    size_t size = data.dtype().itemsize();
    for(int i = 0; i < data.ndim(); i++) {
        size *= data.shape(i);
    }
    const void* buffer = data.data();
    int ret;
    Py_BEGIN_ALLOW_THREADS
    ret = bake_create_write_persist(ph, tid, 
            buffer, size, &rid);
    Py_END_ALLOW_THREADS
    if(ret == 0) return py11::cast(rid);
    else return py11::none();
}
#endif

static py11::object pybake_get_size(
        pybake_provider_handle_t ph,
        const bake_region_id_t& rid)
{
    uint64_t size;
    int ret;
    Py_BEGIN_ALLOW_THREADS
    ret = bake_get_size(ph, rid, &size);
    Py_END_ALLOW_THREADS
    if(ret == 0) return py11::cast(size);
    else return py11::none();
}

static py11::object pybake_read(
        pybake_provider_handle_t ph,
        const bake_region_id_t& rid,
        uint64_t offset,
        size_t size) 
{
    std::string result(size, '\0');
    uint64_t bytes_read;
    int ret;
    Py_BEGIN_ALLOW_THREADS
    ret = bake_read(ph, rid, offset, (void*)result.data(), size, &bytes_read);
    Py_END_ALLOW_THREADS
    if(ret != 0) return py11::none();
    result.resize(bytes_read);
    return py11::bytes(result);
}

static py11::object pybake_migrate(
        pybake_provider_handle_t source_ph,
        const bake_region_id_t& source_rid,
        bool remove_source,
        const std::string& dest_addr,
        uint16_t dest_provider_id,
        bake_target_id_t dest_target_id) {
    bake_region_id_t dest_rid;
    int ret;
    Py_BEGIN_ALLOW_THREADS
    ret = bake_migrate(source_ph, source_rid,
            remove_source, dest_addr.c_str(), dest_provider_id,
            dest_target_id, &dest_rid);
    Py_END_ALLOW_THREADS
    if(ret != BAKE_SUCCESS) return py11::none();
    return py11::cast(dest_rid);
}

#if HAS_NUMPY
static py11::object pybake_read_numpy(
        pybake_provider_handle_t ph,
        const bake_region_id_t& rid,
        uint64_t offset,
        const py11::tuple& shape,
        const np::dtype& dtype)
{
    std::vector<ssize_t> sshape(shape.size());
    for(unsigned int i=0; i<sshape.size(); i++) sshape[i] = shape[i].cast<ssize_t>();
    np::array result(dtype, sshape);
    size_t size = dtype.itemsize();
    for(int i=0; i < result.ndim(); i++) 
        size *= result.shape(i);
    uint64_t bytes_read;
    int ret;
    Py_BEGIN_ALLOW_THREADS
    ret = bake_read(ph, rid, offset, (void*)result.data(), size, &bytes_read);
    Py_END_ALLOW_THREADS
    if(ret != 0) return py11::none();
    if(bytes_read != size) return py11::none();
    else return result;
}
#endif

PYBIND11_MODULE(_pybakeclient, m)
{
#if HAS_NUMPY
    try { py11::module::import("numpy"); }
    catch (...) {
        std::cerr << "[Py-BAKE] Error: could not import numpy at C++ level" << std::endl;
        exit(-1);
    }
#endif
    py11::module::import("_pybaketarget");
    m.def("client_init", &pybake_client_init);
    m.def("client_finalize", [](pybake_client_t clt) {
            return bake_client_finalize(clt);} );
    m.def("provider_handle_create", &pybake_provider_handle_create);
    m.def("provider_handle_ref_incr", [](pybake_provider_handle_t pbph) {
            return bake_provider_handle_ref_incr(pbph); });
    m.def("provider_handle_release", [](pybake_provider_handle_t pbph) {
            return bake_provider_handle_release(pbph); });
    m.def("get_eager_limit", &pybake_get_eager_limit);
    m.def("set_eager_limit", [](pybake_provider_handle_t pbph, uint64_t lim) {
            return bake_provider_handle_set_eager_limit(pbph, lim); });
    m.def("probe", &pybake_probe);
    m.def("create", &pybake_create);
    m.def("write", &pybake_write);
    m.def("persist", &pybake_persist);
    m.def("create_write_persist", &pybake_create_write_persist);
    m.def("get_size", &pybake_get_size);
    m.def("read", &pybake_read);
    m.def("remove", [](pybake_provider_handle_t pbph, bake_region_id_t rid) {
            return bake_remove(pbph, rid);} );
    m.def("migrate", &pybake_migrate);
    m.def("shutdown_service", [](pybake_client_t client, pymargo_addr addr) {
            return bake_shutdown_service(client, addr); });
#if HAS_NUMPY
    m.def("write_numpy", &pybake_write_numpy);
    m.def("create_write_persist_numpy", &pybake_create_write_persist_numpy);
    m.def("read_numpy", &pybake_read_numpy);
#endif
}
