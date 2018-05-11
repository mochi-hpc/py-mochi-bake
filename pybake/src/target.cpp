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

namespace bpl = boost::python;

static std::string pybake_target_id_to_string(bake_target_id_t tid) {
    char id[37];
    uuid_unparse(tid.id, id);
    return std::string(id);
}

static bpl::object pybake_target_id_from_string(const std::string& tidstr) {
    bake_target_id_t tid;
    memset(tid.id, 0, sizeof(uuid_t));
    if(tidstr.size() != 36) return bpl::object();
    int ret = uuid_parse((char*)tidstr.c_str(), tid.id);
    if(ret == 0) return bpl::object(tid);
    else return bpl::object();
}

static std::string pybake_region_id_to_string(const bake_region_id_t& region_id) {
    std::string result((const char*)(&region_id), sizeof(region_id));
    return result;
}

static bpl::object pybake_region_id_from_string(const std::string& region_str) {
    bake_region_id_t result;
    memset(&result, 0, sizeof(result));
    if(region_str.size() != sizeof(bake_region_id_t))
        return bpl::object();
    memcpy(&result, region_str.data(), sizeof(bake_region_id_t));
    return bpl::object(result);
}

BOOST_PYTHON_MODULE(_pybaketarget)
{
    bpl::class_<bake_target_id_t>("bake_target_id", bpl::no_init)
        .def("__str__", pybake_target_id_to_string);
    bpl::def("target_id_from_string", pybake_target_id_from_string);
    bpl::class_<bake_region_id_t>("bake_region_id", bpl::no_init)
        .def("__str__", pybake_region_id_to_string);
    bpl::def("region_id_from_string", pybake_region_id_from_string);    
}
