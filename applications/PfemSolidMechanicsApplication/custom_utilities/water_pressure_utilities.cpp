//
//   Project Name:        KratosSolidMechanicsApplication $
//   Last modified by:    $Author:              LMonforte $
//   Date:                $Date:                July 2015 $
//   Revision:            $Revision:                  0.1 $
//
//
//
#include "custom_utilities/water_pressure_utilities.hpp"
#include "pfem_solid_mechanics_application_variables.h"

namespace Kratos 
{

   WaterPressureUtilities::WaterPressureUtilities()
   {
   }

   // ****************** CALCULATE AND ADD RHS VECTOR *********************************************************
   // ******************************** Add water pressure to Internal and add mass balance equation ***********
   Vector &  WaterPressureUtilities::CalculateAndAddHydroProblem( Vector & rRightHandSide,  const Vector & rBaseClassRHS, const Vector & rVolumeForce, const int number_of_variables, GeometryType &  rGeometry, const PropertiesType & rProperties, const Matrix & rB, const Matrix & rDN_DX, const Vector  & rN, const double & rDetF0, const double & rTimeStep, const Matrix & rTotalF, const double & rIntegrationWeight, const double rCurrentRadius)
   {
      KRATOS_TRY

      HydroMechanicalVariables rVariables( rGeometry, rProperties);
      rVariables.number_of_variables = number_of_variables;
      rVariables.SetBMatrix( rB);
      rVariables.SetShapeFunctionsDerivatives( rDN_DX);
      rVariables.SetDeformationGradient( rTotalF);
      rVariables.SetVolumeForce( rVolumeForce);
      rVariables.SetShapeFunctions( rN);

      rVariables.DeltaTime = rTimeStep;
      rVariables.detF0 = rDetF0;


      Vector Previous = rRightHandSide; 
      const unsigned int number_of_nodes = rVariables.GetGeometry().PointsNumber();
      unsigned int dimension = rVariables.GetGeometry().WorkingSpaceDimension();




      // Check water pressure and bulk modulus

      // 1. Reshape BaseClass RHS
      rRightHandSide = AddReshapeBaseClassRHS( rRightHandSide, rBaseClassRHS, rVariables.number_of_variables, number_of_nodes);

      // 2a. Compute and Addmechanical forces contribution to internal forces
      Vector LocalRHS; 
      LocalRHS = CalculateWaterInternalForcesContribution( rVariables, LocalRHS, rIntegrationWeight); 
      rRightHandSide = AddReshapeWaterInternalForcesContribution( rRightHandSide, LocalRHS, rVariables.number_of_variables, number_of_nodes, dimension); 


      LocalRHS = CalculateVolumeForcesContribution( rVariables, LocalRHS, rIntegrationWeight);
      rRightHandSide = AddReshapeWaterInternalForcesContribution( rRightHandSide, LocalRHS, rVariables.number_of_variables, number_of_nodes, dimension);

      // 2b. Compute the balance of mixture
      LocalRHS = CalculateMassBalance_WaterPressurePart( rVariables, LocalRHS, rIntegrationWeight);
      LocalRHS = this->CalculateAndAddWaterPressureForcesDisplacement(LocalRHS, rGeometry, rProperties, rDN_DX, rN, rDetF0, rTotalF, rTimeStep, rIntegrationWeight, rCurrentRadius);
      rRightHandSide = AddReshapeWaterPressureForces ( rRightHandSide, LocalRHS, number_of_variables, number_of_nodes);


      //std::cout << " VECTOR " << - rRightHandSide + Previous << std::endl;
      return rRightHandSide; 

      KRATOS_CATCH("")
   }

   // ************************** CALCULATE AND ADD rhs STABILIZATION PART. Matrix ***************************
   // ***********************************************************************************************
   Matrix & WaterPressureUtilities::CalculateAndAddStabilizationLHS( Matrix & rLeftHandSide, const int number_of_variables, GeometryType & rGeometry, const PropertiesType & rProperties, const Matrix & rDN_DX, const double & rConstrainedModulus, const double & rDetF0, const double & rTimeStep, const double & rIntegrationWeight)
   {
      KRATOS_TRY

      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();

      // 1. Compute Stabilization LHS
      Matrix LocalLHS; 
      double IntegrationWeight = rIntegrationWeight / rDetF0; 
      LocalLHS = CalculateStabilizationLHS( LocalLHS, rGeometry, rProperties, rDN_DX, rTimeStep, rConstrainedModulus, IntegrationWeight);

      rLeftHandSide = AddReshapeKwPwP( rLeftHandSide, LocalLHS, dimension, number_of_variables, number_of_nodes);   

      return rLeftHandSide; 
      KRATOS_CATCH("")
   }

   // ************************** CALCULATE AND ADD rhs STABILIZATION PART ***************************
   // ***********************************************************************************************
   Vector & WaterPressureUtilities::CalculateAndAddStabilization( Vector & rRightHandSide, const int number_of_variables, GeometryType & rGeometry, const PropertiesType & rProperties, const Matrix & rDN_DX, const double & rConstrainedModulus, const double & rDetF0, const double & rTimeStep, const double & rIntegrationWeight)
   {
      KRATOS_TRY

      const unsigned int number_of_nodes = rGeometry.PointsNumber();

      // 1. Compute Stabilization RHS
      Vector LocalRHS; 
      double IntegrationWeight = rIntegrationWeight / rDetF0; 
      LocalRHS = CalculateStabilization( LocalRHS, rGeometry, rProperties, rDN_DX, rTimeStep, rConstrainedModulus, IntegrationWeight);

      rRightHandSide = AddReshapeWaterPressureForces( rRightHandSide, LocalRHS, number_of_variables, number_of_nodes);   

      return rRightHandSide; 
      KRATOS_CATCH("")
   }


