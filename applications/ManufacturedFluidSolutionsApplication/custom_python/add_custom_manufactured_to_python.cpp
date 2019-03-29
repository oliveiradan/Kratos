//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Author1 Fullname
//                   Author2 Fullname
//


// System includes


// External includes
#include <pybind11/pybind11.h>


// Project includes
#include "includes/define_python.h"
#include "custom_python/add_custom_manufactured_to_python.h"

// Manufactured solutions


namespace Kratos {
namespace Python {

void  AddCustomStrategiesToPython(pybind11::module& m)
{
    namespace py = pybind11;

    //********************************************************************
    //********************************************************************
    //  py::class_< TestStrategy< SparseSpaceType, LocalSpaceType, LinearSolverType>,
    //  		TestStrategy< SparseSpaceType, LocalSpaceType, LinearSolverType>::Pointer,
    //          BaseSolvingStrategyType>(m, "TestStrategy")
    //  	.def(py::init<ModelPart&, LinearSolverType::Pointer, int, int, bool >() )
    //  	.def("MoveNodes",&TestStrategy< SparseSpaceType, LocalSpaceType, LinearSolverType >::MoveNodes)
    //  	;

}

} // namespace Python.
} // Namespace Kratos
