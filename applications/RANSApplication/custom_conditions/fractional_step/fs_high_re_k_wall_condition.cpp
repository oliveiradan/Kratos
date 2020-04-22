//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya
//

// System includes

// External includes

// Project includes
#include "includes/cfd_variables.h"
#include "includes/checks.h"
#include "includes/process_info.h"
#include "includes/variables.h"

// Application includes
#include "custom_utilities/rans_calculation_utilities.h"
#include "rans_application_variables.h"

// Include base h
#include "fs_high_re_k_wall_condition.h"

namespace Kratos
{
template <unsigned int TDim, unsigned int TNumNodes>
void FSHighReKWallCondition<TDim, TNumNodes>::Initialize()
{
    KRATOS_TRY;

    if (RansCalculationUtilities::IsWall(*this))
    {
        const array_1d<double, 3>& rNormal = this->GetValue(NORMAL);
        KRATOS_ERROR_IF(norm_2(rNormal) == 0.0)
            << "NORMAL must be calculated before using this " << this->Info() << "\n";

        KRATOS_ERROR_IF(this->GetValue(NEIGHBOUR_ELEMENTS).size() == 0)
            << this->Info() << " cannot find parent element\n";

        const double nu = RansCalculationUtilities::EvaluateInParentCenter(
            KINEMATIC_VISCOSITY, *this);
        KRATOS_ERROR_IF(nu == 0.0)
            << "KINEMATIC_VISCOSITY is not defined in the parent element of "
            << this->Info() << "\n.";

        mWallHeight = RansCalculationUtilities::CalculateWallHeight(*this, rNormal);
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes>
void FSHighReKWallCondition<TDim, TNumNodes>::CalculateLeftHandSide(MatrixType& rLeftHandSideMatrix,
                                                                    ProcessInfo& rCurrentProcessInfo)
{
    VectorType RHS;
    this->CalculateLocalSystem(rLeftHandSideMatrix, RHS, rCurrentProcessInfo);
}

template <unsigned int TDim, unsigned int TNumNodes>
void FSHighReKWallCondition<TDim, TNumNodes>::CalculateLocalSystem(
    MatrixType& rLeftHandSideMatrix, VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    if (rCurrentProcessInfo[FRACTIONAL_STEP] == 1)
    {
        if (rLeftHandSideMatrix.size1() != VelocityLocalSize ||
            rLeftHandSideMatrix.size2() != VelocityLocalSize)
            rLeftHandSideMatrix.resize(VelocityLocalSize, VelocityLocalSize);

        if (rRightHandSideVector.size() != VelocityLocalSize)
            rRightHandSideVector.resize(VelocityLocalSize);

        noalias(rLeftHandSideMatrix) = ZeroMatrix(VelocityLocalSize, VelocityLocalSize);
        noalias(rRightHandSideVector) = ZeroVector(VelocityLocalSize);

        this->ApplyWallLaw(rLeftHandSideMatrix, rRightHandSideVector, rCurrentProcessInfo);
    }
    else if (rCurrentProcessInfo[FRACTIONAL_STEP] == 5)
    {
        if (rLeftHandSideMatrix.size1() != PressureLocalSize ||
            rLeftHandSideMatrix.size2() != PressureLocalSize)
            rLeftHandSideMatrix.resize(PressureLocalSize, PressureLocalSize);

        if (rRightHandSideVector.size() != PressureLocalSize)
            rRightHandSideVector.resize(PressureLocalSize);

        noalias(rLeftHandSideMatrix) = ZeroMatrix(PressureLocalSize, PressureLocalSize);
        noalias(rRightHandSideVector) = ZeroVector(PressureLocalSize);
    }
    else
    {
        if (rLeftHandSideMatrix.size1() != 0)
            rLeftHandSideMatrix.resize(0, 0, false);

        if (rRightHandSideVector.size() != 0)
            rRightHandSideVector.resize(0, false);
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes>
int FSHighReKWallCondition<TDim, TNumNodes>::Check(const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    int check = BaseType::Check(rCurrentProcessInfo);

    const GeometryType& r_geometry = this->GetGeometry();

    for (IndexType i_node = 0; i_node < TNumNodes; ++i_node)
    {
        const NodeType& r_node = r_geometry[i_node];

        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(VELOCITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(MESH_VELOCITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(ACCELERATION, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(DENSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(KINEMATIC_VISCOSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_KINETIC_ENERGY, r_node);

        KRATOS_CHECK_DOF_IN_NODE(VELOCITY_X, r_node);
        KRATOS_CHECK_DOF_IN_NODE(VELOCITY_Y, r_node);
        if (TDim == 3)
            KRATOS_CHECK_DOF_IN_NODE(VELOCITY_Z, r_node);
        KRATOS_CHECK_DOF_IN_NODE(PRESSURE, r_node);
    }

    return check;

    KRATOS_CATCH("");
}

template <>
void FSHighReKWallCondition<2, 2>::EquationIdVector(EquationIdVectorType& rResult,
                                                    ProcessInfo& rCurrentProcessInfo)
{
    if (rCurrentProcessInfo[FRACTIONAL_STEP] == 1)
    {
        unsigned int LocalIndex = 0;

        if (rResult.size() != VelocityLocalSize)
            rResult.resize(VelocityLocalSize, false);

        for (unsigned int iNode = 0; iNode < 2; ++iNode)
        {
            rResult[LocalIndex++] =
                this->GetGeometry()[iNode].GetDof(VELOCITY_X).EquationId();
            rResult[LocalIndex++] =
                this->GetGeometry()[iNode].GetDof(VELOCITY_Y).EquationId();
        }
    }
    else if (rCurrentProcessInfo[FRACTIONAL_STEP] == 5)
    {
        if (rResult.size() != PressureLocalSize)
            rResult.resize(PressureLocalSize, false);

        for (SizeType iNode = 0; iNode < 2; ++iNode)
        {
            rResult[iNode] = this->GetGeometry()[iNode].GetDof(PRESSURE).EquationId();
        }
    }
    else
    {
        rResult.resize(0, false);
    }
}

template <>
void FSHighReKWallCondition<3, 3>::EquationIdVector(EquationIdVectorType& rResult,
                                                    ProcessInfo& rCurrentProcessInfo)
{
    if (rCurrentProcessInfo[FRACTIONAL_STEP] == 1)
    {
        unsigned int LocalIndex = 0;

        if (rResult.size() != VelocityLocalSize)
            rResult.resize(VelocityLocalSize, false);

        for (unsigned int iNode = 0; iNode < 3; ++iNode)
        {
            rResult[LocalIndex++] =
                this->GetGeometry()[iNode].GetDof(VELOCITY_X).EquationId();
            rResult[LocalIndex++] =
                this->GetGeometry()[iNode].GetDof(VELOCITY_Y).EquationId();
            rResult[LocalIndex++] =
                this->GetGeometry()[iNode].GetDof(VELOCITY_Z).EquationId();
        }
    }
    else if (rCurrentProcessInfo[FRACTIONAL_STEP] == 5)
    {
        if (rResult.size() != PressureLocalSize)
            rResult.resize(PressureLocalSize, false);

        for (SizeType iNode = 0; iNode < 3; ++iNode)
        {
            rResult[iNode] = this->GetGeometry()[iNode].GetDof(PRESSURE).EquationId();
        }
    }
    else
    {
        rResult.resize(0, false);
    }
}

template <>
void FSHighReKWallCondition<2, 2>::GetDofList(DofsVectorType& rConditionDofList,
                                              ProcessInfo& rCurrentProcessInfo)
{
    if (rCurrentProcessInfo[FRACTIONAL_STEP] == 1)
    {
        if (rConditionDofList.size() != VelocityLocalSize)
            rConditionDofList.resize(VelocityLocalSize);

        SizeType LocalIndex = 0;

        for (SizeType iNode = 0; iNode < 2; ++iNode)
        {
            rConditionDofList[LocalIndex++] = this->GetGeometry()[iNode].pGetDof(VELOCITY_X);
            rConditionDofList[LocalIndex++] = this->GetGeometry()[iNode].pGetDof(VELOCITY_Y);
        }
    }
    else if (rCurrentProcessInfo[FRACTIONAL_STEP] == 5)
    {
        if (rConditionDofList.size() != PressureLocalSize)
            rConditionDofList.resize(PressureLocalSize);

        for (SizeType iNode = 0; iNode < 2; ++iNode)
        {
            rConditionDofList[iNode] = this->GetGeometry()[iNode].pGetDof(PRESSURE);
        }
    }
    else
    {
        rConditionDofList.resize(0);
    }
}

template <>
void FSHighReKWallCondition<3, 3>::GetDofList(DofsVectorType& rConditionDofList,
                                              ProcessInfo& rCurrentProcessInfo)
{
    if (rCurrentProcessInfo[FRACTIONAL_STEP] == 1)
    {
        if (rConditionDofList.size() != VelocityLocalSize)
            rConditionDofList.resize(VelocityLocalSize);

        SizeType LocalIndex = 0;

        for (SizeType iNode = 0; iNode < 3; ++iNode)
        {
            rConditionDofList[LocalIndex++] = this->GetGeometry()[iNode].pGetDof(VELOCITY_X);
            rConditionDofList[LocalIndex++] = this->GetGeometry()[iNode].pGetDof(VELOCITY_Y);
            rConditionDofList[LocalIndex++] = this->GetGeometry()[iNode].pGetDof(VELOCITY_Z);
        }
    }
    else if (rCurrentProcessInfo[FRACTIONAL_STEP] == 5)
    {
        if (rConditionDofList.size() != PressureLocalSize)
            rConditionDofList.resize(PressureLocalSize);

        for (SizeType iNode = 0; iNode < 3; ++iNode)
        {
            rConditionDofList[iNode] = this->GetGeometry()[iNode].pGetDof(PRESSURE);
        }
    }
    else
    {
        rConditionDofList.resize(0);
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
void FSHighReKWallCondition<TDim, TNumNodes>::GetValuesVector(Vector& Values, int Step)
{
    unsigned int LocalIndex = 0;

    if (Values.size() != VelocityLocalSize)
    {
        Values.resize(VelocityLocalSize, false);
    }

    for (unsigned int i = 0; i < TNumNodes; ++i)
    {
        array_1d<double, 3>& rVelocity =
            this->GetGeometry()[i].FastGetSolutionStepValue(VELOCITY, Step);
        for (unsigned int d = 0; d < TDim; ++d)
        {
            Values[LocalIndex++] = rVelocity[d];
        }
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
std::string FSHighReKWallCondition<TDim, TNumNodes>::Info() const
{
    std::stringstream buffer;
    buffer << "FSHighReKWallCondition" << TDim << "D";
    return buffer.str();
}

template <unsigned int TDim, unsigned int TNumNodes>
void FSHighReKWallCondition<TDim, TNumNodes>::PrintInfo(std::ostream& rOStream) const
{
    rOStream << "FSHighReKWallCondition";
}

template <unsigned int TDim, unsigned int TNumNodes>
void FSHighReKWallCondition<TDim, TNumNodes>::PrintData(std::ostream& rOStream) const
{
}

template <unsigned int TDim, unsigned int TNumNodes>
void FSHighReKWallCondition<TDim, TNumNodes>::ApplyWallLaw(MatrixType& rLocalMatrix,
                                                           VectorType& rLocalVector,
                                                           ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    if (RansCalculationUtilities::IsWall(*this))
    {
        const GeometryType& r_geometry = this->GetGeometry();
        // Get Shape function data
        Vector gauss_weights;
        Matrix shape_functions;
        RansCalculationUtilities::CalculateConditionGeometryData(
            r_geometry, this->GetIntegrationMethod(), gauss_weights, shape_functions);
        const IndexType num_gauss_points = gauss_weights.size();

        const size_t block_size = TDim;

        const double c_mu_25 = std::pow(rCurrentProcessInfo[TURBULENCE_RANS_C_MU], 0.25);
        const double kappa = rCurrentProcessInfo[WALL_VON_KARMAN];
        const double inv_kappa = 1.0 / kappa;
        const double beta = rCurrentProcessInfo[WALL_SMOOTHNESS_BETA];
        const double y_plus_limit = rCurrentProcessInfo[RANS_Y_PLUS_LIMIT];

        const double eps = std::numeric_limits<double>::epsilon();

        for (size_t g = 0; g < num_gauss_points; ++g)
        {
            const Vector& gauss_shape_functions = row(shape_functions, g);

            const array_1d<double, 3>& r_wall_velocity =
                RansCalculationUtilities::EvaluateInPoint(
                    r_geometry, VELOCITY, gauss_shape_functions);
            const double wall_velocity_magnitude = norm_2(r_wall_velocity);

            const double tke = RansCalculationUtilities::EvaluateInPoint(
                r_geometry, TURBULENT_KINETIC_ENERGY, gauss_shape_functions);
            const double rho = RansCalculationUtilities::EvaluateInPoint(
                r_geometry, DENSITY, gauss_shape_functions);
            const double nu = RansCalculationUtilities::EvaluateInPoint(
                r_geometry, KINEMATIC_VISCOSITY, gauss_shape_functions);

            double y_plus{0.0}, u_tau{0.0};
            RansCalculationUtilities::CalculateYPlusAndUtau(
                y_plus, u_tau, wall_velocity_magnitude, mWallHeight, nu, kappa, beta);
            y_plus = std::max(y_plus, y_plus_limit);

            if (wall_velocity_magnitude > eps)
            {
                const double u_tau = RansCalculationUtilities::SoftMax(
                    c_mu_25 * std::sqrt(std::max(tke, 0.0)),
                    wall_velocity_magnitude / (inv_kappa * std::log(y_plus) + beta));

                const double value = rho * std::pow(u_tau, 2) * gauss_weights[g] / wall_velocity_magnitude;

                for (size_t a = 0; a < r_geometry.PointsNumber(); ++a)
                {
                    for (size_t dim = 0; dim < TDim; ++dim)
                    {
                        for (size_t b = 0; b < r_geometry.PointsNumber(); ++b)
                        {
                            rLocalMatrix(a * block_size + dim, b * block_size + dim) +=
                                gauss_shape_functions[a] * gauss_shape_functions[b] * value;
                        }
                        rLocalVector[a * block_size + dim] -=
                            gauss_shape_functions[a] * value * r_wall_velocity[dim];
                    }
                }
            }
        }
    }

    KRATOS_CATCH("");
}

// template instantiations
template class FSHighReKWallCondition<2>;
template class FSHighReKWallCondition<3>;

} // namespace Kratos.
