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
        uint8_t mplex_id) {

    bake_provider_handle_t providerHandle = BAKE_PROVIDER_HANDLE_NULL;
    bake_provider_handle_create(client, addr, mplex_id, &providerHandle);
    return providerHandle;
}

BOOST_PYTHON_MODULE(_pybakeclient)
{
#define ret_policy_opaque bpl::return_value_policy<bpl::return_opaque_pointer>()

    bpl::opaque<bake_client>();
    bpl::opaque<bake_provider_handle>();
    bpl::def("client_init", &pybake_client_init, ret_policy_opaque);
    bpl::def("client_finalize", &bake_client_finalize);
    bpl::def("provider_handle_create", &pybake_provider_handle_create, ret_policy_opaque);
    bpl::def("provider_handle_ref_incr", &bake_provider_handle_ref_incr);
    bpl::def("provider_handle_release", &bake_provider_handle_release);
    bpl::def("shutdown_service", &bake_shutdown_service);

#undef ret_policy_opaque
}
