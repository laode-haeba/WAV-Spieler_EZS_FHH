/*Irvan Rizqullah Fikardi (MN: 1484213) und Sinan Kocak (MN: 1502451)*/

#include <math.h>
#include <windows.h>

#include "ptl_lib.h"
#include "dig_filter.h"
#include "echo.h"
#include "globals.h"

#include "gui.h"
#include "gui_plotter.h"

#include "app_3_60_bcc.h"
#include "graphapp_3_60_bcc.h"

static App *app;
static Window *w, *w_plot;
static Timer *T;

/* Controls für die GUI */
Control *cbParametricEQ, *cbHideBodeDisplay;
Control *cbEcho;

Control slider_volume;

/* Prototypen für die Callback-Fktn */
void button_load(Control *c);
void use_cbParametricEQ(Control *b);

void hide_plot_win_CB(Control *c);


void use_cbEcho(Control *b);


void redraw_main_win(Window *w, Graphics *g);
void close_plot_win(Window *w);
void close_win_and_shutdown(Window *w);
void Timer_CB(Timer *t);

void place_gui_elements_file(void);
void place_gui_elements_EQ(void);
void place_gui_elements_Echo(void);
void init_gui_elements(void);


/*--------------------------------------------------*/

void gui(int argc, char *argv[])
{
  app = new_app(argc, argv);
  w = new_window(app,rect(50,50,640,660),
                "Wave-Player", STANDARD_WINDOW);
  w_plot = new_window(app,rect(400,200,N_X_PLOT_WIN,N_Y_PLOT_WIN),
                "EQ-Amplitudengang", (TITLEBAR|MINIMIZE));

  place_gui_elements_file();
  place_gui_elements_EQ();
  place_gui_elements_Echo();

  on_window_redraw(w, redraw_main_win);
  on_window_redraw(w_plot, redraw_plot_win);

  init_gui_elements();

  show_window(w);
  show_window(w_plot);

  T = new_timer(app, Timer_CB, 1000);
  on_window_close (w_plot, close_plot_win);
  on_window_close (w, close_win_and_shutdown);

  main_loop(app);
  return;
}


/*-----------------------------*/

void use_cbParametricEQ(Control *b)
{   int flag_use_EQ=0;
    if(is_checked(b))
    {   printf(" EQ  is used\n");
        flag_use_EQ = 1;
    }
    else
    {   printf("EQ is not used\n");
        flag_use_EQ = 0;
    }
    PTL_SemWait(&sRamSema);
    sRam.flag_EQ_is_active = flag_use_EQ;
    PTL_SemSignal(&sRamSema);
}

/*-----------------------------*/
/*-----------------------------*/

void button_load(Control *c){
    char file_name[MAX_PATH] = {0};
    char directory_path[MAX_PATH] = {0};

    // WIN32-API für FileDialog --Fikardi
    OPENFILENAME ofn; //Ref: https://docs.microsoft.com/en-us/windows/win32/api/commdlg/ns-commdlg-openfilenamea

    GetCurrentDirectory(MAX_PATH, directory_path);
    printf("\nPATH %s", directory_path);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = file_name;
    ofn.hwndOwner = NULL;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = TEXT("WAV (*.*)\0*.*\0");
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrFileTitle = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if(GetOpenFileName(&ofn)!= TRUE){
        puts("File cannot be found.");
    }

    printf("FILE: %s", ofn.lpstrFile);

    PTL_SemWait(&sRamSema);
    sRam.cmd_play = 0;
    strcpy(sRam.Dateiname, ofn.lpstrFile);
    set_control_text(wavDatei, sRam.Dateiname);
    PTL_SemSignal(&sRamSema);
}

/*-----------------------------*/

void button_play(Control *c){
    PTL_SemWait(&sRamSema);
    strcpy(sRam.Dateiname, get_control_text(wavDatei));
    printf("\nPlaying file with name: %s", sRam.Dateiname);
    sRam.cmd_play = 1;

    PTL_SemSignal(&sRamSema);
}

/*-----------------------------*/

void button_stop(Control *c){
    printf("Stopp\n");
    PTL_SemWait(&sRamSema);
    sRam.cmd_play = 0;
    printf("\nWAV in memory: %s", sRam.Dateiname);
    PTL_SemSignal(&sRamSema);
}

/*-----------------------------*/

void slider_delay(Control *c){
    PTL_SemWait(&sRamSema);
    sRam.Echo.delay_n0 = (F_S-1) * (1.0/25.0) * get_control_value(c);
    PTL_SemSignal(&sRamSema);
}

/*-----------------------------*/

