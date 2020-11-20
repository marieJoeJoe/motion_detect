
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


void linear_(){



}

typedef struct {
	int	maxFit;			/* max order of saved fit */
	double	*matrix;		/* fitting matrix */
}	FitData, *FitDataPtr;

static	FitDataPtr	pFitData;


int				/* <- 0=ok, -1=error */
PolynomialFit(opcode, maxfit, order, fit, weights, r2, nPts, x, y)
	int	opcode;		/* -> operation to be performed:
				      0 = load data points (x,y)
				      1 = calculate polynomial fit
				      2 = calculate coefficients 
				      3 = calculate y for given x
				      4 = free the fit matrix */
	int	maxfit;		/* -> maximum order to fit */
	int	order;		/* -> requested order of fit */
	double	*fit;		/* <- array of polynomial coefficients
				      >>MUST<< be dimensioned maxfit+2 */
	double	*weights;	/* -> coordinate weight multipliers
				      (1.0=no weight) */
	double	*r2;		/* <- correlation coefficient of points
				      to the fit (r^2) */
	int	nPts;		/* -> number of points in weights, x, & y */
	double	*x;		/* -> x values to be loaded (opcode=0)
				      or calculated (opcode=3) */
	double	*y;		/* <> y values to be loaded (opcode=0)
				      or calculated (opcode=3) */
{
/*
**       PolynomialFit() performs four different functions,
**	 selected by opcode:
**
**       opcode = 0: load data points (x,y) into fit matrix
**       opcode = 1: perform polynomial fit using loaded fit matrix.
**       opcode = 2: calculate coefficients for polynomial of specified order
**		     order+1 coefficients are calculated and returned in fit.
**       opcode = 3: calculate y for given x
**	 opcode = 4: free the fit matrix.
*/
	int	i, j, k, m, n, r, s, t, u, v, maxOrder, point, i1, j1, m1;
	double	*matrix, s0, r1, t0, val;

    /*  Parameter adjustments */
	--fit;

	if ((pFitData==NULL) || pFitData->maxFit != maxfit)
	    if (opcode==0) {
		if (pFitData!=NULL)
		    (void)free((char *)pFitData);
		pFitData = (FitDataPtr)
			    malloc((unsigned)(sizeof (FitData) +
				   sizeof (double)*(maxfit+2)*(maxfit+2)));
		if (pFitData==NULL)
		    return (-1);
		pFitData->matrix = (double *)(pFitData+1);
		pFitData->maxFit = maxfit;
		for (matrix=pFitData->matrix, i=(maxfit+2)*(maxfit+2);
		    i; --i)
		    *matrix++ = 0.0;
	    } else if (opcode!=3)
		return (-1);
	matrix = pFitData->matrix;

	switch (opcode) {
	case 0:				/* (opcode=0): load data */
	    for (n=0; n<nPts; n++, x++, y++) {
		fit[1]  = 1.0;
		fit[2]  = *x;
		for (i = 2; i <= maxfit; ++i)
		    fit[i + 1] = fit[i] * fit[2];
		i1      = maxfit + 2;
		fit[i1] = *y;
		r       = 0;
		for (i = 1; i <= i1; ++i) {
		    for (j = i; j <= i1; ++j) {
			++r;
			val		= fit[i] * fit[j];
			if (weights!=NULL)
			    val	*= *weights;
			matrix[r] 	+= val;
		    }
		}
		if (weights != NULL)
		    weights++;
	    }
	    break;
	case 1:				/* (opcode=1): fit data */
	    point    = 1;
	    maxOrder = maxfit + 1;
	    for (j = 1; j <= maxOrder; ++j) {
		matrix[point] = sqrt(fabs(matrix[point]));
		i1 = maxOrder - j + 1;
		for (i = 1; i <= i1; ++i)
		    matrix[point + i] /= matrix[point];
		r  = point + i1 + 1;
		s  = r;
		i1 = maxOrder - j;
		for (i = 1; i <= i1; ++i) {
		    ++point;
		    m1 = maxOrder + 2 - j - i;
		    for (m = 1; m <= m1; ++m)
			matrix[r + m - 1] -= matrix[point] *
					     matrix[point + m - 1];
		    r += m1;
		}
		point = s;
	    }

	    t  = ((maxOrder+1)*(maxOrder+2)+1) >> 1;
	    i1 = maxOrder - 1;
	    for (i = 1; i <= i1; ++i) {
		t        -= i + 1;
		matrix[t] = 1.0 / matrix[t];
		j1        = maxOrder - i;
		for (j = 1; j <= j1; ++j) {
		    point = maxOrder + 1 - i - j;
		    r     = ((point*(maxOrder+maxOrder+3-point)+1) >> 1) - i;
		    point = r;
		    r     = point - j;
		    s0    = 0.0;
		    u     = i + j + 1;
		    v     = point;
		    for (k = 1; k <= j; ++k) {
			v  += u - k;
			s0 -= matrix[r + k] * matrix[v];
		    }
		    matrix[point] = s0 / matrix[r];
		}
	    }
	    matrix[1] = 1.0 / matrix[1];
	    break;
	case 2:		/* (opcode=2): calculate coefficients and r**2  */
	    t        = 0;
	    maxOrder = maxfit + 1;
	    i1       = order + 1;
	    for (i = 1; i <= i1; ++i) {
		fit[i] = 0.0;
		j1 = order - i + 2;
		for (j = 1; j <= j1; ++j) {
		    r = ((i+j-1) * (maxOrder+maxOrder+4-i-j) + 1) >> 1;
		    fit[i] += matrix[t + j] * matrix[r];
		}
		t = (i * (maxOrder + maxOrder + (3-i)) + 1) >> 1;
	    }
	    r1 = 0.0;
	    i1 = order + 1;
	    for (i = 2; i <= i1; ++i) {
		j   = (i * (maxOrder + maxOrder + (3-i)) + 1) >> 1;
		r1 += matrix[j] * matrix[j];
	    }
	
	    j  = ((maxOrder+1)*(maxOrder+2)+1) >> 1;
	    t0 = matrix[j] - matrix[maxOrder + 1] * matrix[maxOrder + 1];
	
	    /* correlation coefficient r**2  */
	
	    *r2 = r1 / t0;
	    break;
	case 3:			/* (opcode=3): calculate y for given x */
	    for (n=0; n<nPts; n++, x++, y++) {
		*y = fit[order + 1];
		for (j = order; j > 0; --j)
		    *y = *y * *x + fit[j];
	    }
	    break;
	case 4:
	    if (pFitData)
		(void)free((char *)pFitData);
	    pFitData = NULL;
	}
	return (0);
} /* PolynomialFit() */