   // ******* Compute the stabilization factor **********************
   double & WaterPressureUtilities::ComputeStabilizationFactor( double & AlphaStabilization, const PropertiesType & rProperties, const Matrix & rDN_DX, const double & rTimeStep, const double & rConstrainedModulus)
   {

      // Compute element size ( Sun Ostine Salinger IJNAMG, 2013)
      double ElementSize = 0;
      unsigned int number_of_nodes = rDN_DX.size1();
      unsigned int dimension = rDN_DX.size2();
      for (unsigned int i = 0; i < number_of_nodes; i++) {
         double aux = 0;
         for (unsigned int iDim = 0; iDim < dimension; iDim++) {
            aux += rDN_DX(i, iDim);
         }
         ElementSize += fabs( aux); 
      }
      ElementSize *= sqrt( double(dimension) );
      ElementSize = 4.0/ ElementSize; 


      double Permeability = rProperties.GetValue(PERMEABILITY); 
      double StabilizationFactor = rProperties.GetValue( STABILIZATION_FACTOR_WP);

      mPPP = true;

      if ( mPPP) {
         AlphaStabilization = 2.0 / rConstrainedModulus - 12.0 * Permeability * rTimeStep / ElementSize; 
      }
      else {
         AlphaStabilization = pow(ElementSize, 2.0) / ( 6.0 * rConstrainedModulus) - rTimeStep * Permeability/2.0;
      }

      if ( AlphaStabilization < 0)
         AlphaStabilization = 0.0; 
      AlphaStabilization *=  StabilizationFactor;

      return AlphaStabilization; 

   }

   // *************** COMPUTE THE RHS due To STABILIZATION *********************
   Vector & WaterPressureUtilities::CalculateStabilization( Vector & rLocalRHS, GeometryType & rGeometry,  const PropertiesType & rProperties, const Matrix & rDN_DX, const double & rTimeStep, const double & rConstrainedModulus, const double & rIntegrationWeight )
   {
      KRATOS_TRY

      // 1. Some Constant
      double ScalingConstant; 
      GetScalingConstant( ScalingConstant, rProperties);


      // 2. Geometric
      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();

      // 3. Compute Stabilization Factor 
      double AlphaStabilization; 
      AlphaStabilization = ComputeStabilizationFactor( AlphaStabilization, rProperties, rDN_DX, rTimeStep, rConstrainedModulus); 

      rLocalRHS = ZeroVector( number_of_nodes);
      if ( AlphaStabilization <= 1.0e-12) {
         return rLocalRHS;
      }

      if( mPPP) {
         double consistent; 
         for (unsigned int i = 0; i < number_of_nodes; i++) {
            for (unsigned int j = 0; j < number_of_nodes; j++) {
               const double & rCurrentWaterPressure = rGeometry[j].FastGetSolutionStepValue( WATER_PRESSURE );
               const double & rPreviousWaterPressure = rGeometry[j].FastGetSolutionStepValue( WATER_PRESSURE, 1 );
               double DeltaWaterPressure = rCurrentWaterPressure - rPreviousWaterPressure; 

               if ( dimension == 2) {
                  consistent = (-1.0) * AlphaStabilization / 18.0;
                  if ( i == j)
                     consistent = 2.0 * AlphaStabilization / 18.0;
               }
               else {
                  consistent = (-1.0) * AlphaStabilization /80.0;
                  if ( i == j)
                     consistent = 3.0 * AlphaStabilization /80.0;
               }

               rLocalRHS(i) += consistent * DeltaWaterPressure * rIntegrationWeight * ScalingConstant; 

            }

         }
      } else {
         // assume that K is a unit tensor by k
         for (unsigned int i = 0; i < number_of_nodes; i++) {
            for (unsigned int j = 0; j < number_of_nodes; j++) {
               for (unsigned p = 0; p < dimension; p++) {
                  const double & rCurrentWaterPressure = rGeometry[j].FastGetSolutionStepValue( WATER_PRESSURE );
                  const double & rPreviousWaterPressure = rGeometry[j].FastGetSolutionStepValue( WATER_PRESSURE , 1);
                  double DeltaWaterPressure = rCurrentWaterPressure - rPreviousWaterPressure; 

                  rLocalRHS(i) += rDN_DX(i,p) * rDN_DX(j,p) * AlphaStabilization * DeltaWaterPressure * rIntegrationWeight * ScalingConstant;
               }
            }
         }
      }

      return rLocalRHS; 

      KRATOS_CATCH("")

   }

   // *************** COMPUTE THE LHS due To STABILIZATION *********************
   Matrix & WaterPressureUtilities::CalculateStabilizationLHS( Matrix & rLocalLHS, GeometryType & rGeometry,  const PropertiesType & rProperties, const Matrix & rDN_DX, const double & rTimeStep, const double & rConstrainedModulus, const double & rIntegrationWeight )
   {
      KRATOS_TRY

      // 1. Some Constant
      double ScalingConstant; 
      GetScalingConstant( ScalingConstant, rProperties);


      // 2. Geometric
      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();

      // 3. Compute Stabilization Factor 
      double AlphaStabilization; 
      AlphaStabilization = ComputeStabilizationFactor( AlphaStabilization, rProperties, rDN_DX, rTimeStep, rConstrainedModulus); 

      rLocalLHS = ZeroMatrix( number_of_nodes, number_of_nodes);
      if ( AlphaStabilization <= 1.0e-12) {
         return rLocalLHS;
      }

      if ( mPPP) {
         double consistent; 
         for (unsigned int i = 0; i < number_of_nodes; i++) {
            for (unsigned int j = 0; j < number_of_nodes; j++) {
               if ( dimension == 2) {
                  consistent = (-1.0) * AlphaStabilization / 18.0;
                  if ( i == j)
                     consistent = 2.0 * AlphaStabilization / 18.0;
               }
               else {
                  consistent = (-1.0) * AlphaStabilization / 80.0;
                  if ( i == j)
                     consistent = 3.0 * AlphaStabilization / 80.0;
               }

               rLocalLHS(i,j) -= consistent * rIntegrationWeight * ScalingConstant; 

            }

         }
      } else {
         // assume that K is a unit tensor by k
         for (unsigned int i = 0; i < number_of_nodes; i++) {
            for (unsigned int j = 0; j < number_of_nodes; j++) {
               for (unsigned p = 0; p < dimension; p++) {

                  rLocalLHS(i,j) -= rDN_DX(i,p) * rDN_DX(j,p) * AlphaStabilization * rIntegrationWeight * ScalingConstant;
               }
            }
         }
      }

      return rLocalLHS; 

      KRATOS_CATCH("")

   }


