#ifndef _CVODESETTINGSSTRUCT_H_
#define _CVODESETTINGSSTRUCT_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Settings for CVODE Integration */
  struct cvodeSettings {
    double Time;          /**< Time to which model is integrated or if
			       step size if 'Indefinitely' is true */
    int PrintStep;        /**< Number of output steps from 0 to 'Time';
			       ignored if 'Indefinitely' */
    double *TimePoints;   /**< Optional array of designed time-course.
			      If passed by the calling application,
			      Time will be ignored and overruled by
			      TimePoints[Printstep+1], otherwise TimePoints
			      will be calculated from Time and PrintSteps */
    int Indefinitely;     /**< if not 0: run without a defined end
			       time, Time field contains step
			       duration, ignore PrintStep field*/
    double Error;         /**< absolute tolerance in Cvode integration */
    double RError;        /**< relative tolerance in Cvode integration */
    int Mxstep;           /**< maximum step number for CVode integration */
    int CvodeMethod;      /**< set ADAMS-MOULTON (1) or BDF (0)
			       nonlinear solver */
    int IterMethod;       /**< set type of nonlinear solver iteration
			       Newton (0) or Functional (1) */
    int MaxOrder;         /**< set maximum order of ADAMS or BDF method */
    int ResetCvodeOnEvent; /**< restart CVODE when event is triggered */
    int Sensitivity;      /**< if not 0: use CVODES for sensitivity analysis */
    int SensMethod;       /**< set sensitivity analysis method:
			       0: SIMULTANEOUS,
			       1: STAGGERED,
			       2: STAGGERED1
			  */    
    int HaltOnEvent;      /**< if not 0: Stop integration upon an event */
    int SteadyState;      /**< if not 0: Stop integration upon a
			       steady state */
    int UseJacobian;      /**< use of Jacobian ASTs (1) or CVODES'
			     internal approximation (0)*/
    int StoreResults;     /**< if not 0: Store time course history */

    int compileFunctions ;  /**< if 1 use compiled functions for ODE, Jacobian and events */

    /**< Adjoint related flags and settings   */   
    int DoAdjoint;          /**< if 1, the adjoint solution is desired   */
    int AdjointPhase;       /**< if 0, do the forward phase of the normal run 
			         or the forward phase in preparation for the adjoint  */
    
    double AdjTime;          /**< Time to which model is integrated or if
			       step size if 'Indefinitely' is true */
    int AdjPrintStep;        /**< Number of output steps from 0 to 'Time';
			       ignored if 'Indefinitely' */ 
     
    double *AdjTimePoints;   /**< Optional array of designed time-course.
			      If passed by the calling application,
			      AdjTime will be ignored and overruled by
			      AdjTimePoints[AdjPrintstep+1], otherwise AdjTimePoints
			      will be calculated from AdjTime and AdjPrintSteps */

    int nSaveSteps;           /**< Number of steps saved in forward phase  */    
    int ncheck;              /**< Number of checkpoints, as returned by CvodeF */

    double AdjError;         /**< absolute tolerance in adjoint integration */
    double AdjRError;        /**< relative tolerance in adjoint integration */ 
    int AdjStoreResults;     /**< if not 0: Store adjoint time course history */


  } ;

#ifdef __cplusplus
}
#endif

#endif
