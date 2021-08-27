/* See LICENSE file for copyright and license details
 * Anyways, this software is released under the MIT license
 *
 * clarawm is designed to be a simple tiling window manager,
 * fast and with a light and simple code base, so that
 * anyone can understand the code.
 */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define tiling 1
#define floating 0
typedef int mode;

Display *dpy;
Screen *screen;
GC gc;

Window root, menu;
XWindowAttributes attr;

XKeyEvent key;
XButtonEvent start;
XEvent ev;

mode wm_mode = tiling; /* default mode is tiling */

static char *dmenu[] = {"dmenu_run", NULL};
static char *xterm[] = {"xterm", NULL};

void spawn(char **args)
{
    if (fork() == 0)
    {
        setsid();
        execvp(args[0], args);
        exit(EXIT_SUCCESS);
    }
}

void kill_win(Window w)
{
    XEvent ke;
    ke.type = ClientMessage;
    ke.xclient.window = w;
    ke.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
    ke.xclient.format = 32;
    ke.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
    ke.xclient.data.l[1] = CurrentTime;
    XSendEvent(dpy, w, False, NoEventMask, &ke);
}

void die(char *error)
{
    fprintf(stderr, "clarawm: %s", error);
    exit(EXIT_FAILURE);
}

XColor color;
void set_color(char *hex)
{
    Colormap colormap = DefaultColormap(dpy, 0);
    XParseColor(dpy, colormap, hex, &color);
    XAllocColor(dpy, colormap, &color);
}

void draw_menu(void)
{
    set_color("#1b496e");
    menu = XCreateSimpleWindow(dpy, root,
                               0, 0, screen->width, 16,
                               0, 0, color.pixel);
    XMapWindow(dpy, menu);
    XFlush(dpy);
}

void draw_text(const char *text, int x, int y)
{
    set_color("#ffffff");
    XSetForeground(dpy, gc, color.pixel);
    XSelectInput(dpy, menu, ExposureMask);

    Font font = XLoadFont(dpy, "fixed");
    XSetFont(dpy, gc, font);
    XDrawString(dpy, menu, gc, x, y, text, strlen(text));
}

void tile()
{
    unsigned int n;
    Window d1, d2, *wins = NULL;
    XQueryTree(dpy, root, &d1, &d2, &wins, &n);
    for (int i = 0; i < n; i++)
    {
        if (wins[i] != menu)
            XMoveResizeWindow(dpy, wins[i],
                              (n == 2) ? 0 : screen->width / (n - 1) * i, 16,
                              (n == 2) ? screen->width : screen->width / (n - 1), screen->height - 16);
    }
}

int main(void)
{
    if (!(dpy = XOpenDisplay(NULL)))
        die("cannot open display");

    gc = DefaultGC(dpy, 0);
    root = DefaultRootWindow(dpy);
    screen = DefaultScreenOfDisplay(dpy);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_Return), Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_D), Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_Q), Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_K), Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_M), Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabButton(dpy, AnyButton, Mod4Mask, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask | OwnerGrabButtonMask,
                GrabModeAsync, GrabModeAsync, None, None);

    start.subwindow = None;
    draw_menu();
    for (;;)
    {
        if (wm_mode == tiling)
            tile();

        XDestroyWindow(dpy, menu);
        draw_menu();

        char buff[12];
        sprintf(buff, "clarawm %s", (wm_mode == tiling) ? "[]=" : "<><");
        draw_text(buff, 4, 12);

        time_t now = time(NULL);
        struct tm *time = localtime(&now);
        sprintf(buff, "%d:%02d:%02d", time->tm_hour, time->tm_min, time->tm_sec);
        draw_text(buff, screen->width - 52, 12); /* only updates on events */

        XNextEvent(dpy, &ev);
        if (ev.type == KeyPress)
        {
            key = ev.xkey;
            if (key.keycode == XKeysymToKeycode(dpy, XK_Return))
                spawn(xterm);
            else if (key.keycode == XKeysymToKeycode(dpy, XK_D))
                spawn(dmenu);
            else if (key.keycode == XKeysymToKeycode(dpy, XK_Q))
            {
                if (ev.xbutton.subwindow != None && ev.xbutton.subwindow != menu)
                    kill_win(ev.xbutton.subwindow);
            }
            else if (key.keycode == XKeysymToKeycode(dpy, XK_K))
                break;
            else if (key.keycode == XKeysymToKeycode(dpy, XK_M))
                wm_mode = (wm_mode == tiling) ? floating : tiling;
        }
        if (wm_mode == floating)
        {
            if (ev.type == ButtonPress && ev.xbutton.subwindow != None)
            {
                start = ev.xbutton;
                XRaiseWindow(dpy, start.subwindow);
                XGetWindowAttributes(dpy, start.subwindow, &attr);
            }
            else if (ev.type == MotionNotify && start.subwindow != None && start.subwindow != menu)
            {
                int xdiff = ev.xbutton.x_root - start.x_root;
                int ydiff = ev.xbutton.y_root - start.y_root;
                XMoveResizeWindow(dpy, start.subwindow,
                                  attr.x + (start.button == 1 ? xdiff : 0),
                                  attr.y + (start.button == 1 ? ydiff : 0),
                                  MAX(1, attr.width + (start.button == 3 ? xdiff : 0)),
                                  MAX(1, attr.height + (start.button == 3 ? ydiff : 0)));
            }
            else if (ev.type == ButtonRelease)
                start.subwindow = None;
        }
    }
    XCloseDisplay(dpy);
    return 0;
}