/***************************************************
  Filename: ti83keypad.c

***************************************************/

// To do:
// Figure out how fast to read the keyboard to make it respond well yet not slow the system down. Start with 1ms/row and go from there
// Properly Debounce buttons
// Store modes (Normal, Alpha, ALPHA, 2nd)
// Create Ascii Keymaps for each mode and allow transitions between modes
// Add checking on/off button
// Check for shutdown 2nd->Off
// Maybe a sleep mode of some kind
// Maybe some key combos to change modes

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
#define KEYPRESS_DELAY  50

#define MODE_NORMAL 1
#define MODE_ALPHA_LOWER 2
#define MODE_ALPHA_LOWER_LOCK 3
#define MODE_ALPHA_UPPER 4
#define MODE_ALPHA_UPPER_LOCK 5
#define MODE_TI83 6

GtkStatusIcon *tray;
Display *display;

// Key Symbols requiring a Shift (Add as needed)
KeySym shiftSymbols[2] = {
    XK_asciitilde,
    XK_asciicircum
};

int colPins[] = {7, 15, 16, 2, 3, 4, 21}; // Columns I, J, K, L, M, N, O
KeySym ti83layout[8][7] = {
    {XK_F11, XK_F6, XK_F7, XK_F8, XK_F9, XK_Escape, NoSymbol},  // Row A: Mode, Math, Apps, Prgm, Vars, Clear
    {XK_Delete, XK_apostrophe, XK_x, XK_F10, NoSymbol, NoSymbol, NoSymbol},      // Row B: Del, Alpha, "X,T,𝚹,n" (GraphVar), Stat
    {XK_Tab, XK_backslash, XK_s, XK_c, XK_t, XK_asciicircum, NoSymbol},      // Row C: 2nd, X^-1, Sin, Cos, Tan, ^
    {XK_F1, XK_semicolon, XK_comma, XK_parenleft, XK_parenright, XK_slash, NoSymbol},      // Row D: Y=, X^2, ',', (, ), ÷
    {XK_F2, XK_o, XK_7, XK_8, XK_9, XK_KP_Multiply, XK_Up},      // Row E: Window, Log, 7, 8, 9, X (Multiply), Up
    {XK_F3, XK_l, XK_4, XK_5, XK_6, XK_KP_Subtract, XK_Right},      // Row F: Zoom, LN, 4, 5, 6, -, Right
    {XK_F4, XK_equal, XK_1, XK_2, XK_3, XK_KP_Add, XK_Left},      // Row G: Trace, Sto->, 1, 2, 3, +, Left
    {XK_F5, NoSymbol, XK_0, XK_period, XK_asciitilde, XK_KP_Enter, XK_Down}       // Row H: Graph, Null, 0, ., (-), Enter, Down
 };

int mode = MODE_TI83;
gboolean isSecondActive = FALSE;
static int counter = 0;
int colCount = 0;

int getColCount(void)
{
    return sizeof(colPins) / sizeof(colPins[0]);
}

void setValue(int outputValue)
{
    int bit;
    for (bit = 0 ; bit < 8; ++bit) {
        digitalWrite (100 + bit, outputValue & (1 << bit));
    }
}

void setBit(int bit)
{
    setValue(1 << bit);
}

void greet( GtkWidget *widget, gpointer data )
{
    // printf equivalent in GTK+
    g_print ("TI-83 Keypad Driver\n");
    g_print ("%s clicked %d times\n",
             (char*)data, ++counter);
}

void destroy( GtkWidget *widget, gpointer data )
{
    gtk_main_quit ();
}

void setup(void)
{
    int i;
   
    if (wiringPiSetup() == -1) {
        g_print("wiringPiSetup error\n");
        exit(1);
    }
    
    if ((display = XOpenDisplay(NULL)) == NULL) {
        g_print("XOpenDisplay Initialization Failure\n");
        exit(2);
    }
    
    sr595Setup (100, 8, DATA_PIN, CLOCK_PIN, LATCH_PIN) ;
    
    colCount = getColCount();
    
    for (i = 0; i < colCount; i++) {   // Set column pins for input, with pullup.
        pinMode(colPins[i], INPUT);
        pullUpDnControl (colPins[i], PUD_DOWN);
    }
}

