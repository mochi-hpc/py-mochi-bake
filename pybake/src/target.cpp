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
    uuid_unparse(tid.id, id);
    return py11::bytes(std::string(id));
}

static py11::object pybake_target_id_from_string(const py11::bytes& btidstr) {
    bake_target_id_t tid;
    memset(tid.id, 0, sizeof(uuid_t));
    std::string tidstr = (std::string)btidstr;
    if(tidstr.size() != 36) return py11::none();
    int ret = uuid_parse((char*)tidstr.c_str(), tid.id);
    if(ret == 0) return py11::cast(tid);
    else return py11::none();
}

static py11::bytes pybake_region_id_to_string(const bake_region_id_t& region_id) {
    std::string result((const char*)(&region_id), sizeof(region_id));
    return py11::bytes(result);
}

static py11::object pybake_region_id_from_string(const py11::bytes& bregion_str) {
    bake_region_id_t result;
    std::string region_str = (std::string)bregion_str;
    memset(&result, 0, sizeof(result));
    if(region_str.size() != sizeof(bake_region_id_t))
        return py11::none();
    memcpy(&result, region_str.data(), sizeof(bake_region_id_t));
    return py11::cast(result);
}

PYBIND11_MODULE(_pybaketarget, m)
{
    py11::class_<bake_target_id_t>(m,"bake_target_id")
        .def("__str__", pybake_target_id_to_string);
    m.def("target_id_from_string", pybake_target_id_from_string);
    py11::class_<bake_region_id_t>(m,"bake_region_id")
        .def("__str__", pybake_region_id_to_string);
    m.def("region_id_from_string", pybake_region_id_from_string);    
}
