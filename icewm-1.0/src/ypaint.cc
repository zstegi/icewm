/*
 * IceWM
 *
 * Copyright (C) 1997-2001 Marko Macek
 */
#include "config.h"
#include "ylib.h"
#include "ypixbuf.h"
#include "ypaint.h"

#include "yapp.h"
#include "sysdep.h"
#include "prefs.h"

#include "intl.h"

YColor::YColor(unsigned short red, unsigned short green, unsigned short blue) {
    fDarker = fBrighter = 0;
    fRed = red;
    fGreen = green;
    fBlue = blue;

    alloc();
}

YColor::YColor(unsigned long pixel) {
    XColor color;
    fDarker = fBrighter = 0;

    color.pixel = pixel;
    XQueryColor(app->display(),
                defaultColormap,
                &color);

    fRed = color.red;
    fGreen = color.green;
    fBlue = color.blue;
    alloc();
}

YColor::YColor(const char *clr) {
    XColor color;
    fDarker = fBrighter = 0;

    XParseColor(app->display(),
                defaultColormap,
                clr ? clr : "rgb:00/00/00",
                &color);

    fRed = color.red;
    fGreen = color.green;
    fBlue = color.blue;
    alloc();
}

void YColor::alloc() {
    XColor color;

    color.red = fRed;
    color.green = fGreen;
    color.blue = fBlue;
    color.flags = DoRed | DoGreen | DoBlue;

    if (XAllocColor(app->display(),
                    defaultColormap,
                    &color) == 0)
    {
        int j, ncells;
        double long d = 65536. * 65536. * 65536. * 24;
        XColor clr;
        unsigned long pix;
        long d_red, d_green, d_blue;
        double long u_red, u_green, u_blue;

        pix = 0xFFFFFFFF;
        ncells = DisplayCells(app->display(), DefaultScreen(app->display()));
        for (j = 0; j < ncells; j++) {
            clr.pixel = j;
            XQueryColor(app->display(), defaultColormap, &clr);

            d_red   = color.red   - clr.red;
            d_green = color.green - clr.green;
            d_blue  = color.blue  - clr.blue;

            u_red   = 3UL * (d_red   * d_red);
            u_green = 4UL * (d_green * d_green);
            u_blue  = 2UL * (d_blue  * d_blue);

            double d1 = u_red + u_blue + u_green;

            if (pix == 0xFFFFFFFF || d1 < d) {
                pix = j;
                d = d1;
            }
        }
        if (pix != 0xFFFFFFFF) {
            clr.pixel = pix;
            XQueryColor(app->display(), defaultColormap, &clr);
            /*DBG(("color=%04X:%04X:%04X, match=%04X:%04X:%04X\n",
                   color.red, color.blue, color.green,
                   clr.red, clr.blue, clr.green));*/
            color = clr;
        }
        if (XAllocColor(app->display(), defaultColormap, &color) == 0)
            if (color.red + color.green + color.blue >= 32768)
                color.pixel = WhitePixel(app->display(),
                                         DefaultScreen(app->display()));
            else
                color.pixel = BlackPixel(app->display(),
                                         DefaultScreen(app->display()));
    }
    fRed = color.red;
    fGreen = color.green;
    fBlue = color.blue;
    fPixel = color.pixel;
}

YColor *YColor::darker() { // !!! fix
    if (fDarker == 0) {
        unsigned short red, blue, green;

        red = ((unsigned int)fRed) * 2 / 3;
        blue = ((unsigned int)fBlue) * 2 / 3;
        green = ((unsigned int)fGreen) * 2 / 3;
        fDarker = new YColor(red, green, blue);
    }
    return fDarker;
}

YColor *YColor::brighter() { // !!! fix
    if (fBrighter == 0) {
        unsigned short red, blue, green;

#define maxx(x) (((x) <= 0xFFFF) ? (x) : 0xFFFF)

        red = maxx(((unsigned int)fRed) * 4 / 3);
        blue = maxx(((unsigned int)fBlue) * 4 / 3);
        green = maxx(((unsigned int)fGreen) * 4 / 3);

        fBrighter = new YColor(red, green, blue);
    }
    return fBrighter;
}