   // ******************************  CALCULATE AND ADD LHS MATRIX *************************************
   // ***************************************************************************************************
   Matrix &  WaterPressureUtilities::CalculateAndAddHydroProblemLHS( Matrix & rLeftHandSide,  const Matrix & rBaseClassLHS, const Vector & rVolumeForce, const int number_of_variables, GeometryType &  rGeometry, const PropertiesType & rProperties, const Matrix & rB, const Matrix & rDN_DX, const Vector  & rN, const double & rDetF0, const double & rTimeStep, const Matrix & rTotalF, const double & rIntegrationWeight, const double rCurrentRadius)
   {
      KRATOS_TRY

      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();

      Matrix Previous = rLeftHandSide; 
      // 1. Reshape BaseClass LHS
      rLeftHandSide = AddReshapeBaseClassLHS( rLeftHandSide, rBaseClassLHS, dimension,  number_of_variables, number_of_nodes);

      // 2. Add water Pressure geometric stiffness matrix
      Matrix LocalLHS; 
      LocalLHS = ComputeWaterPressureKuug(LocalLHS, rGeometry, rB, rN, rIntegrationWeight);
      rLeftHandSide = AddReshapeKUU( rLeftHandSide, LocalLHS, dimension, number_of_variables, number_of_nodes);

      // 3. Add KUwP
      LocalLHS = ComputeWaterPressureKUwP( LocalLHS, rGeometry, rDN_DX, rN, rIntegrationWeight, rCurrentRadius);
      rLeftHandSide = AddReshapeKUwP( rLeftHandSide, LocalLHS, dimension, number_of_variables, number_of_nodes);

      // 4. Add KwPwP
      double IntegrationWeight = (rIntegrationWeight / rDetF0);
      LocalLHS = ComputeWaterPressureKwPwP( LocalLHS, rGeometry, rProperties, rDN_DX, rTotalF,  rN, rTimeStep, IntegrationWeight);
      rLeftHandSide = AddReshapeKwPwP( rLeftHandSide, LocalLHS, dimension,  number_of_variables, number_of_nodes);

      // 5. Add Solid Skeleton Deformation
      LocalLHS = ComputeSolidSkeletonDeformationMatrix(LocalLHS, rGeometry, rProperties, rDN_DX, rN, IntegrationWeight, rCurrentRadius); // Changes in elements that have Jacobian
      rLeftHandSide = AddReshapeSolidSkeletonDeformationMatrix( rLeftHandSide, LocalLHS, dimension, number_of_variables, number_of_nodes);

      // 6. Add Darcy Geometric Stiffness
      LocalLHS = ComputeDarcyFlowGeometricTerms( LocalLHS, rGeometry, rProperties, rB, rDN_DX, rTotalF, rN, rTimeStep, IntegrationWeight);
      rLeftHandSide = AddReshapeKwPU( rLeftHandSide, LocalLHS, dimension,  number_of_variables, number_of_nodes);


      // 7. Add the term due to the density change
      LocalLHS = ComputeDensityChangeTerm( LocalLHS, rGeometry, rProperties, rVolumeForce, rDN_DX, rN, rDetF0, rIntegrationWeight, rCurrentRadius);
      rLeftHandSide = AddReshapeKUU( rLeftHandSide, LocalLHS, dimension, number_of_variables, number_of_nodes);

      //std::cout << " THIS MATRIX " << - rLeftHandSide + Previous << std::endl;
      //std::cout << " " << std::endl;
      return rLeftHandSide; 
      KRATOS_CATCH("")
   }

   // **** Geometric like term due to the variation of the density effect on the linear momentum balance ******
   // *********************************************************************************************************
   Matrix & WaterPressureUtilities::ComputeDensityChangeTerm( Matrix & rLocalLHS, GeometryType & rGeometry, const PropertiesType & rProperties, const Vector & rVolumeForce,  const Matrix & rDN_DX, const Vector & rN, const double & rDetF0, const double & rIntegrationWeight, const double & rCurrentRadius)
   {
      KRATOS_TRY

      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();
      Vector VolumeForce = rVolumeForce;
      VolumeForce *= rDetF0; // due to the volumechange

      rLocalLHS = ZeroMatrix( dimension*number_of_nodes, dimension*number_of_nodes);
      double density_mixture0 = rProperties.GetValue(DENSITY);
      if ( density_mixture0 > 0) {
         VolumeForce /= density_mixture0; 
      }
      else {
         return rLocalLHS; 
      }

      double density_water = rProperties.GetValue(DENSITY_WATER);


      for (unsigned int i = 0; i < number_of_nodes; i++) {
         for (unsigned int iDim = 0; iDim < dimension; iDim++) {
            for (unsigned int p = 0; p < number_of_nodes; p++) {
               for (unsigned int pDim = 0 ; pDim < dimension; pDim++) {
                  rLocalLHS( i*dimension + iDim, p*dimension + pDim) -= rN(i) * rVolumeForce(iDim) * rDN_DX(p, pDim);
               }
            }
         }
      }

      rLocalLHS *= (rIntegrationWeight * density_water); 
      return rLocalLHS; 

      KRATOS_CATCH("")
   }


   // ***** Geometric like terms that appear in the Darcy flow *****************************************
   // **************************************************************************************************

