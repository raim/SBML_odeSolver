#ifndef _CVODESETTINGS_H_
#define _CVODESETTINGS_H_

/* Settings for CVODE Integration */
typedef struct _CvodeSettings {
  double Time;          /* Time to which model is integrated or if 'Indefinitely' its the step size */
  double PrintStep;     /* Number of output steps from 0 to 'Time' ignored if 'Indefinitely' */
  double Error;         /* absolute tolerance in Cvode integration */
  double RError;        /* relative tolerance in Cvode integration */
  double Mxstep;        /* maximum step number for CVode integration */
  int Indefinitely;     /* run without a defined end time, Time field contains step duration, ignore PrintStep field*/
  int PrintMessage;     /* Print messages */
  int PrintOnTheFly;    /* Print species concentration during integration */
  int HaltOnEvent;      /* Stops integration upon an event */
  int SteadyState;      /* Stops integration upon a steady state */
  int UseJacobian;      /* Toggle use of Jacobian ASTs or approximation */
  int StoreResults;     /* Store time course history */
} CvodeSettings;

#endif /* _CVODESETTINGS_H_ */