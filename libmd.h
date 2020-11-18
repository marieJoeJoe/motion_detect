#ifndef __LIBMD_H__
#define __LIBMD_H__
#include <math.h>

#define SAMPLES_NUM 30//242

#define AMP_DIFF_THRES 3.1415926

#define INTERVAL 100 //ms

extern int I_VAL0[SAMPLES_NUM];
extern int Q_VAL0[SAMPLES_NUM];
extern int I_VAL[SAMPLES_NUM];
extern int Q_VAL[SAMPLES_NUM];
extern double AMP_N[SAMPLES_NUM];
extern double AMP_P[SAMPLES_NUM];
extern double AMP_DIFF[SAMPLES_NUM];



void amp_array_calcu(int *i_array , int *q_array, double *amp);

void amp_difference_array_calcu(double* amp_n,double* amp_p,double* amp_diff);

int get_ampdiff_maximum_index(double *amp_diff, double thres ,int * index);

#endif
