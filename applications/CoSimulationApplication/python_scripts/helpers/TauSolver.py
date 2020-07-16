# -*- coding: utf-8 -*-
import shutil, sys, os, time, json
import numpy as np
import CoSimIO

def print_on_rank_zero(*args):
    if tau_mpi_rank() == 0:
        print(args)

working_path = os.getcwd() + '/'
#path = "/work/piquee/MembraneWing/kratos_fsi_big"
with open(working_path + 'tau_settings.json') as json_file:
    tau_settings = json.load(json_file)

start_step = tau_settings["start_step"]
tau_path = tau_settings["tau_path"]
rotate = tau_settings["rotate"]
sys.path.append(tau_settings["kratos_path"])
sys.path.append(tau_path + "py_turb1eq/")


#sys.path.append("/work/piquee/Softwares/TAU/TAU_2016.2/2016.2.0/bin/py_turb1eq/")
import PyPara, PyPrep, PySolv, PyDeform, PyCopyCluster, PyMotionExternalDelegate, PySurfDeflect
from tau_python import tau_solver_unsteady_get_physical_time
from tau_python import tau_mpi_rank
rank = tau_mpi_rank()
from tau_python import *

# tau_functions can only be imported after appending kratos' path
import tau_functions as TauFunctions
import MotionStringGenerator as MSG

if tau_mpi_rank() == 0:
    # Remove output files and deform mesh files from previous simulations
    TauFunctions.RemoveFilesFromPreviousSimulations()
tau_parallel_sync()

# Definition of the parameter file
if rotate:
    para_path='airfoil_Structured_rotation.cntl'
else:
    para_path='airfoil_Structured.cntl'

para_path_mod = para_path + ".mod"
shutil.copy(para_path, para_path_mod)

# Initialize Tau python classes and auxiliary variable step
Para = PyPara.Parafile(para_path_mod)
Prep = PyPrep.Preprocessing(para_path_mod)
tau_time_step = float(Para.get_para_value('Unsteady physical time step size'))
tau_parallel_sync()

if rotate:
    # --- Prepare parameters for unsteady simulation ---
    Para.update({"Unsteady allow external control over progress in time (0/1)": 1,
                 "Unsteady enable external control over progress in time (0/1)": 1})

    # external output period (needed if output period in para_path not working properly)
    pitchDeg = 0  # mean pitch angle

    # --- Load external excitation files ---
    # --- thetaDeg -> pitch angle per timestep in deg
    # --- thetaRate -> pitch rate per timestep in deg/s
    thetaDeg = np.loadtxt(working_path + 'signal/APRBSDeg_membrane.dat')
    thetaRate = np.loadtxt(working_path + 'signal/APRBSRate_membrane.dat')

    MyMotionStringGenerator = MSG.MotionStringGenerator(
        tau_time_step, pitchDeg, thetaDeg, thetaRate)

    # --- Prepare parameters for unsteady rotating simulation ---
    nodeNames = ["MEMBRANE"]

    # --- Instanciate required modules ---
    MyTauMotionDelegate = PyMotionExternalDelegate.MotionExternalDelegate(
        nodeNames)

    Solver = PySolv.Solver(para_path_mod, delegate=MyTauMotionDelegate)

    # --- Init motion stack ---
    motionString = MyMotionStringGenerator(0)
    MyTauMotionDelegate.UpdateMotion("MEMBRANE", motionString)
    MyTauMotionDelegate.PushMotion()
    TauFunctions.PrintBlockHeader("Inital Motionstring: %s" %(motionString))
else:
    Solver = PySolv.Solver(para_path_mod)
    tau_parallel_sync()

Deform = PyDeform.Deformation(para_path_mod)
tau_parallel_sync()
step = start_step
tau_parallel_sync()

