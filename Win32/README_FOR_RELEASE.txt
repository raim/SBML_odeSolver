This zip file contains a number of executables (*.exe)
and their associated dlls (*.dll).  

These executables are designed to be executed as command line
applications from a DOS, Cygwin shell or similar shell.  This file documents
how to use those executables in detail.

Installation without easy uninstall
===================================

Copy the files in this zip file into the C:\WINDOWS\SYSTEM32 directory.
Uninstall is difficult as you have to remove these files by hand.

(Later versions of SBML_odeSolver will have an installer/uninstaller.)

Installation for ease of uninstallation
=======================================

To install these applications copy the contents of this zip file into
a new directory.  Then put
the full path to this directory on the path environment variable.
(see following section).

To uninstall simply delete the directory you have created.

Adding the executables directory to the path variable
======================================================
To do this on Windows XP proceed as follows:
Open the directory containing the contents of the zip.
Select any file in the directory.
Click the right mouse button and select properties from the menu
In the resulting dialog select the content of location field 
(the path of the directory containing the file)
Using the control-c key copy the value to the clipboard
On the windows start menu select control panel
select performance and maintainence 
select system
in the resulting dialog select the advanced tab
select the environment varibales button
in the resulting dialog scroll the lower 'System Variables' list down
and select the 'path' variable
select the edit button
in the value field append ';' to the end of the existing value
then using the control-v key paste the value on the clipboard immediately following the ';'
select the OK button
select the OK button
select the OK button
close the control panel window

Tutorial on using the Excutables in Cygwin
==========================================

Using Cygwin
------------

An effective environment to run SBML_odeSolver is in the Cygwin shell.
(The following command sequence can be adapted for DOS shells but details are not given.)
To install Cygwin go to http://www.cygwin.com/
click on the install link, run the installer directly and follow the instructions using defaults where ever possible.
In addition download the gnuplot for windows gp400win32.zip from ftp://ftp.gnuplot.info/pub/gnuplot/
Proceed as for the previous two sections above for the gnuplot download being careful to append the gnuplot/bin
directory to the path environment variable (or perhaps copy the contents of gnuplot/bin to C:\WINDOWS\SYSTEM32)

After running the installation select Cygwin/Cygwin Bash Shell from the start menu.

Simulation
----------

To simulate a model type a command similar to the following in the shell:

SBML_odeSolverApp model.xml --time 100 --printstep 200 > model.txt

This command runs a simulation of SBML model model.xml for 200 timesteps for a simulation time of 100 time units.
The results are placed in the file model.txt

To display the results we need to first create a gnuplot file from the model output using the following command:

gnuplotscript.awk model.txt S1 S2 > model.gnuplot

This generates a gnuplot script to display the data in model.txt for the time course of S1 and S2 

to display the plot type the command:

wgnuplot -persist model.gnuplot &

Parameter Scanning
------------------

To generate a 3D plot of a species concentration over time series for a range of parameter or species concentrations
type a command as follows:

ParameterScanner model.xml 200 0.5 k1 0 1 0.1 S1 > pscan.gnuplot

This creates plot data of a parameter scan in model.xml of parameter k1 from 0 through 1 in steps of 0.1.
The surface plotted is the contentration of S1 in 200 timesteps of 0.5 time units.

to display the plot type the command:

wgnuplot -persist pscan.gnuplot &

Sensitivity Analysis
--------------------

To perform solution forward sensitivity analysis type a command as follows:

Sense model.xml 200 0.5 p k1 S1 S2 > sense.gnuplot

This creates a plot of the sensitivity of species S1 and S2 to the values of the parameter k1
for the timecourse of 200 timesteps of 0.5 time units.

to display the plot type the command:

wgnuplot -persist sense.gnuplot &

Alternative one can type a command as follows:

Sense model.xml 200 0.5 v S1 k1 k2 > sense.gnuplot

This creates a plot of the sensitivity of species S1 to the values of the parameters k1 and k2
for the timecourse of 200 timesteps of 0.5 time units.

Learning more
-------------

You can learn more about the various 3rd party tools used above as follows:

The Bash shell

http://pegasus.rutgers.edu/~elflord/unix/bash-tute.html
http://www.tldp.org/HOWTO/Bash-Prog-Intro-HOWTO.html

GNUPLOT

http://t16web.lanl.gov/Kawano/gnuplot/index-e.html
http://www.gnuplot.info/faq/faq.html
http://www.cs.uni.edu/Help/gnuplot/

AWK

http://www.vectorsite.net/tsawk2.html
http://www.gnu.org/software/gawk/manual/gawk.html

Executing executables in a DOS shell
=====================================

If you don't wish to use cygwin then its possible to the DOS shell

from the start menu select all programs
select accessories
select command prompt
in the resulting window type the commands as detailed below

Note that the gnuplotscript script can't be invoked directly in DOS
and requires an awk installation and explicit invocation of awk

Reference Guide to SOSlib Applications
======================================

All the executables in the directory when run without arguments
output a lisiting of the various arguments and options required
to run the excutable.

SBML_odeSolverApp
-----------------

This executable can be excuted with a command of the following form:

SBML_odeSolverApp <SBML File> [OPTIONS]

