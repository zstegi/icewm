/*
 * IceWM
 *
 * Copyright (C) 1999-2001 Marko Macek
 */
#include "config.h"

#ifdef CONFIG_TASKBAR
#include "objbar.h"
#include "objmenu.h"
#include "objbutton.h"
#include "ybutton.h"
#include "prefs.h"
#include "wmtaskbar.h"

YColor * ObjectBar::bgColor(NULL);
YColor * ObjectButton::bgColor(NULL);

YPixmap *toolbuttonPixmap(NULL);

#ifdef CONFIG_GRADIENTS
class YPixbuf *toolbuttonPixbuf(NULL);
#endif

ObjectBar::ObjectBar(YWindow *parent): YWindow(parent) {
    if (bgColor == 0)
        bgColor = new YColor(clrDefaultTaskBar);
    setSize(1, 1);
}

ObjectBar::~ObjectBar() {
}

void ObjectBar::addButton(const char *name, YIcon *icon, YButton *button) {
    button->setToolTip(name);
    if (icon && icon->small()) {
        button->setPixmap(icon->small());
        button->setSize(button->width() + 4, button->width() + 4);
    } else
        button->setText(name);

    button->setPosition(width(), 0);
    unsigned int h = button->height();

    if (h < height())
        h = height();

    setSize(width() + button->width() + 1, h);
    button->show();
}

void ObjectBar::paint(Graphics &g, int /*x*/, int /*y*/, unsigned int /*width*/, unsigned int /*height*/) {
    if (taskbackPixmap)
        g.fillPixmap(taskbackPixmap, 0, 0, width(), height());
    else {
	g.setColor(bgColor);
        g.fillRect(0, 0, width(), height());
    }
}

void ObjectBar::addObject(DObject *object) {
    YButton *button = new ObjectButton(this, object);
    addButton(object->getName(), object->getIcon(), button);
}

void ObjectBar::addSeparator() {
    setSize(width() + 4, height());
}

void ObjectBar::addContainer(char *name, YIcon *icon, ObjectContainer *container) {
    if (container) {
        YButton *button = new ObjectButton(this, (ObjectMenu*) container);
        addButton(name, icon, button);
    }
}

YSurface ObjectButton::getSurface() {
    if (bgColor == 0)
        bgColor = new YColor(*clrToolButton ? clrToolButton : clrNormalButton);

#ifdef CONFIG_GRADIENTS    
    return YSurface(bgColor, toolbuttonPixmap, toolbuttonPixbuf);
#else		     
    return YSurface(bgColor, toolbuttonPixmap);
#endif		     
}

#endif

