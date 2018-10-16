/***************************************************
 Filename: ti83keypad.h
 
 ***************************************************/

#ifndef ti83keypad_h
#define ti83keypad_h

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <sr595.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/keysymdef.h>
#include <X11/extensions/XTest.h>

// WiringPi Pins, not GPIOs
#define CLOCK_PIN   25
#define DATA_PIN    27
#define LATCH_PIN   26
#define ONKEY_PIN   9

#define SCAN_DELAY      5 // In Milliseconds
#define BOUNCE_DELAY    250 // In MilliSeconds
#define KEYPRESS_DELAY  50 // The length of time a button is held pressed In MilliSeconds

// Mode corresponds to the keyboard layout used as well as the icon displayed
#define MODE_NORMAL 1       // numbers.png
#define MODE_ALPHA_LOWER 2  // lowercase.png
#define MODE_ALPHA_UPPER 3  // uppercase.png
#define MODE_SECOND 4       // 2nd.png
#define MODE_TI83 5         // ti83mode.png

// None of the defined KeySymbols are in the 0x8000 range
#define SPECIAL_ALPHA_UPPER_KEY       0x8000
#define SPECIAL_ALPHA_LOWER_KEY       0x8001
#define SPECIAL_2ND_KEY               0x8002
#define SPECIAL_LOCK_KEY              0x8003
#define SPECIAL_NORMAL_KEY            0x8004
#define SPECIAL_BRIGHT_UP_KEY         0x8005
#define SPECIAL_BRIGHT_DOWN_KEY       0x8006

GtkStatusIcon *tray;
Display *display;

// Key Symbols requiring a Shift (Add as needed)
KeySym shiftSymbols[] = {
    XK_asciitilde,
    XK_asciicircum,
    XK_greater,
    XK_dollar,
    XK_percent,
    XK_quotedbl,
    XK_colon,
    XK_question,
    XK_ampersand,
    XK_braceleft,
    XK_braceright,
    XK_underscore,
    XK_bar,
    XK_A,
    XK_B,
    XK_C,
    XK_D,
    XK_E,
    XK_F,
    XK_G,
    XK_H,
    XK_I,
    XK_J,
    XK_K,
    XK_L,
    XK_M,
    XK_N,
    XK_O,
    XK_P,
    XK_Q,
    XK_R,
    XK_S,
    XK_T,
    XK_U,
    XK_V,
    XK_W,
    XK_X,
    XK_Y,
    XK_Z
};

int colPins[] = {7, 15, 16, 2, 3, 4, 21}; // Columns I, J, K, L, M, N, O

KeySym normalLayout[8][7] = {
    {XK_F11, XK_grave, XK_exclam, XK_at, XK_numbersign, XK_Escape, NoSymbol},  // Row A: Mode, Math, Apps, Prgm, Vars, Clear
    {XK_Delete, SPECIAL_ALPHA_LOWER_KEY, XK_apostrophe, XK_semicolon, NoSymbol, NoSymbol, NoSymbol},      // Row B: Del, Alpha, "X,T,𝚹,n" (GraphVar), Stat
    {SPECIAL_2ND_KEY, XK_less, XK_greater, XK_dollar, XK_percent, XK_asciicircum, NoSymbol},      // Row C: 2nd, X^-1, Sin, Cos, Tan, ^
    {XK_F1, XK_slash, XK_comma, XK_parenleft, XK_parenright, XK_slash, NoSymbol},      // Row D: Y=, X^2, ',', (, ), ÷
    {XK_F2, XK_backslash, XK_7, XK_8, XK_9, XK_KP_Multiply, XK_Up},      // Row E: Window, Log, 7, 8, 9, X (Multiply), Up
    {XK_F3, XK_Tab, XK_4, XK_5, XK_6, XK_KP_Subtract, XK_Right},      // Row F: Zoom, LN, 4, 5, 6, -, Right
    {XK_F4, XK_equal, XK_1, XK_2, XK_3, XK_KP_Add, XK_Left},      // Row G: Trace, Sto->, 1, 2, 3, +, Left
    {XK_F5, NoSymbol, XK_0, XK_period, XK_KP_Subtract, XK_KP_Enter, XK_Down}       // Row H: Graph, Null, 0, ., (-), Enter, Down
};

KeySym alphaUpperLayout[8][7] = {
    {XK_F11, XK_A, XK_B, XK_C, NoSymbol, XK_Escape, NoSymbol},  // Row A: Mode, Math, Apps, Prgm, Vars, Clear
    {XK_BackSpace, SPECIAL_ALPHA_LOWER_KEY, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol},      // Row B: Del, Alpha, "X,T,𝚹,n" (GraphVar), Stat
    {SPECIAL_2ND_KEY, XK_D, XK_E, XK_F, XK_G, XK_H, NoSymbol},      // Row C: 2nd, X^-1, Sin, Cos, Tan, ^
    {XK_F1, XK_I, XK_J, XK_K, XK_L, XK_M, NoSymbol},      // Row D: Y=, X^2, ',', (, ), ÷
    {XK_F2, XK_N, XK_O, XK_P, XK_Q, XK_R, XK_Page_Up},      // Row E: Window, Log, 7, 8, 9, X (Multiply), Up
    {XK_F3, XK_S, XK_T, XK_U, XK_V, XK_W, XK_Right},      // Row F: Zoom, LN, 4, 5, 6, -, Right
    {XK_F4, XK_X, XK_Y, XK_Z, NoSymbol, XK_quotedbl, XK_Left},      // Row G: Trace, Sto->, 1, 2, 3, +, Left
    {XK_F5, NoSymbol, XK_space, XK_colon, XK_question, XK_KP_Enter, XK_Page_Down}       // Row H: Graph, Null, 0, ., (-), Enter, Down
};

