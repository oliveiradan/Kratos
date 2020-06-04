//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Ignasi de Pouplana
//                   Guillermo Casas
//


#ifndef KRATOS_MULTIAXIAL_CONTROL_MODULE_GENERALIZED_2D_UTILITIES
#define KRATOS_MULTIAXIAL_CONTROL_MODULE_GENERALIZED_2D_UTILITIES

// /* External includes */

// System includes

// Project includes
#include "includes/variables.h"

/* System includes */
#include <limits>
#include <iostream>
#include <iomanip>
#include <map>

/* External includes */
#ifdef _OPENMP
#include <omp.h>
#endif

/* Project includes */
#include "geometries/geometry.h"
#include "includes/define.h"
#include "includes/model_part.h"

#include "includes/table.h"
#include "includes/kratos_parameters.h"
#include "utilities/math_utils.h"

// Application includes
#include "custom_elements/spheric_continuum_particle.h"

#include "DEM_application_variables.h"


namespace Kratos
{
class MultiaxialControlModuleGeneralized2DUtilities
{
public:

KRATOS_CLASS_POINTER_DEFINITION(MultiaxialControlModuleGeneralized2DUtilities);

/// Defining a table with double argument and result type as table type.
typedef Table<double,double> TableType;

/// Default constructor.

MultiaxialControlModuleGeneralized2DUtilities(ModelPart& rDemModelPart,
                                ModelPart& rFemModelPart,
                                Parameters& rParameters
                                ) :
                                mrDemModelPart(rDemModelPart),
                                mrFemModelPart(rFemModelPart)
{
    KRATOS_TRY

    Parameters default_parameters( R"(
        {
            "Parameters"    : {
                "control_module_delta_time": 1.0e-5,
                "perturbation_tolerance": 1.0e-2,
                "perturbation_period": 10,
                "max_reaction_rate_factor": 2.0,
                "stiffness_averaging_time_interval": 1.0,
                "velocity_averaging_time_interval": 1.0,
                "reaction_averaging_time_interval": 1.0,
                "output_interval": 1,
                "final_time": 1.0
            },
            "list_of_actuators" : []
        }  )" );

    // Now validate agains defaults -- this also ensures no type mismatch
    rParameters.ValidateAndAssignDefaults(default_parameters);

    mCMDeltaTime = rParameters["Parameters"]["control_module_delta_time"].GetDouble();
    mCMStep = 0;
    mStep = 0;
    mCMTime = 0.0;
    mKDeltaTime = rParameters["Parameters"]["stiffness_averaging_time_interval"].GetDouble();
    mKStep = 0;
    mKTime = 0.0;
    mActuatorCounter = 0;
    mPerturbationTolerance = rParameters["Parameters"]["perturbation_tolerance"].GetDouble();
    mPerturbationPeriod = rParameters["Parameters"]["perturbation_period"].GetInt();
    mMaxReactionCorrectionFraction = rParameters["Parameters"]["max_reaction_rate_factor"].GetDouble();
    mStiffnessAlpha = 1.0 - mCMDeltaTime / rParameters["Parameters"]["stiffness_averaging_time_interval"].GetDouble();
    mVelocityAlpha = 1.0 - mCMDeltaTime / rParameters["Parameters"]["velocity_averaging_time_interval"].GetDouble();
    mReactionAlpha = 1.0 - mrDemModelPart.GetProcessInfo()[DELTA_TIME] / rParameters["Parameters"]["reaction_averaging_time_interval"].GetDouble();

    const unsigned int number_of_actuators = rParameters["list_of_actuators"].size();
    mVelocity.resize(number_of_actuators, false);
    mAcceleration.resize(number_of_actuators, false);
    mReactionStress.resize(number_of_actuators, false);
    mReactionStressOld.resize(number_of_actuators, false);
    mDisplacement.resize(number_of_actuators, false);
    mDisplacementOld.resize(number_of_actuators, false);
    mElasticReactionStress.resize(number_of_actuators, false);
    mStiffness.resize(number_of_actuators,number_of_actuators,false);
    noalias(mStiffness) = ZeroMatrix(number_of_actuators,number_of_actuators);
    mDeltaDisplacement.resize(number_of_actuators,number_of_actuators,false);
    noalias(mDeltaDisplacement) = ZeroMatrix(number_of_actuators,number_of_actuators);
    mDeltaReactionStress.resize(number_of_actuators,number_of_actuators,false);
    noalias(mDeltaReactionStress) = ZeroMatrix(number_of_actuators,number_of_actuators);
    for (unsigned int i = 0; i < number_of_actuators; i++) {
        const unsigned int number_of_dem_boundaries = rParameters["list_of_actuators"][i]["list_of_dem_boundaries"].size();
        const unsigned int number_of_fem_boundaries = rParameters["list_of_actuators"][i]["list_of_fem_boundaries"].size();
        std::vector<ModelPart*> list_of_dem_submodelparts(number_of_dem_boundaries);
        std::vector<ModelPart*> list_of_fem_submodelparts(number_of_fem_boundaries);
        // std::vector<array_1d<double,3>> list_of_dem_outer_normals(number_of_dem_boundaries);
        std::vector<array_1d<double,3>> list_of_fem_outer_normals(number_of_fem_boundaries);
        for (unsigned int j = 0; j < number_of_dem_boundaries; j++) {
            list_of_dem_submodelparts[j] = 
                &(mrDemModelPart.GetSubModelPart(
                rParameters["list_of_actuators"][i]["list_of_dem_boundaries"][j]["model_part_name"].GetString()
                ));
            // list_of_dem_outer_normals[j][0] = rParameters["list_of_actuators"][i]["list_of_dem_boundaries"][j]["outer_normal"][0].GetDouble();
            // list_of_dem_outer_normals[j][1] = rParameters["list_of_actuators"][i]["list_of_dem_boundaries"][j]["outer_normal"][1].GetDouble();
            // list_of_dem_outer_normals[j][2] = rParameters["list_of_actuators"][i]["list_of_dem_boundaries"][j]["outer_normal"][2].GetDouble();
            // // Unit normal vector
            // double inverse_norm = 1.0/norm_2(list_of_dem_outer_normals[j]);
            // list_of_dem_outer_normals[j][0] *= inverse_norm;
            // list_of_dem_outer_normals[j][1] *= inverse_norm;
            // list_of_dem_outer_normals[j][2] *= inverse_norm;
        }
        for (unsigned int j = 0; j < number_of_fem_boundaries; j++) {
            list_of_fem_submodelparts[j] = 
                &(mrFemModelPart.GetSubModelPart(
                rParameters["list_of_actuators"][i]["list_of_fem_boundaries"][j]["model_part_name"].GetString()
                ));
            list_of_fem_outer_normals[j][0] = rParameters["list_of_actuators"][i]["list_of_fem_boundaries"][j]["outer_normal"][0].GetDouble();
            list_of_fem_outer_normals[j][1] = rParameters["list_of_actuators"][i]["list_of_fem_boundaries"][j]["outer_normal"][1].GetDouble();
            list_of_fem_outer_normals[j][2] = rParameters["list_of_actuators"][i]["list_of_fem_boundaries"][j]["outer_normal"][2].GetDouble();
            // Unit normal vector
            double inverse_norm = 1.0/norm_2(list_of_fem_outer_normals[j]);
            list_of_fem_outer_normals[j][0] *= inverse_norm;
            list_of_fem_outer_normals[j][1] *= inverse_norm;
            list_of_fem_outer_normals[j][2] *= inverse_norm;
        }
        const std::string actuator_name = rParameters["list_of_actuators"][i]["Parameters"]["actuator_name"].GetString();
        mDEMBoundariesSubModelParts[actuator_name] = list_of_dem_submodelparts;
        mFEMBoundariesSubModelParts[actuator_name] = list_of_fem_submodelparts;
        // mDEMOuterNormals[actuator_name] = list_of_dem_outer_normals;
        mFEMOuterNormals[actuator_name] = list_of_fem_outer_normals;
        mTargetStressTableIds[actuator_name] = rParameters["list_of_actuators"][i]["Parameters"]["target_stress_table_id"].GetInt();
        const double compression_length = rParameters["list_of_actuators"][i]["Parameters"]["compression_length"].GetDouble();
        const double initial_velocity = rParameters["list_of_actuators"][i]["Parameters"]["initial_velocity"].GetDouble();
        const double stiffness = rParameters["list_of_actuators"][i]["Parameters"]["young_modulus"].GetDouble()/compression_length; // mStiffness is actually a stiffness over an area
        mVelocity[i] = initial_velocity;
        mStiffness(i,i) = stiffness;
        mAcceleration[i] = 0.0;
        mReactionStress[i] = 0.0;
        mReactionStressOld[i] = 0.0;
        mDisplacement[i] = 0.0;
        mDisplacementOld[i] = 0.0;
        mElasticReactionStress[i] = 0.0;
        mOrderedMapKeys.push_back(actuator_name);
    }

    mrDemModelPart.GetProcessInfo()[TARGET_STRESS_Z] = 0.0;

    CalculateCharacteristicReactionVariationRate(rParameters["Parameters"]["final_time"].GetDouble());

    KRATOS_CATCH("");
}

/// Destructor.

virtual ~MultiaxialControlModuleGeneralized2DUtilities(){}

//***************************************************************************************************************
//***************************************************************************************************************

// Before DEM solution
virtual void ExecuteInitialize();

// Before DEM solution
virtual void ExecuteInitializeSolutionStep();

// After DEM solution
virtual void ExecuteFinalizeSolutionStep();

//***************************************************************************************************************
//***************************************************************************************************************

///@}
///@name Inquiry
///@{


///@}
///@name Input and output
///@{

/// Turn back information as a stemplate<class T, std::size_t dim> tring.

virtual std::string Info() const
{
    return "";
}

/// Print information about this object.

virtual void PrintInfo(std::ostream& rOStream) const
{
}

/// Print object's data.

virtual void PrintData(std::ostream& rOStream) const
{
}


///@}
///@name Friends
///@{

///@}

protected:
///@name Protected static Member r_variables
///@{

