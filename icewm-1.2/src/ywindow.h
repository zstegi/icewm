#ifndef __YWINDOW_H
#define __YWINDOW_H

#include "ypaint.h"
#include "ycursor.h"

class YPopupWindow;
class YToolTip;
class YTimer;
class AutoScroll;

#ifdef CONFIG_GRADIENTS
#define INIT_GRADIENT(Member, Value) , Member(Value)
#else
#define INIT_GRADIENT(Member, Value)
#endif

class YWindowAttributes {
public:
    YWindowAttributes(Window window);
    
    Window root() const { return attributes.root; }
    int x() const { return attributes.x; }
    int y() const { return attributes.y; }
    unsigned width() const { return attributes.width; }
    unsigned height() const { return attributes.height; }
    unsigned border() const { return attributes.border_width; }
    unsigned depth() const { return attributes.depth; }
    Visual * visual() const { return attributes.visual; }
    Colormap colormap() const { return attributes.colormap; }

private:
    XWindowAttributes attributes;
};

class YWindow {
public:
    YWindow(YWindow *aParent = 0, Window win = 0);
    virtual ~YWindow();

    void setStyle(unsigned long aStyle);

    void show();
    void hide();
    virtual void raise();
    virtual void lower();

    void repaint();
    void repaintFocus();

    void reparent(YWindow *parent, int x, int y);

    void setWindowFocus();
    
    void setTitle(char const * title);
    void setClassHint(char const * rName, char const * rClass);

    void setGeometry(int x, int y, unsigned width, unsigned height);
    void setSize(unsigned width, unsigned height);
    void setPosition(int x, int y);
    virtual void configure(const int x, const int y, 
    			   const unsigned width, const unsigned height,
			   const bool resized);

    virtual void paint(Graphics &g, int x, int y, unsigned width, unsigned height);
    virtual void paintFocus(Graphics &, int, int, unsigned, unsigned) {}

    virtual void handleEvent(const XEvent &event);

    virtual void handleExpose(const XExposeEvent &expose);
    virtual void handleGraphicsExpose(const XGraphicsExposeEvent &graphicsExpose);
    virtual void handleConfigure(const XConfigureEvent &configure);
    virtual bool handleKey(const XKeyEvent &key);
    virtual void handleButton(const XButtonEvent &button);
    virtual void handleMotion(const XMotionEvent &motion);
    virtual void handleCrossing(const XCrossingEvent &crossing);
    virtual void handleProperty(const XPropertyEvent &) {}
    virtual void handleColormap(const XColormapEvent &) {}
    virtual void handleFocus(const XFocusChangeEvent &) {}
    virtual void handleClientMessage(const XClientMessageEvent &message);
    virtual void handleSelectionClear(const XSelectionClearEvent &clear);
    virtual void handleSelectionRequest(const XSelectionRequestEvent &request);
    virtual void handleSelection(const XSelectionEvent &selection);
#if 0
    virtual void handleVisibility(const XVisibilityEvent &visibility);
    virtual void handleCreateWindow(const XCreateWindowEvent &createWindow);
#endif
    virtual void handleMap(const XMapEvent &map);
    virtual void handleUnmap(const XUnmapEvent &unmap);
    virtual void handleDestroyWindow(const XDestroyWindowEvent &destroyWindow);
    virtual void handleReparentNotify(const XReparentEvent &) {}
    virtual void handleConfigureRequest(const XConfigureRequestEvent &) {}
    virtual void handleMapRequest(const XMapRequestEvent &) {}
#ifdef CONFIG_SHAPE
    virtual void handleShapeNotify(const XShapeEvent &) {}
#endif

    virtual void handleClickDown(const XButtonEvent &, int) {}
    virtual void handleClick(const XButtonEvent &, int) {}
    virtual void handleBeginDrag(const XButtonEvent &, const XMotionEvent &) {}
    virtual void handleDrag(const XButtonEvent &, const XMotionEvent &) {}
    virtual void handleEndDrag(const XButtonEvent &, const XButtonEvent &) {}

    virtual void handleClose();

    virtual bool handleAutoScroll(const XMotionEvent &mouse);
    void beginAutoScroll(bool autoScroll, const XMotionEvent *motion);

    void setPointer(const YCursor& pointer);
    void setGrabPointer(const YCursor& pointer);
    void grabKeyM(int key, unsigned modifiers);
    void grabKey(int key, unsigned modifiers);
    void grabVKey(int key, unsigned vmodifiers);
    unsigned VMod(int modifiers);
    void grabButtonM(int button, unsigned modifiers);
    void grabButton(int button, unsigned modifiers);

    void captureEvents();
    void releaseEvents();

    Window handle();
    YWindow *parent() const { return fParentWindow; }

    Graphics & getGraphics();
#ifdef CONFIG_GRADIENTS
    virtual class YPixbuf * getGradient() const { 
	return (parent() ? parent()->getGradient() : NULL); }
#endif    

    int x() const { return fX; }
    int y() const { return fY; }
    unsigned width() const { return fWidth; }
    unsigned height() const { return fHeight; }