KeySym alphaLowerLayout[8][7] = {
    {XK_F11, XK_a, XK_b, XK_c, NoSymbol, XK_Escape, NoSymbol},  // Row A: Mode, Math, Apps, Prgm, Vars, Clear
    {XK_BackSpace, SPECIAL_ALPHA_UPPER_KEY, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol},      // Row B: Del, Alpha, "X,T,𝚹,n" (GraphVar), Stat
    {SPECIAL_2ND_KEY, XK_d, XK_e, XK_f, XK_g, XK_h, NoSymbol},      // Row C: 2nd, X^-1, Sin, Cos, Tan, ^
    {XK_F1, XK_i, XK_j, XK_k, XK_l, XK_m, NoSymbol},      // Row D: Y=, X^2, ',', (, ), ÷
    {XK_F2, XK_n, XK_o, XK_p, XK_q, XK_r, XK_Page_Up},      // Row E: Window, Log, 7, 8, 9, X (Multiply), Up
    {XK_F3, XK_s, XK_t, XK_u, XK_v, XK_w, XK_Right},      // Row F: Zoom, LN, 4, 5, 6, -, Right
    {XK_F4, XK_x, XK_y, XK_z, NoSymbol, XK_quotedbl, XK_Left},      // Row G: Trace, Sto->, 1, 2, 3, +, Left
    {XK_F5, NoSymbol, XK_space, XK_colon, XK_question, XK_KP_Enter, XK_Page_Down}       // Row H: Graph, Null, 0, ., (-), Enter, Down
};

KeySym secondLayout[8][7] = {
    {XK_F11, XK_asciitilde, NoSymbol, NoSymbol, NoSymbol, XK_Escape, NoSymbol},  // Row A: Mode, Math, Apps, Prgm, Vars, Clear
    {XK_Insert, SPECIAL_LOCK_KEY, XK_F12, NoSymbol, NoSymbol, NoSymbol, NoSymbol},      // Row B: Del, Alpha, "X,T,𝚹,n" (GraphVar), Stat
    {SPECIAL_NORMAL_KEY, NoSymbol, NoSymbol, NoSymbol, NoSymbol, XK_ampersand, NoSymbol},      // Row C: 2nd, X^-1, Sin, Cos, Tan, ^
    {XK_F6, NoSymbol, NoSymbol, XK_braceleft, XK_braceright, XK_e, NoSymbol},      // Row D: Y=, X^2, ',', (, ), ÷
    {XK_F7, XK_bar, XK_u, XK_v, XK_w, XK_bracketleft, SPECIAL_BRIGHT_UP_KEY},      // Row E: Window, Log, 7, 8, 9, X (Multiply), Up
    {XK_F8, NoSymbol, XK_Num_Lock, NoSymbol, NoSymbol, XK_bracketright, XK_End},      // Row F: Zoom, LN, 4, 5, 6, -, Right
    {XK_F9, NoSymbol, XK_Print, XK_Scroll_Lock, XK_Pause, NoSymbol, XK_Home},      // Row G: Trace, Sto->, 1, 2, 3, +, Left
    {XK_F10, NoSymbol, NoSymbol, XK_i, XK_underscore, XK_KP_Enter, SPECIAL_BRIGHT_DOWN_KEY}       // Row H: Graph, Null, 0, ., (-), Enter, Down
};

KeySym ti83Layout[8][7] = {
    {XK_F11, XK_F6, XK_F7, XK_F8, XK_F9, XK_Escape, NoSymbol},  // Row A: Mode, Math, Apps, Prgm, Vars, Clear
    {XK_Delete, XK_apostrophe, XK_x, XK_F10, NoSymbol, NoSymbol, NoSymbol},      // Row B: Del, Alpha, "X,T,𝚹,n" (GraphVar), Stat
    {XK_Tab, XK_backslash, XK_s, XK_c, XK_t, XK_asciicircum, NoSymbol},      // Row C: 2nd, X^-1, Sin, Cos, Tan, ^
    {XK_F1, XK_semicolon, XK_comma, XK_parenleft, XK_parenright, XK_slash, NoSymbol},      // Row D: Y=, X^2, ',', (, ), ÷
    {XK_F2, XK_o, XK_7, XK_8, XK_9, XK_KP_Multiply, XK_Up},      // Row E: Window, Log, 7, 8, 9, X (Multiply), Up
    {XK_F3, XK_l, XK_4, XK_5, XK_6, XK_KP_Subtract, XK_Right},      // Row F: Zoom, LN, 4, 5, 6, -, Right
    {XK_F4, XK_equal, XK_1, XK_2, XK_3, XK_KP_Add, XK_Left},      // Row G: Trace, Sto->, 1, 2, 3, +, Left
    {XK_F5, NoSymbol, XK_0, XK_period, XK_asciitilde, XK_KP_Enter, XK_Down}       // Row H: Graph, Null, 0, ., (-), Enter, Down
};

int mode = MODE_NORMAL;
gboolean isAlphaLockActive = FALSE;
static int counter = 0;
int colCount = 0;

int getColCount(void);
void setValue(int outputValue);
void setBit(int bit);
gchar * getImagePath(char * imageFile);
gchar * getModeIconImage(void);
void updateStatusIcon(void);
void changeMode(int newMode);
void cycleModes(void);
void greet(GtkWidget *widget, gpointer data);
void destroy(GtkWidget *widget, gpointer data);
void setup(void);
void emulateKeyPress(KeySym keySym);
KeySym getKeySymbol(int row, int col);
gboolean loop(gpointer data);
int main(int argc, char *argv[]);

#endif /* ti83keypad_h */