YFont *YFont::getFont(const char *name) {
    YFont *f = new YFont(name);

    if (f) {
        if (
#if I18N
            multiByte && f->font_set == 0 ||
            !multiByte &&
#endif
            f->afont == 0)
        {

            delete f;
            return 0;
        }
    }
    return f;
}
#ifdef I18N
void YFont::GetFontNameElement(const char *pattern, char *buf, int bufsiz, int hyphennumber)
{
    const char *p;
    int h, len;
  
    for (p = pattern, h = 0;
         *p && (*p != '-' || ++h != hyphennumber);
         p++);

    if (h != hyphennumber) {
        buf[0] = '*';
        buf[1] = '\0';
        return;
    }

    for (++p, len = 0; p[len] && p[len] != '-' && len < bufsiz; ++len)
        buf[len] = p[len];
    buf[len] = '\0';
}

XFontSet YFont::CreateFontSetWithGuess(Display *d, const char *pattern, char ***miss, int *n_miss, char **def)
{
  XFontSet fs;
  char *pattern2;
  int bufsiz;

#define FONT_ELEMENT_SIZE 50
  char weight[FONT_ELEMENT_SIZE],
       slant[FONT_ELEMENT_SIZE],
       pxlsz[FONT_ELEMENT_SIZE];

  fs = XCreateFontSet(d, pattern, miss, n_miss, def);
  if (fs && !*n_miss) return fs; /* no need for font guessing */

  /* for non-iso8859-1 language and iso8859-1 specification */
  /* This 'fs' is only for pattern analysis. */
#ifdef    HAVE_SETLOCALE
  if (!fs) {
    if (*n_miss) XFreeStringList(*miss);
    setlocale(LC_CTYPE, "C");
    fs = XCreateFontSet(d, pattern, miss, n_miss, def);
    setlocale(LC_CTYPE, "");
  }
#endif // HAVE_SETLOCALE

  /* make XLFD font name for pattern analysis */
  if (fs) {
    XFontStruct **fontstructs;
    char **fontnames;
    XFontsOfFontSet(fs, &fontstructs, &fontnames);
    pattern = fontnames[0];
  }

  /* read elements of font name */
  GetFontNameElement(pattern, weight, sizeof(weight), 3);
  GetFontNameElement(pattern, slant,  sizeof(slant), 4);
  GetFontNameElement(pattern, pxlsz,  sizeof(pxlsz), 7);

  /* modify elements of font name to fit usual font names */
  if (!strcmp(weight, "*")) strncpy(weight, "medium", sizeof(weight));
  if (!strcmp(slant,  "*")) strncpy(slant,  "r",      sizeof(slant));

  /* build font pattern for better matching for various charsets */
  bufsiz = strlen(pattern) + FONT_ELEMENT_SIZE*4 + 59;
  pattern2 = new char[bufsiz];
  if (pattern2) {
    snprintf(pattern2, bufsiz-1, "%s,"
             "-*-*-%s-%s-*-*-%s-*-*-*-*-*-*-*,"
             "-*-*-*-*-*-*-%s-*-*-*-*-*-*-*,*",
             pattern,
             weight, slant, pxlsz,
             pxlsz);
    pattern = pattern2;
  } else
    warn(_("Out of memory (len=%d)."), bufsiz);

  if (*n_miss) XFreeStringList(*miss);
  if (fs) XFreeFontSet(d, fs);

  /* create fontset */
  fs = XCreateFontSet(d, pattern, miss, n_miss, def);
  if (pattern2) delete pattern2;
  return fs;
}
#endif // I18N

YFont::YFont(const char *name) {
#ifdef I18N
    if (multiByte) {
        char **missing, *def_str;
        int missing_num;

        fontAscent = fontDescent = 0;

        font_set = CreateFontSetWithGuess(app->display(), name, &missing,
                                          &missing_num, &def_str);

        if (font_set == 0) {
            warn(_("Could not load fontset '%s'."), name);
            font_set = XCreateFontSet(app->display(), "*fixed*", &missing,
                                      &missing_num, &def_str);
            if (font_set == 0)
                warn(_("Fallback to '*fixed*' failed."));
        }
        if (font_set) {
            if (missing_num) {
                int i;
                warn(_("Missing fontset in loading '%s'"), name);
                for (i = 0; i < missing_num; i++)
                    fprintf(stderr, "%s\n", missing[i]);
                XFreeStringList(missing);
            }
            XFontSetExtents *extents = XExtentsOfFontSet(font_set);
            if (extents) {
                fontAscent = -extents->max_logical_extent.y;
                fontDescent = extents->max_logical_extent.height - fontAscent;
            }
        }
    } else
#endif
    {
        afont = XLoadQueryFont(app->display(), name);
        if (afont == 0)  {
            warn(_("Could not load font '%s'."), name);
            afont = XLoadQueryFont(app->display(), "fixed");
            if (afont == 0)
                warn(_("Fallback to 'fixed' failed."));
        }

        fontAscent = afont ? afont->max_bounds.ascent : 0;
        fontDescent = afont ? afont->max_bounds.descent : 0;
    }
}

