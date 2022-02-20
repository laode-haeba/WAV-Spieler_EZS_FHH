/* ------------------------dig_filter.c--------------------------------
   Authoren: Irvan Fikardi (1484213), Sinan Kocak (1502451)
   -------------------------------------------------------------------- */

#include <stdio.h>
#include <math.h>
#include "dig_filter.h"
#include "cplx.h"

/****************************************************************/
/* TODO: Filter implementieren:                                 */
/*   Differenzengleichungen fuer TP, BP, HP je links und rechts  */
/*   Filterkoeffizienten berechen                               */
/*   Frequenzgang und Amplitudengang berechnen                  */
/****************************************************************/

IIR_2_coeff_t compute_TP_Filter_Parameters(double fu_Hz, double fa_Hz){
    //Referenz: Uebungsskript WAV_Player Seite 30
    IIR_2_coeff_t param_TP;
    float omega = 2.0 * M_PI * fu_Hz;
    float k = omega * (1/tan((M_PI * fu_Hz/fa_Hz)));
    float d = omega + k;

    param_TP.b0 = ((omega / d) * (omega / d)); // "^2" geht nicht
    param_TP.b1 = 2.0 * param_TP.b0;
    param_TP.b2 = param_TP.b0;
    param_TP.a1 = (2*(omega-k))/d;
    param_TP.a2 = (param_TP.a1 * param_TP.a1)/4;

    return param_TP;
}

IIR_2_coeff_t compute_BP_Filter_Parameters(double f0_Hz, double Q, double fa_Hz){
    //Referenz: Uebungsskript WAV_Player Seite 34
    IIR_2_coeff_t param_BP;
    float omega = 2.0 * M_PI * f0_Hz;
    float k = omega * (1/tan((M_PI * f0_Hz/fa_Hz)));
    float d = (omega * omega * Q + omega * k + k * k * Q);// "^2" geht nicht

    param_BP.b0 = ((omega * k) / d);
    param_BP.b1 = 0.0;
    param_BP.b2 = -1.0 * param_BP.b0;
    param_BP.a1 = 2.0*Q*((omega * omega)-(k*k))/d;
    param_BP.a2 = ((omega*omega*Q)-(omega*k)+(k*k*Q))/d;

    return param_BP;
}

IIR_2_coeff_t compute_HP_Filter_Parameters(double fo_Hz, double fa_Hz){
    //Referenz: Uebungsskript WAV_Player Seite 38
    IIR_2_coeff_t param_HP;
    float omega = 2.0 * M_PI * fo_Hz;
    float k = omega * (1/tan((M_PI * fo_Hz/fa_Hz)));
    float d = omega + k;// "^2" geht nicht

    param_HP.b0 = (k/d) * (k/d);
    param_HP.b1 = (-2.0) * param_HP.b0;
    param_HP.b2 = param_HP.b0;
    param_HP.a1 = 2.0*((omega-k)/d);
    param_HP.a2 = (param_HP.a1 * param_HP.a1 / 4);

    return param_HP;
}


//Tiefpass Filter--------------------------------------------------
float TP_filter_left(float x, IIR_2_coeff_t p){
    float ausgangswert;
    static float x1=0, x2=0, y1=0, y2=0;

    //DGL Funktion
    //b0, b1, b2, a1, a2 sind nach dem Typ des Filters abhaengig
    ausgangswert = p.b0*x + p.b1*x1 + p.b2*x2 - p.a1*y1 - p.a2*y2;
    x2 = x1;
    x1 = x;
    y2 = y1;
    y1 = ausgangswert;
    return ausgangswert;
}

float TP_filter_right(float x, IIR_2_coeff_t p){
    float ausgangswert;
    static float x1=0, x2=0, y1=0, y2=0;

    //DGL Funktion
    //b0, b1, b2, a1, a2 sind nach dem Typ des Filters abhaengig
    ausgangswert = p.b0*x + p.b1*x1 + p.b2*x2 - p.a1*y1 - p.a2*y2;
    x2 = x1;
    x1 = x;
    y2 = y1;
    y1 = ausgangswert;
    return ausgangswert;
}

//Bandpass-Filter ----------------------------------------------------
float BP_filter_left(float x, IIR_2_coeff_t p){
    float ausgangswert;
    static float x1=0, x2=0, y1=0, y2=0;

    //DGL Funktion
    //b0, b1, b2, a1, a2 sind nach dem Typ des Filters abhaengig
    ausgangswert = p.b0*x + p.b1*x1 + p.b2*x2 - p.a1*y1 - p.a2*y2;
    x2 = x1;
    x1 = x;
    y2 = y1;
    y1 = ausgangswert;
    return ausgangswert;
}

