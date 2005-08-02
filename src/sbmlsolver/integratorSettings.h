#ifndef _CVODESETTINGS_H_
#define _CVODESETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

  /* structures */
  typedef struct _CvodeSettings cvodeSettings_t;
  typedef struct timeSettings timeSettings_t;

  /* timeSettings: start- and end times, timesteps, number of timesteps */
  struct timeSettings {
    double t0;
    double tmult;
    double tend;
    int nout;
  } ;

  /* Settings for CVODE Integration */
  struct _CvodeSettings {
    double Time;          /* Time to which model is integrated or if
			     'Indefinitely' its the step size */
    double PrintStep;     /* Number of output steps from 0 to 'Time'
			     ignored if 'Indefinitely' */
    double Error;         /* absolute tolerance in Cvode integration */
    double RError;        /* relative tolerance in Cvode integration */
    double Mxstep;        /* maximum step number for CVode integration */
    int Indefinitely;     /* run without a defined end time, Time field
			     contains step duration, ignore PrintStep
			     field*/
    int HaltOnEvent;      /* Stops integration upon an event */
    int SteadyState;      /* Stops integration upon a steady state */
    int UseJacobian;      /* Toggle use of Jacobian ASTs or approximation */
    int StoreResults;     /* Store time course history */
    int EnableVariableChanges; /* enable modification of variables
				  between timesteps set this to 0 for
				  better performance from the solver */
  } ;

  /* functions */
  timeSettings_t * TimeSettings_createWith(double t0, double tend, int nout);
  void TimeSettings_free(timeSettings_t *time);
  
  cvodeSettings_t * CvodeSettings_createDefaults();  
  cvodeSettings_t * CvodeSettings_createWith(double EndTime, int PrintStep,
					     double Error, double RError,
					     double Mxstep, int UseJacobian,
					     int EnableVariableChanges,
					     int Indefinitely, int HaltOnEvent,
					     int SteadyState,
					     int StoreResults);
  void CvodeSettings_free(cvodeSettings_t *set);
#ifdef __cplusplus
}
#endif

#endif /* _CVODESETTINGS_H_ */

