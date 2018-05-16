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
#include <bake-server.h>

BOOST_PYTHON_OPAQUE_SPECIALIZED_TYPE_ID(margo_instance)
BOOST_PYTHON_OPAQUE_SPECIALIZED_TYPE_ID(bake_server_context_t)

namespace bpl = boost::python;

static bake_provider_t pybake_provider_register(margo_instance_id mid, uint8_t provider_id) {
    bake_provider_t provider;
    int ret = bake_provider_register(mid, provider_id, BAKE_ABT_POOL_DEFAULT, &provider);
    if(ret != 0) return NULL;
    else return provider;
}

static bpl::object pybake_provider_add_storage_target(
        bake_provider_t provider,
        const std::string& target_name) {
    bake_target_id_t target_id;
    std::memset(&target_id, 0, sizeof(target_id));
    int ret = bake_provider_add_storage_target(
                provider, target_name.c_str(), &target_id);
    if(ret != 0) return bpl::object();
    return bpl::object(target_id);
}

static bool pybake_provider_remove_storage_target(
        bake_provider_t provider,
        bake_target_id_t target_id)
{
    return 0 == bake_provider_remove_storage_target(provider, target_id);
}

static bool pybake_provider_remove_all_storage_targets(
        bake_provider_t provider)
{
    return 0 == bake_provider_remove_all_storage_targets(provider);
}

static uint64_t pybake_provider_count_storage_targets(
        bake_provider_t provider)
{
    uint64_t n = 0;
    bake_provider_count_storage_targets(provider, &n);
    return n;
}

static bpl::list pybake_provider_list_storage_targets(
            bake_provider_t provider)
{
    std::vector<bake_target_id_t> result;
    uint64_t n = pybake_provider_count_storage_targets(provider);
    if(n == 0) return bpl::list();
    result.resize(n);
    bake_provider_list_storage_targets(provider, result.data());
    bpl::list list_result;
    for(const auto& t : result) list_result.append(t);
    return list_result;
}

static bool pybake_make_pool(const std::string& pool_name,
                size_t pool_size, mode_t mode) {
    return 0 == bake_makepool(pool_name.c_str(), pool_size, mode);
}

BOOST_PYTHON_MODULE(_pybakeserver)
{
#define ret_policy_opaque bpl::return_value_policy<bpl::return_opaque_pointer>()

    bpl::import("_pybaketarget");
    bpl::opaque<bake_server_context_t>();
    bpl::def("register", &pybake_provider_register, ret_policy_opaque);
    bpl::def("add_storage_target", &pybake_provider_add_storage_target);
    bpl::def("remove_storage_target", &pybake_provider_remove_storage_target);
    bpl::def("remove_all_storage_targets", &pybake_provider_remove_all_storage_targets);
    bpl::def("count_storage_targets", &pybake_provider_count_storage_targets);
    bpl::def("list_storage_targets", &pybake_provider_list_storage_targets);
    bpl::def("make_pool", &pybake_make_pool);

#undef ret_policy_opaque
}
