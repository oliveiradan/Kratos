//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:      BSD License
//                Kratos default license: kratos/license.txt
//
//  Main authors: Miguel Angel Celigueta
//

#ifndef POST_PROCESS_UTILITIES_H
#define POST_PROCESS_UTILITIES_H

#include "includes/define.h"
#include "includes/variables.h"
#include "includes/model_part.h"
namespace Kratos
{
  ///@addtogroup FluidDynamicsApplication
  ///@{

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
  ///
  /** Auxiliary utility for computing different values as post processing.
   */
  class KRATOS_API(FLUID_DYNAMICS_APPLICATION) FluidPostProcessUtilities {
  public:

    ///@name Type Definitions
    ///@{

    typedef Node<3>                                               NodeType;
    typedef Geometry<NodeType>                                 GeometryType;
    typedef IntegrationPoint<3>                       IntegrationPointType;
    typedef std::vector<IntegrationPointType>   IntegrationPointsArrayType;

    /// Pointer definition of DragUtilities
    KRATOS_CLASS_POINTER_DEFINITION(FluidPostProcessUtilities);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor
    FluidPostProcessUtilities() {};

    /// Destructor.
    virtual ~FluidPostProcessUtilities() = default;


    /// Assignment operator.
    FluidPostProcessUtilities& operator=(FluidPostProcessUtilities const& rOther) = delete;

    /// Copy constructor.
    FluidPostProcessUtilities(FluidPostProcessUtilities const& rOther) = delete;

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    /**
     *  @brief This method calculates the flow throught the given modelpart (surface/line) in the normal
     *          direction.
     *  @param[in] rModelPart The model part instance where statistics are recorded.
     *  @return flow through the modelpart.
     */
    double CalculateFlow(const ModelPart& rModelPart);

    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    ///@}
    ///@name Friends
    ///@{


    ///@}
private:
    ///@name Static Member Variables
    ///@{


    ///@}
    ///@name Member Variables
    ///@{


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


}; // Class FluidPostProcessUtilities

} // namespace Kratos.

#endif // POST_PROCESS_UTILITIES_H
