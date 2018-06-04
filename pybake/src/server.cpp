/*
 * (C) 2018 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include <pybind11/pybind11.h>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <margo.h>
#include <bake.h>
#include <bake-server.h>

namespace py11 = pybind11;

typedef py11::capsule pymargo_instance_id;
typedef py11::capsule pymargo_addr;
typedef py11::capsule pybake_provider_t;

#define MID2CAPSULE(__mid)   py11::capsule((void*)(__mid), "margo_instance_id", nullptr)
#define ADDR2CAPSULE(__addr) py11::capsule((void*)(__addr), "hg_addr_t", nullptr)
#define BAKEPR2CAPSULE(__bpr) py11::capsule((void*)(__bpr), "bake_provider_t", nullptr)

static pybake_provider_t pybake_provider_register(pymargo_instance_id mid, uint8_t provider_id) {
    bake_provider_t provider;
    int ret = bake_provider_register(mid, provider_id, BAKE_ABT_POOL_DEFAULT, &provider);
    if(ret != 0) return py11::none();
    else return BAKEPR2CAPSULE(provider);
}

static py11::object pybake_provider_add_storage_target(
        pybake_provider_t provider,
        const std::string& target_name) {
    bake_target_id_t target_id;
    std::memset(&target_id, 0, sizeof(target_id));
    int ret = bake_provider_add_storage_target(
                provider, target_name.c_str(), &target_id);
    if(ret != 0) {
        return py11::none();
    }
    return py11::cast(target_id);
}

static bool pybake_provider_remove_storage_target(
        pybake_provider_t provider,
        bake_target_id_t target_id)
{
    return 0 == bake_provider_remove_storage_target(provider, target_id);
}

static bool pybake_provider_remove_all_storage_targets(
        pybake_provider_t provider)
{
    return 0 == bake_provider_remove_all_storage_targets(provider);
}

static uint64_t pybake_provider_count_storage_targets(
        pybake_provider_t provider)
{
    uint64_t n = 0;
    bake_provider_count_storage_targets(provider, &n);
    return n;
}

static py11::list pybake_provider_list_storage_targets(
            pybake_provider_t provider)
{
    std::vector<bake_target_id_t> result;
    uint64_t n = pybake_provider_count_storage_targets(provider);
    if(n == 0) return py11::list();
    result.resize(n);
    bake_provider_list_storage_targets(provider, result.data());
    py11::list list_result;
    for(const auto& t : result) list_result.append(t);
    return list_result;
}

static bool pybake_make_pool(const std::string& pool_name,
                size_t pool_size, mode_t mode) {
    return 0 == bake_makepool(pool_name.c_str(), pool_size, mode);
}

PYBIND11_MODULE(_pybakeserver, m)
{
    py11::module::import("_pybaketarget");
    m.def("register", &pybake_provider_register);
    m.def("add_storage_target", &pybake_provider_add_storage_target);
    m.def("remove_storage_target", &pybake_provider_remove_storage_target);
    m.def("remove_all_storage_targets", &pybake_provider_remove_all_storage_targets);
    m.def("count_storage_targets", &pybake_provider_count_storage_targets);
    m.def("list_storage_targets", &pybake_provider_list_storage_targets);
    m.def("make_pool", &pybake_make_pool);
}
