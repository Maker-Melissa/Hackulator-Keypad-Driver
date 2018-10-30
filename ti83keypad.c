/***************************************************
  Filename: ti83keypad.c

***************************************************/

// To Do:
// Add Screen to Black for Shutdown
// Add functionality so that if you rightclick the status icon, it shows the about dialog (optional)
// Remove any unused functions
// Remove Debugging Messages (Search for g_print)
// Polish up any other small details

#include "ti83keypad.h"

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

gboolean specialKey(KeySym keySym, int eventType)
{
    // If Special Key, respond and return true
    if (eventType == EVENT_RELEASE) {
        if (keySym == SPECIAL_ALPHA_UPPER_KEY) {
            changeMode(MODE_ALPHA_UPPER);
        } else if (keySym == SPECIAL_ALPHA_LOWER_KEY) {
            changeMode(MODE_ALPHA_LOWER);
        } else if (keySym == SPECIAL_2ND_KEY) {
            changeMode(MODE_SECOND);
        } else if (keySym == SPECIAL_NORMAL_KEY) {
            changeMode(MODE_NORMAL);
        } else if (keySym == SPECIAL_LOCK_KEY) {
            changeAlphaLock();
        } else if (keySym == SPECIAL_BRIGHT_UP_KEY) {
            brightnessUp();
        } else if (keySym == SPECIAL_BRIGHT_DOWN_KEY) {
            brightnessDown();
        } else if (keySym == SPECIAL_CONTROL_LOCK) {
            changeControlLock();
        }
    }
    
    if (keySym == SPECIAL_ALPHA_UPPER_KEY ||
        keySym == SPECIAL_ALPHA_LOWER_KEY ||
        keySym == SPECIAL_2ND_KEY ||
        keySym == SPECIAL_LOCK_KEY ||
        keySym == SPECIAL_NORMAL_KEY ||
        keySym == SPECIAL_BRIGHT_UP_KEY ||
        keySym == SPECIAL_BRIGHT_DOWN_KEY ||
        keySym == SPECIAL_CONTROL_LOCK) {
        return TRUE;
    }
    
    return FALSE;
}

void brightnessUp(void)
{
    if (brightness < MAX_BRIGHTNESS) {
        brightness += 1;
        softPwmWrite (BACKLIGHT_PIN, brightness);
    }
    g_print("Brightness Up [%i/%i]", brightness, MAX_BRIGHTNESS);
    changeMode(MODE_NORMAL);
}

void brightnessDown(void)
{
    if (brightness > 0) {
        brightness -= 1;
        softPwmWrite (BACKLIGHT_PIN, brightness);
    }
    g_print("Brightness Down [%i/%i]", brightness, MAX_BRIGHTNESS);
    changeMode(MODE_NORMAL);
}

// This function handles lock status for the non-special keys
void handleLockStatus(KeySym keySym)
{
    if (mode == MODE_SECOND) {
        changeMode(lastMode);
    } else if ((mode == MODE_ALPHA_LOWER || mode == MODE_ALPHA_UPPER) && isAlphaLockActive == FALSE) {
        changeMode(MODE_NORMAL);
    }
}

void changeAlphaLock(void)
{
    if (mode != MODE_SECOND) {
        return;
    }
    
    if (isAlphaLockActive) {
        isAlphaLockActive = FALSE;
    } else {
        isAlphaLockActive = TRUE;
    }
    
    if (lastMode == MODE_ALPHA_UPPER) {
        changeMode(MODE_ALPHA_UPPER);
    } else if (lastMode == MODE_ALPHA_LOWER || lastMode == MODE_NORMAL) {
        changeMode(MODE_ALPHA_LOWER);
    }
}

void changeControlLock(void)
{
    if (isControlLockActive) {
        isControlLockActive = FALSE;
    } else {
        isControlLockActive = TRUE;
    }
}

