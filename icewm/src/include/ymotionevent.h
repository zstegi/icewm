#ifndef __YMOTIONEVENT_H
#define __YMOTIONEVENT_H

#include "yevent.h"

class YMotionEvent: public YWindowEvent {
public:
    YMotionEvent(): YWindowEvent() {}

    YMotionEvent(int aType, int x, int y, int x_root, int y_root, int modifiers, long time, long window):
        YWindowEvent(aType, modifiers, time, window),
        fX(x), fY(y), fXroot(x_root), fYroot(y_root)
    {
    }

    int x() const { return fX; }
    int y() const { return fY; }
    int x_root() const { return fXroot; }
    int y_root() const { return fYroot; }
private:
    int fX;
    int fY;
    int fXroot;
    int fYroot;
};

#endif