void slider_gain(Control *c){
    PTL_SemWait(&sRamSema);
    sRam.Echo.gain = (1.0/25.0) * get_control_value(c);
    PTL_SemSignal(&sRamSema);
}

/*-----------------------------*/

void slider_feedback(Control *c){
    PTL_SemWait(&sRamSema);
    sRam.Echo.feedback = (1.0/25.0) * get_control_value(c);
    PTL_SemSignal(&sRamSema);
}

/*-----------------------------*/

void slide_volume(Control *c){
    int vol_zahl = get_control_value(c);
    vol_zahl = 15 - vol_zahl;
    printf("Lautstaerke: %i\n", vol_zahl); //fuer debug

    PTL_SemWait(&sRamSema);
    sRam.volume = vol_zahl;
    PTL_SemSignal(&sRamSema);
}

/*-----------------------------
Berechnung:
Sei
y = Ahp, Abp, Atp, Frequenz
x = Anzahl der Schritte vom Slider
a = Konstante

y = m*x + a

Fuer Ahp, Abp, Atp mit -0.1<y<9 und x = 20 Schritte vom Slider
m = 9 - (-0.9) / 20 - 0 = 0.495
a = -0.9
y = (0.495)*x-0.9

Fuer f_hz mit 1<f_hz<20000 und x = 20 Schritte vom Slider
m = 20000 - 10 / 20 - 0 = 999.5
a = 10
y = 999.5*x+10

---------------------Fikardi
*/

/*-----------------------------*/

void slider_tief_freq(Control *c){
    PTL_SemWait(&sRamSema);
    sRam.TP= compute_TP_Filter_Parameters(10+999.5*get_control_value(c),F_S); //Fuer 20 Schritte
    //sRam.TP= compute_TP_Filter_Parameters(10+199.9*get_control_value(c),F_S); //Fuer 100 Schritte
    PTL_SemSignal(&sRamSema);
}
/*-----------------------------*/

void slider_tief_gain(Control *c){
    int wert = get_control_value(c);
    PTL_SemWait(&sRamSema);
    sRam.A_TP= (0.495)*wert-0.9; //+-20dB Verstaerkungsbereich
    //sRam.A_TP= (0.075)*wert-0.5; //+-6dB Verstaerkungsbereich
    PTL_SemSignal(&sRamSema);
}

/*-----------------------------*/

void slider_band_freq(Control *c){
    PTL_SemWait(&sRamSema);
    sRam.BP = compute_BP_Filter_Parameters(10+999.5*get_control_value(c),sRam.Q,F_S); //Fuer 20 Schritte
    //sRam.BP = compute_BP_Filter_Parameters(10+199.9*get_control_value(c),sRam.Q,F_S); //Fuer 100 Schritte
    PTL_SemSignal(&sRamSema);
}

/*-----------------------------*/

void slider_band_q(Control *c){
    PTL_SemWait(&sRamSema);
    sRam.Q = (get_control_value(c)/2); //Q Eingabevariable sind zu groß, muss geteilt werden.
    sRam.BP = compute_BP_Filter_Parameters(10+999.5*get_control_value(slider_band_freq),sRam.Q,F_S);
    PTL_SemSignal(&sRamSema);
}

/*-----------------------------*/

void slider_band_gain(Control *c){
    int wert = get_control_value(c);
    PTL_SemWait(&sRamSema);
    sRam.A_BP= (0.495)*wert-0.9; //+-20dB Verstaerkungsbereich
    //sRam.A_BP= (0.075)*wert-0.5; //+-6dB Verstaerkungsbereich
    PTL_SemSignal(&sRamSema);
}
/*-----------------------------*/

void slider_hoch_freq(Control *c){
    PTL_SemWait(&sRamSema);
    sRam.HP= compute_HP_Filter_Parameters(10+999.5*get_control_value(c),F_S); // fuer 20 Schritte
    //sRam.TP= compute_HP_Filter_Parameters(10+199.9*get_control_value(c),F_S); //fuer 100 Schritte
    PTL_SemSignal(&sRamSema);
}
/*-----------------------------*/

void slider_hoch_gain(Control *c){
    int wert = get_control_value(c);
    PTL_SemWait(&sRamSema);
    sRam.A_HP= (0.495)*wert-0.9; //+-20dB Verstaerkungsbereich
    //sRam.A_HP= (0.075)*wert-0.5; //+-6dB Verstaerkungsbereich
    PTL_SemSignal(&sRamSema);
}

/*-----------------------------*/

void hide_plot_win_CB(Control *c)
{
    if(is_checked(c))
    {
        hide_window(w_plot);
    }
    else
    {
        show_window(w_plot);
    }
}

