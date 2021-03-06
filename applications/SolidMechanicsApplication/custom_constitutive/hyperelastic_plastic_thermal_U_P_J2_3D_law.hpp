//
//   Project Name:        KratosSolidMechanicsApplication $
//   Created by:          $Author:            JMCarbonell $
//   Last modified by:    $Co-Author:                     $
//   Date:                $Date:                July 2018 $
//   Revision:            $Revision:                  0.0 $
//
//

#if !defined (KRATOS_HYPERELASTIC_PLASTIC_THERMAL_U_P_J2_3D_LAW_H_INCLUDED)
#define  KRATOS_HYPERELASTIC_PLASTIC_THERMAL_U_P_J2_3D_LAW_H_INCLUDED

// System includes

// External includes

// Project includes
#include "custom_constitutive/hyperelastic_plastic_U_P_3D_law.hpp"



namespace Kratos
{
/**
 * Defines a hyperelastic-plastic thermal isotropic constitutive law J2 3D
 * With stress split in an isochoric and volumetric parts
 * This material law is defined by the parameters needed by the yield criterion:

 * The functionality is limited to large displacements
 */

class KRATOS_API(SOLID_MECHANICS_APPLICATION) HyperElasticPlasticThermalUPJ23DLaw : public HyperElasticPlasticUP3DLaw
{
public:
    /**
     * Type Definitions
     */
    typedef ProcessInfo      ProcessInfoType;
    typedef ConstitutiveLaw         BaseType;
    typedef std::size_t             SizeType;

    typedef FlowRule::Pointer                FlowRulePointer;
    typedef YieldCriterion::Pointer    YieldCriterionPointer;
    typedef HardeningLaw::Pointer        HardeningLawPointer;
    typedef Properties::Pointer            PropertiesPointer;

    /**
     * Counted pointer of HyperElasticPlasticThermalUPJ23DLaw
     */

    KRATOS_CLASS_POINTER_DEFINITION(HyperElasticPlasticThermalUPJ23DLaw);

    /**
     * Life Cycle
     */

    /**
     * Default constructor.
     */
    HyperElasticPlasticThermalUPJ23DLaw();


    HyperElasticPlasticThermalUPJ23DLaw(FlowRulePointer pFlowRule, YieldCriterionPointer pYieldCriterion, HardeningLawPointer pHardeningLaw);

    /**
     * Copy constructor.
     */
    HyperElasticPlasticThermalUPJ23DLaw (const HyperElasticPlasticThermalUPJ23DLaw& rOther);


    /**
     * Assignment operator.
     */

    //HyperElasticPlasticThermalUPJ23DLaw& operator=(const HyperElasticPlasticThermalUPJ23DLaw& rOther);

    /**
     * Clone function (has to be implemented by any derived class)
     * @return a pointer to a new instance of this constitutive law
     */
    ConstitutiveLaw::Pointer Clone() const override;

    /**
     * Destructor.
     */
    ~HyperElasticPlasticThermalUPJ23DLaw() override;

    /**
     * Operators
     */

    /**
     * Calculates the Temperature of the domain (element)
     * @param rElementGeometry the element geometry
     * @param rShapeFunctions the element shape functions
     * @param rTemperature the calculated temperature to be returned
     */
    double& CalculateDomainTemperature (const MaterialResponseVariables & rElasticVariables,
					double & rTemperature) override;

    /**
     * Operations needed by the base class:
     */


    /**
     * This function is designed to be called once to perform all the checks needed
     * on the input provided. Checks can be "expensive" as the function is designed
     * to catch user's errors.
     * @param rMaterialProperties
     * @param rElementGeometry
     * @param rCurrentProcessInfo
     * @return
     */
    //int Check(const Properties& rMaterialProperties, const GeometryType& rElementGeometry, const ProcessInfo& rCurrentProcessInfo);

    bool Has( const Variable<double>& rThisVariable ) override;

    double & GetValue( const Variable<double>& rThisVariable, double& rValue ) override;


    /**
     * Input and output
     */
    /**
     * Turn back information as a string.
     */
    //virtual String Info() const;
    /**
     * Print information about this object.
     */
    //virtual void PrintInfo(std::ostream& rOStream) const;
    /**
     * Print object's data.
     */
    //virtual void PrintData(std::ostream& rOStream) const;

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


    ///@}
    ///@name Serialization
    ///@{
    friend class Serializer;

    void save(Serializer& rSerializer) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, HyperElasticPlasticUP3DLaw);
    }

    void load(Serializer& rSerializer) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, HyperElasticPlasticUP3DLaw);
    }



}; // Class HyperElasticPlasticThermalUPJ23DLaw
}  // namespace Kratos.
#endif // KRATOS_HYPERELASTIC_PLASTIC_THERMAL_U_P_J2_3D_LAW_H_INCLUDED defined