YFont::~YFont() {
#ifdef I18N
    if (font_set) XFreeFontSet(app->display(), font_set);
#endif
    if (afont) XFreeFont(app->display(), afont);
}

unsigned YFont::textWidth(const char *str) const {
#ifdef I18N
    if (multiByte)
        return font_set ? XmbTextEscapement(font_set, str, strlen(str)) : 0;
    else
#endif
        return afont ? XTextWidth(afont, str, strlen(str)) : 0;
}

unsigned YFont::textWidth(const char *str, int len) const {
#ifdef I18N
    if (multiByte)
        return font_set ? XmbTextEscapement(font_set, str, len) : 0;
    else
#endif
        return afont ? XTextWidth(afont, str, len) : 0;
}

unsigned YFont::multilineTabPos(const char *str) const {
    unsigned tabPos(0);

    for (const char * end(strchr(str, '\n')); end;
	 str = end + 1, end = strchr(str, '\n')) {
	int const len(end - str);
	const char * tab((const char *) memchr(str, '\t', len));

	if (tab) tabPos = max(tabPos, textWidth(str, tab - str));
    }
    
    const char * tab(strchr(str, '\t'));
    if (tab) tabPos = max(tabPos, textWidth(str, tab - str));
    
    return (tabPos ? tabPos + 3 * textWidth(" ", 1) : 0);
}

YDimension YFont::multilineAlloc(const char *str) const {
    unsigned const tabPos(multilineTabPos(str));
    YDimension alloc(0, ascent());

    for (const char * end(strchr(str, '\n')); end;
	 str = end + 1, end = strchr(str, '\n')) {
	int const len(end - str);
	const char * tab((const char *) memchr(str, '\t', len));

	alloc.w = max(tab ? tabPos + textWidth(tab + 1, end - tab - 1)
			  : textWidth(str, len), alloc.w);
	alloc.h+= height();
    }

    const char * tab(strchr(str, '\t'));
    alloc.w = max(alloc.w, tab ? tabPos + textWidth(tab + 1) : textWidth(str));

    return alloc;
}

Graphics::Graphics(YWindow *window):
    display(app->display()), drawable(window->handle()) {
    XGCValues gcv; gcv.graphics_exposures = False;
    gc = XCreateGC(display, drawable, GCGraphicsExposures, &gcv);
}

Graphics::Graphics(YPixmap *pixmap):
    display(app->display()), drawable(pixmap->pixmap()) {
    XGCValues gcv; gcv.graphics_exposures = False;
    gc = XCreateGC(display, drawable, GCGraphicsExposures, &gcv);
}

Graphics::Graphics(Drawable drawable):
    display(app->display()), drawable(drawable) {
    XGCValues gcv; gcv.graphics_exposures = False;
    gc = XCreateGC(display, drawable, GCGraphicsExposures, &gcv);
}

Graphics::~Graphics() {
    XFreeGC(display, gc);
}

void Graphics::copyArea(const int x, const int y,
			const int width, const int height,
			const int dx, const int dy) {
    XCopyArea(display, drawable, drawable, gc,
              x, y, width, height, dx, dy);
}

void Graphics::copyDrawable(Drawable const d, const int x, const int y, 
			    const int w, const int h, const int dx, const int dy) {
    XCopyArea(display, d, drawable, gc, x, y, w, h, dx, dy);
}
    
void Graphics::copyPixbuf(const YPixbuf & pixbuf,
			  const int x, const int y, const int w, const int h,
			  const int dx, const int dy) {
    pixbuf.copyToDrawable(drawable, gc, x, y, w, h, dx, dy);
}

void Graphics::drawPoint(int x, int y) {
    XDrawPoint(display, drawable, gc, x, y);
}

void Graphics::drawLine(int x1, int y1, int x2, int y2) {
    XDrawLine(display, drawable, gc, x1, y1, x2, y2);
}

