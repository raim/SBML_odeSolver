#ifndef SBW_ODE_SOLVER_H
#define SBW_ODE_SOLVER_H

#include <SBW/SBW.h>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <time.h>

#include <sbml/SBMLTypes.h>

#include "../src/sbmlsolver/odeSolver.h"


class SBWOdeSolver : public SBWListener
{
public:
	SBWOdeSolver ();
	~SBWOdeSolver ();

	
	// descriptive methods
	std::string getName()			{ return "SBWOdeSolver";}
	SystemsBiologyWorkbench::DataBlockWriter getNameSBW (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader /*reader*/)
	{
		return SystemsBiologyWorkbench::DataBlockWriter() << getName();
	}
	std::string getVersion()		{ return "CVS 10/13/2007";}
	SystemsBiologyWorkbench::DataBlockWriter getVersionSBW (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader /*reader*/)
	{
		return SystemsBiologyWorkbench::DataBlockWriter() << getVersion();
	}
	std::string getAuthor()			{ return "Frank Bergmann"; }
	SystemsBiologyWorkbench::DataBlockWriter getAuthorSBW (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader /*reader*/)
	{
		return SystemsBiologyWorkbench::DataBlockWriter() << getAuthor();
	}
	std::string getDescription()	{ return "SBW Interface to SBML ODE Solver";}
	SystemsBiologyWorkbench::DataBlockWriter getDescriptionSBW (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader /*reader*/)
	{
		return SystemsBiologyWorkbench::DataBlockWriter() << getDescription();
	}
	std::string getDisplayName()	{ return "SBWOdeSolver"; } 
	SystemsBiologyWorkbench::DataBlockWriter getDisplayNameSBW (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader /*reader*/)
	{
		return SystemsBiologyWorkbench::DataBlockWriter() << getDisplayName();
	}
	std::string getCopyright()		{ return "(c) 2006 Frank Bergmann"; }
	SystemsBiologyWorkbench::DataBlockWriter getCopyrightSBW (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader /*reader*/)
	{
		return SystemsBiologyWorkbench::DataBlockWriter() << getCopyright();
	}
	std::string getURL()			{ return "http://www.sys-bio.org"; }
	SystemsBiologyWorkbench::DataBlockWriter getURLSBW (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader /*reader*/)
	{
		return SystemsBiologyWorkbench::DataBlockWriter() << getURL();
	}

	double getFloatingSpeciesByIndex(int index);
	SystemsBiologyWorkbench::DataBlockWriter sbwGetFloatingSpeciesByIndex(SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader);

	void loadSBML(std::string sSBML);
	SystemsBiologyWorkbench::DataBlockWriter sbwLoadSBML (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader);
	std::string getSBML();
	SystemsBiologyWorkbench::DataBlockWriter sbwGetSBML (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader );
	void setTimeStart(double dStart);
	SystemsBiologyWorkbench::DataBlockWriter sbwSetTimeStart (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader);
	void setTimeEnd (double dEnd);
	SystemsBiologyWorkbench::DataBlockWriter sbwSetTimeEnd (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader);
	void setNumPoints (int nPoints);
	SystemsBiologyWorkbench::DataBlockWriter sbwSetNumpoints (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader);
	void reset();
	SystemsBiologyWorkbench::DataBlockWriter sbwReset (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader);
	double** simulate(int &xDim, int &yDim);
	SystemsBiologyWorkbench::DataBlockWriter sbwSimulate (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader);
	double** simulateEx(double dStart, double dEnd, int nPoints, int &xDim, int &yDim);
	SystemsBiologyWorkbench::DataBlockWriter sbwSimulateEx (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader);
	void setFloatingSpeciesConcentrations(std::vector< double > oValues);
	SystemsBiologyWorkbench::DataBlockWriter sbwSetFloatingSpeciesConcentrations (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader);
	std::vector<double> getReactionRates();
	SystemsBiologyWorkbench::DataBlockWriter sbwGetReactionRates (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader);
	std::vector<double> getRatesOfChange();
	SystemsBiologyWorkbench::DataBlockWriter sbwGetRatesOfChange (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader);
	std::vector<std::string> getFloatingSpeciesNames();
	SystemsBiologyWorkbench::DataBlockWriter sbwGetFloatingSpeciesNames (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader);
	std::vector<std::string> getReactionNames();
	SystemsBiologyWorkbench::DataBlockWriter sbwGetReactionNames (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader);

	double oneStep(double dCurrentTime, double dStepSize);
	SystemsBiologyWorkbench::DataBlockWriter sbwOneStep (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader);


