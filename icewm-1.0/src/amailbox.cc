/*
 * IceWM
 *
 * Copyright (C) 1997-2001 Marko Macek
 *
 * MailBox Status
 * !!! this should be external module (replacable for POP,IMAP,...)
 */
#include "config.h"

#include "intl.h"

#ifdef CONFIG_APPLET_MAILBOX
#include "ylib.h"
#include "amailbox.h"

#include "sysdep.h"
#include "base.h"
#include "prefs.h"
#include "wmapp.h"
#include "wmtaskbar.h"
#include <sys/socket.h>
#include <netdb.h>

extern YColor *taskBarBg;
extern YPixmap *taskbackPixmap;

YPixmap *mailPixmap = 0;
YPixmap *noMailPixmap = 0;
YPixmap *errMailPixmap = 0;
YPixmap *unreadMailPixmap = 0;
YPixmap *newMailPixmap = 0;

MailCheck::MailCheck(MailBoxStatus *mbx):
    state(IDLE), fMbx(mbx), fLastSize(-1), fLastCount(-1),
    fLastUnseen(0), fLastCountSize(-1), fLastCountTime(0) {
    sk.socketListener(this);
}

MailCheck::~MailCheck() {
    sk.close();
}

void MailCheck::setURL(char const * url) {
    char const * validURL(*url == '/' ? strJoin("file://", url, NULL) : url);

    if ((fURL = validURL).scheme()) {
	if (!(strcmp(fURL.scheme(), "pop3") &&
	      strcmp(fURL.scheme(), "imap"))) {
	    protocol = (*fURL.scheme() == 'i' ? IMAP : POP3);

	    server_addr.sin_family = AF_INET;
	    server_addr.sin_port =
		htons(fURL.port() ? atoi(fURL.port())
				  : (protocol == IMAP ? 143 : 110));

	    if (fURL.host()) { /// !!! fix, need nonblocking resolve
		struct hostent const * host(gethostbyname(fURL.host()));

		if (host)
		    memcpy(&server_addr.sin_addr,
			   host->h_addr_list[0],
			   sizeof(server_addr.sin_addr));
		else
		    state = ERROR;
	    } else
		server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        } else if (!strcmp(fURL.scheme(), "file"))
	    protocol = FILE;
	else
	    warn(_("Invalid mailbox protocol: \"%s\""), fURL.scheme());
    } else
	warn(_("Invalid mailbox path: \"%s\""), url);

    if (validURL != url) delete[] validURL;
}

void MailCheck::countMessages() {
    int fd = open(fURL.path(), O_RDONLY);
    int mails = 0;

    if (fd != -1) {
        char buf[4096];
        const char *pat = "\nFrom ";
        int plen = strlen(pat);
        int pos = 1;
        int i, len;

        while ((len = read(fd, buf, sizeof(buf))) > 0) {
            for (i = 0; i < len;) {
                if (buf[i] != pat[pos])
                    if (pos)
                        pos = 0;
                    else
                        i++;
                else {
                    i++;
                    if (++pos == plen) {
                        pos = 0;
                        mails++;
                    }
                }
            }
        }
        close(fd);
    }
    fLastCount = mails;
}

void MailCheck::startCheck() {
    if (state == ERROR)
        state = IDLE;
    if (state != IDLE && state != SUCCESS)
        return ;
    //puts(protocol == FILE ? "file" : protocol == POP3 ? "POP3" : "IMAP");
    if (protocol == FILE) {
        struct stat st;
        //MailBoxStatus::MailBoxState fNewState = fState;
        if (fURL.path() == 0)
            return;

        if (!countMailMessages)
            fLastCount = -1;
        if (stat(fURL.path(), &st) == -1) {
            fMbx->mailChecked(MailBoxStatus::mbxNoMail, 0);
            fLastCount = 0;
            fLastSize = 0;
        } else {
            if (countMailMessages &&
                (st.st_size != fLastCountSize || st.st_mtime != fLastCountTime))
            {
                countMessages();
                fLastCountTime = st.st_mtime;
                fLastCountSize = fLastSize;
            }
            if (st.st_size == 0)
                fMbx->mailChecked(MailBoxStatus::mbxNoMail, fLastCount);
            else if (st.st_size > fLastSize && fLastSize != -1)
                fMbx->mailChecked(MailBoxStatus::mbxHasNewMail, fLastCount);
            else if (st.st_mtime > st.st_atime)
                fMbx->mailChecked(MailBoxStatus::mbxHasUnreadMail, fLastCount);
            else
                fMbx->mailChecked(MailBoxStatus::mbxHasMail, fLastCount);
            fLastSize = st.st_size;
        }
    } else {
        sk.connect((struct sockaddr *) &server_addr, sizeof(server_addr));
        state = CONNECTING;
        got = 0;
    }
}

