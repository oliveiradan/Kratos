//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics FemDem Application
//
//  License:		 BSD License
//					 Kratos default license:
//kratos/license.txt
//
//  Main authors:    Alejandro Cornejo Velazquez
//

#include "custom_processes/generate_dem_process.h"
#include "includes/define.h"
#include "includes/kratos_flags.h"

namespace Kratos {

template <SizeType TDim>
GenerateDemProcess<TDim>::GenerateDemProcess(
    ModelPart& rModelPart,
    ModelPart& rDemModelPart)
    : mrModelPart(rModelPart),
      mrDEMModelPart(rDemModelPart)
{
}

/***********************************************************************************/
/***********************************************************************************/

template <>
void GenerateDemProcess<2>::Execute() 
{
    const auto it_element_begin = mrModelPart.ElementsBegin();
    //#pragma omp parallel for
    for (int i = 0; i < static_cast<int>(mrModelPart.Elements().size()); i++) {
        auto it_elem = it_element_begin + i;
        auto& r_geom = it_elem->GetGeometry();

        bool is_active = true;
        if (it_elem->IsDefined(ACTIVE))
            is_active = it_elem->Is(ACTIVE);

        bool dem_generated = it_elem->GetValue(DEM_GENERATED);
        const int number_of_dem = this->GetNumberOfDemOnElement(it_elem);

        if (!is_active && !dem_generated) {
            // std::cout << "Elemento eliminado -- >" << it_elem->Id() << std::endl;
            auto p_DEM_properties = mrDEMModelPart.pGetProperties(1);
			auto& node0 = r_geom[0];
			auto& node1 = r_geom[1];
			auto& node2 = r_geom[2];
            const double dist01 = this->CalculateDistanceBetweenNodes(node0, node1);
            const double dist02 = this->CalculateDistanceBetweenNodes(node0, node2);
            const double dist12 = this->CalculateDistanceBetweenNodes(node1, node2);

            if (number_of_dem == 0) {
                // Node 0
                const double r0 = this->GetMinimumValue2(dist01, dist02)*0.5;
                const array_1d<double, 3> coordinates0 = this->GetNodeCoordinates(node0);
                this->CreateDEMParticle(node0.Id(), coordinates0, p_DEM_properties, r0, node0);
                // Node 1
                const double r1 = dist01 - r0;
                const array_1d<double, 3> coordinates1 = this->GetNodeCoordinates(node1);
                this->CreateDEMParticle(node1.Id(), coordinates1, p_DEM_properties, r1, node1);
                // Node 2
                const double r2 = this->GetMinimumValue2(dist02 - r0, dist12 - r1);
                const array_1d<double, 3> coordinates2 = this->GetNodeCoordinates(node2);
                this->CreateDEMParticle(node2.Id(), coordinates2, p_DEM_properties, r2, node2);
            }









        }
    }
}

/***********************************************************************************/
/***********************************************************************************/

template <>
void GenerateDemProcess<3>::Execute() 
{

}

/***********************************************************************************/
/***********************************************************************************/

template <SizeType TDim>
void GenerateDemProcess<TDim>::CreateDEMParticle(
    const int Id,
    const array_1d<double, 3> Coordinates,
    const Properties::Pointer pProperties,
    const double Radius,
    NodeType& rNode
)
{
    mParticleCreator.CreateSphericParticle(mrDEMModelPart, Id, Coordinates, pProperties, Radius, "SphericParticle3D");
    rNode.SetValue(IS_DEM, true);
    rNode.SetValue(RADIUS, Radius);
}

/***********************************************************************************/
/***********************************************************************************/

template <SizeType TDim>
int GenerateDemProcess<TDim>::GetNumberOfDemOnElement(
    ModelPart::ElementsContainerType::iterator ItElem
    )
{
    int number_dem = 0;
    auto& r_geom = ItElem->GetGeometry();
    for (int i = 0; i < r_geom.size(); i++) {
        auto& r_node = r_geom[i];
        if (r_node.GetValue(IS_DEM)) 
            number_dem++;
    }
    return number_dem;
}

/***********************************************************************************/
/***********************************************************************************/

template <SizeType TDim>
double GenerateDemProcess<TDim>::CalculateDistanceBetweenNodes(
    const NodeType& rNode1, 
    const NodeType& rNode2
    )
{
    const double X1 = rNode1.X();
    const double X2 = rNode2.X();
    const double Y1 = rNode1.Y();
    const double Y2 = rNode2.Y();
    const double Z1 = rNode1.Z();
    const double Z2 = rNode2.Z();
    return std::sqrt(std::pow(X1-X2, 2.0) + std::pow(Y1-Y2, 2.0) + std::pow(Z1-Z2, 2.0));
}

/***********************************************************************************/
/***********************************************************************************/

template <SizeType TDim>
array_1d<double,3> GenerateDemProcess<TDim>::GetNodeCoordinates(
    const NodeType& rNode
    )
{
    array_1d<double,3> coordinates;
    coordinates[0] = rNode.X();
    coordinates[1] = rNode.Y();
    coordinates[2] = rNode.Z();
    return coordinates;
}

/***********************************************************************************/
/***********************************************************************************/

template <SizeType TDim>
double GenerateDemProcess<TDim>::GetMinimumValue2(
    const double a, 
    const double b
    )
{
    if (a < b) return a;
    else return b;
}

/***********************************************************************************/
/***********************************************************************************/

template <SizeType TDim>
double GenerateDemProcess<TDim>::GetMinimumValue3(
    const double a, 
    const double b,
    const double c
    )
{
    double aux = a;
    if (aux > b) aux = b;
    if (aux > c) aux = c;
    return aux;
}

/***********************************************************************************/
/***********************************************************************************/

template class GenerateDemProcess<2>;
template class GenerateDemProcess<3>;

}  // namespace Kratos