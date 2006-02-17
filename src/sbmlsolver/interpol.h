/*
  Last changed Time-stamp: <2006-02-17 17:42:32 raim>
  $Id: interpol.h,v 1.2 2006/02/17 17:07:32 raimc Exp $
*/

#ifndef _INTERPOL_H_
#define _INTERPOL_H_

typedef struct ts {
    int    n_var;   /* number of variables in the list */
    char   **var;   /* list of variables */

    int    n_data;  /* number of variables for which data is stored */
    double **data;  /* time series data for variables */
    int    type;    /* interpolation type */
    double **data2; /* interpolation data for variables */
    
    int    n_time;  /* number of time points */
    double *time;   /* time points */
    
    int    last;    /* last interpolation interval */

    char   **mess;  /* list of warning messages */
    int    *warn;   /* number of warnings */
} time_series_t;

void free_data(time_series_t *ts);
void print_data(time_series_t *ts);
void test_interpol(time_series_t *ts);

time_series_t *read_data(char *file, int num, char **var);

double call(int i, double x, time_series_t *ts);

void spline(int n, double *x, double *y, double *y2);
void splint(int n, double *x, double *y, double *y2,
	    double x_, double *y_, int *j);

void linint(int n, double *x, double *y,
	    double x_, double *y_, int *j);
	    
int  bisection(int n, double *x, double x_);
void hunt(int n, double *x, double x_, int *low);

#endif

/* end of file */