   Matrix & WaterPressureUtilities::ComputeDarcyFlowGeometricTerms( Matrix & rLocalLHS, GeometryType & rGeometry, const PropertiesType & rProperties, const Matrix & rB, const Matrix & rDN_DX, const Matrix & rTotalF, const Vector & rN, const double & rDeltaTime, const double & rIntegrationWeight)
   {
      KRATOS_TRY

      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();

      // 1. Some constants
      double ScalingConstant;
      GetScalingConstant( ScalingConstant, rProperties);

      //2. Gravity & Permeability
      Vector b = ZeroVector(dimension);
      double WaterDensity = rProperties.GetValue(DENSITY_WATER);
      b(dimension-1) = -10.0*WaterDensity;
      Matrix K;
      double initial_porosity = rProperties.GetValue(INITIAL_POROSITY);

      double VolumeChange = this->CalculateVolumeChange(rGeometry, rN, rTotalF );
      GetPermeabilityTensor( rProperties, rTotalF, K, initial_porosity, dimension, VolumeChange);


      Vector GradP = ZeroVector(dimension);

      for (unsigned int iN = 0; iN <  number_of_nodes; iN++) {
         for (unsigned int dim = 0; dim < dimension; dim++) {
            GradP(dim) += rGeometry[iN].FastGetSolutionStepValue( WATER_PRESSURE ) * rDN_DX(iN, dim);
         }
      }

      Vector DarcyFlow = prod( K, GradP-b);

      rLocalLHS = ZeroMatrix( number_of_nodes, number_of_nodes * dimension);

      Matrix AllwaysTerm = rLocalLHS; 

      for (unsigned int i = 0; i < number_of_nodes; i++)
      {
         for (unsigned int iDim = 0; iDim < dimension; iDim++)
         {
            for (unsigned int qDim = 0; qDim < dimension; qDim++)
            {
               for (unsigned int q = 0; q < number_of_nodes; q++)
               {
                  AllwaysTerm(i, q*dimension + iDim) -= rDN_DX(i,iDim) * DarcyFlow(qDim) * rDN_DX(q, qDim);
               }
            }
         }
      }

      // Try To do it the other way
      Matrix SpatialTerm = rLocalLHS;
      for (unsigned int i = 0; i < number_of_nodes; i++) {
         for (unsigned int iDim = 0; iDim < dimension; iDim++) {
            for (unsigned int qDim = 0; qDim < dimension; qDim++) {
               for (unsigned int pDim = 0; pDim < dimension; pDim++) {
                  for (unsigned int a = 0; a < number_of_nodes; a++) {
                     SpatialTerm(i,a*dimension +pDim ) += rDN_DX(i, iDim) * K(iDim, qDim) * GradP( pDim) * rDN_DX( a, qDim);
                  }
               }
            }
         }
      }


      AllwaysTerm += SpatialTerm; 
      AllwaysTerm *= (-rIntegrationWeight * ScalingConstant * rDeltaTime);

      rLocalLHS = AllwaysTerm; 
      return rLocalLHS; 

      KRATOS_CATCH("")
   }

   // ****** Tanget To Mass conservation, part of the solid skeleton deformation for displ form *********
   // ***************************************************************************************************
   Matrix & WaterPressureUtilities::ComputeSolidSkeletonDeformationMatrix( Matrix & rLocalLHS, GeometryType & rGeometry, const PropertiesType & rProperties, const Matrix & rDN_DX, const Vector & rN, const double & rIntegrationWeight, const double & rCurrentRadius)
   {
      KRATOS_TRY

      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();

      // 1. Some constants
      double ScalingConstant;
      GetScalingConstant( ScalingConstant, rProperties);

      // 2. Calculate Matrix

      rLocalLHS = ZeroMatrix( number_of_nodes, number_of_nodes * dimension);

      // VelocityGradient
      Matrix Velocity_DX = ZeroMatrix(dimension, dimension);
      for (unsigned int k = 0; k < number_of_nodes; k++)
      {
         const array_1d<double, 3 > &  CurrentDisplacement = rGeometry[k].FastGetSolutionStepValue( DISPLACEMENT );
         const array_1d<double, 3 > & PreviousDisplacement = rGeometry[k].FastGetSolutionStepValue( DISPLACEMENT , 1);
         array_1d<double, 3 > DeltaDisplacement = CurrentDisplacement - PreviousDisplacement; 
         for (unsigned int j = 0; j < dimension; j++)
         {
            for (unsigned int i = 0; i < dimension; i++)
            {
               Velocity_DX(i,j) += DeltaDisplacement[i] * rDN_DX(k,j);
            }
         }
      }

      for (unsigned int i = 0; i < number_of_nodes; i++)
      {
         for (unsigned int p = 0; p < number_of_nodes; p++) {
            for (unsigned int pDim = 0; pDim < dimension; pDim++) {
               rLocalLHS( i, p*dimension + pDim) += rN(i)*rDN_DX(p, pDim);

               // LD Term
               for (unsigned int l = 0; l < dimension; l++)
               {
                  rLocalLHS(i, p*dimension + pDim) -= rN(i) * Velocity_DX(l, pDim) * rDN_DX( p ,l);
               }
            }
         }
      }

      rLocalLHS *= ( rIntegrationWeight * ScalingConstant);

      return rLocalLHS;

      KRATOS_CATCH("")

   }
   // ******* Tangent to Mass conservation with Respect WaterPressure ***********************************
   // ***************************************************************************************************
   Matrix & WaterPressureUtilities::ComputeWaterPressureKwPwP( Matrix & LocalLHS, GeometryType & rGeometry, const PropertiesType & rProperties, const Matrix & rDN_DX, const Matrix & rTotalF, const Vector & rN, const double & rDeltaTime, const double & rIntegrationWeight)
   {
      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();

      // 1. Some constants
      double ScalingConstant;
      double WaterBulk = rProperties.GetValue(WATER_BULK_MODULUS);
      GetScalingConstant( ScalingConstant, rProperties);

      Matrix K;
      double initial_porosity = rProperties.GetValue(INITIAL_POROSITY);
      double VolumeChange = this->CalculateVolumeChange(rGeometry, rN, rTotalF );
      GetPermeabilityTensor( rProperties, rTotalF, K, initial_porosity, VolumeChange);

      LocalLHS = ZeroMatrix( number_of_nodes, number_of_nodes);

      for (unsigned int i = 0; i < number_of_nodes; i++)
      {
         for (unsigned int j = 0; j < number_of_nodes; j++)
         {
            LocalLHS(i,j) -= (1.0/WaterBulk) * rN(i)*rN(j); 
            for (unsigned p = 0; p < dimension; p++) {
               for (unsigned q = 0; q < dimension; q++) {
                  LocalLHS(i,j) -= rDeltaTime * rDN_DX(i,p) * K(p,q) * rDN_DX(j,q);
               }
            }
         }
      }
      LocalLHS *= ( rIntegrationWeight * ScalingConstant);

      return LocalLHS; 

   }

