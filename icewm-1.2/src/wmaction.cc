#include "config.h"
#include "wmaction.h"
#include "yaction.h"

YAction *actionCascade(0);
YAction *actionArrange(0);
YAction *actionTileVertical(0);
YAction *actionTileHorizontal(0);
YAction *actionUndoArrange(0);
YAction *actionArrangeIcons(0);
YAction *actionMinimizeAll(0);
YAction *actionHideAll(0);

#ifndef CONFIG_PDA
YAction *actionHide(0);
#endif
YAction *actionShow(0);
YAction *actionRaise(0);
YAction *actionLower(0);
YAction *actionDepth(0);
YAction *actionMove(0);
YAction *actionSize(0);
YAction *actionMaximize(0);
YAction *actionMaximizeVert(0);
YAction *actionMinimize(0);
YAction *actionRestore(0);
YAction *actionRollup(0);
YAction *actionClose(0);
YAction *actionKill(0);
YAction *actionOccupyAllOrCurrent(0);
YAction *actionDoNotCover(0);

YAction *actionWindowList(0);
YAction *actionLogout(0);
YAction *actionCancelLogout(0);
YAction *actionRefresh(0);
#ifndef LITE
YAction *actionAbout(0);
YAction *actionHelp(0);
YAction *actionLicense(0);
#endif
YAction *actionRun(0);
YAction *actionExit(0);

void initActions() {
    actionCascade = new YAction();
    actionArrange = new YAction();
    actionTileVertical = new YAction();
    actionTileHorizontal = new YAction();
    actionUndoArrange = new YAction();
    actionArrangeIcons = new YAction();
    actionMinimizeAll = new YAction();
    actionHideAll = new YAction();
#ifndef CONFIG_PDA
    actionHide = new YAction();
#endif
    actionShow = new YAction();
    actionRaise = new YAction();
    actionLower = new YAction();
    actionDepth = new YAction();
    actionMove = new YAction();
    actionSize = new YAction();
    actionMaximize = new YAction();
    actionMaximizeVert = new YAction();
    actionMinimize = new YAction();
    actionRestore = new YAction();
    actionRollup = new YAction();
    actionClose = new YAction();
    actionKill = new YAction();
    actionOccupyAllOrCurrent = new YAction();
    actionDoNotCover = new YAction();
    actionWindowList = new YAction();
    actionLogout = new YAction();
    actionCancelLogout = new YAction();
    actionRefresh = new YAction();
#ifndef LITE
    actionAbout = new YAction();
    actionHelp = new YAction();
    actionLicense = new YAction();
#endif
    actionRun = new YAction();
    actionExit = new YAction();
}
