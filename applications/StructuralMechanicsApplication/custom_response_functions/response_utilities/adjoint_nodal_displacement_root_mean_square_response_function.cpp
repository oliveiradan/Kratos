// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:		 BSD License
//					 license: structural_mechanics_application/license.txt
//
//  Main authors:    
//

// System includes

// External includes

// Project includes
#include "adjoint_nodal_displacement_root_mean_square_response_function.h"

namespace Kratos
{
    AdjointNodalDisplacementRootMeanSquareResponseFunction::AdjointNodalDisplacementRootMeanSquareResponseFunction(ModelPart& rModelPart, Parameters ResponseSettings)
    : AdjointStructuralResponseFunction(rModelPart, ResponseSettings)
    {
        // Get id of node where a displacement should be traced
        const int id_traced_node = ResponseSettings["traced_node_id"].GetInt();

        // Get start and end time 
        mStartTime = 0.0;
        mEndTime = 10.0;

        // Get the corresponding dof to the displacement which should be traced
        // by this response function e.g. DISPLACEMENT_X, ROTATION_X,...
        mTracedDofLabel = ResponseSettings["traced_dof"].GetString();

        // Get pointer to traced node
        mpTracedNode = rModelPart.pGetNode(id_traced_node);

        // Check if variable for traced dof is valid
        if( !( KratosComponents<Variable<double>>::Has(mTracedDofLabel)) )
            KRATOS_ERROR << "Specified traced DOF is not available. Specified DOF: " << mTracedDofLabel << std::endl;
        else
        {
            const DoubleVariableType& r_traced_dof =
                KratosComponents<DoubleVariableType>::Get(mTracedDofLabel);
            KRATOS_ERROR_IF_NOT( mpTracedNode->SolutionStepsDataHas(r_traced_dof) )
                << "Specified DOF is not available at traced node." << std::endl;
        }

        // Check if variable for traced adjoint dof is valid
        if( !(KratosComponents<Variable<double>>::Has(std::string("ADJOINT_") + mTracedDofLabel)) )
        {
            KRATOS_ERROR << "Specified traced adjoint DOF is not available." << std::endl;
        }

        this->GetNeighboringElementPointer();
    }

    AdjointNodalDisplacementRootMeanSquareResponseFunction::~AdjointNodalDisplacementRootMeanSquareResponseFunction(){}

    void AdjointNodalDisplacementRootMeanSquareResponseFunction::CalculateGradient(const Element& rAdjointElement,
                                   const Matrix& rResidualGradient,
                                   Vector& rResponseGradient,
                                   const ProcessInfo& rProcessInfo)
    {
        KRATOS_TRY;

        if (rResponseGradient.size() != rResidualGradient.size1())
            rResponseGradient.resize(rResidualGradient.size1(), false);

        rResponseGradient.clear();

        rResponseGradient(2) = 2 * rAdjointElement.GetGeometry()[0].FastGetSolutionStepValue(DISPLACEMENT_Z);
        
        KRATOS_CATCH("");
        
    }

    void AdjointNodalDisplacementRootMeanSquareResponseFunction::CalculateFirstDerivativesGradient(
        const Element& rAdjointElement,
        const Matrix& rResidualGradient,
        Vector& rResponseGradient,
        const ProcessInfo& rProcessInfo)
    {
        KRATOS_TRY;

        if (rResponseGradient.size() != rResidualGradient.size1())
            rResponseGradient.resize(rResidualGradient.size1(), false);

        rResponseGradient.clear();

        rResponseGradient(2) = (1 - mAlphaResponseWeight) * 2 * rAdjointElement.GetGeometry()[0].FastGetSolutionStepValue(VELOCITY_Z);

        KRATOS_CATCH("");
    }

    void AdjointNodalDisplacementRootMeanSquareResponseFunction::CalculateFirstDerivativesGradient(
        const Condition& rAdjointCondition,
        const Matrix& rResidualGradient,
        Vector& rResponseGradient,
        const ProcessInfo& rProcessInfo)
    {
        KRATOS_TRY;
        rResponseGradient = ZeroVector(rResidualGradient.size1());
        KRATOS_CATCH("");
    }

    void AdjointNodalDisplacementRootMeanSquareResponseFunction::CalculateSecondDerivativesGradient(
        const Element& rAdjointElement,
        const Matrix& rResidualGradient,
        Vector& rResponseGradient,
        const ProcessInfo& rProcessInfo)
    {
        KRATOS_TRY;
        rResponseGradient = ZeroVector(rResidualGradient.size1());
        KRATOS_CATCH("");
    }

    void AdjointNodalDisplacementRootMeanSquareResponseFunction::CalculateSecondDerivativesGradient(
        const Condition& rAdjointCondition,
        const Matrix& rResidualGradient,
        Vector& rResponseGradient,
        const ProcessInfo& rProcessInfo)
    {
        KRATOS_TRY;
        rResponseGradient = ZeroVector(rResidualGradient.size1());
        KRATOS_CATCH("");
    }

    void AdjointNodalDisplacementRootMeanSquareResponseFunction::CalculatePartialSensitivity(Element& rAdjointElement,
                                             const Variable<double>& rVariable,
                                             const Matrix& rSensitivityMatrix,
                                             Vector& rSensitivityGradient,
                                             const ProcessInfo& rProcessInfo)
    {
        KRATOS_TRY;
        rSensitivityGradient = ZeroVector(rSensitivityMatrix.size1());
        KRATOS_CATCH("");
    }

    void AdjointNodalDisplacementRootMeanSquareResponseFunction::CalculatePartialSensitivity(Condition& rAdjointCondition,
                                             const Variable<double>& rVariable,
                                             const Matrix& rSensitivityMatrix,
                                             Vector& rSensitivityGradient,
                                             const ProcessInfo& rProcessInfo)
    {
        KRATOS_TRY;
        rSensitivityGradient = ZeroVector(rSensitivityMatrix.size1());
        KRATOS_CATCH("");
    }

    void AdjointNodalDisplacementRootMeanSquareResponseFunction::CalculatePartialSensitivity(Element& rAdjointElement,
                                             const Variable<array_1d<double, 3>>& rVariable,
                                             const Matrix& rSensitivityMatrix,
                                             Vector& rSensitivityGradient,
                                             const ProcessInfo& rProcessInfo)
    {
        KRATOS_TRY;
        rSensitivityGradient = ZeroVector(rSensitivityMatrix.size1());
        KRATOS_CATCH("");
    }

    void AdjointNodalDisplacementRootMeanSquareResponseFunction::CalculatePartialSensitivity(Condition& rAdjointCondition,
                                             const Variable<array_1d<double, 3>>& rVariable,
                                             const Matrix& rSensitivityMatrix,
                                             Vector& rSensitivityGradient,
                                             const ProcessInfo& rProcessInfo)
    {
        KRATOS_TRY;
        rSensitivityGradient = ZeroVector(rSensitivityMatrix.size1());
        KRATOS_CATCH("");
    }

    double AdjointNodalDisplacementRootMeanSquareResponseFunction::CalculateValue(ModelPart& rModelPart)
    {
        KRATOS_TRY;

        const double& x = mrModelPart.GetNode(1).FastGetSolutionStepValue(DISPLACEMENT_Z);
        const double& v = mrModelPart.GetNode(1).FastGetSolutionStepValue(VELOCITY_Z);  
        return mAlphaResponseWeight * x * x + (1 - mAlphaResponseWeight) * v * v;

        KRATOS_CATCH("");
    }

    
} // namespace Kratos.

