#include "QtCylinderGridFrame.h"
#include <QMouseEvent>

QtCylinderGridFrame::QtCylinderGridFrame(QWidget *parent, int widgetSize, int cylinderSize, double id, double od, double step = 0.5, double epsize=0.25)
: QFrame(parent)
, _size(widgetSize, widgetSize)
, _widgetSize(widgetSize)
, _cylinderSize(cylinderSize)
, _id(id), _od(od)
, _step(step)
, _epSize(epsize)
, _selectedX(0)
, _selectedY(0)
{
	setFrameStyle(QFrame::Box | QFrame::Plain);
	setLineWidth(1);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setFixedSize(sizeHint());
};

QtCylinderGridFrame::~QtCylinderGridFrame() {};

void QtCylinderGridFrame::setID(double mm)
{
	_id = mm;
}

void QtCylinderGridFrame::setOD(double mm)
{
	_od = mm;
}

void QtCylinderGridFrame::setStep(double mm)
{
	_step = mm;
}

void QtCylinderGridFrame::setDotSize(double mm)
{
	_epSize = mm;
}

void QtCylinderGridFrame::setSelected(int x, int y)
{
	_selectedX = x;
	_selectedY = y;
}

void QtCylinderGridFrame::setSelectedEntry(double x, double y)
{
	if (_step != 0)
	{
		_selectedX = (int)(x/_step);
		_selectedY = (int)(y/_step);
	}
	update();
}


void QtCylinderGridFrame::toCylinderCoord(int xwidget, int ywidget, double *px, double *py)
{
	*px = (xwidget - _widgetSize/2)*_id/_cylinderSize;
	*py = -(ywidget - _widgetSize/2)*_id/_cylinderSize;
}

// WARNING divide by zero on _id here!
void QtCylinderGridFrame::fromCylinderCoord(double x, double y, int *pxwidget, int *pywidget)
{
	*pxwidget = (int)(x * _cylinderSize/_id + _widgetSize/2);
	*pywidget = (int)(-y * _cylinderSize/_id + _widgetSize/2);
}

bool QtCylinderGridFrame::inCylinder(double x, double y)
{
	return (x*x + y*y < _id*_id/4);
}

inline bool QtCylinderGridFrame::isSelected(int i, int j)
{
	return (i==_selectedX && j==_selectedY);
}

inline int QtCylinderGridFrame::selectedX() { return _selectedX; };
inline int QtCylinderGridFrame::selectedY() { return _selectedY; };
inline double QtCylinderGridFrame::selectedXEntry() { return _selectedX*_step; };
inline double QtCylinderGridFrame::selectedYEntry() { return _selectedY*_step; };

void QtCylinderGridFrame::drawEntryPoint(QPainter *p, int i, int j, double epsize)
{
	double x = i * _step;
	double y = j * _step;
	int ix, iy, iw;

	// convert x,y to pixel coords.
	// Don't use fromCylinderCoord here because that gives us values in widget coordinates.
	ix = (int)(x * _cylinderSize / _id);
	iy = (int)(-y * _cylinderSize / _id);
	iw = (int)(0.5 * epsize * _cylinderSize / _id);
	p->drawEllipse(ix-iw, iy-iw, 2*iw, 2*iw);
}

void QtCylinderGridFrame::drawContents( QPainter *p )
{
	int i, j;
	int outerSize;
	QBrush b1(Qt::black);
	QPen p1(Qt::black);
	QPen p2(Qt::red);
	QPen p3(Qt::blue);

	// Uninitialized grids will div by zero!
	if (_id == 0) return;
	outerSize = (int)(_cylinderSize * _od / _id);
	p->setWindow(-_widgetSize/2, -_widgetSize/2, _widgetSize, _widgetSize);
	p->setPen( p1 );
	p->drawEllipse( -_cylinderSize/2, -_cylinderSize/2, _cylinderSize, _cylinderSize);
	p->drawEllipse( -outerSize/2, -outerSize/2, outerSize, outerSize);
	p->drawLine(0, -outerSize/2, 0, -_cylinderSize/2);

	for (i=0; inCylinder(i*_step, 0); i++)
	{
		for (j=0; inCylinder(i*_step, j*_step); j++)
		{
			if (((i+j)%2) == 0)
			{
				// standard grid, use red
				p->setPen(p2);
			}
			else
			{
				p->setPen(p3);
			}
			drawEntryPoint(p, i, j, _epSize);
			drawEntryPoint(p, -i, j, _epSize);
			drawEntryPoint(p, i, -j, _epSize);
			drawEntryPoint(p, -i, -j, _epSize);
		}
	}
	p->setPen(p1);
	p->setBrush(b1);
	drawEntryPoint(p, _selectedX, _selectedY, _epSize*2);
}


void QtCylinderGridFrame::mousePressEvent ( QMouseEvent * e )
{
	int x=0, y=0;
	double x2, y2;

	// uninitialized grid!
	if (_id==0) return;

//	std::cout << "Mouse pressed: " << e->pos().x() << "," << e->pos().y() << std::endl;
	toCylinderCoord(e->pos().x(), e->pos().y(), &x2, &y2);
	if (inCylinder(x2, y2))
	{
		if (x2>0)	x = (int)(x2/_step + 0.5);
		else x = (int)(x2/_step - 0.5);
		if (y2>0)	y = (int)(y2/_step + 0.5);
		else y = (int)(y2/_step - 0.5);
//		std::cout << "Nearest x,y=" << x << "," << y << " x2,y2=" << x2 << "," << y2 << std::endl;
//		std::cout << "y2=" << y2 << " step=" << _step << " y/s=" << y2/_step << std::endl;
		_selectedX = x;
		_selectedY = y;
		emit entryPointChanged(selectedXEntry(), selectedYEntry());
		update();
	}
}

