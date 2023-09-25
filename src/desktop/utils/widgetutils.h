// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WIDGETUTILS_H
#define WIDGETUTILS_H
#include <QObject>
#include <QScroller>

class QAbstractScrollArea;
class QCursor;
class QFormLayout;
class QFrame;
class QVBoxLayout;
class QWidget;

namespace utils {

class ScopedOverrideCursor {
public:
	ScopedOverrideCursor(); // Uses Qt::WaitCursor.
	ScopedOverrideCursor(const QCursor &cursor);
	~ScopedOverrideCursor();
};

class ScopedUpdateDisabler {
public:
	ScopedUpdateDisabler(QWidget *widget);
	~ScopedUpdateDisabler();

private:
	QWidget *m_widget;
	bool m_wasEnabled;
};


// SPDX-SnippetBegin
// SPDX-License-Identifier: GPL-3.0-or-later
// SDPX—SnippetName: Kinetic scroll event filter from Krita
class KisKineticScrollerEventFilter : public QObject {
	Q_OBJECT
public:
	KisKineticScrollerEventFilter(
		QScroller::ScrollerGestureType gestureType,
		QAbstractScrollArea *parent);

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

	QAbstractScrollArea *m_scrollArea;
	QScroller::ScrollerGestureType m_gestureType;
};
// SPDX-SnippetEnd

void showWindow(QWidget *widget, bool maximized = false);

void setWidgetRetainSizeWhenHidden(QWidget *widget, bool retainSize);

void initKineticScrolling(QAbstractScrollArea *scrollArea);
bool isKineticScrollingBarsHidden();

QFormLayout *addFormSection(QVBoxLayout *layout);
int getFormSpacing(QVBoxLayout *layout);
void addFormSpacer(QVBoxLayout *layout);
void addFormSeparator(QVBoxLayout *layout);

}

#endif