void Graphics::drawLines(XPoint *points, int n, int mode) {
    XDrawLines(display, drawable, gc, points, n, mode);
}

void Graphics::drawRect(int x, int y, int width, int height) {
    XDrawRectangle(display, drawable, gc, x, y, width, height);
}

void Graphics::drawArc(int x, int y, int width, int height, int a1, int a2) {
    XDrawArc(display, drawable, gc, x, y, width, height, a1, a2);
}

void Graphics::drawChars(const char *data, int offset, int len, int x, int y) {
#ifdef I18N
    if (multiByte) {
        if (font && font->font_set)
            XmbDrawString(display, drawable, font->font_set, gc,
                          x, y, data + offset, len);
    } else
#endif
    {
        XDrawString(display, drawable, gc, x, y, data + offset, len);
    }
}

void Graphics::drawCharsEllipsis(const char *str, int len, int x, int y, int maxWidth) {
    int w = font ? font->textWidth(str, len) : 0;

    if (font == 0 || w <= maxWidth) {
        drawChars(str, 0, len, x, y);
    } else {
        int l = 0;
        int maxW = maxWidth - font->textWidth("...", 3);
        int w = 0;
        int wc;

        if (maxW > 0) {
            while (w < maxW && l < len) {
                int nc = 1;
#ifdef I18N
                if (multiByte) {
                    nc = mblen(str + l, len - l);
                    wc = font->textWidth(str + l, nc);
                } else
#endif
                {
                    wc = font->textWidth(str + l, 1);
                }
                if (w + wc < maxW) {
                    l += nc;
                    w += wc;
                } else
                    break;
            }
        }

        if (l > 0)
	    drawChars(str, 0, l, x, y);
        if (l < len)
            drawChars("...", 0, 3, x + w, y);
    }
}

void Graphics::drawCharUnderline(int x, int y, const char *str, int charPos) {
    int left = font ? font->textWidth(str, charPos) : 0;
    int right = font ? font->textWidth(str, charPos + 1) - 1 : 0;

    drawLine(x + left, y + 2, x + right, y + 2);
}

void Graphics::drawCharsMultiline(const char *str, int x, int y) {
    unsigned const tx(x + font->multilineTabPos(str));

    for (const char * end(strchr(str, '\n')); end;
	 str = end + 1, end = strchr(str, '\n')) {
	int const len(end - str);
	const char * tab((const char *) memchr(str, '\t', len));

	if (tab) {
	    drawChars(str, 0, tab - str, x, y);
	    drawChars(tab + 1, 0, end - tab - 1, tx, y);
        }
	else
	    drawChars(str, 0, end - str, x, y);
	    
	y+= font->height();
    }

    const char * tab(strchr(str, '\t'));

    if (tab) {
	drawChars(str, 0, tab - str, x, y);
	drawChars(tab + 1, 0, strlen(tab + 1), tx, y);
    }
    else
	drawChars(str, 0, strlen(str), x, y);
}

void Graphics::fillRect(int x, int y, int width, int height) {
    XFillRectangle(display, drawable, gc,
                   x, y, width, height);
}

void Graphics::fillPolygon(XPoint * points, int const n, int const shape,
			  int const mode) {
    XFillPolygon(display, drawable, gc, points, n, shape, mode);
}

void Graphics::fillArc(int x, int y, int width, int height, int a1, int a2) {
    XFillArc(display, drawable, gc, x, y, width, height, a1, a2);
}

void Graphics::setColor(YColor *aColor) {
    color = aColor;
    XSetForeground(display, gc, color->pixel());
}

void Graphics::setFont(YFont const *aFont) {
    font = aFont;
#ifdef I18N
    if (!multiByte)
#endif
        if (font && font->afont) XSetFont(display, gc, font->afont->fid);
}

void Graphics::setPenStyle(bool dotLine) {
    XGCValues gcv;

    if (dotLine) {
        char c = 1;
        gcv.line_style = LineOnOffDash;
        XSetDashes(display, gc, 0, &c, 1);
    } else {
        gcv.line_style = LineSolid;
    }

    XChangeGC(display, gc, GCLineStyle, &gcv);
}

void Graphics::drawPixmap(YPixmap const * pix, int const x, int const y) {
    if (pix->mask())
        drawClippedPixmap(pix->pixmap(),
                          pix->mask(),
                          0, 0, pix->width(), pix->height(), x, y);
    else
        XCopyArea(display, pix->pixmap(), drawable, gc,
                  0, 0, pix->width(), pix->height(), x, y);
}