void MailCheck::socketConnected() {
    //puts("CONNECTED");
    got = 0;
    sk.read(bf, sizeof(bf));
    state = WAIT_READY;
}

void MailCheck::socketError(int err) {
    //if (err) printf("error: %d\n", err);
    //else puts("EOF");
    //app->exit(err ? 1 : 0);
    if (err != 0 || state != SUCCESS)
        error();
    else {
        sk.close();
    }
}

void MailCheck::error() {
    sk.close();
    state = ERROR;
    fMbx->mailChecked(MailBoxStatus::mbxError, -1);
}

void MailCheck::socketDataRead(char *buf, int len) {
    //msg("got %d state=%d", len, state);

    bool found = false;
    for (int i = 0; i < len; i++) {
        //putchar(buf[i]);
        if (buf[i] == '\n') {
            found = true;
            buf[i] = 0;
            break;
        }
    }
    got += len;
    if (!found) {
        if (got < sizeof(bf)) {
            sk.read(bf + got , sizeof(bf) - got);
            return ;
        } else {
            error();
            return ;
        }
    }
    
    if (protocol == POP3) {
        if (strncmp(bf, "+OK ", 4) != 0) {
            error();
            return ;
        }
        if (state == WAIT_READY) {
	    char * user(strJoin("USER ", fURL.user(), "\r\n", NULL));
            sk.write(user, strlen(user));
            state = WAIT_USER;
	    delete[] user;
        } else if (state == WAIT_USER) {
	    char * pass(strJoin("PASS ", fURL.password(), "\r\n", NULL));
            sk.write(pass, strlen(pass));
            state = WAIT_PASS;
	    delete[] pass;
        } else if (state == WAIT_PASS) {
            static char stat[] = "STAT\r\n";
            sk.write(stat, strlen(stat));
            state = WAIT_STAT;
        } else if (state == WAIT_STAT) {
            static char quit[] = "QUIT\r\n";
            //puts(bf);
            if (sscanf(bf, "+OK %lu %lu", &fCurCount, &fCurSize) != 2) {
                fCurCount = 0;
                fCurSize = 0;
            }
            sk.write(quit, strlen(quit));
            state = WAIT_QUIT;
        } else if (state == WAIT_QUIT) {
            //puts("GOT QUIT");
            //app->exit(0);
            sk.close();
            state = SUCCESS;
            if (fCurSize == 0)
                fMbx->mailChecked(MailBoxStatus::mbxNoMail, fCurCount);
            else if (fCurSize > fLastSize && fLastSize != -1)
                fMbx->mailChecked(MailBoxStatus::mbxHasNewMail, fCurCount);
            else
                fMbx->mailChecked(MailBoxStatus::mbxHasMail, fCurCount);
            fLastSize = fCurSize;
            fLastCount = fCurCount;
            return ;
        }
    } else if (protocol == IMAP) {
        if (state == WAIT_READY) {
	    char * login(strJoin("0000 LOGIN ",
	    			 fURL.user(), " ", 
	    			 fURL.password(), "\r\n", NULL));
            sk.write(login, strlen(login));
            state = WAIT_USER;
	    delete[] login;
        } else if (state == WAIT_USER) {
            char * status(strJoin("0001 STATUS ",
				  fURL.path() ? fURL.path() + 1 : "INBOX",
				  " (MESSAGES UNSEEN)\r\n", NULL));
            sk.write(status, strlen(status));
            state = WAIT_STAT;
	    delete[] status;
        } else if (state == WAIT_STAT) {
            char logout[] = "0002 LOGOUT\r\n", folder[128];
	    if (sscanf(bf, "* STATUS %127s (MESSAGES %lu UNSEEN %lu)",
	    	       folder, &fCurCount, &fCurUnseen) != 3) {
                fCurCount = 0;
                fCurUnseen = 0;
            }
            sk.write(logout, strlen(logout));
            state = WAIT_QUIT;
        } else if (state == WAIT_QUIT) {
            //app->exit(0);
            sk.close();
            state = SUCCESS;
            if (fCurCount == 0)
                fMbx->mailChecked(MailBoxStatus::mbxNoMail, fCurCount);
            else if (fCurCount > fLastCount && fLastCount != -1)
                fMbx->mailChecked(MailBoxStatus::mbxHasNewMail, fCurCount);
            else if (fCurUnseen != 0)
                fMbx->mailChecked(MailBoxStatus::mbxHasUnreadMail, fCurCount);
            else
                fMbx->mailChecked(MailBoxStatus::mbxHasMail, fCurCount);
            fLastUnseen = fCurUnseen;
            fLastCount = fCurCount;
            return ;
        }
    }
    got = 0;
    sk.read(bf, sizeof(bf));
}