    bool visible() const { return (flags & wfVisible); }
    bool created() const { return (flags & wfCreated); }
    bool adopted() const { return (flags & wfAdopted); }
    bool destroyed() const { return (flags & wfDestroyed); }
    bool unmapped() const { return (flags & wfUnmapped); }

    static bool viewable(Drawable drawable);

    bool viewable() const { return viewable(fHandle); }

    virtual void donePopup(YPopupWindow * /*command*/);

    typedef enum {
        wsOverrideRedirect = 1 << 0,
        wsSaveUnder        = 1 << 1,
        wsManager          = 1 << 2,
        wsInputOnly        = 1 << 3,
        wsOutputOnly       = 1 << 4,
        wsPointerMotion    = 1 << 5
    } WindowStyle;

    virtual bool isFocusTraversable();
    bool isFocused();
    bool isEnabled() const { return fEnabled; }
    void setEnabled(bool enable);
    void nextFocus();
    void prevFocus();
    bool changeFocus(bool next);
    void requestFocus();
    void setFocus(YWindow *window);
    YWindow *getFocusWindow();
    virtual void gotFocus();
    virtual void lostFocus();

    bool isToplevel() const { return fToplevel; }
    void setToplevel(bool toplevel) { fToplevel = toplevel; }

    YWindow *toplevel();

    void installAccelerator(unsigned key, unsigned mod, YWindow *win);
    void removeAccelerator(unsigned key, unsigned mod, YWindow *win);

    void setToolTip(const char *tip);

    void mapToGlobal(int &x, int &y);
    void mapToLocal(int &x, int &y);

    void setWinGravity(int gravity);
    void setBitGravity(int gravity);

    void setDND(bool enabled);

    void XdndStatus(bool acceptDrop, Atom dropAction);
    virtual void handleXdnd(const XClientMessageEvent &message);

    virtual void handleDNDEnter();
    virtual void handleDNDLeave();
    virtual void handleDNDPosition(int x, int y);

    bool getCharFromEvent(const XKeyEvent &key, char *c);
    int getClickCount() { return fClickCount; }

    void scrollWindow(int dx, int dy);

    bool toolTipVisible();
    virtual void updateToolTip();

    void acquireSelection(bool selection);
    void clearSelection(bool selection);
    void requestSelection(bool selection);

private:
    typedef enum {
        wfVisible   = 1 << 0,
        wfCreated   = 1 << 1,
        wfAdopted   = 1 << 2,
        wfDestroyed = 1 << 3,
        wfUnmapped  = 1 << 4,
        wfNullSize  = 1 << 5
    } WindowFlags;

    void create();
    void destroy();

    void insertWindow();
    void removeWindow();

    bool nullGeometry();
    
    YWindow *fParentWindow;
    YWindow *fNextWindow;
    YWindow *fPrevWindow;
    YWindow *fFirstWindow;
    YWindow *fLastWindow;
    YWindow *fFocusedWindow;

    Window fHandle;
    unsigned long flags;
    unsigned long fStyle;
    int fX, fY;
    unsigned fWidth, fHeight;
    YCursor fPointer;
    int unmapCount;
    Graphics *fGraphics;
    long fEventMask;
    int fWinGravity, fBitGravity;

    bool fEnabled;
    bool fToplevel;

    typedef struct _YAccelerator {
        unsigned key;
        unsigned mod;
        YWindow *win;
        struct _YAccelerator *next;
    } YAccelerator;

    YAccelerator *accel;
#ifdef CONFIG_TOOLTIP
    YToolTip *fToolTip;
#endif

    static XButtonEvent fClickEvent;
    static YWindow *fClickWindow;
    static Time fClickTime;
    static int fClickCount;
    static int fClickDrag;
    static unsigned fClickButton;
    static unsigned fClickButtonDown;
    static YTimer *fToolTipTimer;

    bool fDND;
    Window XdndDragSource;
    Window XdndDropTarget;

    static AutoScroll *fAutoScroll;
};

class YDesktop: public YWindow {
public:
    YDesktop(YWindow *aParent = 0, Window win = 0);
    virtual ~YDesktop();
    
    virtual void resetColormapFocus(bool active);
};

extern YDesktop *desktop;

#ifdef CONFIG_SHAPE
extern int shapesSupported;
extern int shapeEventBase, shapeErrorBase;
#endif

extern Atom _XA_WM_PROTOCOLS;
extern Atom _XA_WM_DELETE_WINDOW;
extern Atom _XA_WM_TAKE_FOCUS;
extern Atom _XA_WM_STATE;
extern Atom _XA_WM_CHANGE_STATE;
extern Atom _XATOM_MWM_HINTS;
extern Atom _XA_WM_COLORMAP_WINDOWS;
extern Atom _XA_CLIPBOARD;

/* Xdnd */
extern Atom XA_XdndAware;
extern Atom XA_XdndEnter;
extern Atom XA_XdndLeave;
extern Atom XA_XdndPosition;
extern Atom XA_XdndStatus;
extern Atom XA_XdndDrop;
extern Atom XA_XdndFinished;

#endif