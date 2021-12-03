/***************************************
EZS, AP7
Datei: cplx.c
Gruppe:     -
    E. Forgber      MatrNr: -

****************************************/

#include <stdio.h>
#include <math.h>
#include "cplx.h"



double real(cplx z){
    return z.r;
}
/*-----------------------*/
double imag(cplx z){
    return z.i;
}
/*-----------------------*/

double betrag(cplx z){
    return sqrt(z.r*z.r + z.i*z.i);
}
/*-----------------------*/

double phase(cplx z){
    double phi;
    /* die pathologischen Fälle abfangen: */
    if (z.r == 0.0)
    {  if(z.i == 0.0)
       {  phi=0.0;
       }
       else
       { if(z.i > 0) phi = M_PI / 2.0;
         else        phi = -M_PI / 2.0;
       }
    }
    else
    {  phi = atan2(z.i, z.r); /* -pi...pi */
    }
    /* transformiere von -pi..pi nach 0..2pi: */
    if(phi<0)
    {  phi = 2*M_PI + phi;
    }
    return(phi);
}
/*-----------------------*/

cplx make_cplx(double r, double i)
{   cplx z;

    z.r = r;
    z.i = i;
    return z;
}

/*-----------------------*/

void c_print_kart(cplx z)
{
    printf("(%.4lf|%.4lf) ",z.r,z.i);
}
/*-----------------------*/
void c_print_polar(cplx z)
{
    printf("{modulus:%.4lf angle:%.4lf rad} ",
           betrag(z), phase(z));
}
/*-----------------------*/
void c_print_polar_grad(cplx z)
{
    printf("{modulus:%.4lf angle:%.2lf Grad} ",
           betrag(z), phase(z)/M_PI*180.0);
}
/*-----------------------*/
cplx c_input_kart(void)
{   cplx z;

    printf("Realteil: ");
    scanf("%lf", &z.r);
    printf("Imaginaerteil: ");
    scanf("%lf", &z.i);
    return z;
}
/*-----------------------*/
cplx c_input_polar(void)
{   cplx z;
    double r, phi_grad, phi_rad;

    printf("Betrag: ");
    scanf("%lf", &r);
    printf("Winkel in Grad: ");
    scanf("%lf", &phi_grad);
    phi_rad = phi_grad / 180.0 * M_PI;
    z.r = r * cos(phi_rad);
    z.i = r * sin(phi_rad);
    return z;
}

/*-----------------------*/

cplx c_add(cplx z1, cplx z2){
  cplx z;
  z.r = z1.r + z2.r;
  z.i = z1.i + z2.i;
  return z;
 }
/*-----------------------*/

cplx c_sub(cplx z1, cplx z2){
  cplx z;
  z.r = z1.r - z2.r;
  z.i = z1.i - z2.i;
  return z;
 }
/*-----------------------*/

cplx c_mult(cplx z1, cplx z2){
  cplx z;
  z.r = z1.r*z2.r - z1.i*z2.i;
  z.i = z1.r*z2.i + z2.r*z1.i;
  return z;

}
/*-----------------------*/
cplx c_div(cplx z1, cplx z2){
  cplx z;
  double a;
  a = betrag(z2);
  a = a*a;
  z.r = (z1.r*z2.r + z1.i*z2.i) / a;
  z.i = (z2.r*z1.i - z1.r*z2.i) / a;
  return z;
}
/*-----------------------*/
cplx c_exp(cplx z){
  cplx z1;
  z1.r = exp(z.r) * cos(z.i);
  z1.i = exp(z.r) * sin(z.i);
  return z1;
}
/*-----------------------*/

cplx c_sqrt(cplx z){
  cplx z1;
  z1.r = sqrt(betrag(z)) * cos(phase(z)/2.0);
  z1.i = sqrt(betrag(z)) * sin(phase(z)/2.0);
  return(z1);
}



