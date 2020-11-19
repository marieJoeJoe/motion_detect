
#include "libmd.h"



int I_VAL0[SAMPLES_NUM];
int Q_VAL0[SAMPLES_NUM];
int I_VAL[SAMPLES_NUM];
int Q_VAL[SAMPLES_NUM];
float AMP_N[SAMPLES_NUM];
float AMP_P[SAMPLES_NUM];
float AMP_DIFF[SAMPLES_NUM];


void amp_array_calcu(int *i_array , int *q_array, float *amp){

  for(int i = 0; i < SAMPLES_NUM; i++){
      amp[i] = sqrt((float)(i_array[i]*i_array[i]+q_array[i]*q_array[i]));
  }


}

void amp_difference_array_calcu(float* curr_round, float* last_round, float* amp_diff){
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

int get_ampdiff_maximum_index(float *amp_diff, float thres ,int * index, int * start){


  int data_start_index = 0;
  float maximum = amp_diff[0];
  int i = 1;

  *index = 0;
  *start = 0;

  for(; i < SAMPLES_NUM; i++){
      if(amp_diff[i]>maximum){
           mdetect_print("%f\n",amp_diff[i]);
           *index = i;
           maximum = amp_diff[i];
      }
  }

  data_start_index = *index;

  while((data_start_index > 0)&&(amp_diff[data_start_index] > thres)){
    data_start_index--;
  }

  *start = data_start_index;

  return (i == SAMPLES_NUM);

}








