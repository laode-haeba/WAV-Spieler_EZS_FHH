/***************************************
EZS, AP7
Datei: cplx.h
Gruppe:     -
    E. Forgber      MatrNr: -

****************************************/

#ifndef _cplx_h_
#define _cplx_h_

typedef struct {
double r;
double i;
} cplx;





double real(cplx z);
double imag(cplx z);
double betrag(cplx z);
double phase(cplx z);  /* 0.. 2 * M_PI */

cplx make_cplx(double r, double i);

void c_print_kart(cplx z);
void c_print_polar(cplx z);
void c_print_polar_grad(cplx z);

cplx c_input_kart(void);
cplx c_input_polar(void);

cplx c_add(cplx z1, cplx z2);
cplx c_sub(cplx z1, cplx z2);
cplx c_mult(cplx z1, cplx z2);
cplx c_div(cplx z1, cplx z2);
cplx c_exp(cplx z);
cplx c_sqrt(cplx z);



#endif
