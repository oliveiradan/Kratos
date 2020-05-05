import subprocess
import os.path

# import Kratos
import KratosMultiphysics
import KratosMultiphysics.FluidDynamicsApplication
import KratosMultiphysics.RANSApplication

# Import Kratos "wrapper" for unittests
import KratosMultiphysics.KratosUnittest as KratosUnittest

import KratosMultiphysics.kratos_utilities as kratos_utilities

# Import the tests o test_classes to create the suites
from evm_k_epsilon_tests import EvmKEpsilonTest
from custom_process_tests import CustomProcessTest
from adjoint_k_epsilon_sensitivity_2d import AdjointKEpsilonSensitivity2D
import run_cpp_unit_tests
from incompressible_potential_flow_solver_formulation_tests import IncompressiblePotentialFlowSolverFormulationTest
from monolithic_k_omega_formulation_tests import MonolithicKOmegaTest
from monolithic_k_epsilon_high_re_formulation_tests import MonolithicKEpsilonHighReTest
from monolithic_velocity_pressure_formulation_tests import MonolithicVelocityPressureFormulationTest
from fractional_step_k_omega_formulation_tests import FractionalStepKOmegaTest
from fractional_step_k_epsilon_high_re_formulation_tests import FractionalStepKEpsilonHighReTest
from fractional_step_velocity_pressure_formulation_tests import FractionalStepVelocityPressureFormulationTest


def AssembleTestSuites():
    ''' Populates the test suites to run.

    Populates the test suites to run. At least, it should populate the suites:
    "small", "nighlty" and "all"

    Return
    ------

    suites: A dictionary of suites
        The set of suites with its test_cases added.
    '''
    suites = KratosUnittest.KratosSuites

    # Create a test suite with the selected tests (Small tests):
    smallSuite = suites['small']
    smallSuite.addTest(EvmKEpsilonTest('testOneElementKEpsilonSteady'))
    smallSuite.addTest(AdjointKEpsilonSensitivity2D('testOneElementSteady'))

    # Create a test suite with the selected tests plus all small tests
    nightSuite = suites['nightly']
    nightSuite.addTests(smallSuite)

    nightSuite.addTest(CustomProcessTest('testCheckScalarBoundsProcess'))
    nightSuite.addTest(CustomProcessTest('testCheckVectorBoundsProcess'))
    nightSuite.addTest(CustomProcessTest('testClipScalarVariableProcess'))
    nightSuite.addTest(CustomProcessTest('testApplyFlagProcess'))
    nightSuite.addTest(CustomProcessTest('testScalarCellCenterAveragingProcess'))
    nightSuite.addTest(CustomProcessTest('testVectorCellCenterAveragingProcess'))
    nightSuite.addTest(CustomProcessTest('testVectorAlignProcessTangential'))
    nightSuite.addTest(CustomProcessTest('testVectorAlignProcessNormal'))
    nightSuite.addTest(CustomProcessTest('testWallDistanceCalculationProcess'))
    nightSuite.addTest(CustomProcessTest('testLogarithmicYPlusCalculationProcess'))
    nightSuite.addTest(CustomProcessTest('testLogarithmicYPlusVelocitySensitivitiesProcessFlow'))
    nightSuite.addTest(CustomProcessTest('testNutKEpsilonHighReCalculationProcess'))
    nightSuite.addTest(CustomProcessTest('testNutKEpsilonHighReSensitivitiesProcess'))
    nightSuite.addTest(EvmKEpsilonTest('testBackwardFacingStepKEpsilonTransient'))
    nightSuite.addTest(EvmKEpsilonTest('testChannelFlowKEpsilonSteady'))
    nightSuite.addTest(EvmKEpsilonTest('testChannelFlowKEpsilonSteadyPeriodic'))

    # incompressible potential flow tests
    nightSuite.addTest((IncompressiblePotentialFlowSolverFormulationTest('testIncompressiblePotentialFlow')))

    # monolithic tests
    nightSuite.addTest((MonolithicVelocityPressureFormulationTest('testMonolithicVelocityPressure')))
    nightSuite.addTest((MonolithicKEpsilonHighReTest('testMonolithicKEpsilonHighReAfcTke')))
    nightSuite.addTest((MonolithicKEpsilonHighReTest('testMonolithicKEpsilonHighReAfcVelocity')))
    nightSuite.addTest((MonolithicKEpsilonHighReTest('testMonolithicKEpsilonHighReRfcTke')))
    nightSuite.addTest((MonolithicKEpsilonHighReTest('testMonolithicKEpsilonHighReRfcVelocity')))

    nightSuite.addTest((MonolithicKOmegaTest('testMonolithicKOmegaAfcTke')))
    nightSuite.addTest((MonolithicKOmegaTest('testMonolithicKOmegaAfcVelocity')))
    nightSuite.addTest((MonolithicKOmegaTest('testMonolithicKOmegaRfcTke')))
    nightSuite.addTest((MonolithicKOmegaTest('testMonolithicKOmegaRfcVelocity')))

    # fractional step tests
    nightSuite.addTest((FractionalStepVelocityPressureFormulationTest('testFractionalStepVelocityPressure')))
    nightSuite.addTest((FractionalStepKEpsilonHighReTest('testFractionalStepKEpsilonHighReAfcTke')))
    nightSuite.addTest((FractionalStepKEpsilonHighReTest('testFractionalStepKEpsilonHighReAfcVelocity')))
    nightSuite.addTest((FractionalStepKEpsilonHighReTest('testFractionalStepKEpsilonHighReRfcTke')))
    nightSuite.addTest((FractionalStepKEpsilonHighReTest('testFractionalStepKEpsilonHighReRfcVelocity')))

    nightSuite.addTest((FractionalStepKOmegaTest('testFractionalStepKOmegaAfcTke')))
    nightSuite.addTest((FractionalStepKOmegaTest('testFractionalStepKOmegaAfcVelocity')))
    nightSuite.addTest((FractionalStepKOmegaTest('testFractionalStepKOmegaRfcTke')))
    nightSuite.addTest((FractionalStepKOmegaTest('testFractionalStepKOmegaRfcVelocity')))

    # For very long tests that should not be in nighly and you can use to validate
    # validationSuite = suites['validation']

    # Create a test suite that contains all the tests:
    allSuite = suites['all']
    allSuite.addTests(nightSuite)

    # allSuite.addTests(validationSuite)

    return suites


