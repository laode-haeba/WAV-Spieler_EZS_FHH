

/*#######################################################*/
/* Routinen für das Zeichnen des Amplitudengangs         */
/* Dieser Programmteil ist schon fertig.                 */
/*#######################################################*/

#include <math.h>

#include "globals.h"
#include "gui_plotter.h"



typedef struct
{   Rect r;
    int border_x, border_y;
    double alpha, beta;
    int err_window_is_too_small;
} Trafo_t;

/************* private modul function prototypes **************/
static Trafo_t initTrafoStruct(Window *w);
static int frequ_to_pixel_x(float f_Hz, Trafo_t t);
static int H_dB_to_pixel_y(float H_dB, Trafo_t t);
static void plot_f_axis_and_scale(Window *w, Graphics *g, Trafo_t t);
static void plot_H_dB_axis_and_scale(Window *w, Graphics *g, Trafo_t t);
static void plot_H_db_over_frequency(Window *w, Graphics *g, Trafo_t t);
static float frequency_clipping(float f_Hz);
static float H_dB_clipping(float H_dB);





/************* private modul functions ************************/
static Trafo_t initTrafoStruct(Window *w)
{   Trafo_t t;

    t.r = get_window_area(w);
    t.border_x=50;
    t.border_y=35;
    t.alpha = (t.r.width - 2*t.border_x) / log(2e4); /* 1Hz ... 20kHz */
    t.beta  = (t.r.height- 2*t.border_y)/80.0;  /* 40dB...-40dB */
    /* Frequenz -> pixel : px_f=border_x+alpha*ln(f) */
    /* |H| -> Pixel      : py_H=border_y-beta*(H_dB+40) */
    if((t.r.height<150) || (t.r.width<200))
    {   t.err_window_is_too_small = 1;
    }
    else
    {   t.err_window_is_too_small = 0;
    }
    return t;
}
/*-------------------------------------------------------------*/
/* Frequenz -> pixel : px_f=border_x+alpha*ln(f) */
static int frequ_to_pixel_x(float f_Hz, Trafo_t t)
{   int px_f;
    px_f = t.border_x + t.alpha * log(f_Hz);
    return px_f;
}
/*-------------------------------------------------------------*/
/* |H| -> Pixel      : py_H=border_y-beta*(H_dB+40) */
static int H_dB_to_pixel_y(float H_dB, Trafo_t t)
{   int py_H;
    py_H = t.border_y - t.beta * (H_dB - 40);
    return py_H;
}
/*-------------------------------------------------------------*/
static void plot_f_axis_and_scale(Window *w, Graphics *g, Trafo_t t)
{   int i, px_f;
    char strbuf[20];

    /* f-Achse */
    draw_line(g, pt(t.border_x,t.r.height-t.border_y),
                 pt(t.r.width-t.border_x,t.r.height-t.border_y));
    draw_line(g, pt(t.border_x,t.border_y),
                 pt(t.r.width-t.border_x,t.border_y));  /* oberer Abschluss */

    /* zeichne Frequenzlinen */
    set_colour(g, BLACK);
    for(i=2; i<=10; i+=1)
    {   px_f = frequ_to_pixel_x(i, t);
        draw_line(g, pt(px_f,t.border_y), pt(px_f,t.r.height-t.border_y));
    }
    for(i=20; i<=100; i+=10)
    {   px_f = frequ_to_pixel_x(i, t);
        draw_line(g, pt(px_f,t.border_y), pt(px_f,t.r.height-t.border_y));
    }
    for(i=200; i<=1000; i+=100)
    {   px_f = frequ_to_pixel_x(i, t);
        draw_line(g, pt(px_f,t.border_y), pt(px_f,t.r.height-t.border_y));
    }
    for(i=2000; i<=10000; i+=1000)
    {   px_f = frequ_to_pixel_x(i, t);
        draw_line(g, pt(px_f,t.border_y), pt(px_f,t.r.height-t.border_y));
    }
    /* f-Achse beschriften */
    for(i=1;i<=10000; i*=10)
    {   px_f = frequ_to_pixel_x(i, t);
        sprintf(strbuf,"%d",i);
        draw_utf8(g, pt(px_f-(strlen(strbuf)*3),t.r.height-t.border_y),strbuf, strlen(strbuf));
    }
    draw_utf8(g, pt(t.r.width-t.border_x-10,t.r.height-t.border_y+10),"f/Hz", strlen("f/Hz"));
}
/*-------------------------------------------------------------*/
static void plot_H_dB_axis_and_scale(Window *w, Graphics *g, Trafo_t t)
{   int i, py_H;
    char strbuf[20];


    /* horzontale Hilfslinien alle 10 dB */
    for(i=40; i>-40; i-=10)
    {   py_H = H_dB_to_pixel_y(i, t);
        draw_line(g, pt(t.border_x,py_H), pt(t.r.width-t.border_x,py_H));
    }
    /* H-Achse */
    draw_line(g, pt(t.border_x,t.border_y),
                 pt(t.border_x,t.r.height-t.border_y));
    draw_line(g, pt(t.r.width-t.border_x, t.border_y),
                 pt(t.r.width-t.border_x, t.r.height-t.border_y));
    draw_utf8(g, pt(t.border_x-30,t.border_y-30), "|H|/dB", strlen("|H|/dB"));
    for(i=40; i>=-40; i-=10)
    {   py_H = H_dB_to_pixel_y(i, t);
        sprintf(strbuf,"%3d",i);
        draw_utf8(g, pt(t.border_x-30,py_H-10), strbuf, strlen(strbuf));
    }
}
/*-------------------------------------------------------------*/
static void plot_H_db_over_frequency(Window *w, Graphics *g, Trafo_t t)
{
      int i, x_old, y_H_old;
      int px_f, py_H;
      double H_dB, f_Hz;

      /* Graphen zeichnen */
      PTL_SemWait(&plotSema);

        /* Startpunkt für Linie */
        f_Hz  = frequency_clipping(plot_data.f_Hz[0]);
        x_old = frequ_to_pixel_x(f_Hz, t);
        H_dB  = H_dB_clipping(plot_data.H_dB[0]);
        y_H_old = H_dB_to_pixel_y(H_dB, t);

        for(i=1; i<N_PLOT_POINTS; i++)
        {   f_Hz  = frequency_clipping(plot_data.f_Hz[i]);
            px_f  = frequ_to_pixel_x(f_Hz, t);
            H_dB  = H_dB_clipping(plot_data.H_dB[i]);
            py_H  = H_dB_to_pixel_y(H_dB, t);
            draw_line(g, pt(x_old,y_H_old), pt(px_f,py_H));
            /* alte Werte merken */
            x_old     = px_f;
            y_H_old   = py_H;
        }

      PTL_SemSignal(&plotSema);

}

/*-------------------------------------------------------------*/
static float frequency_clipping(float f_Hz)
{   if(f_Hz<1) return 1.0;
    if(f_Hz>20000) return 20000.0;
    return f_Hz;
}
/*-------------------------------------------------------------*/
static float H_dB_clipping(float H_dB)
{   if(H_dB<-40) return -40.0;
    if(H_dB>40)  return 40.0;
    return H_dB;
}
/*-------------------------------------------------------------*/
/*-------------------------------------------------------------*/

/************* exproted modul functions ************************/

/*-----------------------------*/
void redraw_plot_win(Window *w, Graphics *g)
{   Trafo_t t;

    t = initTrafoStruct(w);
    if(t.err_window_is_too_small)
    {   puts("Window too small to plot...");
        return;
    }
    set_colour(g, BLACK);
    set_line_width(g, 1);
    plot_f_axis_and_scale(w, g, t);
    plot_H_dB_axis_and_scale(w, g, t);

    set_colour(g, BLUE);
    set_line_width(g, 3);
    plot_H_db_over_frequency(w, g, t);

    puts("redraw_plot_win");

}
/*-----------------------------*/



