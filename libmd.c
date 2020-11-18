
#include "libmd.h"

int I_VAL0[SAMPLES_NUM];
int Q_VAL0[SAMPLES_NUM];
int I_VAL[SAMPLES_NUM];
int Q_VAL[SAMPLES_NUM];
double AMP_N[SAMPLES_NUM];
double AMP_P[SAMPLES_NUM];
double AMP_DIFF[SAMPLES_NUM];


void amp_array_calcu(int *i_array , int *q_array, double *amp){

  for(int i = 0; i < SAMPLES_NUM; i++){
      amp[i] = sqrt((double)(i_array[i]*i_array[i]+q_array[i]*q_array[i]));
  }


}

void amp_difference_array_calcu(double* curr_round, double* last_round, double* amp_diff){
/*
  static unsigned int turn = 0;

  turn++;

  turn &=1; 

  double *last_round = (0 == turn)?amp_n:amp_p;
  double *curr_round = (0 == turn)?amp_p:amp_n;
*/
  for(int i = 0; i < SAMPLES_NUM; i++){
      amp_diff[i] = curr_round[i] - last_round[i];
  }

}

int get_ampdiff_maximum_index(double *amp_diff, double thres ,int * index){

  *index = 0;

  double maximum = amp_diff[0];

  for(int i = 1; i < SAMPLES_NUM; i++){
      if(amp_diff[i]>maximum){
           *index = i;
           maximum = amp_diff[i];
      }
  }

  return 1;

}