where the options are as follows

GENERAL OPTIONS
 -h, --help            Print (this) usage information.
 -i, --interactive     Turn on interactive mode
     --gvformat <Str>  Set output format for graph drawings (now set
                       to: ps); ignored if compiled w/o graphviz)
SBML FILE PARSING
 -v, --validate        Validate SBML file before further processing
     --model <Str>     SBML file name (not needed!, see USAGE)
                       (now set to: )
     --mpath <Dir>     Set Model File Path
                       (now set to: ./)
     --schema11 <Str>  Set filename for SBML schema Level 1 Version 1
                       (now set to: sbml-l1v1.xsd)
     --schema12 <Str>  Set filename for SBML schema Level 1 Version 2
                       (now set to: sbml-l1v2.xsd)
     --schema21 <Str>  Set filename for SBML schema Level 2 Version 1
                       (now set to: sbml-l2v1.xsd)
     --spath <Dir>     Set schema file path, absolute or relative to
                       model path (now set to: ./)
(1) PRINT REACTIONS AND DERIVED ODEs
 -e, --equations       Print Reactions and derived ODE system
 -o, --printsbml       Construct ODEs and print as SBML model
 -g, --modelgraph      Draw bipartite graph of reaction network
                       (to .dot text file if compiled w/o graphviz)
(2) INTEGRATING
 -f, --onthefly        Print results during integration
 -l, --message         Print messages, and integration statistics
 -j, --jacobian        Toggle use of the jacobian matrix or CVODE's
                       internal approximation (default: jacobian)
 -s, --steadyState     Abort integration at steady states
 -t, --sensitivity     activate sensitivity analysis (default: no)
 -n, --event           Do not abort on event detection, but keep
                       integrating. ACCURACY DEPENDS ON --printstep!!
     --printstep <Int> Time steps of output, or
                       (now set to: 50)
     --time <Float>    Integration end time
                       (now set to: 1)
     --error <Float>   Absolute error tolerance during integration
                       (now set to: 1e-009)
     --rerror <Float>  Relative error tolerance during integration
                       (now set to: 0.0001)
     --mxstep <Int>    Maximum step number during integration
                       (now set to: 10000)
(3) INTEGRATION RESULTS
 -a, --all             Print all available results (y/k/r + conc.).
 -y, --jacobianTime    Print time course of jacobian matrix entries,
                       instead of concentrations
 -k, --reactions       Print time course of the reactions
                       (kinetic laws) instead of concentrations
 -r, --rates           Print time course of the ODEs, instead of
                       concentrations
 -w, --write           Write results to file (path/modelfile.xml.dat)
 -x, --xmgrace         Print results to XMGrace; uses SBML Names
                       instead of Ids (ignored if compiled w/o Grace)
 -m, --matrixgraph     Draw species interactions from the jacobian
                       matrix at last timepoint of integration
                       (to .dot text file if compiled w/o graphviz)
                       
Results are simply output to the standard output stream.  By default
just the species concentration time series is output together with
column headings with species ids.  This output can plotted in gnuplot
using the gnuplotscript.awk command.

gnuplotscript.awk
-----------------

This command generates a gnuplot plot file on standard output from 
the default output of SBML_odeSolverApp.  This command has the form:

gnuplotscript.awk odeSolverApp-output-file species

where odeSolverApp-output-file contains the output of SBML_odeSolverApp
and species is a sequence of species ids which are required for plotting.

The generated gnuplot simply refers to to the content of odeSolverApp-output-file 

ParameterScanner
----------------

This command generates a gnuplot surface plot which contains the effect
of a range of values of a given parameter on the simulated time series
of a given species.

The command line for this executable has the following form:

ParameterScanner sbml-model-file
    time-steps time-step-length parameter parameter-start parameter-end
    parameter-step species [absolute-error-tolerance] [relative-error-tolerance]
    [maximum integration steps]

where 'time-steps' is the number of time steps to take
where 'time-step-length' is the interval in simulation time between time steps
where 'parameter-start' is the lower bound on the scanned range of parameter values
where 'parameter-end' is the upper bound on the scanned range of parameters values
where 'parameter-step' is the interval between sampled parameter values
where 'species' is the species whose concentration is plotted as a surface

The gnuplot output is simply output to the standard output stream.

Sense
-----

This command generates a gnuplot 2D file which plots the forward solution sensitivity
of one or more species concentrations to one or more parameters.
The command line for this executable has the following form:

Sense sbml-model-file time-steps time-step-length mode symbol [symbols]
  [absolute-error-tolerance] [relative-error-tolerance] [maximum integration steps]
  
where 'time-steps' is the number of time steps to take
where 'time-step-length' is the interval in simulation time between time steps

mode is either 'p' or 'v'.  If mode is 'p' then the sensitivies of one or more
given species to a given parameter is displayed.  If mode is 'v' then the sensitivies
of a given species to one or more given parameters is displayed.  

symbol is a variable (non constant species, compartment or parameter) if mode is 'v'.
symbol is a parameter if mode is 'p'

symbols is a sequence of parameters if mode is 'v'
symbols is a sequence of variables if mode is 'p'

The gnuplot output is simply output to the standard output stream.
