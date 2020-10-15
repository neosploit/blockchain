/* nuklear - v1.32.0 - public domain */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <limits.h>
#include <jansson.h>
#include "client.h"
#include "../hashing.h"
#include "../auxilary.h"
#include "../wallet/wallet.h"
#include "../wallet/transaction.h"
#include "wallet_gui.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_XLIB_GL3_IMPLEMENTATION
#define NK_XLIB_LOAD_OPENGL_EXTENSIONS
#include "nuklear.h"
#include "nuklear_xlib_gl3.h"

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

/* enums */
enum main_sections {info, wallet, transaction};
enum wallet_sections {buttons, create, recover};
enum create_sections {create_input, create_success, create_failure};
enum recover_sections {recover_input, recover_success, recover_failure};
enum transaction_sections {transaction_input, transaction_success, transaction_failure};

/* bib0039 wordlist */
extern const char bib0039[WORD_COUNT][WORD_LENGTH];

/* ===============================================================
 *
 *                          Global Variables
 *
 * ===============================================================*/

/* Connection Status */
int connected;
int nodes_connected;

/* ===============================================================
 *
 *                          GUI
 *
 * ===============================================================*/
struct XWindow {
    Display *dpy;
    Window win;
    XVisualInfo *vis;
    Colormap cmap;
    XSetWindowAttributes swa;
    XWindowAttributes attr;
    GLXFBConfig fbc;
    Atom wm_delete_window;
    int width, height;
};
static int gl_err = nk_false;
static int gl_error_handler(Display *dpy, XErrorEvent *ev)
{NK_UNUSED(dpy); NK_UNUSED(ev); gl_err = nk_true;return 0;}

static void
die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

static int
has_extension(const char *string, const char *ext)
{
    const char *start, *where, *term;
    where = strchr(ext, ' ');
    if (where || *ext == '\0')
        return nk_false;

    for (start = string;;) {
        where = strstr((const char*)start, ext);
        if (!where) break;
        term = where + strlen(ext);
        if (where == start || *(where - 1) == ' ') {
            if (*term == ' ' || *term == '\0')
                return nk_true;
        }
        start = term;
    }
    return nk_false;
}

