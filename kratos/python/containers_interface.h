//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//


#if !defined(KRATOS_CONTAINERS_PYTHON_INTERFACE_H_INCLUDED )
#define KRATOS_CONTAINERS_PYTHON_INTERFACE_H_INCLUDED



// System includes
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// External includes

// Project includes
#include "includes/define_python.h"

namespace Kratos
{

namespace Python
{
using namespace pybind11;

template< class TContainerType >
class ContainerInterface
{
    public:
    
    ContainerInterface(){};
    virtual ~ContainerInterface(){};
    
    void CreateInterface(pybind11::module& m, std::string ContainerName)
    {
        class_<TContainerType, typename TContainerType::Pointer  >(m,ContainerName.c_str())
        .def(init<>())
        .def("__len__",     [](TContainerType& self){return self.size();} )     
        .def("__setitem__", [](TContainerType& self, unsigned int i, typename TContainerType::value_type& value){self[i] = value;} ) 
        .def("__getitem__", [](TContainerType& self, unsigned int i){return self(i);} )
        .def("__iter__",    [](TContainerType& self){return make_iterator(self.begin(), self.end());},  keep_alive<0,1>())  
        .def("append",    [](TContainerType& self, typename TContainerType::value_type& value){self.push_back(value);}  ) 
        ;
    }
};

template< class TContainerType >
class PointerVectorSetPythonInterface
{
public:
    
    PointerVectorSetPythonInterface(){};
    virtual ~PointerVectorSetPythonInterface(){};
    
    void CreateInterface(pybind11::module& m, std::string ContainerName)
    {
        class_<TContainerType, typename TContainerType::Pointer  >(m,ContainerName.c_str())
        .def(init<>())
        .def("__len__",     [](TContainerType& self){return self.size();} )     
        .def("__contains__", [](TContainerType& self, const typename TContainerType::value_type& value){return (self.find(value.Id()) != self.end());} ) 
        .def("__contains__", [](TContainerType& self, unsigned int i){return (self.find(i) != self.end());} ) 
        .def("__setitem__", [](TContainerType& self, typename TContainerType::value_type& value){self[value.Id()] = value;} ) 
        .def("__setitem__", [](TContainerType& self, typename TContainerType::pointer& pvalue){self(pvalue->Id()) = pvalue;} ) 
        .def("__getitem__", [](TContainerType& self, unsigned int i){return self(i);} )
        .def("__iter__",    [](TContainerType& self){return make_iterator(self.begin(), self.end());},  keep_alive<0,1>()) 
        .def("append",    [](TContainerType& self, typename TContainerType::pointer value){self.push_back(value);}  ) 
        ;
    }
};

}  // namespace Python.

}  // namespace Kratos.

#endif // KRATOS_POINTER_VECTOR_SET_PYTHON_INTERFACE_H_INCLUDED defined 


