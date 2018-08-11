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

#if !defined(KRATOS_RESIDUALBASED_INCREMENTALUPDATE_STATIC_SCHEME_H )
#define  KRATOS_RESIDUALBASED_INCREMENTALUPDATE_STATIC_SCHEME_H

/* System includes */

/* External includes */

/* Project includes */
#include "solving_strategies/schemes/scheme.h"
#include "includes/variables.h"

namespace Kratos
{
///@name Kratos Globals
///@{
///@}
///@name Type Definitions
///@{
///@}
///@name  Enum's
///@{
///@}
///@name  Functions
///@{
///@}
///@name Kratos Classes
///@{

/** 
 * @class ResidualBasedIncrementalUpdateStaticScheme
 * @ingroup KratosCore
 * @brief This class provides the implementation of a static scheme
 * @details The only operation done in this  scheme is the update of the database, no predict is done
 * @tparam TSparseSpace The sparse space considered
 * @tparam TDenseSpace The dense space considered
 * @see Scheme
 * @author Riccardo Rossi
 */
template<class TSparseSpace,
         class TDenseSpace //= DenseSpace<double>
         >
class ResidualBasedIncrementalUpdateStaticScheme : public Scheme<TSparseSpace,TDenseSpace>
{

public:
    ///@name Type Definitions
    ///@{
    
    /// Pointer definition of ResidualBasedIncrementalUpdateStaticScheme
    KRATOS_CLASS_POINTER_DEFINITION( ResidualBasedIncrementalUpdateStaticScheme);

    /// Base class definition
    typedef Scheme<TSparseSpace,TDenseSpace>                      BaseType;

    /// DoF array type definition
    typedef typename BaseType::DofsArrayType                 DofsArrayType;

    /// Data type definition
    typedef typename BaseType::TDataType                         TDataType;
    /// Matrix type definition
    typedef typename BaseType::TSystemMatrixType         TSystemMatrixType;
    /// Vector type definition
    typedef typename BaseType::TSystemVectorType         TSystemVectorType;
    /// Local system matrix type definition
    typedef typename BaseType::LocalSystemVectorType LocalSystemVectorType;
    /// Local system vector type definition
    typedef typename BaseType::LocalSystemMatrixType LocalSystemMatrixType;

    /// Elements containers definition
    typedef ModelPart::ElementsContainerType             ElementsArrayType;
    /// Conditions containers definition
    typedef ModelPart::ConditionsContainerType         ConditionsArrayType;

    ///@}
    ///@name Life Cycle
    ///@{

    /** Constructor.
    */
    ResidualBasedIncrementalUpdateStaticScheme()
        : Scheme<TSparseSpace,TDenseSpace>()
    {}

    /** Destructor.
    */
    ~ResidualBasedIncrementalUpdateStaticScheme() override {}

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief Performing the update of the solution.
     * @param rModelPart The model part of the problem to solve
     * @param rDofSet Set of all primary variables
     * @param A LHS matrix
     * @param Dx Incremental update of primary variables
     * @param b RHS Vector
     */
    void Update(
        ModelPart& rModelPart,
        DofsArrayType& rDofSet,
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b
        ) override
    {
        KRATOS_TRY

        mpDofUpdater->UpdateDofs(rDofSet,Dx);

        KRATOS_CATCH("")
    }

    /**
     * @brief Performing the prediction of the solution.
     * @param rModelPart The model part of the problem to solve
     * @param A LHS matrix
     * @param Dx Incremental update of primary variables
     * @param b RHS Vector
     */
    void Predict(
        ModelPart& rModelPart,
        DofsArrayType& rDofSet,
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b
        ) override
    {
        KRATOS_TRY

        KRATOS_CATCH("")
    }

    /**
     * @brief It initializes a non-linear iteration (for the element)
     * @param rModelPart The model of the problem to solve
     * @param A LHS matrix
     * @param Dx Incremental update of primary variables
     * @param b RHS Vector
     */
    void InitializeNonLinIteration(
        ModelPart& rModelPart,
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b
        ) override
    {
        KRATOS_TRY;

        ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();
    
        // Initialize solution step for all of the elements
        #pragma omp parallel for
        for(int i=0; i<static_cast<int>(rModelPart.Elements().size()); i++) {
            auto it_elem = rModelPart.ElementsBegin() + i;
            it_elem->InitializeNonLinearIteration(r_current_process_info);
        }
        
        // Initialize solution step for all of the conditions
        #pragma omp parallel for
        for(int i=0; i<static_cast<int>(rModelPart.Conditions().size()); i++) {
            auto it_cond = rModelPart.ConditionsBegin() + i;
            it_cond->InitializeNonLinearIteration(r_current_process_info);
        } 
        
        KRATOS_CATCH( "" );
    }

    /**
     * @brief It initializes a non-linear iteration (for an individual condition)
     * @param rCurrentConditiont The condition to compute
     * @param rCurrentProcessInfo The current process info instance
     */
    void InitializeNonLinearIteration(
        Condition::Pointer rCurrentCondition,
        ProcessInfo& rCurrentProcessInfo
        ) override
    {
        (rCurrentCondition) -> InitializeNonLinearIteration(rCurrentProcessInfo);
    }

    /**
     * @brief It initializes a non-linear iteration (for an individual element)
     * @param rCurrentElement The element to compute
     * @param rCurrentProcessInfo The current process info instance
     */
    void InitializeNonLinearIteration(
        Element::Pointer rCurrentElement,
        ProcessInfo& rCurrentProcessInfo
        ) override
    {
        (rCurrentElement) -> InitializeNonLinearIteration(rCurrentProcessInfo);
    }

