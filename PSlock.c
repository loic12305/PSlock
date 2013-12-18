/* See LICENSE file for license details. */
#define _XOPEN_SOURCE 500
#if HAVE_SHADOW_H
#include <shadow.h>
#endif

#include <ctype.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/dpms.h>

#include <X11/xpm.h> //to load xpm
#include <X11/extensions/shape.h> //transparent background
#include <time.h> //alternative to sleep
//conf file
#define RCFILE ".pslockrc"

/* XPM */
#include "myimage.xpm"







#if HAVE_BSD_AUTH
#include <login_cap.h>
#include <bsd_auth.h>
#endif

static void
die(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

#ifndef HAVE_BSD_AUTH
static const char *
get_password() { /* only run as root */
	const char *rval;
	struct passwd *pw;

	if(geteuid() != 0)
		die("pupslock: cannot retrieve password entry (make sure to suid slock)\n");
	pw = getpwuid(getuid());
	endpwent();
	rval =  pw->pw_passwd;

#if HAVE_SHADOW_H
	{
		struct spwd *sp;
		sp = getspnam(getenv("USER"));
		endspent();
		rval = sp->sp_pwdp;
	}
#endif

	/* drop privileges */
	if(setgid(pw->pw_gid) < 0 || setuid(pw->pw_uid) < 0)
		die("pupslock: cannot drop privileges\n");
	return rval;
}
#endif

#define BUFFER_SIZE 64
char imagefile[BUFFER_SIZE - 1] = "";
char p[BUFFER_SIZE - 1] = "no";
int s = 14400; //, k = 0;
char bc[BUFFER_SIZE - 1] = "black";
char t[BUFFER_SIZE - 1] = "no";
char a[BUFFER_SIZE - 1] = "no";
char appname[BUFFER_SIZE - 1] = "pupslock";
Window y;
Display *dpy;
int pswd = 1;
int Xwidth=800, Xheight=600;
int Swidth=0, Sheight=0, testint=0;
time_t start,end;
char *lvalue;
		
///Rc file functions///
void parse_rc (char *dir, char *file) {
FILE *rc;
char *rc_file, buf[1024]; //, *lvalue;

if ((rc_file = malloc (strlen (dir) + strlen (file) + 2)) == NULL) {
	printf ("invalid rcfile\n");
	return;
}

sprintf (rc_file, "%s/%s", dir, file);

if ((rc = fopen (rc_file, "r")) != (FILE *)0) {
	while((fgets (buf, sizeof buf, rc)) != (char *)0 ) {
		lvalue = strtok (buf, "\n");
	} 
} else {
	printf("*** Running without rcfile ***\n");
	return;
} 
	
fclose (rc);
free (rc_file);

int lvalue_length=strlen(lvalue);
char *rvalue = NULL;
rvalue = strtok (lvalue, " ");
	
while (lvalue_length >= 0) {
	if (!strcmp(rvalue, "-pu")) {
		rvalue = strtok (NULL, " "); 
		strcpy (imagefile, rvalue);
		imagefile[BUFFER_SIZE - 1] = '\0';
		lvalue_length=lvalue_length-strlen(rvalue)-1;
		
	} else if (!strcmp(rvalue, "-p")) {
		strcpy (p, "yes");
		p[BUFFER_SIZE - 1] = '\0'; 
						
	} else if (!strcmp(rvalue, "-s")) {
		rvalue = strtok (NULL, " "); 
		s = atoi(rvalue);
		lvalue_length=lvalue_length-strlen(rvalue)-1;
				
	} else if (!strcmp(rvalue, "-bc")) {
		rvalue = strtok (NULL, " "); 
		strcpy (bc, rvalue);
		bc[BUFFER_SIZE - 1] = '\0'; 
		lvalue_length=lvalue_length-strlen(rvalue)-1;
						
	} else if (!strcmp(rvalue, "-t")) {
		strcpy (t, "yes");
		t[BUFFER_SIZE - 1] = '\0';
		
	} else {
		if (!strcmp(rvalue, "-a") ) {
			strcpy (a, "yes");
			a[BUFFER_SIZE - 1] = '\0';
		}
	}
		
	if ((rvalue = strtok (NULL, " "))) {
		lvalue_length=lvalue_length-strlen(rvalue)-1;
		continue;
	} else {
		break;
	}

} //end while

}

///Execute command///
void fork_exec (char *cmd) {
pid_t pid = fork ();
switch (pid) {
	case 0:
    	execlp ("/bin/sh", "sh", "-c", cmd, NULL);
    	fprintf (stderr, "Exec failed.\n");
    	exit (0);
    	break;
	case -1:
    	fprintf (stderr, "Fork failed.\n");
    	break;
}
}

///main program///
int
main(int argc, char **argv) {
	char curs[] = {0, 0, 0, 0, 0, 0, 0, 0};
	char buf[32], passwd[256];
	int num, screen, i;

#ifndef HAVE_BSD_AUTH
	const char *pws;
#endif
	unsigned int len;
	Bool running = True;
	Cursor invisible;
	KeySym ksym;
	Pixmap pmap;
	Window root, w;
	XColor xc, dummy;
	XEvent ev;
	XSetWindowAttributes wa;
		
	//if conf file present
	parse_rc (getenv ("HOME"), RCFILE);
			
	//taking in args from commandline
	for(i=1; i <argc; i++) {
		if (!strcmp(argv[i], "-pu") && (argc > i+1)) {
		strcpy (imagefile, argv[++i]);
		imagefile[BUFFER_SIZE - 1] = '\0';
				
		} else if (!strcmp(argv[i], "-p") ) {
		strcpy (p, "yes");
		p[BUFFER_SIZE - 1] = '\0'; 
						    
		} else if (!strcmp(argv[i], "-s") && (argc > i+1)) {
		s = atoi(argv[++i]);
				      
		} else if (!strcmp(argv[i], "-bc") && (argc > i+1)) {
		strcpy (bc, argv[++i]);
		bc[BUFFER_SIZE - 1] = '\0'; 
      
		} else if (!strcmp(argv[i], "-t") ) {
		strcpy (t, "yes");
		t[BUFFER_SIZE - 1] = '\0';
		
		} else if (!strcmp(argv[i], "-a") ) {
		strcpy (a, "yes");
		a[BUFFER_SIZE - 1] = '\0';
      
		} else {
		if (!(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))) {
		printf("%s: bad option or missing argument.\n",argv[i]);
		return 0;
		}
		printf("%s-%s - a modified slock by goingnuts.dk\n"
		"Options:\n"
		"-pu \"path to image\"     User defined image. Only xpm-image!\n"
		"-p                      Use build in image.\n"
		"-s  XX                  Activate screen saver function. XX seconds delay.\n"
		"-bc \"background color\"  Ex. \"#fffacd\" or steelblue.\n"
		"-t                      Use transparent image background if appropriate.\n"
		"-a                      Disable ctrl+alt+backspace (needs xmodmap).\n"
		"-h,  --help             Display this help text and exit.\n"
		"\n", appname, VERSION);
		return 0;
		}
	}

#ifndef HAVE_BSD_AUTH
	pws = get_password();
#endif

	if(!(dpy = XOpenDisplay(0)))
		die("pupslock: cannot open display\n");
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	Xwidth=DisplayWidth(dpy, screen);
	Xheight=DisplayHeight(dpy, screen);

	//init
	wa.override_redirect = 1;
	XAllocNamedColor(dpy, DefaultColormap(dpy, screen), bc, &xc, &dummy);
	wa.background_pixel = xc.pixel;
	w = XCreateWindow(dpy, root, 0, 0,
			Xwidth ,Xheight,
			0, DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen), CWOverrideRedirect | CWBackPixel, &wa);
	
	//xpmimage
	if ( ! strcmp(imagefile, "") == 0 || strcmp(p, "yes") == 0) {
		XpmAttributes xpm_attr;
		Pixmap pixmap, mask;
		if ( ! XpmReadFileToPixmap (dpy, root, imagefile, &pixmap, &mask, &xpm_attr) == 0) {
			XpmCreatePixmapFromData(dpy,root,image_xpm,&pixmap,&mask,&xpm_attr);
		}
	
		Swidth=(Xwidth-xpm_attr.width);
		Sheight=(Xheight-xpm_attr.height);
	
		y = XCreateWindow(dpy, w,
	        ((Xwidth-xpm_attr.width)/2),
	        ((Xheight-xpm_attr.height)/2),
	        xpm_attr.width, xpm_attr.height,
			0, DefaultDepth(dpy, screen), CopyFromParent,
			DefaultVisual(dpy, screen), CWOverrideRedirect | CWBackPixel, &wa);
			XSetWindowBackgroundPixmap (dpy, y, pixmap);
				if ( strcmp(t, "yes") == 0 ) {
				XShapeCombineMask (dpy, y, ShapeBounding, 0, 0, mask, ShapeSet);
				}
			XFreePixmap(dpy, pixmap);
			XMapWindow(dpy, y);
			
	}			
	
	XAllocNamedColor(dpy, DefaultColormap(dpy, screen), bc, &xc, &dummy);
	pmap = XCreateBitmapFromData(dpy, w, curs, 8, 8);
	invisible = XCreatePixmapCursor(dpy, pmap, pmap, &xc, &xc, 0, 0);
	XDefineCursor(dpy, w, invisible);
	XFreePixmap(dpy, pmap);
	
	XMapWindow(dpy, w);
	
	XFlush(dpy);
		
	for(len = 1000; len; len--) {
		if(XGrabPointer(dpy, root, False, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		GrabModeAsync, GrabModeAsync, None, invisible, CurrentTime) == GrabSuccess)
		break;
			
		usleep(1000);
		
	}
	if((running = running && (len > 0))) {
		for(len = 1000; len; len--) {
			if(XGrabKeyboard(dpy, root, True, GrabModeAsync, GrabModeAsync, CurrentTime)
				== GrabSuccess)
				break;
			usleep(1000);
			}
		running = (len > 0);
		}
		
	//disable ctrl+alt+backspace and friends
	if ( strcmp(a, "yes") == 0 ) {
		fork_exec ("xmodmap -e 'keycode 37='");
   		fork_exec ("xmodmap -e 'keycode 109='");
	}
	
	len = 0;
	XSync(dpy, False);
	if ( ! strcmp(imagefile, "" ) == 0 || strcmp(p, "yes") == 0 ) {	
		time(&start);	
	}
	