/*-----------------------------*/

void use_cbEcho(Control *b)
{   int flag_use_Echo=0;

    if(is_checked(b))
    {   printf("Echo is used\n");
        flag_use_Echo=1;
    }
    else
    {   printf("Echo is not used\n");
        flag_use_Echo=0;
    }
    PTL_SemWait(&sRamSema);
    sRam.flag_Echo_is_active = flag_use_Echo;
    PTL_SemSignal(&sRamSema);
}

/*-----------------------------*/

/*-----------------------------*/
void redraw_main_win(Window *w, Graphics *g)
{   Rect r;
    Point p_left,p_right,p_up,p_down,p_middle;
    int dx=32, dy=8;

    r = rect(10,10,620,130);
    draw_rect(g, r);            // Rahmen um File-Elemente
    r.y += (10 + r.height);
    r.height = 350;
    draw_rect(g, r);            // Rahmen um EQ-Elemente
    r.y += (10 + r.height);
    r.height = 140;
    draw_rect(g, r);            // Rahmen um Echo-Elemente

    // Kreuz bei Nullstellung der Gain-SLider des EQ
    // Low
    p_middle = pt(305,280);
    p_left   = pt(p_middle.x-dx/2,p_middle.y);
    p_right  = pt(p_middle.x+dx/2,p_middle.y);
    p_up     = pt(p_middle.x,p_middle.y-dy/2);
    p_down   = pt(p_middle.x,p_middle.y+dy/2);
    draw_line(g,p_left,p_right);
    draw_line(g,p_up,p_down);

    // Mid
    p_middle = pt(305,400);
    p_left   = pt(p_middle.x-dx/2,p_middle.y);
    p_right  = pt(p_middle.x+dx/2,p_middle.y);
    p_up     = pt(p_middle.x,p_middle.y-dy/2);
    p_down   = pt(p_middle.x,p_middle.y+dy/2);
    draw_line(g,p_left,p_right);
    draw_line(g,p_up,p_down);

    // High
    p_middle = pt(305,490);
    p_left   = pt(p_middle.x-dx/2,p_middle.y);
    p_right  = pt(p_middle.x+dx/2,p_middle.y);
    p_up     = pt(p_middle.x,p_middle.y-dy/2);
    p_down   = pt(p_middle.x,p_middle.y+dy/2);
    draw_line(g,p_left,p_right);
    draw_line(g,p_up,p_down);
}

/*-----------------------------*/
void Timer_CB(Timer *t)
{
    redraw_window(w_plot);
}


/*-----------------------------*/
void close_plot_win(Window *w)
{   hide_window(w);
    check(cbHideBodeDisplay);

}
/*-----------------------------*/
 void close_win_and_shutdown(Window *w)
 {
    /* Thread-Ende kommandieren: */
    PTL_SemWait(&sRamSema);
    sRam.cmd_play = 0;
    sRam.cmd_end  = 1;
    PTL_SemSignal(&sRamSema);
    puts("WAV-Player: main() wartet auf das Ende des Player-Threads...\n");
    PTL_SemWait(&endSema);
    puts("WAV-Player: main() wartet auf das Ende des Plotter-Threads...\n");
    PTL_SemWait(&endSema);
    puts("threads beendet...");
    puts("WAV-Player: main() ist beendet...\n");
    exit(-1);
    //del_timer(T);
    //del_app(app);
    //exit(-1);
 }
/*-----------------------------*/


