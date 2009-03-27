#include "SBWOdeSolver.h"

#include <sbml/SBMLWriter.h>
#include <sbml/SBMLReader.h>
#include <sbml/SBMLTypes.h>
#include <iostream>

using namespace std;

SBWOdeSolver::SBWOdeSolver () 
{
	_dTimeStart = 0.0;
	_dTimeEnd = 25.0;
	_nNumPoints = 100;
	_bChangedNumPoints = false;
	_bChangedTimeStart = false;
	_bChangedTimeEnd   = false;
	_bContinuous = false;
}
SBWOdeSolver::~SBWOdeSolver ()
{
}


void SBWOdeSolver::freeOldModel()
{
	try
	{
		_oConstant.clear();
		_oVariables.clear();
		_oAssigned.clear();

		_oParameters.clear();
		_oSpecies.clear();
		_oReactions.clear();
		_oCompartments.clear();

		if (_oIntegrator != NULL)
			IntegratorInstance_free(_oIntegrator);
		if (_oSettings != NULL)
			CvodeSettings_free(_oSettings);
		if (_oDet != NULL)
		{
			ASTNode_free(_oDet);
			_oDet = NULL;
		}
		if (_oOdeModel != NULL)
		{
			ODEModel_free(_oOdeModel);
			_oOdeModel = NULL;
		}
		if (_oOde != NULL)
		{
			Model_free(_oOde);
			_oOde = NULL;
		}
		//if (_model != NULL)
		//{
		//	Model_free(_model);
		//	_model = NULL;
		//}
		if (_sbmlDoc != NULL)
		{
			SBMLDocument_free(_sbmlDoc);
			_sbmlDoc = NULL;
		}

	}
	catch (...)
	{
	}
	_oDet = NULL;
	_oOdeModel = NULL;
	_oOde = NULL;
	_model = NULL;
	_sbmlDoc = NULL;
	_oSettings = NULL;
	_oIntegrator = NULL;

}
void SBWOdeSolver::loadSBML(std::string sSBML)
{

	freeOldModel();
	SBMLReader oReader;

	//if ( (_sbmlDoc = parseModelFromString(strdup(sSBML.c_str()), 0, 0, "",
	//	"", "", "")) == NULL )

	if ((_sbmlDoc = oReader.readSBMLFromString(sSBML.c_str())) == NULL)
	{
		_sbmlDoc  = NULL;
		throw new SBWApplicationException(
			"The model could not be read.", 
			"Please validate that the given model is indeed valid SBML.");
	}
	else
	{
		SBMLDocument *oDoc = (SBMLDocument*) _sbmlDoc;
		if (oDoc->getLevel() == 1)
		{
			oDoc->setLevelAndVersion(2,1);
			
			SBMLWriter oWriter; string sSBML = oWriter.writeToString(oDoc);
			SBMLReader oReader; oDoc = oReader.readSBMLFromString(sSBML);
			_sbmlDoc = oDoc;
		}
		_model = SBMLDocument_getModel(oDoc);
		_oModel = (Model*) _model;
		
		_oOdeModel = ODEModel_create(_model);
		if (_oOdeModel == NULL)
			throw new SBWApplicationException("Loading failed", "The model could not be loaded due to problems with the simulator. The most likely cause is that this model uses a feature not supported by the simulator");
		_oDet= ODEModel_constructDeterminant(_oOdeModel);
		_oOde = Model_reduceToOdes(_model);
	}	

	int nCount = _oOdeModel->nalg + _oOdeModel->nass + _oOdeModel->nconst + _oOdeModel->neq;
	/*for (int i = 0; i < nCount; i++)
	{
		cout << i << "\t" << _oOdeModel->names[i] << endl;
	}*/

	for (int i = 0; i < _oOdeModel->neq; i++)
	{
		_oVariables.push_back(_oOdeModel->names[i]);
	}
	for (int i = _oOdeModel->neq; i < _oOdeModel->neq + _oOdeModel->nass; i++)
	{
		_oAssigned.push_back(_oOdeModel->names[i]);
	}
	for (int i = _oOdeModel->neq + _oOdeModel->nass; i < _oOdeModel->neq + _oOdeModel->nass + _oOdeModel->nconst; i++)
	{
		_oConstant.push_back(_oOdeModel->names[i]);
	}

	_nNumReactions = _oModel->getNumReactions();
	for (int i = 0; i < _nNumReactions ; i++)
	{
		_oReactions.push_back(_oModel->getReaction(i)->getId());
	}
	_nNumCompartments = _oModel->getNumCompartments();
	for (int i = 0; i < _nNumCompartments; i++)
	{
		_oCompartments.push_back(_oModel->getCompartment(i)->getId());
	}
	_nNumSpecies = _oModel->getNumSpecies();
	for (int i = 0; i < _nNumSpecies ; i++)
	{
		if (!_oModel->getSpecies(i)->getBoundaryCondition())
		_oSpecies.push_back(_oModel->getSpecies(i)->getId());
	}
	_nNumParameters = _oModel->getNumParameters();
	for (int i = 0; i < _nNumParameters; i++)
	{
		_oParameters.push_back(_oModel->getParameter(i)->getId());
	}

	// little bit of sanity checking ... 
	if (_model == NULL || _oOdeModel == NULL ||  _oOde == NULL)
	{
		freeOldModel();
		throw new SBWApplicationException("Loading failed", "The model could not be loaded due to problems with the simulator. The most likely cause is that this model uses a feature not supported by the simulator");
	}


}
std::string SBWOdeSolver::getSBML()
{
	if (_sbmlDoc == NULL)
		throw new SBWApplicationException(
			"No model loaded yet.", 
			"Please load a model before using this function.");
	SBMLWriter_t *oWriter = SBMLWriter_create();
	SBMLWriter_setProgramName (oWriter, "SBW ODE Solver Module");
	SBMLWriter_setProgramVersion (oWriter, "CVS 10/13/2007");
	char *result = SBMLWriter_writeSBMLToString(oWriter, _sbmlDoc);
	SBMLWriter_free(oWriter);
	return result;
}
void SBWOdeSolver::setTimeStart(double dStart)
{
	if (dStart != _dTimeStart)
	{
		_dTimeStart = dStart;
		_bChangedTimeStart = true;
	}
}
void SBWOdeSolver::setTimeEnd (double dEnd)
{
	if (dEnd != _dTimeEnd)
	{
		_dTimeEnd = dEnd;
		_bChangedTimeEnd = true;
	}
}
void SBWOdeSolver::setNumPoints (int nPoints)
{
	if (nPoints != _nNumPoints)
	{
		_nNumPoints = nPoints;
		_bChangedNumPoints = true;
	}
}
void SBWOdeSolver::reset()
{
	if (_oIntegrator == NULL)
		return;
		//throw new SBWApplicationException ("Please load a model first.", "IntegratorInstance was NULL");
	if (_oIntegrator->data->model  != NULL)
	IntegratorInstance_reset(_oIntegrator);	
}