gchar * getImagePath(char * imageFile)
{
    // Can't get it working with autostart, so just using Absolute path for now
    //char currentFolder[256];
    //char pathSave[256];
    const char* imageFolder = "/images/";
    GString * imagePath = g_string_new("/home/pi/ti83keypad");
/*
    gchar * result = g_strstr_len (executable->str, 1, "/");
    if (result == NULL) {
        if (getcwd(currentFolder, sizeof(currentFolder)) == NULL) {
            g_print ("Error getting Current Working Directory\n");
        }
    } else {
        getcwd(pathSave, sizeof(pathSave));
        chdir(executable->str);
        getcwd(currentFolder, sizeof(currentFolder));
        chdir(pathSave);
    }*/

    //g_string_append(imagePath, (char*) currentFolder);
    g_string_append(imagePath, imageFolder);
    g_string_append(imagePath, imageFile);

    return imagePath->str;
}

gchar * getModeIconImage(void)
{
    if (mode == MODE_SECOND) {
        return "2nd.png";
    } else if (mode == MODE_ALPHA_LOWER) {
        return "lowercase.png";
    } else if (mode == MODE_ALPHA_UPPER) {
        return "uppercase.png";
    } else if (mode == MODE_TI83) {
        return "ti83mode.png";
    }
    
    return "numbers.png";
}

void updateStatusIcon(void)
{
    gtk_status_icon_set_from_file (tray, getImagePath(getModeIconImage()));
}

void changeMode(int newMode)
{
    lastMode = mode;
    mode = newMode;
    if (newMode == MODE_NORMAL || newMode == MODE_TI83) {
        isAlphaLockActive = FALSE;
        isControlLockActive = FALSE;
    }
    if (newMode == MODE_SECOND) {
        isControlLockActive = FALSE;
    }
    updateStatusIcon();
}

void cycleModes(void)
{
    if (mode == MODE_TI83) {
        changeMode(MODE_NORMAL);
    } else {
        changeMode(MODE_TI83);
    }
}

void destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit ();
}

gboolean isShiftRequired(KeySym keySym)
{
    // Check if a Shift is Required
    for (int i = 0; i < SHIFT_SYMBOL_SIZE; i++) {
        if (keySym == shiftSymbols[i]) {
            return TRUE;
        }
    }
    
    return FALSE;
}

void emulateKeyPress(KeySym keySym)
{
    KeyCode modcode = 0; //init value
    isKeyPressed = TRUE;
    
    if (specialKey(keySym, EVENT_PRESS)) {
        return;
    } else {
        handleLockStatus(keySym);
    }

    if (keySym == NoSymbol) {
        return;
    }
    
    modcode = XKeysymToKeycode(display, keySym);
    
    if (isShiftRequired(keySym)) {
        //g_print("Event: Shift Pressed\n");
        XTestFakeKeyEvent(display, XKeysymToKeycode(display, XK_Shift_L), True, 0);
        XFlush(display);
    }

    if (isControlLockActive) {
        //g_print("Event: Control Pressed\n");
        XTestFakeKeyEvent(display, XKeysymToKeycode(display, XK_Control_L), True, 0);
        XFlush(display);
    }

    //g_print("Event: Key Pressed\n");
    
    XTestFakeKeyEvent(display, modcode, True, 0);
    XFlush(display);
}

void emulateKeyRelease(KeySym keySym)
{
    KeyCode modcode = 0; //init value
    isKeyPressed = FALSE;
    
    if (specialKey(keySym, EVENT_RELEASE)) {
        return;
    }
    
    if (keySym == NoSymbol) {
        return;
    }

    modcode = XKeysymToKeycode(display, keySym);
    
    //g_print("Event: Key Released\n");
    XTestFakeKeyEvent(display, modcode, False, 0);
    XFlush(display);

    if (isControlLockActive) {
        //g_print("Event: Control Pressed\n");
        XTestFakeKeyEvent(display, XKeysymToKeycode(display, XK_Control_L), True, 0);
        XFlush(display);
        isControlLockActive = FALSE;
    }

    if (isShiftRequired(keySym)) {
        //g_print("Event: Shift Released\n");
        XTestFakeKeyEvent(display, XKeysymToKeycode(display, XK_Shift_L), False, 0);
        XFlush(display);
    }
    
}

