#ifndef QTCYLINDERGRIDFRAME_H_
#define QTCYLINDERGRIDFRAME_H_

#include <qframe.h>
#include <qpainter.h>

class QtCylinderGridFrame : public QFrame
{
	Q_OBJECT
public:
	QtCylinderGridFrame(QWidget *parent, int widgetSize, int cylinderSize, double id, double od, double step, double epsize);
	~QtCylinderGridFrame();
	void toCylinderCoord(int xwidget, int ywidget, double *px, double *py);
	void fromCylinderCoord(double x, double y, int *pxwidget, int *pywidget);
	bool inCylinder(double x, double y);
	int selectedX();
	int selectedY();
	double selectedXEntry();
	double selectedYEntry();
	void setSelectedEntry(double x, double y);
	void setID(double mm);		// set inner diam mm
	void setOD(double mm);		// cylinder outer diam size in mm
	void setStep(double mm);		// entry point spacing in mm
	void setDotSize(double mm);	// entry point dot size in mm
	void setSelected(int x, int y);

signals:
	void entryPointChanged(double x, double y);
protected:
	void drawContents( QPainter *p );
	void mousePressEvent(QMouseEvent* e);
	QSize sizeHint() const { return _size; };
private:
	void drawEntryPoint(QPainter *p, int i, int j, double epsize);
	bool isSelected(int x, int y);
	QSize _size;
	int _widgetSize;		// widget total size in pixels
	int _cylinderSize;	// cylinder size in pixels
	double _id;			// cylinder inner diam size in mm
	double _od;			// cylinder outer diam size in mm
	double _step;		// entry point spacing in mm
	double _epSize;	// entry point dot size in mm
	int _selectedX;
	int _selectedY;
};

#endif /*QTCYLINDERGRIDFRAME_H_*/