void emulateKeyPress(KeySym keySym)
{
    KeyCode modcode = 0; //init value
    gboolean doShift = FALSE;
    
    if (KeySym == NoSymbol) {
        return;
    }
    
    // Check if a Shift is Required
    for (int i = 0; i < sizeof(shiftSymbols); i++) {
        if (keySym == shiftSymbols[i]) {
            doShift = TRUE;
            break;
        }
    }

    modcode = XKeysymToKeycode(display, keySym);
    
    if (doShift == TRUE) {
        XTestFakeKeyEvent(display, XKeysymToKeycode(display, XK_Shift_L), True, 0);
        XFlush(display);
    }
    
    XTestFakeKeyEvent(display, modcode, True, 0);
    XFlush(display);
    delay(KEYPRESS_DELAY);
    XTestFakeKeyEvent(display, modcode, False, 0);
    XFlush(display);
    
    if (doShift == TRUE) {
        XTestFakeKeyEvent(display, XKeysymToKeycode(display, XK_Shift_L), False, 0);
        XFlush(display);
    }

}

KeySym getKeySymbol(int row, int col)
{
    if (mode == MODE_TI83) {
        return ti83layout[row][col];
    }
    
    return ti83layout[row][col];
}

gboolean loop(gpointer data)
{   
    int row, col;
    gboolean keyFound = FALSE;
    KeySym ks;

    for (row = 0; row < 8; row++) {
        setBit(row);
        for (col = 0; col < colCount; col++) {
            if (digitalRead(colPins[col]) == HIGH) {
                ks = getKeySymbol(row, col);
                emulateKeyPress(ks);
                keyFound = TRUE;                            // Force exit of both for loops.
            }
            
            if (keyFound) break;
        }
        if (keyFound) {
            delay(BOUNCE_DELAY);
            break;
        }
    }
    
    if (digitalRead(ONKEY_PIN) == LOW) {
        if (!keyFound) {
            if (mode = MODE_TI83) {
                emulateKeyPress(XK_F12);
                delay(BOUNCE_DELAY);
            }
        } else if (ks == XK_F5) {
            g_print("Mode Change Key Combo Detected\n");
            cycleModes();
        }
    }
    
    return TRUE;
}

/*
static void show_about( GtkWidget *widget, gpointer data )
{
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_INFO,
                                             GTK_BUTTONS_OK,
                                             "TI-83 Keypad Driver\nBy Melissa LeBlanc-Williams");
    
    gtk_widget_show_all (dialog);
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
*/

// To Do Function to cycle through modes and call changeMode()

void cycleModes(void)
{
    
}

void changeMode(int newMode)
{
    mode = newMode;
    // Change to appropriate Icon
}
                               
int main(int argc, char *argv[])
{
    gtk_init (&argc, &argv);

    if (geteuid() != 0) {
        fprintf (stderr, "You need to be root to run this program. (sudo?)\n");
        exit(0);
    }
    
    changeMode(MODE_TI83);
    tray = gtk_status_icon_new_from_file("/home/pi/ti83keypad/images/ti83mode.png");
    gtk_status_icon_set_tooltip_text(tray, "Normal");
    
    setup();
    gint func_ref = g_timeout_add (SCAN_DELAY, loop, FALSE);

    gtk_main();
    
    g_source_remove (func_ref);
    
    return 0;
}
/*
int si_main(int argc, char *argv[])
{
    GtkStatusIcon *si;
    int i=1, watch, fd;
    gtk_init (&argc, &argv); // loop through icon, tooltip, click messages
    while (i<argc) {
        fd = inotify_init(); // get file descriptor to write on if image changes
        si = gtk_status_icon_new_from_file(argv[i]); // get a status icon widget
        gtk_status_icon_set_title(si,argv[i]); // hack to store the image path
        watch = inotify_add_watch( fd, argv[i++], IN_MODIFY );
        gdk_input_add( fd, GDK_INPUT_READ, refresh, si ); // inotify fd is ready for reading, refresh
        gtk_status_icon_set_tooltip_text(si,argv[i++]);
        g_signal_connect(G_OBJECT(si), "activate", G_CALLBACK(leftclick),(gpointer) argv[i++]);
        g_signal_connect(G_OBJECT(si), "popup-menu", G_CALLBACK(rightclick), (gpointer) argv[i++]);
        
    }
    gtk_main ();
    
}*/