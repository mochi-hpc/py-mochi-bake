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

static py11::bytes pybake_target_id_to_string(bake_target_id_t tid) {
    char id[37];
    int ret = bake_target_id_to_string(tid, id, 37);
    if(ret != BAKE_SUCCESS) return py11::bytes();
    else return py11::bytes(std::string(id));
}

static py11::object pybake_target_id_from_string(const py11::bytes& btidstr) {
    bake_target_id_t tid;
    memset(tid.id, 0, sizeof(uuid_t));
    std::string tidstr = (std::string)btidstr;
    if(tidstr.size() != 36) return py11::none();
    int ret = bake_target_id_from_string(tidstr.c_str(), &tid);
    if(ret == BAKE_SUCCESS) return py11::cast(tid);
    else return py11::none();
}

static py11::bytes pybake_region_id_to_string(const bake_region_id_t& region_id) {
    char id[128];
    memset(id, 0, 128);
    int ret = bake_region_id_to_string(region_id, id, 128);
    if(ret == BAKE_SUCCESS) {
        std::string result(id);
        return py11::bytes(result);
    } else {
        return py11::bytes();
    }
}

static py11::object pybake_region_id_from_string(const py11::bytes& bregion_str) {
    bake_region_id_t result;
    std::string region_str = (std::string)bregion_str;
    memset(&result, 0, sizeof(result));
    int ret = bake_region_id_from_string(region_str.c_str(), &result);
    if(ret != BAKE_SUCCESS) return py11::none();
    return py11::cast(result);
}

PYBIND11_MODULE(_pybaketarget, m)
{
    py11::class_<bake_target_id_t>(m,"bake_target_id");
    // now done at Python level
    //        .def("__str__", pybake_target_id_to_string);
    m.def("target_id_to_string", pybake_target_id_to_string);
    m.def("target_id_from_string", pybake_target_id_from_string);
    py11::class_<bake_region_id_t>(m,"bake_region_id");
    // now done at Python level
    //        .def("__str__", pybake_region_id_to_string);
    m.def("region_id_from_string", pybake_region_id_from_string);
    m.def("region_id_to_string", pybake_region_id_to_string);    
}