double** SBWOdeSolver::simulate(int &xDim, int &yDim)
{
	if (_oVariables.size() == 0)
		return (double**)malloc(sizeof(double*)*1);
	try
	{
		if (_oIntegrator == NULL || _oSettings == NULL || _bChangedNumPoints || _bChangedTimeEnd || _bContinuous)
		{	
			if (_oSettings != NULL)
			{
				CvodeSettings_free(_oSettings);
				IntegratorInstance_free(_oIntegrator);
				_bChangedNumPoints = false;	
				_bChangedTimeEnd = false;			
			}

			_oSettings = CvodeSettings_createWith(_dTimeEnd, _nNumPoints,
				_dAbsTol, _dRelTol,
				_nMaxNumSteps, 1, 0,
				0, 0, 0,
				0, 1, 0, 2);
			CvodeSettings_setCompileFunctions(_oSettings, 0);
			CvodeSettings_setResetCvodeOnEvent(_oSettings, 0);
			_oIntegrator = IntegratorInstance_create(_oOdeModel, _oSettings);		
			_bContinuous = false;
		}

		
		
		integrator(_oIntegrator);		
		return getConcentrationTimeCourse(_oIntegrator->data, xDim, yDim);

	}
	catch(...)
	{
		freeOldModel();
		throw new SBWApplicationException("Error during simulation, please reload your model, making sure that it is valid.");
	}

}