int start_gui(client_settings_t *client)
{
    /* Platform */
    int running = 1;
    struct XWindow win;
    GLXContext glContext;
    struct nk_context *ctx;
    struct nk_colorf bg;

    memset(&win, 0, sizeof(win));
    win.dpy = XOpenDisplay(NULL);
    if (!win.dpy) die("Failed to open X display\n");
    {
        /* check glx version */
        int glx_major, glx_minor;
        if (!glXQueryVersion(win.dpy, &glx_major, &glx_minor))
            die("[X11]: Error: Failed to query OpenGL version\n");
        if ((glx_major == 1 && glx_minor < 3) || (glx_major < 1))
            die("[X11]: Error: Invalid GLX version!\n");
    }
    {
        /* find and pick matching framebuffer visual */
        int fb_count;
        static GLint attr[] = {
            GLX_X_RENDERABLE,   True,
            GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
            GLX_RENDER_TYPE,    GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
            GLX_RED_SIZE,       8,
            GLX_GREEN_SIZE,     8,
            GLX_BLUE_SIZE,      8,
            GLX_ALPHA_SIZE,     8,
            GLX_DEPTH_SIZE,     24,
            GLX_STENCIL_SIZE,   8,
            GLX_DOUBLEBUFFER,   True,
            None
        };
        GLXFBConfig *fbc;
        fbc = glXChooseFBConfig(win.dpy, DefaultScreen(win.dpy), attr, &fb_count);
        if (!fbc) die("[X11]: Error: failed to retrieve framebuffer configuration\n");
        {
            /* pick framebuffer with most samples per pixel */
            int i;
            int fb_best = -1, best_num_samples = -1;
            for (i = 0; i < fb_count; ++i) {
                XVisualInfo *vi = glXGetVisualFromFBConfig(win.dpy, fbc[i]);
                if (vi) {
                    int sample_buffer, samples;
                    glXGetFBConfigAttrib(win.dpy, fbc[i], GLX_SAMPLE_BUFFERS, &sample_buffer);
                    glXGetFBConfigAttrib(win.dpy, fbc[i], GLX_SAMPLES, &samples);
                    if ((fb_best < 0) || (sample_buffer && samples > best_num_samples))
                        fb_best = i, best_num_samples = samples;
                }
            }
            win.fbc = fbc[fb_best];
            XFree(fbc);
            win.vis = glXGetVisualFromFBConfig(win.dpy, win.fbc);
        }
    }
    {
        /* create window */
        win.cmap = XCreateColormap(win.dpy, RootWindow(win.dpy, win.vis->screen), win.vis->visual, AllocNone);
        win.swa.colormap =  win.cmap;
        win.swa.background_pixmap = None;
        win.swa.border_pixel = 0;
        win.swa.event_mask =
            ExposureMask | KeyPressMask | KeyReleaseMask |
            ButtonPress | ButtonReleaseMask| ButtonMotionMask |
            Button1MotionMask | Button3MotionMask | Button4MotionMask | Button5MotionMask|
            PointerMotionMask| StructureNotifyMask;
        win.win = XCreateWindow(win.dpy, RootWindow(win.dpy, win.vis->screen), 0, 0,
            WINDOW_WIDTH, WINDOW_HEIGHT, 0, win.vis->depth, InputOutput,
            win.vis->visual, CWBorderPixel|CWColormap|CWEventMask, &win.swa);
        if (!win.win) die("[X11]: Failed to create window\n");
        XFree(win.vis);
        XStoreName(win.dpy, win.win, "ARELKA WALLET");
        XMapWindow(win.dpy, win.win);
        win.wm_delete_window = XInternAtom(win.dpy, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(win.dpy, win.win, &win.wm_delete_window, 1);
    }
    {
        /* create opengl context */
        typedef GLXContext(*glxCreateContext)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
        int(*old_handler)(Display*, XErrorEvent*) = XSetErrorHandler(gl_error_handler);
        const char *extensions_str = glXQueryExtensionsString(win.dpy, DefaultScreen(win.dpy));
        glxCreateContext create_context = (glxCreateContext)
            glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

        gl_err = nk_false;
        if (!has_extension(extensions_str, "GLX_ARB_create_context") || !create_context) {
            fprintf(stdout, "[X11]: glXCreateContextAttribARB() not found...\n");
            fprintf(stdout, "[X11]: ... using old-style GLX context\n");
            glContext = glXCreateNewContext(win.dpy, win.fbc, GLX_RGBA_TYPE, 0, True);
        } else {
            GLint attr[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                GLX_CONTEXT_MINOR_VERSION_ARB, 0,
                None
            };
            glContext = create_context(win.dpy, win.fbc, 0, True, attr);
            XSync(win.dpy, False);
            if (gl_err || !glContext) {
                /* Could not create GL 3.0 context. Fallback to old 2.x context.
                 * If a version below 3.0 is requested, implementations will
                 * return the newest context version compatible with OpenGL
                 * version less than version 3.0.*/
                attr[1] = 1; attr[3] = 0;
                gl_err = nk_false;
                fprintf(stdout, "[X11] Failed to create OpenGL 3.0 context\n");
                fprintf(stdout, "[X11] ... using old-style GLX context!\n");
                glContext = create_context(win.dpy, win.fbc, 0, True, attr);
            }
        }
        XSync(win.dpy, False);
        XSetErrorHandler(old_handler);
        if (gl_err || !glContext)
            die("[X11]: Failed to create an OpenGL context\n");
        glXMakeCurrent(win.dpy, win.win, glContext);
    }

    ctx = nk_x11_init(win.dpy, win.win);

    {
        struct nk_font_atlas *atlas;
        nk_x11_font_stash_begin(&atlas);
        nk_x11_font_stash_end();
    }

    // black-grey background
    bg.r = 0.03f, bg.g = 0.03f, bg.b = 0.03f, bg.a = 1.0f;

    // gui flags
    enum main_sections main_section = info;
    enum wallet_sections wallet_section = buttons;
    enum create_sections create_section = create_input;
    enum recover_sections recover_section = recover_input;
    enum transaction_sections transaction_section = transaction_input;

    /* Connection Status */
    char connections[25];

    /* Wallet Info */
    int wallet_count;
    char **wallet_addresses;
    char **wallet_balances;
    const char **_wallet_addresses;
    wallet_addresses = (char**) malloc(MAX_WALLET_COUNT * sizeof(char*));
    wallet_balances = (char**) malloc(MAX_WALLET_COUNT * sizeof(char*));
    for(int i = 0; i < MAX_WALLET_COUNT; i++)
    {
        wallet_addresses[i] = (char*) malloc((HASH_LENGTH + 1) * sizeof(char));
        wallet_balances[i] = (char*) malloc(50 * sizeof(char));
    }
    _wallet_addresses = wallet_addresses;

    /* Previous Transactions */
    int transaction_count;
    char **transaction_sender;
    char **transaction_receiver;
    char **transaction_amount;
    char **transaction_time;
    int *transaction_direction; /* 0: outgoing, 1: incoming  */
    int *transaction_status; /* 0: unconfirmed, 1: confirmed */

    transaction_sender = (char**) malloc(MAX_TRANSACTION_DISPLAY_COUNT * sizeof(char*));
    transaction_receiver = (char**) malloc(MAX_TRANSACTION_DISPLAY_COUNT * sizeof(char*));
    transaction_amount = (char**) malloc(MAX_TRANSACTION_DISPLAY_COUNT * sizeof(char*));
    transaction_time = (char**) malloc(MAX_TRANSACTION_DISPLAY_COUNT * sizeof(char*));
    for(int i = 0; i < MAX_TRANSACTION_DISPLAY_COUNT; i++)
    {
        transaction_sender[i] = (char*) malloc((HASH_LENGTH + 1) * sizeof(char));
        transaction_receiver[i] = (char*) malloc((HASH_LENGTH + 1) * sizeof(char));
        transaction_amount[i] = (char*) malloc(12 * sizeof(char));
        transaction_time[i] = (char*) malloc(30 * sizeof(char));
    }
    transaction_direction = (int*) calloc(MAX_TRANSACTION_DISPLAY_COUNT, sizeof(int));
    transaction_status = (int*) calloc(MAX_TRANSACTION_DISPLAY_COUNT, sizeof(int));

    /* Previous Transactions */
    char sender_label[10];
    char receiver_label[10];
    int show_sender;
    int show_receiver;

    /* Wallet Creation and Recovery */
    char wallet_name_buf[80];
    char **wallet_seed_phrase;
    wallet_t *_wallet;
    json_t *wallets;

    _wallet = malloc(1* sizeof(wallet_t));
    wallet_seed_phrase = (char**) malloc(SEED_PHRASE_LENGTH * sizeof(char*));
    for(int i = 0; i < SEED_PHRASE_LENGTH; i++)
    {
        wallet_seed_phrase[i] = (char*) malloc(WORD_LENGTH * sizeof(char));
    }

    /* Wallet Recovery only */
    int wallet_seed_phrase_indexes[SEED_PHRASE_LENGTH];
    const char **_seed_phrase_word_choices;
    char **seed_phrase_word_choices;
    for(int i = 0; i < SEED_PHRASE_LENGTH; i++)
    {
        wallet_seed_phrase_indexes[i] = 0;
    }

    seed_phrase_word_choices = (char**) malloc(WORD_COUNT * sizeof(char*));
    for(int i = 0; i < WORD_COUNT; i++)
    {
        seed_phrase_word_choices[i] = (char*) malloc((strlen(bib0039[i]) + 1) * sizeof(char));
        strcpy(seed_phrase_word_choices[i], bib0039[i]);
    }
    _seed_phrase_word_choices = seed_phrase_word_choices;

    /* Transactions */
    int wallet_address_index = 0;
    char transaction_receiver_buf[40];
    char transaction_amount_buf[10];
    transaction_t *_transaction;
    double _transaction_amount;

    _transaction = malloc(1 * sizeof(transaction_t));

    /* Conversion */
    char i_string[5];

    srand(time(NULL));
    
    while (running)
    {
        /* Input */
        XEvent evt;
        nk_input_begin(ctx);
        while (XPending(win.dpy)) {
            XNextEvent(win.dpy, &evt);
            if (evt.type == ClientMessage) goto cleanup;
            if (XFilterEvent(&evt, win.win)) continue;
            nk_x11_handle_event(&evt);
        }
        nk_input_end(ctx);
        
        /* Update Status */
        sprintf(connections, "%d", nodes_connected);
        strcat(connections, " Node Connection(s)");

        /* Wallet Info */
        wallet_count = count_wallets(client);
        get_wallet_info(client, wallet_addresses, wallet_balances, MAX_WALLET_COUNT);

        /* Previous Transactions */
        get_transaction_info(client, &transaction_count, transaction_sender, transaction_receiver, transaction_amount, transaction_time, transaction_direction, transaction_status, MAX_TRANSACTION_DISPLAY_COUNT);
        
        /* Menu */
        if (nk_begin(ctx, "menu", nk_rect(0, 0, WINDOW_WIDTH*2/3, WINDOW_HEIGHT/9), NK_WINDOW_BORDER))
        {
            nk_layout_row_dynamic(ctx, WINDOW_HEIGHT/14, 3);
            if (nk_button_label(ctx, "INFO"))
            {
                main_section = info;
            }
            if (nk_button_label(ctx, "WALLET CREATION"))
            {
                main_section = wallet;
                wallet_section = buttons;
            }
            if (nk_button_label(ctx, "TRANSACTIONS"))
            {
                strcpy(transaction_receiver_buf, "");
                strcpy(transaction_amount_buf, "");
                wallet_address_index = 0;
                main_section = transaction;
                transaction_section = transaction_input;
            }
        }
        nk_end(ctx);
        
        /* Main Section */
        if (nk_begin(ctx, "main", nk_rect(0, WINDOW_HEIGHT/9, WINDOW_WIDTH*2/3, WINDOW_HEIGHT*8/9), NK_WINDOW_BORDER))
        {
            switch(main_section)
            {
                case info:
                    nk_layout_row_dynamic(ctx, 30, 1);
                    nk_label(ctx, "WALLET INFO", NK_TEXT_ALIGN_CENTERED);
                    
                    nk_layout_row_dynamic(ctx, 30, 2);
                    nk_label_colored(ctx, "ADDRESS", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                    nk_label_colored(ctx, "BALANCE", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));

                    for (int i = 0; i < wallet_count; i++)
                    {
                        nk_layout_row_dynamic(ctx, 30, 2);

                        // wallet address
                        nk_text(ctx, wallet_addresses[i], strlen(wallet_addresses[i]), NK_TEXT_ALIGN_CENTERED);

                        // wallet balance
                        nk_text(ctx, wallet_balances[i], strlen(wallet_balances[i]), NK_TEXT_ALIGN_CENTERED);
                    }
                    break;
                case wallet:
                    /* Generate the correct wallet window */
                    switch(wallet_section)
                    {
                        case buttons:
                            nk_layout_row_dynamic(ctx, WINDOW_HEIGHT/9, 1);
                            if (nk_button_label(ctx, "CREATE WALLET"))
                            {
                                strcpy(wallet_name_buf, "");
                                wallet_section = create;
                                create_section = create_input;
                            }

                            nk_layout_row_dynamic(ctx, WINDOW_HEIGHT/9, 1);
                            if (nk_button_label(ctx, "RECOVER WALLET"))
                            {
                                strcpy(wallet_name_buf, "");
                                for(int i = 0; i < SEED_PHRASE_LENGTH; i++){
                                    wallet_seed_phrase_indexes[i] = 0;
                                    strcpy(wallet_seed_phrase[i], "");
                                }
                                wallet_section = recover;
                                recover_section = recover_input;
                            }
                            break;
                        case create:
                            switch(create_section)
                            {
                                case create_input:
                                    nk_layout_row_dynamic(ctx, 30, 1);
                                    nk_label(ctx, "CREATE WALLET", NK_TEXT_ALIGN_CENTERED);

                                    nk_layout_row_dynamic(ctx, 20, 2);
                                    nk_label_colored(ctx, "WALLET NAME:", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                                    nk_edit_string_zero_terminated (ctx, NK_EDIT_FIELD, wallet_name_buf, 80, nk_filter_default);

                                    nk_layout_row_dynamic(ctx, WINDOW_HEIGHT/9, 3);
                                    nk_label(ctx, "", NK_TEXT_ALIGN_CENTERED);
                                    if (nk_button_label(ctx, "SUBMIT"))
                                    {
                                        if(strlen(wallet_name_buf) > 0)
                                        {
                                            /* Create Wallet */
                                            _wallet->name = wallet_name_buf;
                                            for(int i = 0; i < SEED_PHRASE_LENGTH; i++)
                                            {
                                                sprintf(wallet_seed_phrase[i], "%s", bib0039[rand() % WORD_COUNT]);
                                            }

                                            _wallet->private_key = generate_private_key(wallet_seed_phrase);
                                            _wallet->public_key = generate_public_key(_wallet->private_key);
                                            _wallet->address = generate_address(_wallet->public_key);

                                            /* Store Wallet */
                                            wallets = json_load_file(client->wallets_path, JSON_DECODE_ANY, NULL);
                                            json_array_append_new(wallets, json_construct_wallet(_wallet));
                                            json_dump_file(wallets, client->wallets_path, JSON_ENCODE_ANY);

                                            create_section = create_success;
                                        }
                                        else
                                        {
                                           create_section = create_failure;
                                        }
                                    }
                                    nk_label(ctx, "", NK_TEXT_ALIGN_CENTERED);
                                    break;
                                case create_success:
                                    nk_layout_row_dynamic(ctx, 30, 1);
                                    nk_label_colored(ctx, "WALLET CREATED SUCCESSFULLY!", NK_TEXT_ALIGN_CENTERED, nk_rgb(0,255,0));
                                    
                                    nk_layout_row_dynamic(ctx, 20, 1);
                                    nk_label_colored(ctx, "SEED PHRASE:", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                                    for(int i = 0; i < SEED_PHRASE_LENGTH; i++)
                                    {
                                        nk_layout_row_dynamic(ctx, 20, 2);
                                        sprintf(i_string, "%d", i);
                                        strcat(i_string, ":");
                                        nk_label(ctx, i_string, NK_TEXT_ALIGN_CENTERED);
                                        nk_text(ctx, wallet_seed_phrase[i], strlen(wallet_seed_phrase[i]), NK_TEXT_ALIGN_CENTERED);
                                    }

                                    nk_layout_row_dynamic(ctx, 20, 2);
                                    nk_label_colored(ctx, "PRIVATE KEY", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                                    nk_text(ctx, _wallet->private_key, strlen(_wallet->private_key), NK_TEXT_ALIGN_CENTERED);

                                    nk_layout_row_dynamic(ctx, 20, 2);
                                    nk_label_colored(ctx, "PUBLIC KEY", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                                    nk_text(ctx, _wallet->public_key, strlen(_wallet->public_key), NK_TEXT_ALIGN_CENTERED);

                                    nk_layout_row_dynamic(ctx, 20, 2);
                                    nk_label_colored(ctx, "ADDRESS", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                                    nk_text(ctx, _wallet->address, strlen(_wallet->address), NK_TEXT_ALIGN_CENTERED);

                                    nk_layout_row_dynamic(ctx, WINDOW_HEIGHT/9, 1);
                                    if (nk_button_label(ctx, "OK"))
                                    {
                                        strcpy(wallet_name_buf, "");
                                        create_section = create_input;
                                        wallet_section = buttons;
                                    }
                                    break;
                                case create_failure:
                                    nk_layout_row_dynamic(ctx, 30, 1);
                                    nk_label_colored(ctx, "WALLET CREATION FAILED!", NK_TEXT_ALIGN_CENTERED, nk_rgb(255,0,0));

                                    nk_layout_row_dynamic(ctx, WINDOW_HEIGHT/9, 1);
                                    if (nk_button_label(ctx, "OK"))
                                    {
                                        strcpy(wallet_name_buf, "");
                                        create_section = create_input;
                                        wallet_section = buttons;
                                    }
                                    break;
                            }                            
                            break;
                        case recover:
                            switch(recover_section)
                            {
                                case recover_input:
                                    nk_layout_row_dynamic(ctx, 30, 1);
                                    nk_label_colored(ctx, "RECOVER WALLET", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));

                                    nk_layout_row_dynamic(ctx, 20, 2);
                                    nk_label_colored(ctx, "WALLET NAME:", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                                    nk_edit_string_zero_terminated (ctx, NK_EDIT_FIELD, wallet_name_buf, 80, nk_filter_default);

                                    nk_layout_row_dynamic(ctx, 20, 1);
                                    nk_label_colored(ctx, "ENTER SEED PHRASE:", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));

                                    for(int i = 0; i < SEED_PHRASE_LENGTH; i++){
                                        nk_layout_row_dynamic(ctx, 20, 2);
                                        sprintf(i_string, "%d", i);
                                        strcat(i_string, ":");
                                        nk_label(ctx, i_string, NK_TEXT_ALIGN_CENTERED);
                                        wallet_seed_phrase_indexes[i] = nk_combo (ctx, _seed_phrase_word_choices, WORD_COUNT, wallet_seed_phrase_indexes[i], 30, nk_vec2(WINDOW_WIDTH/3, WINDOW_HEIGHT/3));
                                        strcpy(wallet_seed_phrase[i], bib0039[wallet_seed_phrase_indexes[i]]);
                                    }
                                    
                                    nk_layout_row_dynamic(ctx, WINDOW_HEIGHT/9, 3);
                                    nk_label(ctx, "", NK_TEXT_ALIGN_CENTERED);
                                    if (nk_button_label(ctx, "SUBMIT"))
                                    {
                                        if(strlen(wallet_name_buf) > 0)
                                        {
                                            /* create wallet */
                                            _wallet->name = wallet_name_buf;

                                            _wallet->private_key = generate_private_key(wallet_seed_phrase);
                                            _wallet->public_key = generate_public_key(_wallet->private_key);
                                            _wallet->address = generate_address(_wallet->public_key);

                                            /* Store Wallet */
                                            wallets = json_load_file(client->wallets_path, JSON_DECODE_ANY, NULL);
                                            json_array_append_new(wallets, json_construct_wallet(_wallet));
                                            json_dump_file(wallets, client->wallets_path, JSON_ENCODE_ANY);

                                            recover_section = recover_success;
                                        }
                                        else
                                        {
                                            recover_section = recover_failure;
                                        }
                                    }
                                    nk_label(ctx, "", NK_TEXT_ALIGN_CENTERED);
                                    break;
                                case recover_success:
                                    nk_layout_row_dynamic(ctx, 30, 1);
                                    nk_label_colored(ctx, "WALLET RECOVERED SUCCESSFULLY!", NK_TEXT_ALIGN_CENTERED, nk_rgb(0,255,0));

                                    nk_layout_row_dynamic(ctx, 20, 2);
                                    nk_label_colored(ctx, "PRIVATE KEY", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                                    nk_text(ctx, _wallet->private_key, strlen(_wallet->private_key), NK_TEXT_ALIGN_CENTERED);

                                    nk_layout_row_dynamic(ctx, 20, 2);
                                    nk_label_colored(ctx, "PUBLIC KEY", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                                    nk_text(ctx, _wallet->public_key, strlen(_wallet->public_key), NK_TEXT_ALIGN_CENTERED);

                                    nk_layout_row_dynamic(ctx, 20, 2);
                                    nk_label_colored(ctx, "ADDRESS", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                                    nk_text(ctx, _wallet->address, strlen(_wallet->address), NK_TEXT_ALIGN_CENTERED);

                                    nk_layout_row_dynamic(ctx, WINDOW_HEIGHT/9, 1);
                                    if (nk_button_label(ctx, "OK"))
                                    {
                                        strcpy(wallet_name_buf, "");
                                        for(int i = 0; i < SEED_PHRASE_LENGTH; i++){
                                            strcpy(wallet_seed_phrase[i], "");
                                        }
                                        recover_section = recover_input;
                                        wallet_section = buttons;
                                    }
                                    break;
                                case recover_failure:
                                    nk_layout_row_dynamic(ctx, 30, 1);
                                    nk_label_colored(ctx, "WALLET RECOVERY FAILED!", NK_TEXT_ALIGN_CENTERED, nk_rgb(255,0,0));

                                    nk_layout_row_dynamic(ctx, WINDOW_HEIGHT/9, 1);
                                    if (nk_button_label(ctx, "OK"))
                                    {
                                        strcpy(wallet_name_buf, "");
                                        for(int i = 0; i < SEED_PHRASE_LENGTH; i++){
                                            strcpy(wallet_seed_phrase[i], "");
                                        }
                                        recover_section = recover_input;
                                        wallet_section = buttons;
                                    }
                                    break;
                            }
                            break;
                    }
                    break;
                case transaction:
                    switch(transaction_section)
                    {
                        case transaction_input:
                            nk_layout_row_dynamic(ctx, 30, 1);
                            nk_label_colored(ctx, "TRANSACTION", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));

                            nk_layout_row_dynamic(ctx, 20, 2);
                            nk_label_colored(ctx, "SENDER:", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                            wallet_address_index = nk_combo(ctx, _wallet_addresses, wallet_count, wallet_address_index, 20, nk_vec2(WINDOW_WIDTH/3, WINDOW_HEIGHT/3));

                            nk_layout_row_dynamic(ctx, 20, 2);
                            nk_label_colored(ctx, "RECEIVER:", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                            nk_edit_string_zero_terminated (ctx, NK_EDIT_FIELD, transaction_receiver_buf, sizeof(transaction_receiver_buf) - 1, nk_filter_default);
                            
                            nk_layout_row_dynamic(ctx, 20, 2);
                            nk_label_colored(ctx, "AMOUNT:", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 255));
                            nk_edit_string_zero_terminated (ctx, NK_EDIT_FIELD, transaction_amount_buf, sizeof(transaction_amount_buf) - 1, nk_filter_default);

                            nk_layout_row_dynamic(ctx, WINDOW_HEIGHT/9, 3);
                            nk_label(ctx, "", NK_TEXT_ALIGN_CENTERED);
                            if (nk_button_label(ctx, "SUBMIT"))
                            {
                                _transaction_amount = strtod(transaction_amount_buf, NULL);
                                if(address_is_valid(transaction_receiver_buf) && (_transaction_amount > 0))
                                {   
                                    /* create transaction */
                                    _transaction->sender = wallet_addresses[wallet_address_index];
                                    _transaction->receiver = transaction_receiver_buf;
                                    _transaction->amount = _transaction_amount;
                                    _transaction->timestamp = (unsigned long int) time(NULL);

                                    /* send transaction */
                                    if(send_transaction(client, _transaction))
                                    {
                                        transaction_section = transaction_failure;
                                    }
                                    else
                                    {
                                        transaction_section = transaction_success;
                                    }
                                }
                                else
                                {
                                    transaction_section = transaction_failure;
                                }
                            }
                            nk_label(ctx, "", NK_TEXT_ALIGN_CENTERED);
                            break;
                        case transaction_success:
                            nk_layout_row_dynamic(ctx, 30, 1);
                            nk_label_colored(ctx, "TRANSACTION SEND SUCCESSFULLY!", NK_TEXT_ALIGN_CENTERED, nk_rgb(0,255,0));

                            nk_layout_row_dynamic(ctx, WINDOW_HEIGHT/9, 1);
                            if (nk_button_label(ctx, "OK"))
                            {
                                strcpy(transaction_receiver_buf, "");
                                strcpy(transaction_amount_buf, "");
                                wallet_address_index = 0;
                                transaction_section = transaction_input;
                            }
                            break;
                        case transaction_failure:
                            nk_layout_row_dynamic(ctx, 30, 1);
                            nk_label_colored(ctx, "TRANSACTION FAILED!", NK_TEXT_ALIGN_CENTERED, nk_rgb(255,0,0));

                            nk_layout_row_dynamic(ctx, 30, 1);
                            if(address_is_valid(transaction_receiver_buf)){
                                if(_transaction_amount <= 0)
                                {
                                    nk_label(ctx, "Amount <= 0!", NK_TEXT_ALIGN_CENTERED);
                                }
                                else
                                {
                                    nk_label(ctx, "Balance Insufficient!", NK_TEXT_ALIGN_CENTERED);
                                }
                            }
                            else{
                                nk_label(ctx, "Receiver Address Invalid!", NK_TEXT_ALIGN_CENTERED);
                            }

                            nk_layout_row_dynamic(ctx, WINDOW_HEIGHT/9, 1);
                            if (nk_button_label(ctx, "OK"))
                            {
                                strcpy(transaction_receiver_buf, "");
                                strcpy(transaction_amount_buf, "");
                                wallet_address_index = 0;
                                transaction_section = transaction_input;
                            }
                            break;
                    }
                    break;
            }
        }
        nk_end(ctx);
        
        /* Recent Transactions */
        if (nk_begin(ctx, "transactions_title", nk_rect(WINDOW_WIDTH*2/3, 0, WINDOW_WIDTH/3, WINDOW_HEIGHT/12), NK_WINDOW_BORDER))
        {
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "RECENT TRANSACTIONS", NK_TEXT_ALIGN_CENTERED);
        }
        nk_end(ctx);

        if (nk_begin(ctx, "transactions", nk_rect(WINDOW_WIDTH*2/3, WINDOW_HEIGHT/12, WINDOW_WIDTH/3, WINDOW_HEIGHT*9/12), NK_WINDOW_BORDER))
        {
            for (int i = 0; i < transaction_count; i++)
            {
                // transaction participants
                nk_layout_row_dynamic(ctx, 30, 3);
                if(transaction_direction[i])
                {
                    //receiver button
                    snprintf(receiver_label, 7, "%s", transaction_receiver[i]);
                    strcat(receiver_label, "...");
                    show_receiver = 0;
                    nk_button_set_behavior(ctx, NK_BUTTON_REPEATER);
                    if(nk_button_label(ctx, receiver_label))
                    {
                        show_receiver = 1;
                    }
                    // arrow
                    nk_label_colored(ctx, "<-", NK_TEXT_ALIGN_CENTERED, nk_rgb(0, 255, 0));
                    // sender button
                    snprintf(sender_label, 7, "%s...", transaction_sender[i]);
                    strcat(sender_label, "...");
                    show_sender = 0;
                    nk_button_set_behavior(ctx, NK_BUTTON_REPEATER);
                    if(nk_button_label(ctx, sender_label))
                    {
                        show_sender = 1;
                    }
                    // show complete sender-receiver info
                    if(show_receiver)
                    {
                        nk_layout_row_dynamic(ctx, 30, 1);
                        nk_text(ctx, transaction_receiver[i], strlen(transaction_receiver[i]), NK_TEXT_ALIGN_CENTERED);
                    }
                    else if(show_sender)
                    {
                        nk_layout_row_dynamic(ctx, 30, 1);
                        nk_text(ctx, transaction_sender[i], strlen(transaction_sender[i]), NK_TEXT_ALIGN_CENTERED);
                    }
                }
                else
                {
                    // sender button
                    snprintf(sender_label, 7, "%s...", transaction_sender[i]);
                    strcat(sender_label, "...");
                    show_sender = 0;
                    nk_button_set_behavior(ctx, NK_BUTTON_REPEATER);
                    if(nk_button_label(ctx, sender_label))
                    {
                       show_sender = 1;
                    }   
                    // arrow
                    nk_label_colored(ctx, "->", NK_TEXT_ALIGN_CENTERED, nk_rgb(255, 0, 0));
                    //receiver button
                    snprintf(receiver_label, 7, "%s", transaction_receiver[i]);
                    strcat(receiver_label, "...");
                    show_receiver = 0;
                    nk_button_set_behavior(ctx, NK_BUTTON_REPEATER);
                    if(nk_button_label(ctx, receiver_label))
                    {
                        show_receiver = 1;
                    }
                    // show complete sender-receiver info
                    if(show_receiver)
                    {
                        nk_layout_row_dynamic(ctx, 30, 1);
                        nk_text(ctx, transaction_receiver[i], strlen(transaction_receiver[i]), NK_TEXT_ALIGN_CENTERED);
                    }
                    else if(show_sender)
                    {
                        nk_layout_row_dynamic(ctx, 30, 1);
                        nk_text(ctx, transaction_sender[i], strlen(transaction_sender[i]), NK_TEXT_ALIGN_CENTERED);
                    }
                }

                nk_layout_row_dynamic(ctx, 30, 2);
                // amount
                nk_text(ctx, transaction_amount[i], strlen(transaction_amount[i]), NK_TEXT_ALIGN_CENTERED);
                // time
                nk_text(ctx, transaction_time[i], strlen(transaction_time[i]), NK_TEXT_ALIGN_CENTERED);

                // status
                nk_layout_row_dynamic(ctx, 30, 1);
                if(transaction_status[i])
                {
                    nk_label(ctx, "CONFIRMED", NK_TEXT_ALIGN_CENTERED);
                }
                else
                {
                    nk_label(ctx, "UNCONFIRMED", NK_TEXT_ALIGN_CENTERED);
                }

                // empty row
                nk_layout_row_dynamic(ctx, 10, 1);
                nk_text(ctx, "", 0, NK_TEXT_ALIGN_CENTERED);
            }
        }
        nk_end(ctx);

        /* Connection Status */
        if (nk_begin(ctx, "status", nk_rect(WINDOW_WIDTH*2/3, WINDOW_HEIGHT*5/6, WINDOW_WIDTH/3, WINDOW_HEIGHT/6), NK_WINDOW_BORDER))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            if(connected)
            {
                nk_label_colored(ctx, "Connected", NK_TEXT_ALIGN_CENTERED, nk_rgb(0,255,0));
            }
            else
            {
                nk_label_colored(ctx, "Disconnected", NK_TEXT_ALIGN_CENTERED, nk_rgb(255,0,0));
            }

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, connections, NK_TEXT_ALIGN_CENTERED);
        }
        nk_end(ctx);

        /* Draw */
        XGetWindowAttributes(win.dpy, win.win, &win.attr);
        glViewport(0, 0, win.width, win.height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(bg.r, bg.g, bg.b, bg.a);
        /* IMPORTANT: `nk_x11_render` modifies some global OpenGL state
         * with blending, scissor, face culling, depth test and viewport and
         * defaults everything back into a default state.
         * Make sure to either a.) save and restore or b.) reset your own state after
         * rendering the UI. */
        nk_x11_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
        glXSwapBuffers(win.dpy, win.win);
    }

cleanup:
    nk_x11_shutdown();
    glXMakeCurrent(win.dpy, 0, 0);
    glXDestroyContext(win.dpy, glContext);
    XUnmapWindow(win.dpy, win.win);
    XFreeColormap(win.dpy, win.cmap);
    XDestroyWindow(win.dpy, win.win);
    XCloseDisplay(win.dpy);
    return 0;

}