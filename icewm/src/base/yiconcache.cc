/*
 * IceWM
 *
 * Copyright (C) 1997-2000 Marko Macek
 */
#include "config.h"
#include "yfull.h"
#include "ypaint.h"
#include "yapp.h"
#include "yresource.h"

#include "sysdep.h"
#include "prefs.h"
//#include "wmprog.h" // !!! remove this
#include "debug.h"

class YCachedIcon;

static YCachedIcon *firstIcon = 0;

class YCachedIcon {
public:
    YCachedIcon(YIcon *icon) {
        fNext = firstIcon;
        firstIcon = this;

        fIcon = icon;
    }
    ~YCachedIcon() {
        delete fIcon;
    }

    YIcon *getIcon() { return fIcon; }
    YCachedIcon *next() { return fNext; }

private:
    YIcon *fIcon;
    YCachedIcon *fNext;
};

extern YResourcePath *iconPaths;

bool YIcon::findIcon(char *base, char **fullPath) {
    /// !!! fix: do this at startup (merge w/ iconPath)
    for (int i = 0; i < iconPaths->getCount(); i++) {
        const char *path = iconPaths->getPath(i);

        //puts (path);
        if (findPath(path, R_OK, base, fullPath, true)) {
            return true;
        }
    }
    if (findPath(iconPath, R_OK, base, fullPath, true))
        return true;
    return false;
}

bool YIcon::findIcon(char **fullPath, int size) {
    char icons_size[1024];

    sprintf(icons_size, "%s_%dx%d.xpm", REDIR_ROOT(fPath), size, size);

    if (findIcon(icons_size, fullPath))
        return true;
    
    if (size == ICON_LARGE) {
        sprintf(icons_size, "%s.xpm", REDIR_ROOT(fPath));
    } else {
        char name[1024];
        char *p;

        sprintf(icons_size, "%s.xpm", REDIR_ROOT(fPath));
        p = strrchr(icons_size, '/');
        if (!p)
            p = icons_size;
        else
            p++;
        strcpy(name, p);
        sprintf(p, "mini/%s", name);
    }

    if (findIcon(icons_size, fullPath))
        return true;

#ifdef IMLIB    
    sprintf(icons_size, "%s", REDIR_ROOT(fPath));
    if (findIcon(icons_size, fullPath))
        return true;
#endif

#ifdef DEBUG
    if (debug)
        fprintf(stderr, "Icon '%s' not found.\n", fPath);
#endif
    return false;
}

YPixmap *YIcon::loadIcon(int size) {
    YPixmap *icon = 0;

    if (icon == 0) {
#ifdef IMLIB
        if(fPath[0] == '/' && is_reg(fPath)) {
            icon = new YPixmap(fPath, size, size);
            if (icon == 0)
                fprintf(stderr, "Out of memory for pixmap %s", fPath);
        } else
#endif
        {
            char *fullPath;

            if (findIcon(&fullPath, size)) {
#ifdef IMLIB
                icon = app->loadPixmap(fullPath, size, size);
#else
                icon = app->loadPixmap(fullPath);
#endif
                if (icon == 0)
                    fprintf(stderr, "Out of memory for pixmap %s", fullPath);
                delete fullPath;
            }
        }
    }
    return icon;
}

YIcon *YApplication::getIcon(const char *name) {
    YCachedIcon *icn = firstIcon;

    while (icn) {
        if (strcmp(name, icn->getIcon()->iconName()) == 0)
            return icn->getIcon();
        icn = icn->next();
    }
    return new YIcon(name);
}

void YApplication::freeIcons() {
    YCachedIcon *icn, *next;

    for (icn = firstIcon; icn; icn = next) {
        next = icn->next();
        delete icn;
    }
}
