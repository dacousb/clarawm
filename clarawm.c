/* See LICENSE file for copyright and license details
 * Anyways, this software is released under the MIT license
 *
 * clarawm is designed to be a simple floating window manager,
 * fast and with a light and simple code base, so that
 * anyone can understand the code.
 */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

Display *dpy;
Screen *screen;

Window root;
XWindowAttributes attr;

XKeyEvent key;
XButtonEvent start;
XEvent ev;

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

void borders()
{
    unsigned int n;
    Window d1, d2, *wins = NULL;
    XQueryTree(dpy, root, &d1, &d2, &wins, &n);
    for (int i = 0; i < n; i++)
    {
        set_color((wins[i] == start.subwindow) ? "#1b496e" : "#7a7a7a");
        XSetWindowBorder(dpy, wins[i], color.pixel);
    }
    if (wins)
        XFree(wins);
}

int main(void)
{
    if (!(dpy = XOpenDisplay(NULL)))
        die("cannot open display");

    root = DefaultRootWindow(dpy);
    screen = DefaultScreenOfDisplay(dpy);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_Return), Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_D), Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_Q), Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_K), Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabButton(dpy, AnyButton, Mod4Mask, root, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask | OwnerGrabButtonMask,
                GrabModeAsync, GrabModeAsync, None, None);

    start.subwindow = None;
    for (;;)
    {
        borders();
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
                if (ev.xbutton.subwindow != None)
                    kill_win(ev.xbutton.subwindow);
            }
            else if (key.keycode == XKeysymToKeycode(dpy, XK_K))
                break;
        }
        else if (ev.type == ButtonPress && ev.xbutton.subwindow != None)
        {
            start = ev.xbutton;
            XRaiseWindow(dpy, start.subwindow);
            XGetWindowAttributes(dpy, start.subwindow, &attr);
        }
        else if (ev.type == MotionNotify && start.subwindow != None)
        {
            if (ev.xbutton.y_root < 20) /*U*/
                XMoveResizeWindow(dpy, start.subwindow,
                                  0, 0,
                                  screen->width, screen->height / 2);
            else if (ev.xbutton.y_root > screen->height - 20) /*U*/
                XMoveResizeWindow(dpy, start.subwindow,
                                  0, screen->height / 2,
                                  screen->width, screen->height / 2);
            else if (ev.xbutton.x_root < 20) /*L*/
                XMoveResizeWindow(dpy, start.subwindow,
                                  0, 0,
                                  screen->width / 2, screen->height);
            else if (ev.xbutton.x_root > screen->width - 20) /*R*/
                XMoveResizeWindow(dpy, start.subwindow,
                                  screen->width / 2, 0,
                                  screen->width / 2, screen->height);
            else
            {
                int xdiff = ev.xbutton.x_root - start.x_root;
                int ydiff = ev.xbutton.y_root - start.y_root;
                XMoveResizeWindow(dpy, start.subwindow,
                                  attr.x + (start.button == 1 ? xdiff : 0),
                                  attr.y + (start.button == 1 ? ydiff : 0),
                                  MAX(1, attr.width + (start.button == 3 ? xdiff : 0)),
                                  MAX(1, attr.height + (start.button == 3 ? ydiff : 0)));
            }
        }
        else if (ev.type == ButtonRelease)
            start.subwindow = None;
    }
    XCloseDisplay(dpy);
    return 0;
}