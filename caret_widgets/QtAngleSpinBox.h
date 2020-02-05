#ifndef QTANGLESPINBOX_H_
#define QTANGLESPINBOX_H_

#include <QSpinBox>
#include <QString>

class QtAngleSpinBox : public QSpinBox
{
	public:
	QtAngleSpinBox(int minValue, int maxValue, int step = 1, QWidget * parent = 0, const char * name = 0);

	protected:

	int valueFromText(const QString & text) const;
	QString textFromValue(int value) const;

//	QString mapValueToText( int value );
//	int mapTextToValue( bool *ok );
};


#endif /*QTANGLESPINBOX_H_*/