void shutdown(void)
{
    // Would like something to change brightness to 0
    // Maybe it has something to do with system not blacking out screen anymore...
    system ("sudo shutdown -h now");
}

KeySym getKeySymbol(int row, int col)
{
    if (mode == MODE_TI83) {
        return ti83Layout[row][col];
    } else if (mode == MODE_ALPHA_UPPER) {
        return alphaUpperLayout[row][col];
    } else if (mode == MODE_ALPHA_LOWER) {
        return alphaLowerLayout[row][col];
    } else if (mode == MODE_SECOND) {
        return secondLayout[row][col];
    }
    
    return normalLayout[row][col];
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
    softPwmCreate (BACKLIGHT_PIN, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
    colCount = getColCount();
    
    for (i = 0; i < colCount; i++) {   // Set column pins for input, with pullup.
        pinMode(colPins[i], INPUT);
        pullUpDnControl (colPins[i], PUD_DOWN);
    }
}

gboolean loop(gpointer data)
{   
    int row, col;
    gboolean keyFound = FALSE;
    KeySym ks;
    
    if (isKeyPressed) {
        return FALSE;
    }

    for (row = 0; row < 8; row++) {
        setBit(row);
        for (col = 0; col < colCount; col++) {
            if (digitalRead(colPins[col]) == HIGH) {
                ks = getKeySymbol(row, col);
                emulateKeyPress(ks);
                while (keyFound == FALSE && digitalRead(colPins[col]) == HIGH) {
                    if (ks == XK_F11 && digitalRead(ONKEY_PIN) == LOW) {
                        g_print("Mode Change Key Combo Detected\n");
                        cycleModes();
                        keyFound = TRUE;
                    }
                };
                emulateKeyRelease(ks);
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
            if (mode == MODE_TI83) {
                // Emulate F12 and Check For Mode Change Combo
                emulateKeyPress(XK_F12);
                while (keyFound == FALSE && digitalRead(ONKEY_PIN) == LOW) {
                    setBit(0);
                    if (digitalRead(colPins[0]) == HIGH) {
                        g_print("Mode Change Key Combo Detected\n");
                        cycleModes();
                        keyFound = TRUE;
                    }
                };
                emulateKeyRelease(XK_F12);
                delay(BOUNCE_DELAY);
            } else if (mode == MODE_SECOND) {
                emulateKeyPress(NoSymbol);
                // Power Down
                g_print("Power Down\n");
                shutdown();
                // So that we don't keep looping
                while (digitalRead(ONKEY_PIN) == LOW);
                emulateKeyRelease(NoSymbol);
            } else {
                // Check For Mode Change Combo
                emulateKeyPress(NoSymbol);
                while (keyFound == FALSE && digitalRead(ONKEY_PIN) == LOW) {
                    setBit(0);
                    if (digitalRead(colPins[0]) == HIGH) {
                        g_print("Mode Change Key Combo Detected\n");
                        cycleModes();
                        keyFound = TRUE;
                    }
                };
                emulateKeyRelease(NoSymbol);
                delay(BOUNCE_DELAY);
            }
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


int main(int argc, char *argv[])
{
    executable = g_string_new("");
    g_string_append(executable, argv[0]);
    
    gtk_init (&argc, &argv);

    if (geteuid() != 0) {
        fprintf (stderr, "You need to be root to run this program. (sudo?)\n");
        exit(0);
    }
    
    tray = gtk_status_icon_new_from_file(getImagePath(getModeIconImage()));
    gtk_status_icon_set_tooltip_text(tray, "Normal");
    
    setup();
    gint func_ref = g_timeout_add (SCAN_DELAY, loop, FALSE);

    gtk_main();
    
    g_source_remove (func_ref);
    
    return 0;
}
