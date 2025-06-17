/* Parallel Mandlebrot program */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>

#define X_RESN 15000 /* x resolution */
#define Y_RESN 15000 /* y resolution */
#define DEFAULT_OMP_THREADS 1

typedef struct complextype
{
    float real, imag;
} Compl;

void main(int argc, char *argv[])
{
    Window win;                        /* initialization for a window */
    unsigned int width, height,        /* window size */
        x, y,                          /* window position */
        border_width,                  /*border width in pixels */
        display_width, display_height, /* size of screen */
        screen;                        /* which screen */

    char *window_name = "Mandelbrot Set", *display_name = NULL;
    GC gc;
    unsigned long valuemask = 0;
    XGCValues values;
    Display *display;
    XSizeHints size_hints;
    Pixmap bitmap;
    XPoint points[800];
    FILE *fp, *fopen();
    char str[100];

    XSetWindowAttributes attr[1];

    // int pixel_matrix[X_RESN][Y_RESN] = {0};

    /* Mandlebrot variables */
    int i, j, k;
    Compl z, c;
    float lengthsq, temp;

    /* connect to Xserver */
    if ((display = XOpenDisplay(display_name)) == NULL)
    {
        fprintf(stderr, "drawon: cannot connect to X server %s\n",
                XDisplayName(display_name));
        exit(-1);
    }

    /* get screen size */

    screen = DefaultScreen(display);
    display_width = DisplayWidth(display, screen);
    display_height = DisplayHeight(display, screen);

    /* set window size */

    width = X_RESN;
    height = Y_RESN;

    /* set window position */

    x = 0;
    y = 0;

    /* create opaque window */

    border_width = 4;
    win = XCreateSimpleWindow(display, RootWindow(display, screen),
                              x, y, width, height, border_width,
                              BlackPixel(display, screen), WhitePixel(display, screen));

    size_hints.flags = USPosition | USSize;
    size_hints.x = x;
    size_hints.y = y;
    size_hints.width = width;
    size_hints.height = height;
    size_hints.min_width = 300;
    size_hints.min_height = 300;

    XSetNormalHints(display, win, &size_hints);
    XStoreName(display, win, window_name);

    /* create graphics context */

    gc = XCreateGC(display, win, valuemask, &values);

    XColor green_color, red_color;
    Colormap colormap = DefaultColormap(display, screen);

    green_color.red = 0;
    green_color.green = 65535;
    green_color.blue = 0;
    green_color.flags = DoRed | DoGreen | DoBlue;
    XAllocColor(display, colormap, &green_color);

    red_color.red = 65535;
    red_color.green = 0;
    red_color.blue = 0;
    red_color.flags = DoRed | DoGreen | DoBlue;
    XAllocColor(display, colormap, &red_color);

    // XSetBackground(display, gc, WhitePixel(display, screen));
    // XSetForeground(display, gc, BlackPixel(display, screen));
    XSetBackground(display, gc, green_color.pixel);
    XSetForeground(display, gc, red_color.pixel);
    XSetLineAttributes(display, gc, 1, LineSolid, CapRound, JoinRound);

    win = XCreateSimpleWindow(display, RootWindow(display, screen),
                          x, y, width, height, border_width,
                          BlackPixel(display, screen), green_color.pixel);

    attr[0].backing_store = Always;
    attr[0].backing_planes = 1;
    attr[0].backing_pixel = green_color.pixel;

    XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    XMapWindow(display, win);
    XSync(display, 0);

    sleep(1);
    /* Calculate and draw points */

    int omp_threads = DEFAULT_OMP_THREADS;
    if (argc > 1)
    {
        omp_threads = atoi(argv[1]);
        if (omp_threads < 1)
            omp_threads = DEFAULT_OMP_THREADS;
    }

    omp_set_num_threads(omp_threads);

    double start_time, end_time;
    start_time = omp_get_wtime();

    // int pixels[X_RESN][Y_RESN] = {0};

    #pragma omp parallel for collapse(2) shared(display, win, gc) private(z, c, k, temp, lengthsq)
    for (i = 0; i < X_RESN; i++)
        for (j = 0; j < Y_RESN; j++)
        {

            z.real = z.imag = 0.0;
            c.real = ((float) j - (X_RESN / 2.0))/ (X_RESN / 4.0);               /* scale factors for 800 x 800 window */
			c.imag = ((float) i - (Y_RESN / 2.0))/(Y_RESN / 4.0);
            k = 0;

            do
            { /* iterate for pixel color */
                temp = z.real * z.real - z.imag * z.imag + c.real;
                z.imag = 2.0 * z.real * z.imag + c.imag;
                z.real = temp;
                lengthsq = z.real * z.real + z.imag * z.imag;
                k++;

            } while (lengthsq < 4.0 && k < 100);

            if(k == 100){
                #pragma omp critical
                {
                    XDrawPoint(display, win, gc, j, i);
                }
            }
        }

    end_time = omp_get_wtime();
    printf("Elapsed time: %f seconds\n", end_time - start_time);

    XFlush(display);

    getchar();
    // sleep(30);

    /* Program Finished */
}