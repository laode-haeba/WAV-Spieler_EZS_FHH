#ifndef _dig_filter_h_
#define _dig_filter_h_


typedef struct
{   float a1, a2, b0, b1, b2;
} IIR_2_coeff_t;

void print_IIR_2_coeff(IIR_2_coeff_t p);
IIR_2_coeff_t compute_TP_Filter_Parameters(double fu_Hz, double fa_Hz);
IIR_2_coeff_t compute_BP_Filter_Parameters(double f0_Hz, double Q, double fa_Hz);
IIR_2_coeff_t compute_HP_Filter_Parameters(double fo_Hz, double fa_Hz);

float TP_filter_left (float x, IIR_2_coeff_t p);
float TP_filter_right(float x, IIR_2_coeff_t p);

float BP_filter_left (float x, IIR_2_coeff_t p);
float BP_filter_right(float x, IIR_2_coeff_t p);

float HP_filter_left (float x, IIR_2_coeff_t p);
float HP_filter_right(float x, IIR_2_coeff_t p);


float EQ_filter_left(float x,
                     IIR_2_coeff_t p_TP,
                     IIR_2_coeff_t p_BP,
                     IIR_2_coeff_t p_HP,
                     float A_TP,
                     float A_BP,
                     float A_HP,
                     float B);

float EQ_filter_right(float x,
                     IIR_2_coeff_t p_TP,
                     IIR_2_coeff_t p_BP,
                     IIR_2_coeff_t p_HP,
                     float A_TP,
                     float A_BP,
                     float A_HP,
                     float B);


float H_ges_dB(IIR_2_coeff_t p_TP,
                     IIR_2_coeff_t p_BP,
                     IIR_2_coeff_t p_HP,
                     float A_TP,
                     float A_BP,
                     float A_HP,
                     float B,
                     float f_Hz,
                     float fa_Hz);


#endif