if __name__ == '__main__':
    KratosMultiphysics.Logger.GetDefaultOutput().SetSeverity(
        KratosMultiphysics.Logger.Severity.WARNING)
    KratosMultiphysics.Logger.PrintInfo("Unittests", "\nRunning cpp unit tests ...")
    run_cpp_unit_tests.run()
    KratosMultiphysics.Logger.PrintInfo("Unittests", "Finished running cpp unit tests!")

    if kratos_utilities.IsMPIAvailable() and kratos_utilities.CheckIfApplicationsAvailable("MetisApplication", "TrilinosApplication"):
        KratosMultiphysics.Logger.PrintInfo("Unittests", "\nRunning mpi python tests ...")
        p = subprocess.Popen(
            ["mpiexec", "-np", "2", "python3", "test_RANSApplication_mpi.py"],
            stdout=subprocess.PIPE,
            cwd=os.path.dirname(os.path.abspath(__file__)))
        p.communicate()
        KratosMultiphysics.Logger.PrintInfo("Unittests", "Finished mpi python tests!")
    else:
        KratosMultiphysics.Logger.PrintInfo("Unittests", "\nSkipping mpi python tests due to missing dependencies")

    KratosMultiphysics.Logger.PrintInfo("Unittests", "\nRunning python tests ...")
    KratosUnittest.runTests(AssembleTestSuites())
    KratosMultiphysics.Logger.PrintInfo("Unittests", "Finished python tests!")