void Graphics::drawMask(YPixmap const * pix, int const x, int const y) {
    if (pix->mask())
        XCopyArea(display, pix->mask(), drawable, gc,
                  0, 0, pix->width(), pix->height(), x, y);
}

void Graphics::drawClippedPixmap(Pixmap pix, Pixmap clip,
                                 int x, int y, int w, int h, int toX, int toY)
{
    static GC clipPixmapGC = None;
    XGCValues gcv;

    if (clipPixmapGC == None) {
        gcv.graphics_exposures = False;
        clipPixmapGC = XCreateGC(app->display(), desktop->handle(),
                                 GCGraphicsExposures,
                                 &gcv);
    }

    gcv.clip_mask = clip;
    gcv.clip_x_origin = toX;
    gcv.clip_y_origin = toY;
    XChangeGC(display, clipPixmapGC,
              GCClipMask|GCClipXOrigin|GCClipYOrigin, &gcv);
    XCopyArea(display, pix, drawable, clipPixmapGC,
              x, y, w, h, toX, toY);
    gcv.clip_mask = None;
    XChangeGC(display, clipPixmapGC, GCClipMask, &gcv);
}

void Graphics::draw3DRect(int x, int y, int w, int h, bool raised) {
    YColor *back(getColor());
    YColor *bright(back->brighter());
    YColor *dark(back->darker());
    YColor *t(raised ? bright : dark);
    YColor *b(raised ? dark : bright);

    setColor(t);
    drawLine(x, y, x + w, y);
    drawLine(x, y, x, y + h);
    setColor(b);
    drawLine(x, y + h, x + w, y + h);
    drawLine(x + w, y, x + w, y + h);
    setColor(back);
    drawPoint(x + w, y);
    drawPoint(x, y + h);
}

void Graphics::drawBorderW(int x, int y, int w, int h, bool raised) {
    YColor *back(getColor());
    YColor *bright(back->brighter());
    YColor *dark(back->darker());

    if (raised) {
        setColor(bright);
        drawLine(x, y, x + w - 1, y);
        drawLine(x, y, x, y + h - 1);
        setColor(YColor::black);
        drawLine(x, y + h, x + w, y + h);
        drawLine(x + w, y, x + w, y + h);
        setColor(dark);
        drawLine(x + 1, y + h - 1, x + w - 1, y + h - 1);
        drawLine(x + w - 1, y + 1, x + w - 1, y + h - 1);
    } else {
        setColor(bright);
        drawLine(x + 1, y + h, x + w, y + h);
        drawLine(x + w, y + 1, x + w, y + h);
        setColor(YColor::black);
        drawLine(x, y, x + w, y);
        drawLine(x, y, x, y + h);
        setColor(dark);
        drawLine(x + 1, y + 1, x + w - 1, y + 1);
        drawLine(x + 1, y + 1, x + 1, y + h - 1);
    }
    setColor(back);
}

// doesn't move... needs two pixels on all sides for up and down
// position.
void Graphics::drawBorderM(int x, int y, int w, int h, bool raised) {
    YColor *back(getColor());
    YColor *bright(back->brighter());
    YColor *dark(back->darker());

    if (raised) {
        setColor(bright);
        drawLine(x + 1, y + 1, x + w, y + 1);
        drawLine(x + 1, y + 1, x + 1, y + h);
        drawLine(x + 1, y + h, x + w, y + h);
        drawLine(x + w, y + 1, x + w, y + h);

        setColor(dark);
        drawLine(x, y, x + w - 1, y);
        drawLine(x, y, x, y + h);
        drawLine(x, y + h - 1, x + w - 1, y + h - 1);
        drawLine(x + w - 1, y, x + w - 1, y + h - 1);

        ///  how to get the color of the taskbar??
        setColor(back);

        drawPoint(x, y + h);
        drawPoint(x + w, y);
    } else {
        setColor(dark);
        drawLine(x, y, x + w - 1, y);
        drawLine(x, y, x, y + h - 1);

        setColor(bright);
        drawLine(x + 1, y + h, x + w, y + h);
        drawLine(x + w, y + 1, x + w, y + h);

        setColor(back);
        drawRect(x + 1, y + 1, x + w - 2, y + h - 2);
        //drawLine(x + 1, y + 1, x + w - 1, y + 1);
        //drawLine(x + 1, y + 1, x + 1, y + h - 1);
        //drawLine(x, y + h - 1, x + w - 1, y + h - 1);
        //drawLine(x + w - 1, y, x + w - 1, y + h - 1);

        ///  how to get the color of the taskbar??
        drawPoint(x, y + h);
        drawPoint(x + w, y);
    }
}

