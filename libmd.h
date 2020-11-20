#ifndef __LIBMD_H__
#define __LIBMD_H__
#include <math.h>
#include <stdio.h>

#ifdef MD_DEBUG
#define mdetect_print(x,arg...) printf("[%s]"x,__FUNCTION__,##arg)
#define mdetect_err(x,arg...) printf("[%s]"x,__FUNCTION__,##arg)
#else
#define mdetect_print(x,arg...)
#define mdetect_err(x,arg...)
#endif


#define SAMPLES_NUM 242
//242

#define AMP_DIFF_THRES 176.14159

#define INTERVAL 100 //ms

#define INTERVAL


extern int I_VAL0[SAMPLES_NUM];
extern int Q_VAL0[SAMPLES_NUM];
extern int I_VAL[SAMPLES_NUM];
extern int Q_VAL[SAMPLES_NUM];
extern float AMP_N[SAMPLES_NUM];
extern float AMP_P[SAMPLES_NUM];
extern float AMP_DIFF[SAMPLES_NUM];



void amp_array_calcu(int *i_array , int *q_array, float *amp);

void amp_difference_array_calcu(float* amp_n, float* amp_p, float* amp_diff);

int get_ampdiff_maximum_index(float *amp_diff, float thres ,int * max_index, int * start_index);

#endif
