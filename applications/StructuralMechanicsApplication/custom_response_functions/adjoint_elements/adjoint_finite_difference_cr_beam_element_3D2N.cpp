// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:     BSD License
//           license: structural_mechanics_application/license.txt
//
//  Main authors:    Martin Fusseder, https://github.com/MFusseder
//


#include "adjoint_finite_difference_cr_beam_element_3D2N.h"
#include "structural_mechanics_application_variables.h"
#include "custom_response_functions/response_utilities/stress_response_definitions.h"
#include "includes/checks.h"


namespace Kratos
{

AdjointFiniteDifferenceCrBeamElement::AdjointFiniteDifferenceCrBeamElement(Element::Pointer pPrimalElement)
    : AdjointFiniteDifferencingBaseElement(pPrimalElement, true)
{
}

AdjointFiniteDifferenceCrBeamElement::~AdjointFiniteDifferenceCrBeamElement()
{
}

int AdjointFiniteDifferenceCrBeamElement::Check(const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    int return_value = AdjointFiniteDifferencingBaseElement::Check(rCurrentProcessInfo);

    KRATOS_ERROR_IF_NOT(mpPrimalElement) << "Primal element pointer is nullptr!" << std::endl;

    //TODO: Check() of primal element should be called, but is not possible because of DOF check!

    KRATOS_ERROR_IF(GetGeometry().WorkingSpaceDimension() != 3 || GetGeometry().size() != 2)
    << "The beam element works only in 3D and with 2 noded elements" << "" << std::endl;

    KRATOS_CHECK_VARIABLE_KEY(VELOCITY);
    KRATOS_CHECK_VARIABLE_KEY(DISPLACEMENT);
    KRATOS_CHECK_VARIABLE_KEY(ACCELERATION);
    KRATOS_CHECK_VARIABLE_KEY(DENSITY);
    KRATOS_CHECK_VARIABLE_KEY(CROSS_AREA);
    KRATOS_CHECK_VARIABLE_KEY(ADJOINT_DISPLACEMENT);
    KRATOS_CHECK_VARIABLE_KEY(ADJOINT_ROTATION);

    // check dofs
    const GeometryType& r_geom = GetGeometry();
    for (IndexType i = 0; i < r_geom.size(); i++)
    {
        const auto& r_node = r_geom[i];

        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(DISPLACEMENT, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(ROTATION, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(ADJOINT_DISPLACEMENT, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(ADJOINT_ROTATION, r_node);

        KRATOS_CHECK_DOF_IN_NODE(ADJOINT_DISPLACEMENT_X, r_node);
        KRATOS_CHECK_DOF_IN_NODE(ADJOINT_DISPLACEMENT_Y, r_node);
        KRATOS_CHECK_DOF_IN_NODE(ADJOINT_DISPLACEMENT_Z, r_node);
        KRATOS_CHECK_DOF_IN_NODE(ADJOINT_ROTATION_X, r_node);
        KRATOS_CHECK_DOF_IN_NODE(ADJOINT_ROTATION_Y, r_node);
        KRATOS_CHECK_DOF_IN_NODE(ADJOINT_ROTATION_Z, r_node);
    }

    const double numerical_limit = std::numeric_limits<double>::epsilon();

    KRATOS_ERROR_IF(this->GetProperties().Has(CROSS_AREA) == false || this->GetProperties()[CROSS_AREA] <= numerical_limit)
    << "CROSS_AREA not provided for this element" << this->Id() << std::endl;
    KRATOS_ERROR_IF(this->GetProperties().Has(YOUNG_MODULUS) == false || this->GetProperties()[YOUNG_MODULUS] <= numerical_limit)
    << "YOUNG_MODULUS not provided for this element" << this->Id() << std::endl;
    KRATOS_ERROR_IF_NOT( this->GetProperties().Has(DENSITY) )
    << "DENSITY not provided for this element" << this->Id() << std::endl;
    KRATOS_ERROR_IF_NOT( this->GetProperties().Has(POISSON_RATIO) )
    << "POISSON_RATIO not provided for this element" << this->Id() << std::endl;
    KRATOS_ERROR_IF_NOT( this->GetProperties().Has(TORSIONAL_INERTIA) )
    << "TORSIONAL_INERTIA not provided for this element" << this->Id() << std::endl;

    KRATOS_ERROR_IF_NOT( this->GetProperties().Has(I22) )
    << "I22 not provided for this element" << this->Id() << std::endl;
    KRATOS_ERROR_IF_NOT( this->GetProperties().Has(I33) )
    << "I33 not provided for this element" << this->Id() << std::endl;

    return return_value;

    KRATOS_CATCH("")
}

void AdjointFiniteDifferenceCrBeamElement::save(Serializer& rSerializer) const
{
    KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, AdjointFiniteDifferencingBaseElement);
}

void AdjointFiniteDifferenceCrBeamElement::load(Serializer& rSerializer)
{
    KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, AdjointFiniteDifferencingBaseElement);
}

void AdjointFiniteDifferenceCrBeamElement::SetValueOnIntegrationPoints(const Variable<array_1d<double, 3 > >& rVariable,
					     std::vector<array_1d<double, 3 > > rValues,
					     const ProcessInfo& rCurrentProcessInfo)
{   
    const SizeType  write_points_number = GetGeometry()
            .IntegrationPointsNumber(this->GetIntegrationMethod());
    
    if (mForceSensitivity.size() != write_points_number )
        mForceSensitivity.resize(write_points_number);
    if (mMomentSensitivity.size() != write_points_number)
        mMomentSensitivity.resize(write_points_number);    
    
    if (rValues.size() == write_points_number)
    {
	    if (rVariable == FORCE_SENSITIVITY)
		    for (IndexType PointNumber = 0; PointNumber < write_points_number; ++PointNumber) 
                mForceSensitivity[PointNumber] = rValues[PointNumber];
        
        else if (rVariable == MOMENT_SENSITIVITY) 
		    for (IndexType PointNumber = 0; PointNumber < write_points_number; ++PointNumber)                 
			    mMomentSensitivity[PointNumber] = rValues[PointNumber];
    }
    else
        KRATOS_ERROR << "Size of sensitivity vector is unequal to number of integration points" << std::endl;
    
}

void AdjointFiniteDifferenceCrBeamElement::GetValueOnIntegrationPoints(const Variable<array_1d<double, 3 > >& rVariable,
					     std::vector<array_1d<double, 3 > >& rValues,
					     const ProcessInfo& rCurrentProcessInfo)
{ 
    const SizeType  write_points_number = GetGeometry()
            .IntegrationPointsNumber(this->GetIntegrationMethod());
    
    if (rValues.size() != write_points_number)
            rValues.resize(write_points_number);

    if (rVariable == FORCE_SENSITIVITY)
    {
        if(mForceSensitivity.size() == write_points_number)
        {
              for (IndexType PointNumber = 0; PointNumber < write_points_number; ++PointNumber) 
                rValues[PointNumber] = mForceSensitivity[PointNumber];
        }
        else if(this->Has(rVariable))
        {
            const array_1d<double, 3 >& output_value = this->GetValue(rVariable);
            for (IndexType PointNumber = 0; PointNumber < write_points_number; ++PointNumber)
                rValues[PointNumber] = output_value;
        }
        else
            KRATOS_ERROR << "Error at getting " << rVariable << " values on integration points." << std::endl; 
    }
    else if (rVariable == MOMENT_SENSITIVITY)
    {
        if(mMomentSensitivity.size() == write_points_number)
        {
            for (IndexType PointNumber = 0; PointNumber < write_points_number; ++PointNumber)                 
                rValues[PointNumber] = mMomentSensitivity[PointNumber];
        }
        else if(this->Has(rVariable))
        {
            const array_1d<double, 3 >& output_value = this->GetValue(rVariable);
            for (IndexType PointNumber = 0; PointNumber < write_points_number; ++PointNumber)
                rValues[PointNumber] = output_value;
        }
        else
            KRATOS_ERROR << "Error at getting " << rVariable << " values on integration points." << std::endl; 
    }
    else
        KRATOS_ERROR << "Sensitivity Variable "<< rVariable <<" is not compatible with adjoint_finite_difference_cr_beam_element_3D2N." << std::endl;    
}  

} // namespace Kratos.


