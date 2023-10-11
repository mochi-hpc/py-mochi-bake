/*
 * (C) 2018 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <pybind11/pybind11.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <margo.h>
#include <bake.h>
#include <bake-server.h>

namespace py11 = pybind11;

#define HANDLE_ERROR(__func, __ret) do {\
    if(__ret != BAKE_SUCCESS) {\
        std::stringstream ss;\
        ss << #__func << "() failed (ret = " << __ret << ")";\
        throw std::runtime_error(ss.str());\
    }\
} while(0)

typedef py11::capsule pymargo_instance_id;
typedef py11::capsule pymargo_addr;
typedef py11::capsule pybake_provider_t;

#define MID2CAPSULE(__mid)   py11::capsule((void*)(__mid), "margo_instance_id")
#define ADDR2CAPSULE(__addr) py11::capsule((void*)(__addr), "hg_addr_t")
#define BAKEPR2CAPSULE(__bpr) py11::capsule((void*)(__bpr), "bake_provider_t")

static pybake_provider_t pybake_provider_register(
        pymargo_instance_id mid, uint8_t provider_id) {
    bake_provider_t provider;
    int ret = bake_provider_register(mid, provider_id, NULL, &provider);
    HANDLE_ERROR(bake_provider_register, ret);
    return BAKEPR2CAPSULE(provider);
}

static py11::object pybake_provider_attach_target(
        pybake_provider_t provider,
        const std::string& target_name) {
    bake_target_id_t target_id;
    std::memset(&target_id, 0, sizeof(target_id));
    int ret = bake_provider_attach_target(
                provider, target_name.c_str(), &target_id);
    HANDLE_ERROR(bake_provider_attach_target, ret);
    return py11::cast(target_id);
}

static py11::object pybake_provider_create_target(
        pybake_provider_t provider,
        const std::string& target_name,
        size_t target_size) {
    bake_target_id_t target_id;
    std::memset(&target_id, 0, sizeof(target_id));
    int ret = bake_provider_create_target(
                provider, target_name.c_str(), target_size, &target_id);
    HANDLE_ERROR(bake_provider_destroy_target, ret);
    return py11::cast(target_id);
}

static void pybake_provider_detach_target(
        pybake_provider_t provider,
        bake_target_id_t target_id)
{
    int ret = bake_provider_detach_target(provider, target_id);
    HANDLE_ERROR(bake_provider_detach_target, ret);
}

static void pybake_provider_detach_all_targets(
        pybake_provider_t provider)
{
    int ret = bake_provider_detach_all_targets(provider);
    HANDLE_ERROR(bake_provider_detach_all_targets, ret);
}

static uint64_t pybake_provider_count_targets(
        pybake_provider_t provider)
{
    uint64_t n = 0;
    int ret = bake_provider_count_targets(provider, &n);
    HANDLE_ERROR(bake_provider_count_targets, ret);
    return n;
}

static py11::list pybake_provider_list_targets(
            pybake_provider_t provider)
{
    std::vector<bake_target_id_t> result;
    uint64_t n = pybake_provider_count_targets(provider);
    if(n == 0) return py11::list();
    result.resize(n);
    bake_provider_list_targets(provider, result.data());
    py11::list list_result;
    for(const auto& t : result) list_result.append(t);
    return list_result;
}

PYBIND11_MODULE(_pybakeserver, m)
{
    py11::module::import("_pybaketarget");
    m.def("register", &pybake_provider_register);
    m.def("create_target", &pybake_provider_create_target);
    m.def("attach_target", &pybake_provider_attach_target);
    m.def("detach_target", &pybake_provider_detach_target);
    m.def("detach_all_targets", &pybake_provider_detach_all_targets);
    m.def("count_targets", &pybake_provider_count_targets);
    m.def("list_targets", &pybake_provider_list_targets);
}