   // ***************** Tangent To water Contribution to internal forces *********************************
   // ****************************************************************************************************
   Matrix & WaterPressureUtilities::ComputeWaterPressureKUwP( Matrix & rLocalLHS, GeometryType & rGeometry, const Matrix & rDN_DX, const Vector & rN, const double & rIntegrationWeight, const double & rCurrentRadius)
   {
      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();

      rLocalLHS = ZeroMatrix( number_of_nodes*dimension, number_of_nodes);

      for (unsigned int i = 0; i < number_of_nodes; i++) {
         for (unsigned int j = 0; j < number_of_nodes; j++) {
            for (unsigned int dim = 0; dim < dimension; dim++) {
               rLocalLHS(i*dimension + dim, j) += rDN_DX(i, dim) * rN(j) * rIntegrationWeight;
            }
         }
      }

      return rLocalLHS;
   }

   // ************ Contribution of water pressure to geometric stiffness *********************************
   // ****************************************************************************************************
   Matrix & WaterPressureUtilities::ComputeWaterPressureKuug( Matrix & rLocalLHS, GeometryType & rGeometry, const Matrix & rB, const Vector & rN, const double & rIntegrationWeight)
   {
      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();

      unsigned int voigtsize, principal_dimension;
      GetVoigtSize(dimension, voigtsize, principal_dimension);

      double WaterPressure = 0.0;
      for (unsigned int i = 0; i  < number_of_nodes; i++)
         WaterPressure += rN(i) * rGeometry[i].FastGetSolutionStepValue( WATER_PRESSURE );


      // the term is  pw * ( one times one - 2 I4S) + pw * ( ones some_product ones)
      Matrix StiffnessMatrix = ZeroMatrix(voigtsize, voigtsize);
      for (unsigned int i = 0; i < principal_dimension; i++) {
         for (unsigned int j = 0; j < principal_dimension; j++) {
            StiffnessMatrix(i,j) = 1.0;
         }
      }

      
      double voigtNumber = 1.0;
      for (unsigned int i = 0; i < voigtsize; i++)
      {
         if ( i >= principal_dimension)
            voigtNumber = 0.5;
         StiffnessMatrix(i,i) -= voigtNumber;
      }

      StiffnessMatrix *= WaterPressure;
      /*std::cout << " transB " << trans( rB) <<  " STIFF " << StiffnessMatrix << " rB " << rB << std::endl;
        std::cout << " prod 1 " << prod( StiffnessMatrix, rB) << std::endl;
        std::cout << " prod 2 " << Matrix(prod( StiffnessMatrix, rB)) << std::endl;
        std::cout << " prod 3 " << rIntegrationWeight*Matrix(prod( StiffnessMatrix, rB)) << std::endl;*/
      rLocalLHS = prod( trans( rB) , rIntegrationWeight * Matrix( prod( StiffnessMatrix, rB) ) );

      return rLocalLHS;

   }

   // ******************************* RESHAPE BASE CLASS RHS **********************************************
   Vector & WaterPressureUtilities::AddReshapeBaseClassRHS( VectorType & rRightHandSideVector, const VectorType& rBaseClassRHS, const unsigned int & number_of_variables, const unsigned int & number_of_nodes )
   {
      KRATOS_TRY

      for (unsigned int i = 0; i < number_of_nodes; i++)
      {
         unsigned int indexfrom = (number_of_variables -1) * i;
         unsigned int indexto   = (number_of_variables )*i;

         for (unsigned int j = 0; j < number_of_variables-1; j++)
         {
            rRightHandSideVector(indexto + j) += rBaseClassRHS( indexfrom +j);
         }

      }

      return rRightHandSideVector;

      KRATOS_CATCH("")

   }

   // ***************** RESHAPE MATRIX: BaseClass LHS ****************************************************
   // ****************************************************************************************************
   Matrix & WaterPressureUtilities::AddReshapeBaseClassLHS( Matrix & rLeftHandSideMatrix, const Matrix & rBaseClassLHS,const unsigned int dimension,  const unsigned int number_of_variables, const unsigned int number_of_nodes )
   {
      KRATOS_TRY

      unsigned int dimTo = number_of_variables; 
      unsigned int dimFrom = number_of_variables - 1; 
      for (unsigned int iNode = 0 ; iNode < number_of_nodes; iNode++)
      {
         for (unsigned int iDim = 0; iDim < dimFrom; iDim++)
         {
            for (unsigned int jNode = 0; jNode < number_of_nodes; jNode++)
            {
               for (unsigned int jDim = 0; jDim < dimFrom; jDim++)
               {
                  rLeftHandSideMatrix( iNode*dimTo + iDim, jNode*dimTo + jDim) += rBaseClassLHS( iNode*dimFrom + iDim, jNode*dimFrom + jDim);
               }
            }
         }
      }

      return rLeftHandSideMatrix; 

      KRATOS_CATCH("")
   }