	static void registerMethods(SystemsBiologyWorkbench::MethodTable<SBWOdeSolver> &table)
	{
		table.addMethod(&SBWOdeSolver::getAuthorSBW,						"string getAuthor()",							false,"Returns the Author.");
		table.addMethod(&SBWOdeSolver::getCopyrightSBW,						"string getCopyright()",						false,"Returns the Copyright.");
		table.addMethod(&SBWOdeSolver::getDescriptionSBW,					"string getDescription()",						false,"Returns the Description.");
		table.addMethod(&SBWOdeSolver::getDisplayNameSBW,					"string getDisplayName()",						false,"Returns the Display Name.");
		table.addMethod(&SBWOdeSolver::getNameSBW,							"string getName()",								false,"Returns the Name.");
		table.addMethod(&SBWOdeSolver::sbwGetFloatingSpeciesNames,			"{} getFloatingSpeciesNames()",					false,"Returns the list of current floating Species.");
		table.addMethod(&SBWOdeSolver::sbwGetFloatingSpeciesByIndex,		"double getFloatingSpeciesByIndex(int)",		false,"Gets the concentration of a floating species by its index value.");

		table.addMethod(&SBWOdeSolver::sbwGetRatesOfChange,					"double[] getRatesOfChange()",					false,"Returns the rates of change.");
		table.addMethod(&SBWOdeSolver::sbwGetReactionNames,					"{} getReactionNames()",						false,"Returns a list containing the names of all the reactions in the model.");
		table.addMethod(&SBWOdeSolver::sbwGetReactionRates,					"double[] getReactionRates()",					false,"Returns the reaction rates for the current time.");
		table.addMethod(&SBWOdeSolver::sbwGetSBML,							"string getSBML()",								false,"Returns the currently loaded SBML model.");
		table.addMethod(&SBWOdeSolver::sbwLoadSBML,							"void loadSBML(string)",						true,"loads a new SBML model into the simulator.");
		table.addMethod(&SBWOdeSolver::sbwOneStep,							"double oneStep(double,double)",				true,"Compute one integration step over the interval stepSize. Returns the value of the new time (usually currentTime + stepSize)");
		table.addMethod(&SBWOdeSolver::sbwSetFloatingSpeciesConcentrations, "void setFloatingSpeciesConcentrations(double[])",true,"Sets the floating species values to the array vector given in the argument. The number of values in the array must equal the number of species in the model.");
		table.addMethod(&SBWOdeSolver::sbwSetNumpoints,						"void setNumPoints(int)",						true,"Sets the number of data points to generate during the simulation, defaults to 50 points if not set.");
		table.addMethod(&SBWOdeSolver::sbwSetTimeStart,						"void setTimeStart(double)",					true,"Sets the time start for the simulation, defaults to 0.0 if not set.");
		table.addMethod(&SBWOdeSolver::sbwSetTimeEnd,						"void setTimeEnd(double)",						true,"Sets the time end for the simulation, defaults to 25.0 time units if not set.");
		table.addMethod(&SBWOdeSolver::sbwSimulate,							"double[][] simulate()",						true,"Simulates the model as given in the loadSBML() method. Returns a 2D array containing time in the first column and species concentrations in the remaining columns.");
		table.addMethod(&SBWOdeSolver::sbwSimulateEx ,						"double[][] simulateEx(double,double,int)",		true,"Simulate the model with the specified timeStart, timeEnd and numPoints. Returns a 2D array containing time in the first column and species concentrations in the remaining columns.");
		table.addMethod(&SBWOdeSolver::sbwReset,							"void reset()",									false,"Resets the simulator to the initial conditions (initial concentrations) specified in the SBML model.");
	}

	// SBW ShutDown
	virtual void onShutdown() { exit(0); }
protected:
	double **getConcentrationTimeCourse(cvodeData_t *oData , int &xDim, int &yDim);
	int integrator(integratorInstance_t *engine);
	void freeOldModel();
private:
	static SBMLDocument_t *_sbmlDoc; //d
	static Model_t        *_model;   //m
    static Model          *_oModel;   //m
	static Model_t        *_oOde;     //ode
	static ASTNode_t *_oDet; //det;
	static odeModel_t *_oOdeModel; //om;
	static integratorInstance_t *_oIntegrator; // ii;
	static cvodeSettings_t *_oSettings; //set;
	
	int _nNumSpecies;
	std::vector<std::string> _oSpecies;
	int _nNumParameters;
	std::vector<std::string> _oParameters;
	int _nNumCompartments;
	std::vector<std::string> _oCompartments;
	int _nNumReactions;
	std::vector<std::string> _oReactions;

	// vector of the model variables
	std::vector<std::string> _oVariables;
	// vector of assigned variables
	std::vector<std::string> _oAssigned;
	// vector of constants
	std::vector<std::string> _oConstant;

	static double _dAbsTol;
    static double _dRelTol;
    static int _nMaxNumSteps;

	double	_dTimeStart;
	double	_dTimeEnd;
	int		_nNumPoints;

	bool	_bChangedTimeStart;
	bool	_bChangedTimeEnd;
	bool	_bChangedNumPoints;
	bool	_bContinuous;
	
	
};

#endif