def AdvanceInTime(current_time):
    # Preprocessing needs to be done before getting the time and time step
    TauFunctions.PrintBlockHeader("Start Preprocessing at time %s" %(str(time)))
    Prep.run(write_dualgrid=1,free_primgrid=False)
    tau_parallel_sync()
    TauFunctions.PrintBlockHeader("Stop Preprocessing at time %s" %(str(time)))
    TauFunctions.PrintBlockHeader("Initialize Solver at time %s" %(str(time)))
    Solver.init(verbose = 1, reset_steps = True, n_time_steps = 1)
    tau_parallel_sync()

    if rotate:
        motionString = MyMotionStringGenerator(step - start_step)
        MyTauMotionDelegate.UpdateMotion("MEMBRANE", motionString)
        MyTauMotionDelegate.InitExchange()

    # Get current time and time step from tau
    tau_current_time = float(tau_solver_unsteady_get_physical_time())
    tau_parallel_sync()
    
    if tau_settings["echo_level"] > 0:
        print "TAU SOLVER AdvanceInTime"
        print 'tau_current_time = ', tau_current_time
        print 'tau_time_step = ', tau_time_step
    return tau_current_time + tau_time_step

def InitializeSolutionStep():
    if tau_settings["echo_level"] > 0:
        print_on_rank_zero("TAU SOLVER InitializeSolutionStep")

def SolveSolutionStep():
    if tau_settings["echo_level"] > 0:
        print_on_rank_zero("TAU SOLVER SolveSolutionStep")
    Solver.outer_loop()
    tau_parallel_sync()
    Solver.output()
    tau_parallel_sync()
    tau_plt_init_tecplot_params(para_path_mod)
    tau_parallel_sync()
    tau_solver_write_output_conditional()
    tau_parallel_sync()
    # Convert tau output to dat file using tau2plt
    if tau_mpi_rank() == 0:
        TauFunctions.ConvertOutputToDat(working_path, tau_path, step, para_path_mod, start_step)
    tau_parallel_sync()

def FinalizeSolutionStep():
    if tau_settings["echo_level"] > 0:
        print("TAU SOLVER FinalizeSolutionStep")

    tau_parallel_sync()
    Solver.finalize()
    tau_free_dualgrid()
    tau_free_prims()
    Para.free_parameters()
    if tau_mpi_rank() == 0:
        global step
        step += 1
    print step
    tau_parallel_sync()

def ImportData(conn_name, identifier):
    tau_parallel_sync()
    if tau_mpi_rank() == 0:
        if tau_settings["echo_level"] > 0:
            print "TAU SOLVER ImportData"
        displacements = CoSimIO.ImportData(conn_name, identifier)
    tau_parallel_sync()

    # identifier is the data-name in json
    if identifier == "Interface_disp":
###08/07/2020      ###  if tau_mpi_rank() == 0:
###08/07/2020      ### 	    relative_displacements = TauFunctions.ExecuteBeforeMeshDeformation(displacements, step, para_path_mod, start_step)
###08/07/2020      ###        tau_parallel_sync()
        # Read tau's parameter file
###08/07/2020      ###  Para = PyPara.Parafile(para_path_mod)
        Para_origin = PyPara.Parafile(para_path)
        # Read the interface fluid grid
        ids, coordinates = PySurfDeflect.read_tau_grid(Para_origin)
        tau_parallel_sync()
        if tau_mpi_rank() == 0:
            with open('ids_coordinates' + str(step) + '.dat', 'w') as fname:
                for i in range(len(ids)):
                    fname.write("%f %f %f %f\n" % (ids[i],coordinates[0,i],coordinates[1,i],coordinates[2,i]))
        tau_parallel_sync()
        # Write membrane's displacments in a file
        if tau_mpi_rank() == 0:
###08/07/2020      ### TauFunctions.WriteInterfaceDeformationFile(ids, coordinates, relative_displacements)
            new_displacements = TauFunctions.ChangeFormatDisplacements(displacements)
            with open('new_displacement' + str(step) + '.dat','w') as fname:
                for i in range(len(new_displacements)):
                    fname.write("%f %f %f\n" %(new_displacements[i,0] + coordinates[0,i], new_displacements[i,1] + coordinates[1,i],new_displacements[i,2] +coordinates[2,i]))	
            ####08/07/2020      ### print new_displacements
            TauFunctions.WriteInterfaceDeformationFile(ids, coordinates, new_displacements)
        tau_parallel_sync()
        # Deform mesh
        Deform.run(read_primgrid=1, write_primgrid=1, read_deformation=0, field_io=1)
    else:
	raise Exception('TauSolver::ExportData::identifier "{}" not valid! Please use Interface_disp'.format(identifier))
    tau_parallel_sync()

    if tau_mpi_rank() == 0:
        if tau_settings["echo_level"] > 0:
            print 'maximum_displacement_kratos = ', max(displacements)
            print "TAU SOLVER After ImportData"
    tau_parallel_sync()

def ExportData(conn_name, identifier):
    tau_parallel_sync()
    if tau_mpi_rank() == 0:
        if tau_settings["echo_level"] > 0:
            print "TAU SOLVER ExportData"
            
    # identifier is the data-name in json
        if identifier == "Interface_force":
            forces = TauFunctions.ComputeFluidForces(working_path, step, "MEMBRANE_UP", "MEMBRANE_DOWN")
        else:
            raise Exception('TauSolver::ExportData::identifier "{}" not valid! Please use Interface_force'.format(identifier))

        CoSimIO.ExportData(conn_name, identifier, forces)
        if tau_settings["echo_level"] > 0:
            print 'maximum_force_tau = ', max(forces)
            print "TAU SOLVER After ExportData"
    tau_parallel_sync()

def ExportMesh(conn_name, identifier):
    tau_parallel_sync()
    if tau_mpi_rank() == 0:
        if tau_settings["echo_level"] > 0:
            print "TAU SOLVER ExportMesh"

        # identifier is the data-name in json
        if identifier == "Fluid.Interface":
            nodal_coords, elem_connectivities, element_types = TauFunctions.GetFluidMesh(working_path, step)
        else:
            raise Exception(
                'TauSolver::ExportMesh::identifier "{}" not valid! Please use Fluid.Interface'.format(identifier))

        CoSimIO.ExportMesh(conn_name, identifier, nodal_coords, elem_connectivities, element_types)
        if tau_settings["echo_level"] > 0:
            print "TAU SOLVER ExportMesh End"
    tau_parallel_sync()

connection_name = "TAU"

settings = {
    "echo_level" : "0",
    "print_timing" : "1",
    "communication_format" : "file"
}


if rank == 0:
    CoSimIO.Connect(connection_name, settings)

n_steps = int(Para.get_para_value('Unsteady physical time steps'))
coupling_interface_imported = False

for i in range(n_steps):
    AdvanceInTime(0.0)
    InitializeSolutionStep()

    SolveSolutionStep()

    if not coupling_interface_imported:
        ExportMesh(connection_name, "Fluid.Interface")
        coupling_interface_imported = True

    ExportData(connection_name, "Interface_force")
    ImportData(connection_name, "Interface_disp")

    FinalizeSolutionStep()

if rank == 0:
    CoSimIO.Disconnect(connection_name)

'''
if tau_mpi_rank() == 0:
    import tau_python
    for i in sorted(dir(tau_python)):
        print(i)

    CoSimIO.Register_AdvanceInTime(connection_name, AdvanceInTime)
    CoSimIO.Register_InitializeSolutionStep(connection_name, InitializeSolutionStep)
    CoSimIO.Register_SolveSolutionStep(connection_name, SolveSolutionStep)
    CoSimIO.Register_FinalizeSolutionStep(connection_name, FinalizeSolutionStep)
    CoSimIO.Register_ImportData(connection_name, ImportData)
    CoSimIO.Register_ExportData(connection_name, ExportData)
    CoSimIO.Register_ExportMesh(connection_name, ExportMesh)

    # Run the coupled simulation, this returns after the entire CoSim is done
    CoSimIO.Run(connection_name)

    CoSimIO.Disconnect(connection_name)
'''
tau("exit")