int SBWOdeSolver::integrator(integratorInstance_t *engine)
{
	if (engine == NULL)
		throw new SBWApplicationException("Invalid Model, please reload.");
	odeModel_t *om = engine->om;
	cvodeData_t *data = engine->data;
	cvodeSolver_t *solver = engine->solver;


	while (!IntegratorInstance_timeCourseCompleted(engine)) 
	{

		if (!IntegratorInstance_integrateOneStep(engine)) 
		{
			/* SolverError_dump(); */
			return IntegratorInstance_handleError(engine);
		}
	}


	return 0;

} 


double** SBWOdeSolver::simulateEx(double dStart, double dEnd, int nPoints, int &xDim, int &yDim)
{
	setTimeStart(dStart);
	setTimeEnd(dEnd);
	setNumPoints(nPoints);

	return simulate(xDim, yDim);
}
void SBWOdeSolver::setFloatingSpeciesConcentrations(std::vector< double > oValues)
{
	throw new SBWApplicationException ("Not implemented yet.", "SBWOdeSolver::setFloatingSpeciesConcentrations(std::vector< double > oValues)");

}

std::vector<double> SBWOdeSolver::getReactionRates()
{
	if (_sbmlDoc == NULL)
		throw new SBWApplicationException ("Please load a model first.", "A model has to be loaded prior to calling this function.");

	std::vector< double > result;
	bool bFree = false;
	
	cvodeData_t *data = NULL;
	if (_oIntegrator != NULL)
		data = _oIntegrator->data;

	cvodeResults_t *results;
	Reaction_t *r;
	KineticLaw_t *kl;
	ASTNode_t **kls;
	unsigned int i;
	if(!(kls = (ASTNode_t **)calloc(Model_getNumReactions(_model), sizeof(ASTNode_t *)))) 
	{
		throw new SBWApplicationException("Could not find reactions.");
	}

	if ( data == NULL || data->results == NULL ) 
	{

		data = CvodeData_create(_oOdeModel);
		bFree =true;
		for (i=0; i<Model_getNumReactions(_model); i++ ) 
		{
			r = Model_getReaction(_model, i);
			kl = Reaction_getKineticLaw(r);
			kls[i] = copyAST(KineticLaw_getMath(kl));
			AST_replaceNameByParameters(kls[i], KineticLaw_getListOfParameters(kl));
			AST_replaceConstants(_model, kls[i]);
		}

		for (int i = 0; i < _nNumSpecies; i++)
		{			
			const Species_t* species = Model_getSpecies(_model, i);
			data->value[i] = Species_getInitialConcentration (species);
		}


	}
	else
	{
		results = data->results;

		for (i=0; i<Model_getNumReactions(_model); i++ ) 
		{
			r = Model_getReaction(_model, i);
			kl = Reaction_getKineticLaw(r);
			kls[i] = copyAST(KineticLaw_getMath(kl));
			AST_replaceNameByParameters(kls[i], KineticLaw_getListOfParameters(kl));
			AST_replaceConstants(_model, kls[i]);
		}
		i = results->nout-1; // last row

		for (int j=0; j<data->model->neq; j++ ) 
		{
			data->value[j] = results->value[j][i];
		}
	}

	for (unsigned int i=0; i<Model_getNumReactions(_model); i++ ) {
		result.push_back(evaluateAST(kls[i], data));
		ASTNode_free(kls[i]);
	}
	free(kls);

	if (bFree)
	{
		CvodeData_free(data);
	}

	return result;
}
std::vector<double> SBWOdeSolver::getRatesOfChange()
{
	std::vector<double> result;
	int i,j;
	cvodeResults_t *results;
	cvodeData_t *data = NULL;
	if (_oIntegrator != NULL)
		data = _oIntegrator->data;

	if ( data == NULL || data->results == NULL ) 
	{
		data = CvodeData_create(_oOdeModel);
		data->currenttime = 0.0;
		for ( j=0; j<data->model->neq; j++ ) 
		{
			const Species_t* species = Model_getSpecies(_model, j);
			data->value[j] = Species_getInitialConcentration (species);
			result.push_back(evaluateAST(data->model->ode[j],data));			
		}
		CvodeData_free(data);

	}
	else
	{
		results = data->results;
		i = results->nout-1;		
		data->currenttime = (float)results->time[i];
		for ( int j=0; j<data->model->neq; j++ ) {
			data->value[j] = results->value[j][i];
			result.push_back(evaluateAST(data->model->ode[j],data));			
		}
	}


	return result;
}
std::vector<std::string> SBWOdeSolver::getFloatingSpeciesNames()
{
	if (_sbmlDoc == NULL)
		throw new SBWApplicationException ("Please load a model first.", "A model has to be loaded prior to calling this function.");
	return _oVariables;
}

