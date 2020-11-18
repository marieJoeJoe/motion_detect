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
    I_VAL0[i] = rand()%1000;
    Q_VAL0[i] = rand()%1000;
    I_VAL[i] = rand()%1000;
    Q_VAL[i] = rand()%1000;
    //printf("%02d %03d %03d %03d %03d\n",i,I_VAL0[i],Q_VAL0[i],I_VAL[i],Q_VAL[i]);
  }

  amp_array_calcu(I_VAL0, Q_VAL0, AMP_N);
  amp_array_calcu(I_VAL, Q_VAL, AMP_P);


  amp_difference_array_calcu(AMP_N,AMP_P,AMP_DIFF);

  for(i = 0 ;i<SAMPLES_NUM;i++){
    //printf("%02d %3.3f %3.3f %3.3f\n", i, AMP_N[i], AMP_P[i], AMP_DIFF[i]);
  }

  int ret = 0, index;

  ret = get_ampdiff_maximum_index(AMP_DIFF,AMP_DIFF_THRES,&index);

  if(1 == ret){
    printf("maximum index %d\n",index);
  }

  return 0;
}
