// SPDX-License-Identifier: GPL-3.0-or-later
#include "libclient/wasmsupport.h"
#include "libclient/net/login.h"
#include "libclient/utils/wasmpersistence.h"
#include <QString>
#include <emscripten.h>

#define DRAWPILE_BROWSER_AUTH_CANCEL -1
#define DRAWPILE_BROWSER_AUTH_USERNAME_SELECTED 1
#define DRAWPILE_BROWSER_AUTH_IDENTIFIED 2

static net::LoginHandler *currentLoginHandler;
static bool persistentFileSystemSyncInProgress;

extern "C" void drawpileFinishPersistentFileSystemSync()
{
	persistentFileSystemSyncInProgress = false;
}

EM_JS(void, drawpileMountPersistentFileSystem, (int argc, char **argv), {
	try {
		console.log("Loading persistent file system");
		FS.mkdir("/appdata");
		FS.mount(IDBFS, {}, "/appdata");
		FS.syncfs(
			true, function(err) {
				if(err) {
					console.error("Error loading persistent file system:", err);
				} else {
					console.log("Persistent file system loaded");
					_drawpileInitScheduleSyncPersistentFileSystem();
				}
				setTimeout(_drawpileMain.bind(null, argc, argv), 0);
			});
	} catch(e) {
		console.error("Error mounting persistent file system:", e);
		setTimeout(_drawpileMain.bind(null, argc, argv), 0);
	}
})

EM_JS(void, drawpileSyncPersistentFileSystem, (), {
	try {
		FS.syncfs(
			false, function(err) {
				if(err) {
					console.error("Error persisting file system:", err);
				}
				_drawpileFinishPersistentFileSystemSync();
			});
	} catch(e) {
		console.error("Error trying to persist file system:", err);
		_drawpileFinishPersistentFileSystemSync();
	}
})

namespace browser {

bool hasLowPressurePen()
{
	int lowPressurePen = EM_ASM_INT(return window.drawpileHasLowPressurePen(););
	return lowPressurePen != 0;
}

void showLoginModal(net::LoginHandler *loginHandler)
{
	cancelLoginModal(currentLoginHandler);
	currentLoginHandler = loginHandler;
	if(loginHandler) {
		EM_ASM(window.drawpileShowLoginModal(););
	}
}

void cancelLoginModal(net::LoginHandler *loginHandler)
{
	if(loginHandler && currentLoginHandler == loginHandler) {
		currentLoginHandler = nullptr;
		EM_ASM(window.drawpileCancelLoginModal(););
	}
}

void authenticate(net::LoginHandler *loginHandler, const QByteArray &payload)
{
	if(loginHandler == currentLoginHandler) {
		EM_ASM(window.drawpileAuthenticate($0, $1);
			   , payload.constData(), int(payload.size()));
	}
}

void mountPersistentFileSystem(int argc, char **argv)
{
	drawpileMountPersistentFileSystem(argc, argv);
}

bool syncPersistentFileSystem()
{
	if(persistentFileSystemSyncInProgress) {
		return false;
	} else {
		persistentFileSystemSyncInProgress = true;
		drawpileSyncPersistentFileSystem();
		return true;
	}
}

}

// Stuff called from JavaScript.

extern "C" void drawpileHandleBrowserAuth(int type, char *rawArg)
{
	QString arg;
	if(rawArg) {
		arg = QString::fromUtf8(rawArg);
		free(rawArg);
	}

	if(currentLoginHandler) {
		switch(type) {
		case DRAWPILE_BROWSER_AUTH_CANCEL:
			currentLoginHandler->cancelBrowserAuth();
			break;
		case DRAWPILE_BROWSER_AUTH_USERNAME_SELECTED:
			currentLoginHandler->selectBrowserAuthUsername(arg);
			break;
		case DRAWPILE_BROWSER_AUTH_IDENTIFIED:
			currentLoginHandler->browserAuthIdentified(arg);
			break;
		default:
			qWarning(
				"Unknown browser auth type %d with argument '%s'", type,
				qUtf8Printable(arg));
			break;
		}
	} else {
		qWarning(
			"No login handler set to handle auth type %d with argument '%s'",
			type, qUtf8Printable(arg));
	}
}