   // ***************** RESHAPE MATRIX: KUwP LHS *********************************************************
   // ****************************************************************************************************
   Matrix & WaterPressureUtilities::AddReshapeKUwP( Matrix & rLeftHandSide, const Matrix & rLocalLHS, const unsigned int dimension, const unsigned int number_of_variables, const unsigned int number_of_nodes)
   {
      KRATOS_TRY

      for ( unsigned int i = 0; i < number_of_nodes; i++)
      {
         for ( unsigned int iDim = 0; iDim < dimension; iDim++)
         {
            for (unsigned int j = 0; j < number_of_nodes; j++)
            {
               rLeftHandSide( i*number_of_variables + iDim, number_of_variables*(j+1)-1) += rLocalLHS( i*dimension + iDim ,j);
            }
         }
      }

      return rLeftHandSide;
      KRATOS_CATCH("")
   }
   // ***************** RESHAPE MATRIX: KwPwP LHS *********************************************************
   // ****************************************************************************************************
   Matrix & WaterPressureUtilities::AddReshapeKwPwP( Matrix & rLeftHandSide, const Matrix & rLocalLHS, const unsigned int dimension, const unsigned int number_of_variables, const unsigned int number_of_nodes)
   {
      KRATOS_TRY

      for (unsigned int i = 0; i < number_of_nodes; i++) {
         for (unsigned int j = 0; j < number_of_nodes; j++) {
            rLeftHandSide( (i+1)*number_of_variables-1, (j+1)*number_of_variables-1) += rLocalLHS(i,j);
         }
      }

      return rLeftHandSide; 
      KRATOS_CATCH("")
   }

   // ***************** RESHAPE MATRIX: matrix different in Jacobian Elements ******************************************************
   // ******************************************************************************************************************************
   Matrix & WaterPressureUtilities::AddReshapeSolidSkeletonDeformationMatrix( Matrix & rLeftHandSide, const Matrix & rLocalLHS, const unsigned int dimension, const unsigned int number_of_variables, const unsigned int number_of_nodes)
   {

      rLeftHandSide = AddReshapeKwPU( rLeftHandSide, rLocalLHS, dimension, number_of_variables, number_of_nodes);

      return rLeftHandSide; 
   }

   // ***************** RESHAPE MATRIX: KwPU LHS *********************************************************
   // ****************************************************************************************************
   Matrix & WaterPressureUtilities::AddReshapeKwPU( Matrix & rLeftHandSide, const Matrix & rLocalLHS, const unsigned int dimension, const unsigned int number_of_variables, const unsigned int number_of_nodes)
   {
      KRATOS_TRY

      for (unsigned int i = 0; i < number_of_nodes; i++)
      {
         for (unsigned int j = 0; j < number_of_nodes; j++)
         {
            for (unsigned int jDim = 0; jDim < dimension; jDim++)
            {
               rLeftHandSide(number_of_variables*(i+1) -1, j *number_of_variables+ jDim) += rLocalLHS( i, j*dimension + jDim);
            }
         }
      }

      return rLeftHandSide;
      KRATOS_CATCH("")
   }

   // ***************** RESHAPE MATRIX: KUU  LHS *********************************************************
   // ****************************************************************************************************
   Matrix & WaterPressureUtilities::AddReshapeKUU( Matrix & rLeftHandSide, const Matrix & rLocalLHS, const unsigned int dimension, const unsigned int number_of_variables, const unsigned int number_of_nodes)
   {
      KRATOS_TRY

      unsigned int dimTo = number_of_variables; 
      unsigned int dimFrom = dimension; 
      for (unsigned int iNode = 0 ; iNode < number_of_nodes; iNode++)
      {
         for (unsigned int iDim = 0; iDim < dimFrom; iDim++)
         {
            for (unsigned int jNode = 0; jNode < number_of_nodes; jNode++)
            {
               for (unsigned int jDim = 0; jDim < dimFrom; jDim++)
               {
                  rLeftHandSide( iNode*dimTo + iDim, jNode*dimTo + jDim) += rLocalLHS( iNode*dimFrom + iDim, jNode*dimFrom + jDim);
               }
            }
         }
      }

      return rLeftHandSide;
      KRATOS_CATCH("")
   }

   // ************** Add and RESHAPE WATER PRESSURE RHS ***************
   // *****************************************************************
   Vector & WaterPressureUtilities::AddReshapeWaterPressureForces( VectorType & rRightHandSideVector, const VectorType & rPartialRHS, const unsigned int number_of_variables, const unsigned int number_of_nodes)
   {
      //Vector AuxiliarRHS = rRightHandSide; 
      //rRightHandSide = ZeroVector(number_of_variables*number_of_nodes);

      for ( unsigned int i = 0; i < number_of_nodes; i++)
      {
         rRightHandSideVector( (number_of_variables)*(i+1) - 1) += rPartialRHS(i);
      }

      return rRightHandSideVector;

   }

   //
   Vector & WaterPressureUtilities::AddReshapeWaterInternalForcesContribution( Vector & rRightHandSideVector, const Vector & rPartialRHS, const unsigned int number_of_variables, const unsigned int number_of_nodes, const unsigned int dimension)
   {
      KRATOS_TRY

      for (unsigned int i = 0; i < number_of_nodes; i++)
      {
         unsigned int indexfrom = dimension * i; 
         unsigned int indexto   = (number_of_variables ) *i;
         for (unsigned int j = 0; j < dimension; j++)
         {
            rRightHandSideVector( indexto + j) += rPartialRHS(indexfrom + j);
         }

      }

      return rRightHandSideVector; 

      KRATOS_CATCH("")
   }

