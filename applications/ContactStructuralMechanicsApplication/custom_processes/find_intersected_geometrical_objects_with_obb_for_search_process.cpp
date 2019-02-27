// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:             BSD License
//                                       license: StructuralMechanicsApplication/license.txt
//
//  Main authors:    Vicente Mataix Ferrandiz
//

// System includes

// External includes

// Project includes
#include "custom_processes/find_intersected_geometrical_objects_with_obb_for_search_process.h"

namespace Kratos
{
FindIntersectedGeometricalObjectsWithOBBForSearchProcess::FindIntersectedGeometricalObjectsWithOBBForSearchProcess(
    ModelPart& rPart1,
    ModelPart& rPart2,
    const double BoundingBoxFactor,
    const bool DebugOBB
    ) : BaseType(rPart1, rPart2, BoundingBoxFactor, DebugOBB)
{
}

/***********************************************************************************/
/***********************************************************************************/

FindIntersectedGeometricalObjectsWithOBBForSearchProcess::FindIntersectedGeometricalObjectsWithOBBForSearchProcess(
    ModelPart& rPart1,
    ModelPart& rPart2,
    const double* NewScaleFactor,
    const double* NewOffset,
    const double BoundingBoxFactor,
    const bool DebugOBB
    ) : BaseType(rPart1, rPart2, NewScaleFactor, NewOffset, BoundingBoxFactor, DebugOBB)
{
}

/***********************************************************************************/
/***********************************************************************************/

FindIntersectedGeometricalObjectsWithOBBForSearchProcess::FindIntersectedGeometricalObjectsWithOBBForSearchProcess(
    Model& rModel,
    Parameters ThisParameters
    ) : BaseType(rModel, ThisParameters)
{
}

/***********************************************************************************/
/***********************************************************************************/

void FindIntersectedGeometricalObjectsWithOBBForSearchProcess::MarkIfIntersected(
    Condition& rCondition1,
    OtreeCellVectorType& rLeaves
    )
{
    for (auto p_leaf : rLeaves) {
        for (auto p_entity_2 : *(p_leaf->pGetObjects())) {
            if (HasIntersection(rCondition1.GetGeometry(),p_entity_2->GetGeometry())) {
                rCondition1.Set(SELECTED);
                return;
            }
        }
    }
}

/***********************************************************************************/
/***********************************************************************************/

Parameters FindIntersectedGeometricalObjectsWithOBBForSearchProcess::GetDefaultParameters()
{
    Parameters default_parameters = Parameters(R"(
    {
        "first_model_part_name"  : "",
        "second_model_part_name" : "",
        "scale_factor"           : [1.0, 1.0, 1.0],
        "offset"                 : [0.0, 0.0, 0.0],
        "bounding_box_factor"    : -1.0,
        "debug_obb"              : false
    })" );

    return default_parameters;
}

}  // namespace Kratos.
