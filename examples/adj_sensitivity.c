
#include <stdio.h>
#include <stdlib.h>

#include <sbmlsolver/odeSolver.h>

int
main (int argc, char *argv[]){

 /* Setting SBML ODE Solver integration parameters  */
  set = CvodeSettings_create();
  CvodeSettings_setTime(set, 300.0, 10);
  CvodeSettings_setErrors(set, 1e-9, 1e-4, 1e9);


  /*  simply does set->Dodjoint = 1 */
   CvodeSettings_setDoAdj(set);

  /* Do the time settings analogous to forward phase, but only reversed */
   CvodeSettings_setAdjTime(set, 300.0, 10);
   CvodeSettings_setAdjErrors(set, 1e-9, 1e-4); 
   CvodeSettings_setAdjnSaveSteps(set, 100);

 

  
  /* creating the odeModel */
  om = ODEModel_createFromFile("MAPK.xml");
  /* get a parameter for which we will check sensitivities */
  p = ODEModel_getVariableIndex(om, "K1");



  /* calling the integrator */
  ii = IntegratorInstance_create(om, set);

  while( !IntegratorInstance_timeCourseCompleted(ii) ) {
     if ( !IntegratorInstance_integrateOneStep(ii) ) {
      break;
     }
  }


  /* Upon completion of forward run, need to set flag that commences the adjoint phase */
  /* Simply does set->ReadyForAdjoint = 1 */
  CvodeSettings_setReadyAdj(set);


  /* After setting the flag to proceed adjoint phase, need to create the necessary adjoint structures
   */
  ii = IntegratorInstance_initializeSolver(om, set);


  // Now the adjoint solver is called in the integration steps 
   while( !IntegratorInstance_timeCourseCompleted(ii) ) {
     if ( !IntegratorInstance_integrateOneStep(ii) ) {
      break;
     }
   }




}