void place_gui_elements_file(void)
{   Rect r, filename_rect, openbutton_rect, playbutton_rect, stopbutton_rect;

    r = rect(30,20,550,30);
    //new_label(w, r,"hier: load, play, stop...", ALIGN_LEFT);

    /********************************************************/
    /* TODO: hier Ihre GUI-Elemente load, play,stop         */
    /********************************************************/

    filename_rect = rect(r.x + 120, r.y + 6, 350, 30);
    wavDatei = new_field(w, filename_rect, "Click open to choose a .wav file to play");

    openbutton_rect = rect(r.x+=60, r.y+= 6, 50, 30);
    new_button(w,openbutton_rect,"Open",button_load);

    playbutton_rect = rect(r.x + 60, r.y+35, 50,25);
    new_button(w,playbutton_rect,"Play",button_play);

    stopbutton_rect = rect(playbutton_rect.x+playbutton_rect.width+10, playbutton_rect.y, 50, 25);
    new_button(w, stopbutton_rect, "Stop",button_stop);
}
/*-----------------------------*/
void place_gui_elements_EQ(void)
{   Rect r;
    Control *scroll_volume;

    r = rect(30, 160, 250, 20);
    cbParametricEQ = new_check_box(w, r, "use Parametric Filters",
                             use_cbParametricEQ);
    r.x += (10 + r.width);
    cbHideBodeDisplay = new_check_box(w,r, "hide Bode-Plot",
                        hide_plot_win_CB);
    check(cbParametricEQ);
    /********************************************************/
    /* TODO: hier Ihre GUI-Elemente für die Filter erzeugen */
    /********************************************************/
    // Volume Slider
    r = rect((r.x+=(r.width+20)), 160, 100, 20);
    new_label(w, r, "Lautstärke", ALIGN_LEFT);
    r = rect((r.x+14), (r.y += r.height), 30, (r.y * 1.94));
	scroll_volume = new_scroll_bar(w, r, 15, 1, slide_volume);
	change_scroll_bar(scroll_volume, 5, 15, 1); //du hast die Deklaration von Kontrollvariable vergessen--Fikardi


    //----------------Tiefpass-------------------------//
	r = rect(80, 190, 250, 20);

	new_label(w,r,"Tiefpass",ALIGN_LEFT); r.x-=50; r.y+=30;
	new_label(w,r,"Freq.", ALIGN_LEFT); r.y+=30;
	new_label(w,r,"Gain",ALIGN_LEFT); r.y-=30; r.x+=50;

	r.width=450;
    new_scroll_bar(w,r,20,1,slider_tief_freq); r.y+=30; //20 Schritte
    //new_scroll_bar(w,r,100,1,slider_tief_freq); r.y+=30; //100 Schritte
    new_scroll_bar(w,r,20,1,slider_tief_gain);

    //----------------Bandpass-------------------------//

    r = rect(80, 280, 250, 20);

	new_label(w,r,"Bandpass",ALIGN_LEFT); r.x-=50; r.y+=30;
	new_label(w,r,"Freq.", ALIGN_LEFT); r.y+=25;
	new_label(w,r,"Q", ALIGN_LEFT); r.y+=25;
	new_label(w,r,"Gain",ALIGN_LEFT); r.y-=50; r.x+=50;

	r.width=450;
    new_scroll_bar(w,r,20,1,slider_band_freq); r.y+=25; //20 Schritte
    //new_scroll_bar(w,r,100,1,slider_band_freq); r.y+=25; //100 Schritte
    new_scroll_bar(w,r,20,1,slider_band_q); r.y+=25;
    new_scroll_bar(w,r,20,1,slider_band_gain);

    //----------------Hochpass-------------------------//

    r = rect(80, 390, 250, 20);

	new_label(w,r,"Hochpass",ALIGN_LEFT); r.x-=50; r.y+=30;
	new_label(w,r,"Freq.", ALIGN_LEFT); r.y+=30;
	new_label(w,r,"Gain",ALIGN_LEFT); r.y-=30; r.x+=50;

	r.width=450;
    new_scroll_bar(w,r,20,1,slider_hoch_freq); r.y+=30; //20 Schritte
    //new_scroll_bar(w,r,100,1,slider_hoch_freq); r.y+=30; //100 Schritte
    new_scroll_bar(w,r,20,1,slider_hoch_gain);

}
/*-----------------------------*/
void place_gui_elements_Echo(void)
{   Rect r;

    r = rect(30, 520, 450, 20);
    cbEcho = new_check_box(w, r, "use Echo",
                                use_cbEcho);
    /********************************************************/
    /* TODO: hier Ihre GUI-Elemente für Echo erzeugen       */
    /********************************************************/

    //Slider fuer Delay
    r.x=30; r.y+=30; r.width=80;
    new_label(w,r,"Delay",ALIGN_LEFT);
    r.x+=60;r.width=530;
    new_scroll_bar(w, r, 25, 1, slider_delay);

    //Slider fuer Gain
    r.x=30; r.y+=30; r.width=80;
	new_label(w, r, "Gain", ALIGN_LEFT);
	r.x+=60; r.width=530;
	new_scroll_bar(w, r, 25, 1, slider_gain);

    //Slider fuer Feedback
	r.x=30; r.y+=30; r.width=80;
	new_label(w,r,"Feedback",ALIGN_LEFT);
	r.x+=60; r.width=530;
	new_scroll_bar(w, r, 25, 1, slider_feedback);
}
/*-----------------------------*/
void init_gui_elements(void)
{
    uncheck(cbParametricEQ);
    uncheck(cbHideBodeDisplay);


    uncheck(cbEcho);

}
/*-----------------------------*/




