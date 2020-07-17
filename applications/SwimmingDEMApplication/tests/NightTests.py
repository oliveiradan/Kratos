# Definition of the classes for the NIGHTLY TESTS

#Iimport Kratos
import KratosMultiphysics
import KratosMultiphysics.DEMApplication
import KratosMultiphysics.SwimmingDEMApplication

# Import KratosUnittest
import KratosMultiphysics.KratosUnittest as KratosUnittest

# Importing test factories if possible
try:
     import CandelierTestFactory as CandelierTF
     candelier_imports_available = True
except ImportError:
     candelier_imports_available = False
try:
     import InterpolationTestFactory as InterpolationTF
     interpolation_imports_available = True
except ImportError:
     interpolation_imports_available = False

import importlib

if candelier_imports_available:
     class candelier_no_history_test(CandelierTF.TestFactory):
          file_name = "candelier_tests/candelier"
          file_parameters = "candelier_tests/ProjectParametersNoHistory.json"

     class candelier_no_history_with_lift_test(CandelierTF.TestFactory):
          file_name = "candelier_tests/candelier"
          file_parameters = "candelier_tests/ProjectParametersNoHistoryWithLift.json"

     class candelier_no_history_non_inertial_test(CandelierTF.TestFactory):
          file_name = "candelier_tests/candelier"
          file_parameters = "candelier_tests/ProjectParametersNoHistoryNonInertial.json"

     class candelier_with_history_test(CandelierTF.TestFactory):
          file_name = "candelier_tests/candelier"
          file_parameters = "candelier_tests/ProjectParametersWithHistory.json"

     class candelier_with_history_hinsberg_test(CandelierTF.TestFactory):
          file_name = "candelier_tests/candelier"
          file_parameters = "candelier_tests/ProjectParametersWithHistoryHinsberg.json"

     # This test is ready to run but the implementation is not complete
     # # (it is non-trivial), so the result is not correct
     # class candelier_with_history_non_inertial_test(CandelierTF.TestFactory):
     #      file_name = "candelier_tests/candelier"
     #      file_parameters = "candelier_tests/ProjectParametersWithHistoryNonInertial.json"

     candelier_test_list = [test_class for test_class in CandelierTF.TestFactory.__subclasses__()]

if interpolation_imports_available:
     class interpolation_test_linear(InterpolationTF.TestFactory):
          file_name = "interpolation_tests/cube"
          file_parameters = "interpolation_tests/ProjectParametersCubeLinear.json"

     class interpolation_test_nonlinear_time_no_substepping(InterpolationTF.TestFactory):
          file_name = "interpolation_tests/cube"
          file_parameters = "interpolation_tests/ProjectParametersCubeNonlinearTimeNoSubstepping.json"

     interpolation_test_list = [test_class for test_class in InterpolationTF.TestFactory.__subclasses__()]

# List of all classes above
available_tests = candelier_test_list + interpolation_test_list

def SetTestSuite(suites):
    night_suite = suites['nightly']

    night_suite.addTests(KratosUnittest.TestLoader().loadTestsFromTestCases(available_tests))

    return night_suite

def AssembleTestSuites():
    suites = KratosUnittest.KratosSuites
    night_suite = SetTestSuite(suites)
    suites['all'].addTests(night_suite)

    return suites

if __name__ == '__main__':
    KratosMultiphysics.Logger.GetDefaultOutput().SetSeverity(KratosMultiphysics.Logger.Severity.DETAIL)
    KratosUnittest.runTests(AssembleTestSuites())