   // ****************** COMPUTE THE FORCES DUE TO GRAVITY WITH THE DIFFERENT DEN *********************
   // *************************************************************************************************
   Vector & WaterPressureUtilities::CalculateVolumeForcesContribution( HydroMechanicalVariables & rVariables, VectorType & rLocalRHS, const double & rIntegrationWeight)
   {
      KRATOS_TRY

      const unsigned int number_of_nodes = rVariables.GetGeometry().PointsNumber();
      unsigned int dimension = rVariables.GetGeometry().WorkingSpaceDimension();

      Vector VolumeForce = rVariables.GetVolumeForce();
      VolumeForce *= rVariables.detF0; 

      rLocalRHS.resize( number_of_nodes*dimension, false);
      noalias( rLocalRHS ) = ZeroVector( number_of_nodes*dimension);

      double density_mixture0 = rVariables.GetProperties().GetValue(DENSITY);
      if ( density_mixture0 > 0) {
         VolumeForce /= density_mixture0; 
      }
      else {
         return rLocalRHS; 
      }

      double density_water = rVariables.GetProperties().GetValue(DENSITY_WATER);
      double porosity0 = rVariables.GetProperties().GetValue( INITIAL_POROSITY);

      double porosity = 1.0 - (1.0-porosity0) / rVariables.detF0; 
      double density_solid = (density_mixture0 - porosity0*density_water) / ( 1.0 - porosity0);
      double density_mixture = ( 1.0 - porosity) * density_solid + porosity * density_water;


      const VectorType & rN = rVariables.GetShapeFunctions();
      for (unsigned int i = 0; i < number_of_nodes; i++) {
         for (unsigned int iDim = 0; iDim < dimension; iDim++) {
            rLocalRHS(i*dimension + iDim) += rIntegrationWeight * density_mixture *  rN(i) * VolumeForce(iDim);
         }
      }

      return rLocalRHS;

      KRATOS_CATCH("")
   }
   // ****************** COMPUTE WATER PRESSURE CONTRIBUTION TO THE LINEAR MOMENTUM EQUATION ***********
   // **************************************************************************************************
   Vector & WaterPressureUtilities::CalculateWaterInternalForcesContribution( HydroMechanicalVariables& rVariables, Vector & rRightHandSideVector, const double & rIntegrationWeight)  
   {
      KRATOS_TRY

      const GeometryType & rGeometry = rVariables.GetGeometry();
      const VectorType & rN = rVariables.GetShapeFunctions();

      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();

      double WaterPressure = 0.0;
      for (unsigned int i = 0; i < number_of_nodes; i++)
      {
         WaterPressure += rGeometry[i].GetSolutionStepValue( WATER_PRESSURE ) * rN(i);
      }

      unsigned int voigtsize, principal_dimension;
      this->GetVoigtSize( dimension, voigtsize, principal_dimension);
      Vector  WaterPressureVector = ZeroVector( voigtsize);

      for (unsigned int i = 0; i < principal_dimension; i++) // something must be placed here for the axisym ...
         WaterPressureVector(i) = 1.0;
      WaterPressureVector *= WaterPressure;

      const MatrixType rB = rVariables.GetBMatrix();
      rRightHandSideVector.resize( number_of_nodes*dimension, false);
      noalias(rRightHandSideVector) = -rIntegrationWeight * prod( trans( rB), WaterPressureVector );

      return rRightHandSideVector; 


      KRATOS_CATCH("")
   }


   // ***************** COMPUTE WATER PRESSURE RHS (mass balance equation, without solid skeleton deformation) ************
   // *********************************************************************************************************************
   Vector &  WaterPressureUtilities::CalculateMassBalance_WaterPressurePart( HydroMechanicalVariables & rVariables, VectorType & rRightHandSideVector, const double & rIntegrationWeight)
   {

      KRATOS_TRY

      // 1. Some constants
      double ScalingConstant;
      GetScalingConstant( ScalingConstant, rVariables.GetProperties() );
      double WaterBulk = rVariables.GetProperties().GetValue(WATER_BULK_MODULUS);
      const VectorType & rN = rVariables.GetShapeFunctions();
      const MatrixType & rDN_DX = rVariables.GetShapeFunctionsDerivatives();

      // 2. Geometric
      const GeometryType & rGeometry = rVariables.GetGeometry();
      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();

      bool density_considered  = false; 
      if ( rGeometry[0].SolutionStepsDataHas( VOLUME_ACCELERATION) ) {
         const array_1d < double, 3 > VolAcc = rGeometry[0].FastGetSolutionStepValue( VOLUME_ACCELERATION) ;
         if (  ( fabs( VolAcc[0] ) + fabs( VolAcc[1]) + fabs(VolAcc[2] ) ) > 1.0e-12) {
            density_considered = true;
         }
      }


      //3. Gravity & Permeability
      Vector b = ZeroVector(dimension);
      double WaterDensity = 0.0;
      if ( density_considered) 
         WaterDensity = rVariables.GetProperties().GetValue(DENSITY_WATER);
      b(dimension-1) = -10.0*WaterDensity;

      double initial_porosity = rVariables.GetProperties().GetValue(INITIAL_POROSITY);
      double VolumeChange = this->CalculateVolumeChange(rGeometry, rN, rVariables.GetDeformationGradient() );

      Matrix K;
      GetPermeabilityTensor( rVariables.GetProperties(), rVariables.GetDeformationGradient(), K, initial_porosity, VolumeChange);


      rRightHandSideVector.resize( number_of_nodes);
      noalias( rRightHandSideVector ) = ZeroVector( number_of_nodes); 


      for ( unsigned int i = 0; i < number_of_nodes; i++ )
      {
         for ( unsigned int j = 0; j < number_of_nodes; j++ )
         {

            // DeltaPressure
            const double& CurrentPressure   = rGeometry[j].FastGetSolutionStepValue( WATER_PRESSURE );
            const double& PreviousPressure = rGeometry[j].FastGetSolutionStepValue( WATER_PRESSURE , 1);
            double DeltaPressure = CurrentPressure - PreviousPressure;

            // WaterCompressibilityTerm
            rRightHandSideVector(i) += (1.0/WaterBulk) * rN(i) * rN(j) * DeltaPressure * rIntegrationWeight * ScalingConstant / rVariables.detF0;

            for ( unsigned int p = 0; p < dimension; ++p )
            {

               for ( unsigned int q = 0; q < dimension; ++q )
               {

                  // Darcy flow
                  rRightHandSideVector(i) += rVariables.DeltaTime * rDN_DX(i, p) * K( p, q) * rDN_DX(j,q)* CurrentPressure *rIntegrationWeight * ScalingConstant / rVariables.detF0;

                  if (j == 0) {
                     // Gravity term of the DARCY FLOW.
                     rRightHandSideVector(i) += rVariables.DeltaTime * rDN_DX(i,p) * K(p,q)*b(q)*rIntegrationWeight* ScalingConstant / rVariables.detF0;

                  }
               }
            }
         }
      }

      return rRightHandSideVector; 

      KRATOS_CATCH("")

   }

