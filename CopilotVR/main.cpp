#include "overlaywidget.h"
#include "openvroverlaycontroller.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QCoreApplication::addLibraryPath(".\\plugins");
	QApplication a(argc, argv);

	COpenVROverlayController::SharedInstance()->Init();
	OverlayWidget *pOverlayWidget = new OverlayWidget;

	COpenVROverlayController::SharedInstance()->SetWidget( pOverlayWidget );

	// don't show widgets that you're going display in an overlay
	//w.show();

	return a.exec();
}
