#include "QtTermTCP.h"
#include <QApplication>
#include <qscreen.h>

QApplication * a;

int screenHeight = 0;				// used to control dialog sizes on Android
int screenWidth = 0;

int main(int argc, char *argv[])
{
	a = new QApplication (argc, argv);

	QSize r = a->screens()[0]->size();

	screenHeight = r.height();
	screenWidth = r.width();

	QtTermTCP w;

//#ifdef ANDROID
//	w.setWindowFlags(Qt::Window | Qt::CustomizeWindowHint); // Qt::FramelessWindowHint);
//#endif
	w.show();
	return a->exec();
}