void Graphics::drawBorderG(int x, int y, int w, int h, bool raised) {
    YColor *back(getColor());
    YColor *bright(back->brighter());
    YColor *dark(back->darker());

    if (raised) {
        setColor(bright);
        drawLine(x, y, x + w - 1, y);
        drawLine(x, y, x, y + h - 1);
        setColor(dark);
        drawLine(x, y + h, x + w, y + h);
        drawLine(x + w, y, x + w, y + h);
        setColor(YColor::black);
        drawLine(x + 1, y + h - 1, x + w - 1, y + h - 1);
        drawLine(x + w - 1, y + 1, x + w - 1, y + h - 1);
    } else {
        setColor(bright);
        drawLine(x + 1, y + h, x + w, y + h);
        drawLine(x + w, y + 1, x + w, y + h);
        setColor(YColor::black);
        drawLine(x, y, x + w, y);
        drawLine(x, y, x, y + h);
        setColor(dark);
        drawLine(x + 1, y + 1, x + w - 1, y + 1);
        drawLine(x + 1, y + 1, x + 1, y + h - 1);
    }
    setColor(back);
}

void Graphics::drawCenteredPixmap(int x, int y, int w, int h, YPixmap *pixmap) {
    int r = x + w;
    int b = y + h;
    int pw = pixmap->width();
    int ph = pixmap->height();

    drawOutline(x, y, r, b, pw, ph);
    drawPixmap(pixmap, x + w / 2 - pw / 2, y + h / 2 - ph / 2);
}

void Graphics::drawOutline(int l, int t, int r, int b, int iw, int ih) {
    if (l + iw >= r && t + ih >= b)
        return ;

    int li = (l + r) / 2 - iw / 2;
    int ri = (l + r) / 2 + iw / 2;
    int ti = (t + b) / 2 - ih / 2;
    int bi = (t + b) / 2 + ih / 2;

    if (l < li)
        fillRect(l, t, li - l, b - t);

    if (ri < r)
        fillRect(ri, t, r - ri, b - t);

    if (t < ti)
        if (li < ri)
            fillRect(li, t, ri - li, ti - t);

    if (bi < b)
        if (li < ri)
            fillRect(li, bi, ri - li, b - bi);
}

void Graphics::repHorz(Drawable d, int pw, int ph, int x, int y, int w) {
    while (w > 0) {
        XCopyArea(display, d, drawable, gc, 0, 0, min(w, pw), ph, x, y);
        x += pw;
        w -= pw;
    }
}

void Graphics::repVert(Drawable d, int pw, int ph, int x, int y, int h) {
    while (h > 0) {
        XCopyArea(display, d, drawable, gc, 0, 0, pw, min(h, ph), x, y);
        y += ph;
        h -= ph;
    }
}

void Graphics::fillPixmap(YPixmap const * pixmap, int const x, int const y,
			  int const w, int const h, int px, int py) {
    int const pw(pixmap->width());
    int const ph(pixmap->height());

    px%= pw; const int pww(px ? pw - px : 0);
    py%= ph; const int phh(py ? ph - py : 0);

    if (px) {
	if (py)
            XCopyArea(display, pixmap->pixmap(), drawable, gc,
                      px, py, pww, phh, x, y);

        for (int yy(y + phh), hh(h - phh); hh > 0; yy += ph, hh -= ph)
            XCopyArea(display, pixmap->pixmap(), drawable, gc,
                      px, 0, pww, min(hh, ph), x, yy);
    }

    for (int xx(x + pww), ww(w - pww); ww > 0; xx+= pw, ww-= pw) {
	int const www(min(ww, pw));

	if (py)
            XCopyArea(display, pixmap->pixmap(), drawable, gc,
                      0, py, www, phh, xx, y);

        for (int yy(y + phh), hh(h - phh); hh > 0; yy += ph, hh -= ph)
            XCopyArea(display, pixmap->pixmap(), drawable, gc,
                      0, 0, www, min(hh, ph), xx, yy);
    }
}

