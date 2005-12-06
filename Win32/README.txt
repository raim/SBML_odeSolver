SBML_odeSolver for Microsoft Visual C++ 7.1
===========================================

	Andrew Finney
	afinney@physiomics-plc.com

Last change: 9th June 2005 

This directory is an attempt to create a framework for building
SBML_odeSolver in the MSVC 7.1 environment.  

The binary directory at this level just contains binary components that
are combined with elements compiled from the source to create a binary release.

Instructions to build from SOSlib from scratch using this CVS checkout
----------------------------------------------------------------------

a) SBML_odeSolver requires the CVS version of
libSBML checkout the latest version of libSBML and download a
version of the xerces-c libaray binary releases.

b) Set Visual C++ to point to the correct library and include directories
as follows:

select the 'tools'/'Options...' menu item
select the 'Projects'/'VC++ Directories' folder
select 'Include Files' in the 'Show Directories for:' drop down list

add the following entries:

<XERCES-C>\include
<libSBML>\win32\include
<libSBML>\win32\include\sbml
<SBML_odeSolver>\Win32\SBML_odeSolverApp\SBML_odeSolverApp
<SBML_odeSolver>\src
<SBML_odeSolver>\Win32\WinCVODE\sundials\nvec_ser
<SBML_odeSolver>\Win32\WinCVODE\sundials\cvode\include
<SBML_odeSolver>\Win32\WinCVODE\sundials\shared\include
<SBML_odeSolver>\Win32\WinCVODE\sundials\config
<SBML_odeSolver>\Win32\WinCVODE\WinCVODE

(substitute the full paths for these directories)

select 'Library Files' in the 'Show Directories for:' drop down list

add the following entries:

<libSBML>\win32\bin
<SBML_odeSolver>\Win32\bin

b) ensure that the include files for libSBML are placed in the
correct directory structures.  (The libSBML install normally 
restructures this on install).  In its current state you
might want to add the following commands as a custom build step
to <libSBML>\win32\libsbml.vcproj:

xcopy $(ProjectDir)\..\src\sbml\*.h $(ProjectDir)\include\sbml /S /I /F /Y
xcopy $(ProjectDir)\..\src\common\*.h $(ProjectDir)\include\sbml\common /S /I /F /Y
xcopy $(ProjectDir)\..\src\math\*.h $(ProjectDir)\include\sbml\math /S /I /F /Y
xcopy $(ProjectDir)\..\src\util\*.h $(ProjectDir)\include\sbml\util /S /I /F /Y
xcopy $(ProjectDir)\..\src\xml\*.h $(ProjectDir)\include\sbml\xml /S /I /F /Y
xcopy $(ProjectDir)\..\src\validator\*.h $(ProjectDir)\include\sbml\validator /S /I /F /Y

To do this open <libsbml>/win32/libsbml.vcproj then select menu item 'project'/'libSBML properties',
select 'Custom Build Step' and paste the above commands into the 'Command Line' text box.

d) create the directory <SBML_odeSolver>\Win32\bin and then
build the various dlls and the solver executable in the following sequence:

<libSBML>\win32\libsbml.vcproj
<SBML_odeSolver>\Win32\CompleteSOSLib\CompleteSOSLib.sln

e) To run the solver add the following directories to the PATH

<xerces-c>\bin
<libSBML>\win32\bin
<SBML_odeSolver>\win32\bin

IMPORTANT NOTE:

It is possible that you may end up with a conflict between different versions of libSBML if you are using 
other tools that use libSBML.  If this is the case might want to avoid placing the CVS based version of libSBML
on the path (as shown above) and instead copy the dlls from <libSBML>\win32\bin to <SBML_odeSolver>\win32\bin.

f) the solver can then be invoked as SBML_odeSolverApp.exe using command line arguments documented elsewhere.

g) developers wishing to develop other programs that use the SBML_odeSolver dll should use the SBML_odeSolver
libraries and dlls contained in <SBML_odeSolver>\win32\bin combined with the header files in
<SBML_odeSolver>\src\sbmlsolver.  Please read the TODO.txt file.