    /** 
     * @brief This function is designed to be called in the builder and solver to introduce the selected time integration scheme. 
     * @details It "asks" the matrix needed to the element and performs the operations needed to introduce the selected time integration scheme. This function calculates at the same time the contribution to the LHS and to the RHS of the system
     * @param pCurrentElement The element to compute
     * @param LHS_Contribution The LHS matrix contribution
     * @param RHS_Contribution The RHS vector contribution
     * @param EquationId The ID's of the element degrees of freedom
     * @param rCurrentProcessInfo The current process info instance
     */
    void CalculateSystemContributions(
        Element::Pointer rCurrentElement,
        LocalSystemMatrixType& LHS_Contribution,
        LocalSystemVectorType& RHS_Contribution,
        Element::EquationIdVectorType& EquationId,
        ProcessInfo& rCurrentProcessInfo
        ) override
    {
        KRATOS_TRY

        (rCurrentElement) -> CalculateLocalSystem(LHS_Contribution,RHS_Contribution,rCurrentProcessInfo);

        (rCurrentElement) -> EquationIdVector(EquationId,rCurrentProcessInfo);

        KRATOS_CATCH("")
    }
    
    /** 
     * @brief Functions totally analogous to the precedent but applied to the "condition" objects
     * @param pCurrentCondition The condition to compute
     * @param LHS_Contribution The LHS matrix contribution
     * @param RHS_Contribution The RHS vector contribution
     * @param EquationId The ID's of the condition degrees of freedom
     * @param rCurrentProcessInfo The current process info instance
     */
    void Condition_CalculateSystemContributions(
        Condition::Pointer rCurrentCondition,
        LocalSystemMatrixType& LHS_Contribution,
        LocalSystemVectorType& RHS_Contribution,
        Element::EquationIdVectorType& EquationId,
        ProcessInfo& rCurrentProcessInfo
        ) override
    {
        KRATOS_TRY

        (rCurrentCondition) -> CalculateLocalSystem(LHS_Contribution,RHS_Contribution,rCurrentProcessInfo);

        (rCurrentCondition) -> EquationIdVector(EquationId,rCurrentProcessInfo);

        KRATOS_CATCH("")
    }

    /**
     * @brief This function is designed to calculate just the RHS contribution
     * @param pCurrentElement The element to compute
     * @param RHS_Contribution The RHS vector contribution
     * @param EquationId The ID's of the element degrees of freedom
     * @param rCurrentProcessInfo The current process info instance
     */
    void Calculate_RHS_Contribution(
        Element::Pointer rCurrentElement,
        LocalSystemVectorType& RHS_Contribution,
        Element::EquationIdVectorType& EquationId,
        ProcessInfo& rCurrentProcessInfo
        ) override
    {
        KRATOS_TRY

        (rCurrentElement) -> CalculateRightHandSide(RHS_Contribution,rCurrentProcessInfo);

        (rCurrentElement) -> EquationIdVector(EquationId,rCurrentProcessInfo);

        KRATOS_CATCH("")
    }

    /**
     * @brief Functions totally analogous to the precedent but applied to the "condition" objects
     * @param pCurrentCondition The condition to compute
     * @param RHS_Contribution The RHS vector contribution
     * @param EquationId The ID's of the condition degrees of freedom
     * @param rCurrentProcessInfo The current process info instance
     */
    void Condition_Calculate_RHS_Contribution(
        Condition::Pointer rCurrentCondition,
        LocalSystemVectorType& RHS_Contribution,
        Element::EquationIdVectorType& EquationId,
        ProcessInfo& rCurrentProcessInfo
        ) override
    {
        KRATOS_TRY

        (rCurrentCondition) -> CalculateRightHandSide(RHS_Contribution,rCurrentProcessInfo);

        (rCurrentCondition) -> EquationIdVector(EquationId,rCurrentProcessInfo);

        KRATOS_CATCH("")
    }
    
    /**
     * @brief This function is designed to calculate just the LHS contribution
     * @param pCurrentElement The element to compute
     * @param LHS_Contribution The RHS vector contribution
     * @param EquationId The ID's of the element degrees of freedom
     * @param rCurrentProcessInfo The current process info instance
     */
    void Calculate_LHS_Contribution(
        Element::Pointer rCurrentElement,
        LocalSystemMatrixType& LHS_Contribution,
        Element::EquationIdVectorType& EquationId,
        ProcessInfo& rCurrentProcessInfo
        ) override
    {
        KRATOS_TRY

        (rCurrentElement) -> CalculateLeftHandSide(LHS_Contribution,rCurrentProcessInfo);

        (rCurrentElement) -> EquationIdVector(EquationId,rCurrentProcessInfo);

        KRATOS_CATCH("")
    }

    /**
     * @brief Liberate internal storage.
     */
    void Clear() override
    {
        this->mpDofUpdater->Clear();
    }
    
    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Friends
    ///@{

    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{

    ///@}

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    typename TSparseSpace::DofUpdaterPointerType mpDofUpdater = TSparseSpace::CreateDofUpdater(); /// The DoF updater, which will update the values

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    ///@}
    ///@name Private  Access
    ///@{
    
    ///@}
    ///@name Private Inquiry 
    ///@{

    ///@}
    ///@name Un accessible methods
    ///@{

    ///@}

}; // Class ResidualBasedIncrementalUpdateStaticScheme
}  // namespace Kratos

#endif /* KRATOS_RESIDUALBASED_INCREMENTALUPDATE_STATIC_SCHEME_H  defined */
