#ifndef __WMCONTAINER_H
#define __WMCONTAINER_H

#include "ywindow.h"
#include "yconfig.h"

class YFrameWindow;

class YClientContainer: public YWindow {
public:
    YClientContainer(YWindow *parent, YFrameWindow *frame);
    virtual ~YClientContainer();

    virtual bool eventButton(const YButtonEvent &button);
    virtual bool handleCrossing(const XCrossingEvent &crossing);
    virtual void handleConfigureRequest(const XConfigureRequestEvent &configureRequest);
    virtual void handleMapRequest(const XMapRequestEvent &mapRequest);

    void grabButtons();
    void releaseButtons();

    YFrameWindow *getFrame() const { return fFrame; };
private:
    YFrameWindow *fFrame;
    bool fHaveGrab;
    bool fHaveActionGrab;

    static YBoolPrefProperty gClientMouseActions;
    static YBoolPrefProperty gUseMouseWheel;
    static YBoolPrefProperty gPointerColormap;
    static YBoolPrefProperty gPassFirstClickToClient;
    static YBoolPrefProperty gRaiseOnClickClient;
    static YBoolPrefProperty gFocusOnClickClient;
    static YBoolPrefProperty gClickFocus;
private: // not-used
    YClientContainer(const YClientContainer &);
    YClientContainer &operator=(const YClientContainer &);
};

#endif