/* main event loop */
while (1) {
		
	if ( ! strcmp(imagefile, "" ) == 0 || strcmp(p, "yes") == 0 ) {	
		while (!XPending (dpy)) {
			if ( testint > s) { 
				time(&start);
				testint = 0;
				XMoveWindow(dpy, y, rand() % Swidth + 1, rand() % Sheight +1);
			} else {
				time(&end);
				testint = difftime(end, start);
			}
       		usleep(1000);
		}
	}
    
	while (pswd && running && !XNextEvent(dpy, &ev)) {
		if(len == 0 && DPMSCapable(dpy)) {
			DPMSEnable(dpy);
			DPMSForceLevel(dpy, DPMSModeOff);
		}
		if(ev.type == KeyPress) {
			buf[0] = 0;
			num = XLookupString(&ev.xkey, buf, sizeof buf, &ksym, 0);
			if(IsKeypadKey(ksym)) {
				if(ksym == XK_KP_Enter) {
					ksym = XK_Return;
				} else {
					if(ksym >= XK_KP_0 && ksym <= XK_KP_9) {
						ksym = (ksym - XK_KP_0) + XK_0;
					}
				}
			}
			if(IsFunctionKey(ksym) || IsKeypadKey(ksym)
					|| IsMiscFunctionKey(ksym) || IsPFKey(ksym)
					|| IsPrivateKeypadKey(ksym)) {
				continue;
			}
			switch(ksym) {
			case XK_Return:
				passwd[len] = 0;
				running = strcmp(crypt(passwd, pws), pws);
				if (running != 0) {
					XBell(dpy, 100);
					pswd = 0;	//break loop
				}
				len = 0;
				break;
			case XK_Escape:
				len = 0;
				break;
			case XK_BackSpace:
				if(len)
					--len;
				break;
			default:
				if(num && !iscntrl((int) buf[0]) && (len + num < sizeof passwd)) { 
					memcpy(passwd + len, buf, num);
					len += num;
				}
				break;
			}
		} else {
			if ( ! strcmp(imagefile, "" ) == 0 || strcmp(p, "yes") == 0 ) {	
				pswd = 0;	//break loop
		   	} else {
		   		continue;
			}
		}
	}
    
    if ( ! strcmp(imagefile, "" ) == 0 || strcmp(p, "yes") == 0 ) {	
   		pswd = 1;	//reset loop
	}
    
    if (running == 0) {
		if ( strcmp(a, "yes") == 0 ) {
			//re-enable ctrl+alt+backspace and friends again
			fork_exec ("xmodmap -e 'keycode 37=Control_L'");
			fork_exec ("xmodmap -e 'keycode 109=Control_R'");
		}
		XUngrabPointer(dpy, CurrentTime);
		XDestroyWindow(dpy, w);
		XCloseDisplay(dpy);
		break;
	} 
}
return 0;
}	//end main loop