   // *************** RHS of the Hydromechanical Problem. Add the solid skeleton movement part *****************************
   // **********************************************************************************************************************
   Vector &  WaterPressureUtilities::CalculateAndAddWaterPressureForcesDisplacement( Vector & rRightHandSideVector, GeometryType & rGeometry,  const PropertiesType & rProperties, const Matrix & rDN_DX, const Vector & rN, const double & rDetF0, const Matrix & rTotalF, const double& rDeltaTime,  const double & rIntegrationWeight, const double & rCurrentRadius)
   {
      KRATOS_TRY

      double ScalingConstant; 
      GetScalingConstant( ScalingConstant, rProperties);

      // 2. Geometric
      const unsigned int number_of_nodes = rGeometry.PointsNumber();
      unsigned int dimension = rGeometry.WorkingSpaceDimension();

      for ( unsigned int i = 0; i < number_of_nodes; i++ )
      {
         for ( unsigned int j = 0; j < number_of_nodes; j++ )
         {

            // DeltaDisplacement
            const array_1d<double, 3 > & CurrentDisplacement  = rGeometry[j].FastGetSolutionStepValue( DISPLACEMENT );
            const array_1d<double, 3 > & PreviousDisplacement = rGeometry[j].FastGetSolutionStepValue( DISPLACEMENT , 1 );
            array_1d<double, 3 > DeltaDisplacement      = CurrentDisplacement-PreviousDisplacement;

            for ( unsigned int p = 0; p < dimension; ++p )
            {
               // Solid Skeleton volumetric deformation
               rRightHandSideVector(i) -= rN(i)*DeltaDisplacement[p] * rDN_DX(j,p) * rIntegrationWeight * ScalingConstant / rDetF0;

            }

         }

      }

      return rRightHandSideVector; 


      KRATOS_CATCH("")
   }

   // ***** get the constans *********************************
   // ********************************************************
   void WaterPressureUtilities::GetConstants( double& rScalingConstant, double& rWaterBulk, double& rDeltaTime, double& rPermeability, const PropertiesType & rProperties)
   {

      GetScalingConstant(rScalingConstant, rProperties);

      rPermeability = rProperties.GetValue(PERMEABILITY);
      rWaterBulk = rProperties.GetValue(WATER_BULK_MODULUS);


      if ( rPermeability < 1e-12 || rWaterBulk < 1e-12)
      {
         std::cout << " BIG ERROR. WATER BULK MOUDLUS OR PERMEABILITY NOT SETTED (') "<< std::endl;
         KRATOS_THROW_ERROR( std::logic_error, "SOME IMPORTANT CONSTANTS ARE NOT GIVEN", " WaterPressureUtilities" );

      }

   }

   void WaterPressureUtilities::GetScalingConstant( double & rScalingConstant, const PropertiesType & rProperties)
   {
      rScalingConstant= 1.0;
   }

   // ***** SOME SORT OF CONSTITUTIVE LAW FOR THE PERMEABILITY TENSOR *****
   // *********************************************************************

   void WaterPressureUtilities::GetPermeabilityTensor( const PropertiesType & rProperties, const Matrix& rF, Matrix& rPermeabilityTensor , const double & rInitialPorosity, const double & rVolumeChange)
   {
      // BASE CASE //
      unsigned int thisSize = rF.size1();
      (rPermeabilityTensor) = ZeroMatrix( thisSize, thisSize);

      double scalarPermeability = rProperties.GetValue(PERMEABILITY);

      bool KozenyCarman = rProperties.GetValue(KOZENY_CARMAN);
      if ( KozenyCarman == true)
      {
         double Permeability = rProperties.GetValue( PERMEABILITY );
         // vale, això no serveix pels elements mixtes -.
         //double Jacobian = MathUtils<double>::Det( rF);
         double Porosity = 1.0 - (1.0 - rInitialPorosity) / rVolumeChange;
         double voidRatio = Porosity / ( 1.0 - Porosity);
         double initialVoidRatio = rInitialPorosity / ( 1.0 - rInitialPorosity);

         double Constant = Permeability * ( 1.0 + initialVoidRatio) / pow( initialVoidRatio, 3.0);

         scalarPermeability = Constant * pow( voidRatio, 3.0) / ( 1.0 + voidRatio);
         if ( scalarPermeability < Permeability / 1000.0) {
            scalarPermeability = Permeability /1000.0;
         }

      }

      for (unsigned int i = 0; i < thisSize; ++i)
      {
         rPermeabilityTensor(i,i) = scalarPermeability;
      }

      return;

      // after I have the part of the Lagrangian constant
      rPermeabilityTensor = prod(rPermeabilityTensor, trans( rF) );
      rPermeabilityTensor = prod( rF, rPermeabilityTensor);
      rPermeabilityTensor /= MathUtils<double>::Det(rF);

   }

   void WaterPressureUtilities::GetPermeabilityTensor( const PropertiesType & rProperties, const Matrix & rF, Matrix & rPermeabilityTensor, const double & rInitialPorosity, const unsigned int & rDimension, const double & rVolumeChange)
   {
      GetPermeabilityTensor( rProperties, rF, rPermeabilityTensor, rInitialPorosity , rVolumeChange);

      // manual resize. Something wierd in Axisym.
      if (  rPermeabilityTensor.size1() != rDimension )
      {
         Matrix Aux = rPermeabilityTensor;
         rPermeabilityTensor = ZeroMatrix( rDimension, rDimension);
         for (unsigned int i = 0; i < rDimension; i++) {
            for (unsigned int j = 0; j < rDimension; j++) {
               rPermeabilityTensor(i,j) = Aux(i,j);
            }
         }
      }
   }

   double WaterPressureUtilities::CalculateVolumeChange(const GeometryType & rGeometry, const Vector & rN, const Matrix & rTotalF)
   {
      double VolumeChange = MathUtils<double>::Det( rTotalF);
      return VolumeChange; 
   }
   void WaterPressureUtilities::GetVoigtSize( const unsigned int & dimension, unsigned int & voigtsize, unsigned int & principal_dimension)
   {
      voigtsize = 3; 
      principal_dimension = dimension;
      if ( dimension == 3)
         voigtsize = 6;

   }

}
// end namespace kratos