void Graphics::drawGradient(const class YPixbuf & pixbuf,
			    int const x, int const y, const int w, const int h,
			    int const gx, int const gy, const int gw, const int gh) {
    YPixbuf(pixbuf, gw, gh).copyToDrawable(drawable, gc, gx, gy, w, h, x, y);
}

void Graphics::drawArrow(Direction direction, int x, int y, int size, 
			 bool pressed) {
    YColor *nc(getColor());
    YColor *oca(pressed ? nc->darker() : nc->brighter()),
	   *ica(pressed ? YColor::black : nc),
    	   *ocb(pressed ? wmLook == lookGtk ? nc : nc->brighter()
			: nc->darker()),
	   *icb(pressed ? nc->brighter() : YColor::black);

    XPoint points[3];

    short const am(size / 2);
    short const ah(wmLook == lookGtk ||
		   wmLook == lookMotif ? size : size / 2);
    short const aw(wmLook == lookGtk ||
		   wmLook == lookMotif ? size : size - (size & 1));
		   
    switch (direction) {
	case Up:
	    points[0].x = x;
	    points[0].y = y + ah;
	    points[1].x = x + am;
	    points[1].y = y;
	    points[2].x = x + aw;
	    points[2].y = y + ah;
	    break;

	case Down:
	    points[0].x = x;
	    points[0].y = y;
	    points[1].x = x + am;
	    points[1].y = y + ah;
	    points[2].x = x + aw;
	    points[2].y = y;
	    break;

	case Left:
	    points[0].x = x + ah;
	    points[0].y = y;
	    points[1].x = x;
	    points[1].y = y + am;
	    points[2].x = x + ah;
	    points[2].y = y + aw;
	    break;

	case Right:
	    points[0].x = x;
	    points[0].y = y;
	    points[1].x = x + ah;
	    points[1].y = y + am;
	    points[2].x = x;
	    points[2].y = y + aw;
	    break;

	default:
	    return ;
    }

    short const dx0(direction == Up || direction == Down ? 1 : 0);
    short const dy0(direction == Up || direction == Down ? 0 : 1);
    short const dx1(direction == Up || direction == Left ? dy0 : -dy0);
    short const dy1(direction == Up || direction == Left ? dx0 : -dx0);

// ============================================================= inner bevel ===
    if (wmLook == lookGtk || wmLook == lookMotif) {
	setColor(ocb);
	drawLine(points[2].x - dx0 - (size & 1 ? 0 : dx1),
		 points[2].y - (size & 1 ? 0 : dy1) - dy0,
		 points[1].x + 2 * dx1, points[1].y + 2 * dy1);

	setColor(wmLook == lookMotif ? oca : ica);
	drawLine(points[0].x + dx0 - (size & 1 ? dx1 : 0),
		 points[0].y + dy0 - (size & 1 ? dy1 : 0),
		 points[1].x + (size & 1 ? dx0 : 0)
		 	     + (size & 1 ? dx1 : dx1 + dx1),
		 points[1].y + (size & 1 ? dy0 : 0)
		 	     + (size & 1 ? dy1 : dy1 + dy1));
		 
	if ((direction == Up || direction == Left)) setColor(ocb);
	drawLine(points[0].x + dx0 - dx1, points[0].y + dy0 - dy1,
	    	 points[2].x - dx0 - dx1, points[2].y - dy0 - dy1);
    } else if (wmLook == lookWarp3) {
	drawLine(points[0].x + dx0, points[0].y + dy0,
		 points[1].x + dx0, points[1].y + dy0);
	drawLine(points[2].x + dx0, points[2].y + dy0,
		 points[1].x + dx0, points[1].y + dy0);
    } else
        fillPolygon(points, 3, Convex, CoordModeOrigin);
;
// ============================================================= outer bevel ===
    if (wmLook == lookMotif || wmLook == lookGtk) {
	setColor(wmLook == lookMotif ? ocb : icb);

	drawLine(points[2].x, points[2].y, points[1].x, points[1].y);

	if (wmLook == lookGtk || wmLook == lookMotif) setColor(oca);
	drawLine(points[0].x, points[0].y, points[1].x, points[1].y);

	if ((direction == Up || direction == Left))
	    setColor(wmLook == lookMotif ? ocb : icb);

    } else
        drawLines(points, 3, CoordModeOrigin);

    if (wmLook != lookWarp3)
	drawLine(points[0].x, points[0].y, points[2].x, points[2].y);

    setColor(nc);
}
