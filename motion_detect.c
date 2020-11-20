#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "libmd.h"

int main(int argc, char* argv[])
{
  int i = 0;
  for(; i<3;++i){
    printf("****\n"+3-i);
  }

  for(; i>=0;--i){
    printf("****\n"+3-i);
  }

  for(i = 0;i<SAMPLES_NUM;i++){
    I_VAL0[i] = (rand()%500);
    Q_VAL0[i] = (rand()%500);
    I_VAL[i] = (rand()%500 + 300);
    Q_VAL[i] = (rand()%500 + 300);
    //printf("%02d %03d %03d %03d %03d\n",i,I_VAL0[i],Q_VAL0[i],I_VAL[i],Q_VAL[i]);
  }

  amp_array_calcu(I_VAL0, Q_VAL0, AMP_N);
  amp_array_calcu(I_VAL, Q_VAL, AMP_P);


  amp_difference_array_calcu(AMP_P, AMP_N, AMP_DIFF);

  for(i = 0 ;i<SAMPLES_NUM;i++){
    mdetect_print("%02d %3.3f %3.3f %3.3f\n", i, AMP_N[i], AMP_P[i], AMP_DIFF[i]);
  }

  int ret = 0, index, start;

  ret = get_ampdiff_maximum_index(AMP_DIFF,AMP_DIFF_THRES,&index,&start);

  if(1 == ret){
    mdetect_print("maximum index %d datasize %d\n",index,start);
  }

  float *motion_data_d = (float*)malloc((index - start - 1)*sizeof(float));
  float *motion_data_s = (float*)malloc((index - start - 1)*sizeof(float));

  for(int j = start;j<=index;j++){
    motion_data_d[j-start] = AMP_DIFF[j];
    motion_data_s[j-start] = AMP_P[j];
    mdetect_print("%f %f\n",motion_data_d[j-start],motion_data_s[j-start]);
  }

  
  if((index - start + 1) <= 3 ) {
    mdetect_print("data point not enough!!\n");
    return -1;
  }

  



  return 0;
}
