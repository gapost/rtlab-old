#pragma once
#include <cmath>

template<class T>
struct linefit
{
	T a, b, siga, sigb, chi2, q, sigdat;

	template<class V>
	linefit(const V& x, const V& y, int ndata) : chi2(0), q(1), sigdat(0)
	/*
	Fit to y = a + bx
	The uncertainties siga
	and sigb are estimated by assuming equal errors for all points, and that a straight line is
	a good fit. q is returned as 1.0, the normalization of chi2 is to unit standard deviation on
	all points, and sigdat is set to the estimated error of each point.
	*/
	{
		int i;
		T ss,sx(0),sy(0),st2(0),t,sxoss;
		b=T(0); //Accumulate sums ...
		for (i=0;i<ndata;i++) {
			sx += x[i]; //...without weights.
			sy += y[i];
		}
		ss=ndata;
		sxoss=sx/ss;
		for (i=0;i<ndata;i++) {
			t=x[i]-sxoss;
			st2 += t*t;
			b += t*y[i];
		}
		b /= st2; //Solve for a, b, siga, and sigb.
		a=(sy-sx*b)/ss;
		siga=sqrt((1.0+sx*sx/(ss*st2))/ss);
		sigb=sqrt(1.0/st2); //Calculate chi2.
		for (i=0;i<ndata;i++) 
		{
			T d = y[i]-a-b*x[i];
			chi2 += d*d;
		}
		if (ndata > 2) sigdat=sqrt(chi2/(ndata-2)); //For unweighted data evaluate typical
		//sig using chi2, and adjust
		//the standard deviations.
		siga *= sigdat;
		sigb *= sigdat;
	}

	template<class V>
	linefit(const V& y, int ndata) : chi2(0), q(1), sigdat(0)
	/*
	Fit to y = a + b * i
	The uncertainties siga
	and sigb are estimated by assuming equal errors for all points, and that a straight line is
	a good fit. q is returned as 1.0, the normalization of chi2 is to unit standard deviation on
	all points, and sigdat is set to the estimated error of each point.
	*/
	{
		int i;
		T ss,sx(0),sy(0),st2(0),t,sxoss;
		b=T(0); //Accumulate sums ...
		for (i=0;i<ndata;i++) {
			sx += i; //...without weights.
			sy += y[i];
		}
		ss=ndata;
		sxoss=sx/ss;
		for (i=0;i<ndata;i++) {
			t=i-sxoss;
			st2 += t*t;
			b += t*y[i];
		}
		b /= st2; //Solve for a, b, siga, and sigb.
		a=(sy-sx*b)/ss;
		siga=sqrt((1.0+sx*sx/(ss*st2))/ss);
		sigb=sqrt(1.0/st2); //Calculate chi2.
		for (i=0;i<ndata;i++) 
		{
			T d = y[i]-a-b*i;
			chi2 += d*d;
		}
		if (ndata > 2) sigdat=sqrt(chi2/(ndata-2)); //For unweighted data evaluate typical
		//sig using chi2, and adjust
		//the standard deviations.
		siga *= sigdat;
		sigb *= sigdat;
	}
};