double SBWOdeSolver::getFloatingSpeciesByIndex(int index)
{
	if (_sbmlDoc == NULL)
		throw new SBWApplicationException ("Please load a model first.", "A model has to be loaded prior to calling this function.");

	if (index >= (int)_oVariables.size())
		throw new SBWApplicationException ("Index Out of Bounds.", "No floating species exists with the given index.");

	if (_oIntegrator == NULL)
	{
		const Species_t* species = Model_getSpecies(_model, index);
		return Species_getInitialConcentration (species);

	}
	else
	{
		if (_bContinuous)
			return _oIntegrator->data->value[index];		
		else
			return _oIntegrator->data->results->value[index][_oIntegrator->solver->nout-1];		
	}
	

}
std::vector<std::string> SBWOdeSolver::getReactionNames()
{
	if (_sbmlDoc == NULL)
		throw new SBWApplicationException ("Please load a model first.", "A model has to be loaded prior to calling this function.");

	return _oAssigned;
}


double **SBWOdeSolver::getConcentrationTimeCourse(cvodeData_t *oData , int &xDimOut, int &yDimOut)
{
	int xDim, yDim;
	cvodeResults_t *results;
	odeModel_t *om;

	if ( oData == NULL || oData->results == NULL ) {
		throw new SBWApplicationException("No results, please integrate first.", "Error occured while trying to create the results set.");      
	}


	results = oData->results;
	om = oData->model;

	yDim = results->nout;
	//xDim = oData->nvalues+1;
	xDim = (int)_oVariables.size()  + 1;

	double **oResult = (double**)malloc(sizeof(double*) * yDim);
	memset(oResult, 0, sizeof(double*) * yDim);
	for (int i = 0; i < yDim; i++)
	{
		oResult[i] = (double*)malloc(sizeof(double) * xDim);
		memset(oResult[i], 0, sizeof(double) * xDim);
	}

	for (int y = 0; y < yDim; y++)
	{
		for (int x = 0; x < xDim; x++)
		{
			if (x == 0)
			{
				oResult[y][x] = results->time[y];
			}
			else
			{					
				oResult[y][x] = results->value[x-1][y];
			}
		}
	}

	xDimOut = xDim;
	yDimOut = yDim;
	return oResult;
}

double SBWOdeSolver::oneStep(double dCurrentTime, double dStepSize)
{
	if (_oIntegrator == NULL || !_bContinuous)
	{
		if (_oIntegrator != NULL)
			IntegratorInstance_free(_oIntegrator );
		if (_oSettings != NULL)
		{			
			CvodeSettings_free(_oSettings);
		}
		_oSettings = CvodeSettings_createWith(dStepSize, _nNumPoints,
			_dAbsTol, _dRelTol,
			_nMaxNumSteps, 1, 0,
			0, 1, 0,
			0, 1, 0, 2);
		CvodeSettings_setIndefinitely(_oSettings, 1);

		CvodeSettings_setCompileFunctions(_oSettings, 0);
		CvodeSettings_setResetCvodeOnEvent(_oSettings, 0);
		CvodeSettings_setTime(_oSettings, dStepSize, 1);
		_oIntegrator = IntegratorInstance_create(_oOdeModel, _oSettings);				
		//IntegratorInstance_resetAdjPhase(_oIntegrator);
		_bContinuous = true;
	}

	_oIntegrator->solver->iout --; 
	_oIntegrator->solver->t0 = dCurrentTime;
	_oIntegrator->solver->t = dCurrentTime;
	_oIntegrator->opt->Time = dStepSize;	
	IntegratorInstance_integrateOneStep(_oIntegrator);
	return dCurrentTime + dStepSize;
}


SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwGetFloatingSpeciesByIndex (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader)
{
	int index;
	reader >> index;
	return SystemsBiologyWorkbench::DataBlockWriter() << getFloatingSpeciesByIndex(index);
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwLoadSBML (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader)
{
	std::string sSBML;
	reader >> sSBML;
	loadSBML(sSBML);
	return SystemsBiologyWorkbench::DataBlockWriter();
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwOneStep (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader)
{
	double dStartTime; double dStepSize;
	reader >> dStartTime >> dStepSize;
	return SystemsBiologyWorkbench::DataBlockWriter() << oneStep(dStartTime, dStepSize);
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwGetSBML (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader )
{
	return SystemsBiologyWorkbench::DataBlockWriter() << getSBML(); 
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwSetTimeStart (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader)
{
	double dTimeStart;
	reader >> dTimeStart;
	setTimeStart(dTimeStart);
	return SystemsBiologyWorkbench::DataBlockWriter();
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwSetTimeEnd (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader)
{
	double dTimeEnd;
	reader >> dTimeEnd;
	setTimeEnd(dTimeEnd);
	return SystemsBiologyWorkbench::DataBlockWriter();
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwSetNumpoints (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader)
{
	int nPoints; reader >> nPoints;
	setNumPoints(nPoints);
	return SystemsBiologyWorkbench::DataBlockWriter();
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwReset (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader)
{
	reset();
	return SystemsBiologyWorkbench::DataBlockWriter();
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwSimulate (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader)
{
	int xDim = 0; int yDim = 0; double **oResult = NULL;
	oResult = simulate(xDim, yDim);
	SystemsBiologyWorkbench::DataBlockWriter writer; 
	writer.add(yDim, xDim, oResult);
	for (int x = 0; x < yDim; x++)
		free(oResult[x]);
	free (oResult);
	return writer;
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwSimulateEx (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader)
{
	double dTimeStart; double dTimeEnd; int nNumPoints; int xDim; int yDim; double **oResult;
	reader >> dTimeStart >> dTimeEnd >> nNumPoints;
	oResult = simulateEx(dTimeStart, dTimeEnd, nNumPoints, xDim, yDim);
	SystemsBiologyWorkbench::DataBlockWriter writer; 
	writer.add(yDim, xDim, oResult);
	for (int x = 0; x < yDim; x++)
		free(oResult[x]);
	free (oResult);
	return writer;
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwSetFloatingSpeciesConcentrations (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader reader)
{
	std::vector<double> oValues;
	reader >> oValues;
	setFloatingSpeciesConcentrations(oValues);
	return SystemsBiologyWorkbench::DataBlockWriter();
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwGetReactionRates (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader)
{
	return SystemsBiologyWorkbench::DataBlockWriter() << getReactionRates();
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwGetRatesOfChange (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader)
{
	return SystemsBiologyWorkbench::DataBlockWriter() << getRatesOfChange();
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwGetFloatingSpeciesNames (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader)
{
	std::vector<std::string> oNames = getFloatingSpeciesNames();
	SystemsBiologyWorkbench::DataBlockWriter result;
	for (unsigned int i = 0; i < oNames.size(); i++)
		result << oNames[i];
	return  SystemsBiologyWorkbench::DataBlockWriter() << result;
}
SystemsBiologyWorkbench::DataBlockWriter SBWOdeSolver::sbwGetReactionNames (SystemsBiologyWorkbench::Module /*from*/, SystemsBiologyWorkbench::DataBlockReader)
{
	std::vector<std::string> oNames = getReactionNames();
	SystemsBiologyWorkbench::DataBlockWriter result;
	for (unsigned int i = 0; i < oNames.size(); i++)
		result << oNames[i];
	return  SystemsBiologyWorkbench::DataBlockWriter() << result;
}
double SBWOdeSolver::_dAbsTol = 1E-20;
double SBWOdeSolver::_dRelTol = 1E-6;
int    SBWOdeSolver::_nMaxNumSteps = 10000;
SBMLDocument_t *SBWOdeSolver::_sbmlDoc = NULL;
Model_t        *SBWOdeSolver::_model = NULL;
Model          *SBWOdeSolver::_oModel= NULL;
Model_t        *SBWOdeSolver::_oOde= NULL;
ASTNode_t *SBWOdeSolver::_oDet= NULL;
odeModel_t *SBWOdeSolver::_oOdeModel= NULL;
integratorInstance_t *SBWOdeSolver::_oIntegrator= NULL;
cvodeSettings_t *SBWOdeSolver::_oSettings= NULL;