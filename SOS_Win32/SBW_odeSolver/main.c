#include <SBW/SBW.h>
#include <iostream>

#include "SBWOdeSolver.h"


using namespace SystemsBiologyWorkbench;
using namespace std;

int main(int argc, char*argv[])
{
	try
	{
		ModuleImpl modImpl("SBWOdeSolver", "SBWOdeSolver", UniqueModule);
		modImpl.addService("SBWOdeSolver", "SBWOdeSolver", "plugin/simulator/level1", "SBWOdeSolver Simulator");
		modImpl.addServiceObject("SBWOdeSolver", "SBWOdeSolver", "plugin/simulator/level1", new SBWOdeSolver());
		modImpl.run(argc,argv);
	}
	catch(SBWException *ex)
	{
		cerr << "Exception occured: " << ex->getMessage() << endl;
	}
	catch(...)
	{
	}
}