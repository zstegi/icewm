#ifndef __YMENU_H
#define __YMENU_H

#include "ypopup.h"
#include "ytimer.h"

class YAction;
class YActionListener;
class YMenuItem;

class YMenu: public YPopupWindow, public YTimerListener {
public:
    YMenu(YWindow *parent = 0);
    virtual ~YMenu();

    virtual void sizePopup(int hspace);
    virtual void activatePopup();
    virtual void deactivatePopup();
    virtual void donePopup(YPopupWindow *popup);

    virtual void paint(Graphics &g, int x, int y, unsigned int width, unsigned int height);

    virtual bool handleKey(const XKeyEvent &key);
    virtual void handleButton(const XButtonEvent &button);
    virtual void handleMotion(const XMotionEvent &motion);
    virtual bool handleAutoScroll(const XMotionEvent &mouse);

    void trackMotion(const int x_root, const int y_root, const unsigned state);

    YMenuItem *add(YMenuItem *item);
    YMenuItem *addItem(const char *name, int hotCharPos, const char *param, YAction *action);
    YMenuItem *addItem(const char *name, int hotCharPos, YAction *action, YMenu *submenu);
    YMenuItem *addSubmenu(const char *name, int hotCharPos, YMenu *submenu);
    YMenuItem *addSeparator();
    YMenuItem *addLabel(const char *name);
    void removeAll();
    YMenuItem *findAction(const YAction *action);
    YMenuItem *findSubmenu(const YMenu *sub);
    YMenuItem *findName(const char *name, const int first = 0);

    void enableCommand(YAction *action); // 0 == All
    void disableCommand(YAction *action); // 0 == All

    int itemCount() const { return fItemCount; }
    YMenuItem *item(int n) const { return fItems[n]; }

    bool isShared() const { return fShared; }
    void setShared(bool shared) { fShared = shared; }

    void setActionListener(YActionListener *actionListener);
    YActionListener *getActionListener() const { return fActionListener; }

    virtual bool handleTimer(YTimer *timer);

private:
    int fItemCount;
    YMenuItem **fItems;
    int selectedItem;
    int paintedItem;
    int paramPos;
    int namePos;
    YPopupWindow *fPopup;
    YPopupWindow *fPopupActive;
    bool fShared;
    YActionListener *fActionListener;
    int activatedX, activatedY;
    
#ifdef CONFIG_GRADIENTS
    class YPixbuf * fGradient;
#endif

    static YTimer *fMenuTimer;
    static int fTimerX, fTimerY, fTimerItem, fTimerSubmenu;
    static bool fTimerSlow;
    static int fAutoScrollDeltaX, fAutoScrollDeltaY;
    static int fAutoScrollMouseX, fAutoScrollMouseY;

    int getItemHeight(int itemNo, int &h, int &top, int &bottom, int &pad);
    void getItemWidth(int i, int &iw, int &nw, int &pw);
    void getOffsets(int &left, int &top, int &right, int &bottom);
    void getArea(int &x, int &y, int &w, int &h);

    void drawBackground(Graphics &g, int x, int y, int w, int h);
    void drawSeparator(Graphics &g, int x, int y, int w);

    void paintItem(Graphics &g, int i, int &l, int &t, int &r, int minY, int maxY, int paint);
    void paintItems();
    int findItemPos(int item, int &x, int &y);
    int findItem(int x, int y);
    int findActiveItem(int cur, int direction);
    int findHotItem(char k);
    void focusItem(int item, int submenu, int byMouse);
    int activateItem(int no, int byMouse, unsigned int modifiers);
    bool isCondCascade(int selectedItem);
    int onCascadeButton(int selectedItem, int x, int y, bool checkPopup);

    void autoScroll(int deltaX, int deltaY, int mx, int my, const XMotionEvent *motion);
    void finishPopup(YMenuItem *item, YAction *action, unsigned int modifiers);
};

extern YPixmap *menubackPixmap;
extern YPixmap *menuselPixmap;
extern YPixmap *menusepPixmap;

#ifdef CONFIG_GRADIENTS
class YPixbuf;

extern YPixbuf *menubackPixbuf;
extern YPixbuf *menuselPixbuf;
extern YPixbuf *menusepPixbuf;
#endif

#endif