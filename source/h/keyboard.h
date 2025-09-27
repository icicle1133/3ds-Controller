#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <3ds.h>
#include <string.h>

#define KEYBOARD_MAX_TEXT 256

typedef struct {
    SwkbdState swkbd;
    char text[KEYBOARD_MAX_TEXT];
    int status;
} keyboard;

int keyboard_init(keyboard *kb) {
    swkbdInit(&kb->swkbd, SWKBD_TYPE_NORMAL, 2, -1);
    memset(kb->text, 0, KEYBOARD_MAX_TEXT);
    kb->status = 0;
    return 1;
}

int keyboard_get(keyboard *kb, const char *hint, const char *initialtext, int maxlength) {
    if (maxlength > KEYBOARD_MAX_TEXT - 1)
        maxlength = KEYBOARD_MAX_TEXT - 1;
    
    swkbdSetHintText(&kb->swkbd, hint);
    swkbdSetInitialText(&kb->swkbd, initialtext);
    swkbdSetMaxLength(&kb->swkbd, maxlength);
    
    kb->status = swkbdInputText(&kb->swkbd, kb->text, maxlength);
    
    return kb->status == SWKBD_BUTTON_RIGHT;
}

void keyboard_set_validation(keyboard *kb, SwkbdValidInput validation, int checkdigits) {
    swkbdSetValidation(&kb->swkbd, validation, 0, checkdigits);
}

void keyboard_set_filter(keyboard *kb, SwkbdFilterCallback filter) {
    swkbdSetFilterCallback(&kb->swkbd, filter);
}

void keyboard_set_password(keyboard *kb, int enable) {
    swkbdSetPasswordMode(&kb->swkbd, enable);
}

const char* keyboard_get_text(keyboard *kb) {
    return kb->text;
}

void keyboard_clear(keyboard *kb) {
    memset(kb->text, 0, KEYBOARD_MAX_TEXT);
}

#endif