float BP_filter_right(float x, IIR_2_coeff_t p){
    float ausgangswert;
    static float x1=0, x2=0, y1=0, y2=0;

    //DGL Funktion
    //b0, b1, b2, a1, a2 sind nach dem Typ des Filters abhaengig
    ausgangswert = p.b0*x + p.b1*x1 + p.b2*x2 - p.a1*y1 - p.a2*y2;
    x2 = x1;
    x1 = x;
    y2 = y1;
    y1 = ausgangswert;
    return ausgangswert;
}

//Hochpass-Filter -------------------------------------------------

float HP_filter_left(float x, IIR_2_coeff_t p){
    float ausgangswert;
    static float x1=0, x2=0, y1=0, y2=0;

    //DGL Funktion
    //b0, b1, b2, a1, a2 sind nach dem Typ des Filters abhaengig
    ausgangswert = p.b0*x + p.b1*x1 + p.b2*x2 - p.a1*y1 - p.a2*y2;
    x2 = x1;
    x1 = x;
    y2 = y1;
    y1 = ausgangswert;
    return ausgangswert;
}

float HP_filter_right(float x, IIR_2_coeff_t p){
    float ausgangswert;
    static float x1=0, x2=0, y1=0, y2=0;

    //DGL Funktion
    //b0, b1, b2, a1, a2 sind nach dem Typ des Filters abhaengig
    ausgangswert = p.b0*x + p.b1*x1 + p.b2*x2 - p.a1*y1 - p.a2*y2;
    x2 = x1;
    x1 = x;
    y2 = y1;
    y1 = ausgangswert;
    return ausgangswert;
}


float EQ_filter_left(float x,IIR_2_coeff_t p_TP,IIR_2_coeff_t p_BP,IIR_2_coeff_t p_HP,float A_TP,float A_BP,float A_HP,float B){
    //REFERENZ: Siehe Regelungstechnisches Diagramm im Uebungsskript Seite 42
    float st16_left = 0.0;

    st16_left = B * (x+(A_TP * TP_filter_left(x, p_TP)) +
                       (A_BP * BP_filter_left(x, p_BP)) +
                       (A_HP * HP_filter_left(x, p_HP)));


    return st16_left;
}

float EQ_filter_right(float x,IIR_2_coeff_t p_TP,IIR_2_coeff_t p_BP,IIR_2_coeff_t p_HP,float A_TP,float A_BP,float A_HP,float B){
    //REFERENZ: Siehe Regelungstechnisches Diagramm im Uebungsskript Seite 42
    float st16_right = 0.0;

    st16_right = B *(x+(A_TP * TP_filter_right(x, p_TP)) +
                       (A_BP * BP_filter_right(x, p_BP)) +
                       (A_HP * HP_filter_right(x, p_HP)));

    return st16_right;
}

//Einzelne Uebertragungsfunktion----------------------------------

//Ref: Seite 48; Uebung: digitales Filter
cplx H_z(IIR_2_coeff_t p, cplx z){
    cplx ausgabe, b0,b1,b2,a1,a2, eins, z1, z2, zaehler, nenner;

    eins = make_cplx(1,0);

    b0 = make_cplx(p.b0,0);
    b1 = make_cplx(p.b1,0);
    b2 = make_cplx(p.b2,0);
    a1 = make_cplx(p.a1,0);
    a2 = make_cplx(p.a2,0);

    z1 = c_div(eins, z);
    z2 = c_div(eins, (c_mult(z,z)));

    zaehler = c_add(b0, c_add(c_mult(b1,z1), c_mult(b2,z2)));
    nenner = c_add(eins, c_add(c_mult(a1,z1), c_mult(a2,z2)));

    ausgabe = c_div(zaehler,nenner);
    return ausgabe;
}

//Gesamte Uebertragungsfunktionen
float H_ges_dB(IIR_2_coeff_t p_TP,IIR_2_coeff_t p_BP,IIR_2_coeff_t p_HP,float A_TP,float A_BP,float A_HP,float B,float f_Hz,float fa_Hz){
    cplx H_gesamt;
    float H_db;

    //z ist komplexer Frequenzvariabler aus dem Skript Uebung Digitales Filter Kapitel 3
    cplx z = c_exp(make_cplx(0, (2.0*M_PI*f_Hz)/fa_Hz));

    cplx H_TP = H_z(p_TP, z);
    cplx H_BP = H_z(p_BP, z);
    cplx H_HP = H_z(p_HP, z);

    cplx AH_TP = c_mult(make_cplx(A_TP, 0), H_TP);
    cplx AH_BP = c_mult(make_cplx(A_BP, 0), H_BP);
    cplx AH_HP = c_mult(make_cplx(A_HP, 0), H_HP);

    //Referenz: Uebungsskript WAV-Player Seite 47
    H_gesamt = c_mult(make_cplx(B, 0), c_add(c_add(AH_TP, AH_BP), c_add(AH_HP, make_cplx(1, 0))));

    //als Dezibel wieder darstellen
    H_db = 20 * (log10(betrag(H_gesamt)));
    return H_db;
}