MailBoxStatus::MailBoxStatus(const char *mailbox, YWindow *aParent): 
    YWindow(aParent), fMailBox(newstr(mailbox)), check(this) {
    setSize(16, 16);
    fMailboxCheckTimer = 0;
    fState = mbxNoMail;
    if (fMailBox) {
        MSG((_("Using MailBox \"%s\"\n"), fMailBox));
        check.setURL(fMailBox);

        fMailboxCheckTimer = new YTimer(mailCheckDelay * 1000);
        if (fMailboxCheckTimer) {
            fMailboxCheckTimer->timerListener(this);
            fMailboxCheckTimer->start();
        }
        checkMail();
    }
}

MailBoxStatus::~MailBoxStatus() {
    if (fMailboxCheckTimer) {
        fMailboxCheckTimer->stop();
        fMailboxCheckTimer->timerListener(NULL);
    }
    delete fMailboxCheckTimer; fMailboxCheckTimer = 0;
    delete [] fMailBox; fMailBox = 0;
}

void MailBoxStatus::paint(Graphics &g, int /*x*/, int /*y*/, unsigned int /*width*/, unsigned int /*height*/) {
    YPixmap *pixmap;
    switch (fState) {
    case mbxHasMail:
        pixmap = mailPixmap;
        break;
    case mbxHasNewMail:
        pixmap = newMailPixmap;
        break;
    case mbxHasUnreadMail:
        pixmap = unreadMailPixmap;
        break;
    case mbxNoMail:
        pixmap = noMailPixmap;
        break;
    default:
        pixmap = errMailPixmap;
        break;
    }
    
    if (pixmap == NULL || pixmap->mask()) {
#ifdef CONFIG_GRADIENTS
	class YPixbuf * gradient(parent()->getGradient());

	if (gradient)
	    g.copyPixbuf(*gradient, x(), y(), width(), height(), 0, 0);
	else 
#endif	
	if (taskbackPixmap)
	    g.fillPixmap(taskbackPixmap, 0, 0,
			 width(), height(), this->x(), this->y());
        else {
	    g.setColor(taskBarBg);
	    g.fillRect(0, 0, width(), height());
	}
    }
    if (pixmap)
        g.drawPixmap(pixmap, 0, 0);
}

void MailBoxStatus::handleClick(const XButtonEvent &up, int count) {
    if (up.button == 3 && count == 1 && IS_BUTTON(up.state, Button3Mask))
        taskBar->contextMenu(up.x_root, up.y_root);
    else if ((taskBarLaunchOnSingleClick ? up.button == 2
				         : up.button == 1) && count == 1)
	checkMail();
    else if (mailCommand && mailCommand[0] && up.button == 1 &&
	(taskBarLaunchOnSingleClick ? count == 1 : !(count % 2)))
	wmapp->runCommandOnce(mailClassHint, mailCommand);
}

void MailBoxStatus::handleCrossing(const XCrossingEvent &crossing) {
    if (crossing.type == EnterNotify) {
#if 0
        if (countMailMessages) {
            struct stat st;
            unsigned long countSize;
            time_t countTime;

            if (stat(fMailBox, &st) != -1) {
                countSize = st.st_size;
                countTime = st.st_mtime;
            } else {
                countSize = 0;
                countTime = 0;
            }
            if (fLastCountSize != countSize || fLastCountTime != countTime)
            fLastCountSize = countSize;
        } else {
            setToolTip(0);
        }
#endif
    }
    YWindow::handleCrossing(crossing);
}

void MailBoxStatus::checkMail() {
    check.startCheck();
}

void MailBoxStatus::mailChecked(MailBoxState mst, long count) {
    if (mst != mbxError)
        fMailboxCheckTimer->start();
    if (mst != fState) {
        fState = mst;
        repaint();
        if (fState == mbxHasNewMail)
            newMailArrived();
    }
    if (fState == mbxError)
        setToolTip(_("Error checking mailbox."));
    else {
        char s[128];
        if (count != -1) {
            snprintf(s, sizeof(s),
                    count == 1 ?
                    _("%ld mail message.") :
                    _("%ld mail messages."), // too hard to do properly
                    count);
            setToolTip(s);
        } else {
            setToolTip(0);
        }
    }
}

void MailBoxStatus::newMailArrived() {
    if (beepOnNewMail)
	app->alert();
    if (newMailCommand && newMailCommand[0])
        app->runCommand(newMailCommand);
}



bool MailBoxStatus::handleTimer(YTimer *t) {
    if (t != fMailboxCheckTimer)
        return false;
    checkMail();
    return true;
}

#endif