    ModelPart& mrDemModelPart;
    ModelPart& mrFemModelPart;
    double mCMDeltaTime;
    double mCMTime;
    double mKDeltaTime;
    double mKTime;
    unsigned int mStep;
    unsigned int mCMStep;
    unsigned int mKStep;
    unsigned int mActuatorCounter;
    double mPerturbationTolerance;
    unsigned int mPerturbationPeriod;
    double mMaxReactionCorrectionFraction;
    double mStiffnessAlpha;
    double mVelocityAlpha;
    double mReactionAlpha;
    double mCharacteristicReactionVariationRate;
    std::vector<std::string> mOrderedMapKeys; // TODO: we could have std::vectors instead of std::maps
    std::map<std::string, std::vector<ModelPart*>> mFEMBoundariesSubModelParts; /// FEM SubModelParts associated to each boundary of every actuator
    std::map<std::string, std::vector<ModelPart*>> mDEMBoundariesSubModelParts; /// DEM SubModelParts associated to each boundary of every actuator
    std::map<std::string, std::vector<array_1d<double,3>>> mFEMOuterNormals; /// OuterNormal associated to each FEM boundary of every actuator
    // std::map<std::string, std::vector<array_1d<double,3>>> mDEMOuterNormals; /// OuterNormal associated to each DEM boundary of every actuator. TODO: not used for now...
    std::map<std::string, unsigned int> mTargetStressTableIds; /// TargetStressTableIds associated to every actuator
    Vector mVelocity;
    Vector mAcceleration;
    Vector mReactionStress;
    Vector mReactionStressOld;
    Vector mDisplacement;
    Vector mDisplacementOld;
    Vector mElasticReactionStress;
    Matrix mStiffness;
    Matrix mDeltaDisplacement;
    Matrix mDeltaReactionStress;

///@}
///@name Protected member r_variables
///@{ template<class T, std::size_t dim>


///@}
///@name Protected Operators
///@{


///@}
///@name Protected Operations
///@{

virtual Vector MeasureReactionStress(const Variable<array_1d<double,3>>& rVariable);

Vector GetPerturbations(const Vector& rTargetStress, const double& rTime);

double GetConditionNumber(const Matrix& rInputMatrix, const Matrix& rInvertedMatrix);

void CalculateVelocity(const Vector& r_next_target_stress, const double& r_current_time);

void CalculateAcceleration(const Vector& r_next_target_stress, const double& r_current_time);

void CalculateStiffness();

void CalculateCharacteristicReactionVariationRate(const double& r_final_time);

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

///@name Static Member r_variables
///@{


///@}
///@name Member r_variables
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

/// Assignment operator.
MultiaxialControlModuleGeneralized2DUtilities & operator=(MultiaxialControlModuleGeneralized2DUtilities const& rOther);


///@}

}; // Class MultiaxialControlModuleGeneralized2DUtilities

}  // namespace Python.

#endif // KRATOS_MULTIAXIAL_CONTROL_MODULE_GENERALIZED_2D_UTILITIES
