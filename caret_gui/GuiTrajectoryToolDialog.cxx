/*LICENSE_START*/
/*
 *  Copyright 1995-2002 Washington University School of Medicine
 *
 *  http://brainmap.wustl.edu
 *
 *  This file is part of CARET.
 *
 *  CARET is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  CARET is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CARET; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*LICENSE_END*/


#include <sstream>
#include <set>
#include <cmath>

#include <QApplication>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QToolTip>
#include <QValidator>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QColorDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QObjectList>
#include <QtDebug>
#include <QString>

#include "BrainModelVolume.h"
#include "BrainSet.h"
#include "DebugControl.h"
#include "GuiBrainModelOpenGL.h"
#include "GuiMainWindow.h"
#include "WuQMessageBox.h"
#include "GuiToolBar.h"
#include "GuiTrajectoryToolDialog.h"
#include "TrajectoryFile.h"
#include "global_variables.h"
#include "QtAngleSpinBox.h"
#include "vtkMath.h"


void dumpvec(std::string s, float *v);



/**
 * constructor.
 */
GuiTrajectoryToolDialog::GuiTrajectoryToolDialog(QWidget* parent, BrainSet *bs)
   : QDialog(parent), m_brainSet(bs), m_bSuppressUpdates(false)
{
	QObject *pobj;
	setWindowTitle("SPLASh");
	QTabWidget *tabWidget = new QTabWidget;
	tabWidget->addTab(m_tab1 = setupTab1(), "General");
	tabWidget->addTab(m_tab2 = setupTab2(), "Path");
	tabWidget->addTab(m_tab3 = setupTab3(), "Settings");

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(tabWidget);
	setLayout(vbox);
	refreshAll();
}

QWidget* GuiTrajectoryToolDialog::setupTab1()
{
	QWidget *widget = new QWidget;
	QLabel* label;
	QHBoxLayout *row;
	QVBoxLayout *check1, *check2;
	QVBoxLayout *target1, *target2;
	QVBoxLayout *buttons;
	QVBoxLayout *tab1 = new QVBoxLayout;

	// group box for electrode trajectory list
	QGroupBox* tlistGroup = new QGroupBox("Trajectories");
	tlistGroup->setObjectName("tlistGroup");

	// list widget for trajectory names, and a slot for changed selection
	m_listwidgetTrajectories = new QListWidget;
	QObject::connect( m_listwidgetTrajectories, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT( slotActiveTrajectoryChanged(QListWidgetItem*, QListWidgetItem*) ));

	// and buttons for manipulating the list
	buttons = new QVBoxLayout;
	m_pushbuttonNew = new QPushButton("New");
	buttons->addWidget(m_pushbuttonNew);
	QObject::connect( m_pushbuttonNew, SIGNAL( clicked() ), this, SLOT( slotNewTrajectory() ));
	m_pushbuttonRemove = new QPushButton("Remove");
	buttons->addWidget(m_pushbuttonRemove);
	QObject::connect( m_pushbuttonRemove, SIGNAL( clicked() ), this, SLOT( slotRemoveTrajectory() ));
	m_pushbuttonRename = new QPushButton("Rename");
	buttons->addWidget(m_pushbuttonRename);
	QObject::connect( m_pushbuttonRename, SIGNAL( clicked() ), this, SLOT( slotRenameTrajectory() ));

	// make a row for the list widget and buttons
	row = new QHBoxLayout;
	row->addWidget(m_listwidgetTrajectories);
	row->addLayout(buttons);

	// assign the row to the list group, then add the group to the tab's vbox layout
	tlistGroup->setLayout(row);
	tab1->addWidget(tlistGroup);


	// Group box for display controls
	m_targetDisplayGroup = new QGroupBox("Display");
	m_targetDisplayGroup->setObjectName("displayGroup");

	// Put a couple of the checkboxes here
	check1 = new QVBoxLayout;
	check2 = new QVBoxLayout;

	// display skull?
	m_checkboxDisplaySkull = new QCheckBox("Show Skull");
	check2->addWidget(m_checkboxDisplaySkull);
	QObject::connect( m_checkboxDisplaySkull, SIGNAL( toggled( bool ) ), this, SLOT( slotCheckBoxDisplaySkullToggled( bool ) ) );

	// display brain?
	m_checkboxDisplayBrain = new QCheckBox("Show Brain");
	check2->addWidget(m_checkboxDisplayBrain);
	QObject::connect( m_checkboxDisplayBrain, SIGNAL( toggled( bool ) ), this, SLOT( slotCheckBoxDisplayBrainToggled( bool ) ) );

	// display cylinder?
	m_checkboxDisplayCylinder = new QCheckBox("Show Cylinder");
	check1->addWidget(m_checkboxDisplayCylinder);
	QObject::connect( m_checkboxDisplayCylinder, SIGNAL( toggled( bool ) ), this, SLOT( slotCheckBoxDisplayCylinderToggled( bool ) ) );

	// display trajectory?
	m_checkboxDisplayTrajectory = new QCheckBox("Show Trajectory");
	check1->addWidget(m_checkboxDisplayTrajectory);
	QObject::connect( m_checkboxDisplayTrajectory, SIGNAL( toggled( bool ) ), this, SLOT( slotCheckBoxDisplayTrajectoryToggled( bool ) ) );

	// put the two checkbox vbox into a row
	row = new QHBoxLayout;
	row->addLayout(check1);
	row->addLayout(check2);

	// assign row to display group
	m_targetDisplayGroup->setLayout(row);

	// add display group to tab vbox
	tab1->addWidget(m_targetDisplayGroup);


	// Group box for target location stuff
	m_targetNodeGroup = new QGroupBox("Target");

	// Target group box has two columns
	target1 = new QVBoxLayout;
	target2 = new QVBoxLayout;

	// Target X in second column
	row = new QHBoxLayout;
	label = new QLabel("Target X: ");
	m_lineeditTargetX = new QLineEdit;
	m_lineeditTargetX->setValidator( new QDoubleValidator( -999.0, 999.0, 2 ) );
	row->addWidget(label);
	row->addWidget(m_lineeditTargetX);
	target2->addLayout(row);

	// target Y in second column
	row = new QHBoxLayout;
	label = new QLabel("Target Y: ");
	m_lineeditTargetY = new QLineEdit;
	m_lineeditTargetY->setValidator( new QDoubleValidator( -999.0, 999.0, 2 ) );
	row->addWidget(label);
	row->addWidget(m_lineeditTargetY);
	target2->addLayout(row);

	// Target Z in second column
	row = new QHBoxLayout;
	label = new QLabel("Target Z: ");
	m_lineeditTargetZ = new QLineEdit;
	m_lineeditTargetZ->setValidator( new QDoubleValidator( -999.0, 999.0, 2 ) );
	row->addWidget(label);
	row->addWidget(m_lineeditTargetZ);
	target2->addLayout(row);

	// use node id checkbox
	m_checkboxUseNodeID = new QCheckBox("Use Node ID");
	m_checkboxUseNodeID->setObjectName("Check1");
	QObject::connect( m_checkboxUseNodeID, SIGNAL( toggled( bool ) ), this, SLOT( slotCheckBoxToggled( bool ) ) );
	target1->addWidget(m_checkboxUseNodeID);

	// node id

	row = new QHBoxLayout;
	label = new QLabel("Node ID: ");
	m_lineeditNodeID = new QLineEdit;
	m_lineeditNodeID->setValidator(
			new QIntValidator(0, m_brainSet->getActiveFiducialSurface()->getCoordinateFile()->getNumberOfCoordinates()-1) );
	QObject::connect( m_lineeditNodeID, SIGNAL(lostFocus()), this, SLOT(slotNodeIDLostFocus()));
	row->addWidget(label);
	row->addWidget(m_lineeditNodeID);
	target1->addLayout(row);

	QPushButton *pushbuttonLastSelected = new QPushButton("Use Selection");
	pushbuttonLastSelected->setObjectName("pb1");
	QObject::connect( pushbuttonLastSelected, SIGNAL( clicked() ), this, SLOT( slotLastSelectedButtonPushed() ));
	target1->addWidget(pushbuttonLastSelected);

	// now make a row for target1 and target2, then make the row layout for target group box
	row = new QHBoxLayout;
	row->addLayout(target1);
	row->addLayout(target2);
	m_targetNodeGroup->setLayout(row);
	tab1->addWidget(m_targetNodeGroup);


	// Group box for trajectory stuff
	m_trajectoryGroup = new QGroupBox("Cylinder Placement");

	// layout for group box
	QVBoxLayout *trajVBoxLayout = new QVBoxLayout;

	// closest point button
	QPushButton *pushbuttonCylinder = new QPushButton("Default Cylinder Placement");
	pushbuttonCylinder->setObjectName("pb2");
	trajVBoxLayout->addWidget(pushbuttonCylinder);
	QObject::connect( pushbuttonCylinder, SIGNAL( clicked() ), this, SLOT( slotCylinderButtonPushed() ));

	// angle spinboxes
	row = new QHBoxLayout;
	label = new QLabel("Elevation: ");
	m_spinboxTheta = new QtAngleSpinBox(-90000, 90000, 500);
	m_spinboxTheta->setValue(0);
	QObject::connect( m_spinboxTheta, SIGNAL(valueChanged(int)), this, SLOT(slotThetaValueChanged(int)));
	row->addWidget(label);
	row->addWidget(m_spinboxTheta);
	trajVBoxLayout->addLayout(row);

	row = new QHBoxLayout;
	label = new QLabel("Tilt: ");
	m_spinboxPhi = new QtAngleSpinBox(-90000, 90000, 500);
	m_spinboxPhi->setValue(0);
	QObject::connect( m_spinboxPhi, SIGNAL(valueChanged(int)), this, SLOT(slotPhiValueChanged(int)));
	row->addWidget(label);
	row->addWidget(m_spinboxPhi);
	trajVBoxLayout->addLayout(row);

	// Center X
	row = new QHBoxLayout;
	label = new QLabel("Center X: ");
	m_lineeditCenterX = new QLineEdit;
	m_lineeditCenterX->setReadOnly(true);
	row->addWidget(label);
	row->addWidget(m_lineeditCenterX);
	trajVBoxLayout->addLayout(row);

	// Center y
	row = new QHBoxLayout;
	label = new QLabel("Center Y: ");
	m_lineeditCenterY = new QLineEdit;
	m_lineeditCenterY->setReadOnly(true);
	row->addWidget(label);
	row->addWidget(m_lineeditCenterY);
	trajVBoxLayout->addLayout(row);

	// center z
	row = new QHBoxLayout;
	label = new QLabel("Center Z: ");
	m_lineeditCenterZ = new QLineEdit;
	m_lineeditCenterZ->setReadOnly(true);
	row->addWidget(label);
	row->addWidget(m_lineeditCenterZ);
	trajVBoxLayout->addLayout(row);

	// rest angle
	row = new QHBoxLayout;
	label = new QLabel("Rest angle: ");
	m_lineeditRestAngle = new QLineEdit;
	m_lineeditRestAngle->setReadOnly(true);
	row->addWidget(label);
	row->addWidget(m_lineeditRestAngle);
	trajVBoxLayout->addLayout(row);

	m_trajectoryGroup->setLayout(trajVBoxLayout);
	tab1->addWidget(m_trajectoryGroup);

	widget->setLayout(tab1);

	return widget;
}

QWidget* GuiTrajectoryToolDialog::setupTab2()
{
	QWidget *widget = new QWidget;
	QLabel* label;
	QHBoxLayout *row;
	QVBoxLayout *tab2;
	QVBoxLayout *groupLayout = new QVBoxLayout;
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();

	tab2 = new QVBoxLayout;

	// Group box for path analysis stuff
	QGroupBox* pathGroup = new QGroupBox("Trajectory Path Analysis");
	pathGroup->setObjectName("pathGroup");

	// path radius
	row = new QHBoxLayout;
	label = new QLabel("Path radius: ");
	m_spinboxPathRadius = new QDoubleSpinBox;
	m_spinboxPathRadius->setMinimum(0.5);
	m_spinboxPathRadius->setMaximum(10);
	m_spinboxPathRadius->setSingleStep(0.5);
	m_spinboxPathRadius->setDecimals(1);
	QObject::connect( m_spinboxPathRadius, SIGNAL(valueChanged(double)), this, SLOT(slotPathRadiusChanged(double)));
	row->addWidget(label);
	row->addWidget(m_spinboxPathRadius);
	groupLayout->addLayout(row);

	m_checkboxUseDepthTool = new QCheckBox("Use Depth Tool");
	groupLayout->addWidget(m_checkboxUseDepthTool);
	QObject::connect( m_checkboxUseDepthTool, SIGNAL( toggled( bool ) ), this, SLOT( slotCheckBoxUseDepthToolToggled( bool ) ) );

	// DepthTool tolerance
	row = new QHBoxLayout;
	label = new QLabel("Depth tool tolerance: ");
	m_spinboxDepthToolTolerance = new QDoubleSpinBox;
	m_spinboxDepthToolTolerance->setMinimum(0.1);
	m_spinboxDepthToolTolerance->setMaximum(100);
	m_spinboxDepthToolTolerance->setSingleStep(0.1);
	m_spinboxDepthToolTolerance->setDecimals(1);
	QObject::connect(m_spinboxDepthToolTolerance, SIGNAL(floatValueChanged(double)), this, SLOT(slotDepthToolToleranceChanged(double)));
	row->addWidget(label);
	row->addWidget(m_spinboxDepthToolTolerance);
	groupLayout->addLayout(row);

	// and the slider
	row = new QHBoxLayout;
	m_sliderDepthTool = new QSlider(Qt::Horizontal);
	m_sliderDepthTool->setTickInterval(10);
	m_sliderDepthTool->setMinimum(0);
	m_sliderDepthTool->setMaximum(100);
	connect( m_sliderDepthTool, SIGNAL( valueChanged( int ) ), this, SLOT( slotDepthToolSliderValueChanged( int ) ) );
	label = new QLabel("Depth(mm):");
	m_labelDepth = new QLabel("0");
	row->addWidget(m_sliderDepthTool);
	row->addWidget(label);
	groupLayout->addLayout(row);

	row = new QHBoxLayout;
	label = new QLabel("Grid orientation: ");
	m_spinboxGridRotation = new QtAngleSpinBox(-90000, 90000, 500);
	m_spinboxGridRotation->setValue(0);
	QObject::connect( m_spinboxGridRotation, SIGNAL(valueChanged(int)), this, SLOT(slotGridRotationChanged(int)));

	row->addWidget(label);
	row->addWidget(m_spinboxGridRotation);
	groupLayout->addLayout(row);


	// grid
	m_cylinderGridFrame = new QtCylinderGridFrame(NULL, 300, 250, 0, 0, 0.5, 0.25);
	connect( m_cylinderGridFrame, SIGNAL(entryPointChanged(double, double)), this, SLOT( slotCylinderGridSelectionChanged(double, double)));
	m_cylinderGridFrame->show();
	groupLayout->addWidget(m_cylinderGridFrame);

	// grid entry point
	m_gridEntryPoint = new QLabel;
	groupLayout->addWidget(m_gridEntryPoint);

	// assign layout to group box, then group box to layout, then layout to widget
	pathGroup->setLayout(groupLayout);
	tab2->addWidget(pathGroup);
	widget->setLayout(tab2);

	return widget;
}

QWidget * GuiTrajectoryToolDialog::setupTab3()
{
	QWidget *widget = new QWidget;
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	QLabel* label;
	QHBoxLayout *row;
	QVBoxLayout *tab3;

	tab3 = new QVBoxLayout;

	// Group box for colors et al
	QGroupBox* settings = new QGroupBox("Colors");
	QVBoxLayout *vboxGroup = new QVBoxLayout;

	// path radius
	row = new QHBoxLayout;
	row->addWidget(new QLabel("Skull surface color: "));
	m_labelSkullSurfaceColor = new QLabel;
	m_labelSkullSurfaceColor->setScaledContents(true);
	row->addWidget(m_labelSkullSurfaceColor);
	QPushButton *pushbuttonSkullSurface = new QPushButton("Edit");
	row->addWidget(pushbuttonSkullSurface);
	QObject::connect( pushbuttonSkullSurface, SIGNAL( clicked() ), this, SLOT( slotEditSkullSurfaceColor() ));
	vboxGroup->addLayout(row);

	row = new QHBoxLayout;
	row->addWidget(new QLabel("Skull annulus color: "));
	m_labelSkullSurfaceAnnulusColor = new QLabel;
	m_labelSkullSurfaceAnnulusColor->setScaledContents(true);
	row->addWidget(m_labelSkullSurfaceAnnulusColor);
	QPushButton *pushbuttonSkullSurfaceAnnulus = new QPushButton("Edit");
	QObject::connect( pushbuttonSkullSurfaceAnnulus, SIGNAL( clicked() ), this, SLOT( slotEditSkullSurfaceAnnulusColor() ));
	row->addWidget(pushbuttonSkullSurfaceAnnulus);
	vboxGroup->addLayout(row);

	row = new QHBoxLayout;
	row->addWidget(new QLabel("Brain surface color: "));
	m_labelBrainSurfaceColor = new QLabel;
	m_labelBrainSurfaceColor->setScaledContents(true);
	row->addWidget(m_labelBrainSurfaceColor);
	QPushButton *pushbuttonBrainSurface = new QPushButton("Edit");
	row->addWidget(pushbuttonBrainSurface);
	QObject::connect( pushbuttonBrainSurface, SIGNAL( clicked() ), this, SLOT( slotEditBrainSurfaceColor() ));
	vboxGroup->addLayout(row);

	row = new QHBoxLayout;
	row->addWidget(new QLabel("Cylinder sides color: "));
	m_labelCylinderSidesColor = new QLabel;
	m_labelCylinderSidesColor->setScaledContents(true);
	row->addWidget(m_labelCylinderSidesColor);
	QPushButton *pushbuttonCylinderSides = new QPushButton("Edit");
	QObject::connect( pushbuttonCylinderSides, SIGNAL( clicked() ), this, SLOT( slotEditCylinderSidesColor() ));
	row->addWidget(pushbuttonCylinderSides);
	vboxGroup->addLayout(row);

	row = new QHBoxLayout;
	row->addWidget(new QLabel("Cylinder ends color: "));
	m_labelCylinderEndsColor = new QLabel;
	m_labelCylinderEndsColor->setScaledContents(true);
	row->addWidget(m_labelCylinderEndsColor);
	QPushButton *pushbuttonCylinderEnds = new QPushButton("Edit");
	QObject::connect( pushbuttonCylinderEnds, SIGNAL( clicked() ), this, SLOT( slotEditCylinderEndsColor() ));
	row->addWidget(pushbuttonCylinderEnds);
	vboxGroup->addLayout(row);

	row = new QHBoxLayout;
	row->addWidget(new QLabel("Path line color: "));
	m_labelPathLineColor = new QLabel;
	m_labelPathLineColor->setScaledContents(true);
	row->addWidget(m_labelPathLineColor);
	QPushButton *pushbuttonPathLine = new QPushButton("Edit");
	QObject::connect( pushbuttonPathLine, SIGNAL( clicked() ), this, SLOT( slotEditPathLineColor() ));
	row->addWidget(pushbuttonPathLine);
	vboxGroup->addLayout(row);

	row = new QHBoxLayout;
	row->addWidget(new QLabel("Path markers color: "));
	m_labelPathMarkersColor = new QLabel;
	m_labelPathMarkersColor->setScaledContents(true);
	row->addWidget(m_labelPathMarkersColor);
	QPushButton *pushbuttonPathMarkers = new QPushButton("Edit");
	QObject::connect( pushbuttonPathMarkers, SIGNAL( clicked() ), this, SLOT( slotEditPathMarkersColor() ));
	row->addWidget(pushbuttonPathMarkers);
	vboxGroup->addLayout(row);

	row = new QHBoxLayout;
	row->addWidget(new QLabel("Flat map nodes color: "));
	m_labelFlatMapNodesColor = new QLabel;
	m_labelFlatMapNodesColor->setScaledContents(true);
	row->addWidget(m_labelFlatMapNodesColor);
	QPushButton *pushbuttonFlatMapNodes = new QPushButton("Edit");
	QObject::connect( pushbuttonFlatMapNodes, SIGNAL( clicked() ), this, SLOT( slotEditFlatMapNodesColor() ));
	row->addWidget(pushbuttonFlatMapNodes);
	vboxGroup->addLayout(row);

	row = new QHBoxLayout;
	row->addWidget(new QLabel("Volume nodes color: "));
	m_labelVolumeNodesColor = new QLabel;
	m_labelVolumeNodesColor->setScaledContents(true);
	row->addWidget(m_labelVolumeNodesColor);
	QPushButton *pushbuttonVolumeNodes = new QPushButton("Edit");
	QObject::connect( pushbuttonVolumeNodes, SIGNAL( clicked() ), this, SLOT( slotEditVolumeNodesColor() ));
	row->addWidget(pushbuttonVolumeNodes);
	vboxGroup->addLayout(row);

	settings->setLayout(vboxGroup);
	tab3->addWidget(settings);

	// Group box for cylinder dimensions
	QGroupBox* cylinderGroup = new QGroupBox("Cylinder");
	vboxGroup = new QVBoxLayout;

	// Cylinder ID
	row = new QHBoxLayout;
	label = new QLabel("Cylinder I.D.: ");
	m_lineeditCylinderID = new QLineEdit;
	m_lineeditCylinderID->setValidator( new QDoubleValidator( 0.1, 100.0, 2 ) );
	QObject::connect( m_lineeditCylinderID, SIGNAL(textChanged(const QString &)), this, SLOT(slotCylinderIDTextChanged(const QString &)));
	row->addWidget(label);
	row->addWidget(m_lineeditCylinderID);
	vboxGroup->addLayout(row);

	// Cylinder OD
	row = new QHBoxLayout;
	label = new QLabel("Cylinder O.D.: ");
	m_lineeditCylinderOD = new QLineEdit;
	m_lineeditCylinderOD->setValidator( new QDoubleValidator( 0.1, 100.0, 2 ) );
	QObject::connect( m_lineeditCylinderOD, SIGNAL(textChanged(const QString &)), this, SLOT(slotCylinderODTextChanged(const QString &)));
	row->addWidget(label);
	row->addWidget(m_lineeditCylinderOD);
	vboxGroup->addLayout(row);

	// Cylinder H
	row = new QHBoxLayout;
	label = new QLabel("Cylinder H: ");
	m_lineeditCylinderH = new QLineEdit;
	m_lineeditCylinderH->setValidator( new QDoubleValidator( 0.1, 100.0, 2 ) );
	QObject::connect( m_lineeditCylinderH, SIGNAL(textChanged(const QString &)), this, SLOT(slotCylinderHTextChanged(const QString &)));
	row->addWidget(label);
	row->addWidget(m_lineeditCylinderH);
	vboxGroup->addLayout(row);

	// Max penetration depth
	row = new QHBoxLayout;
	label = new QLabel("Max Penetration (mm): ");
	m_spinboxMaxPenetrationDepth = new QSpinBox;
	m_spinboxMaxPenetrationDepth->setMinimum(1);
	m_spinboxMaxPenetrationDepth->setMaximum(100);
	m_spinboxMaxPenetrationDepth->setSingleStep(1);
	QObject::connect( m_spinboxMaxPenetrationDepth, SIGNAL(valueChanged(int)), this, SLOT(slotMaxPenetrationDepthChanged(int)));
	row->addWidget(label);
	row->addWidget(m_spinboxMaxPenetrationDepth);
	vboxGroup->addLayout(row);
	cylinderGroup->setLayout(vboxGroup);
	tab3->addWidget(cylinderGroup);

	// Group box for etcetera
	QGroupBox* etcGroup = new QGroupBox("Misc");
	vboxGroup = new QVBoxLayout;

	// Nearby diam
	row = new QHBoxLayout;
	label = new QLabel("Skull nearby diam: ");
	m_lineeditSkullNearbyD = new QLineEdit;
	m_lineeditSkullNearbyD->setValidator( new QDoubleValidator( 0.1, 100.0, 2, m_lineeditSkullNearbyD ) );
	QObject::connect( m_lineeditSkullNearbyD, SIGNAL(textChanged(const QString &)), this, SLOT(slotSkullNearbyDTextChanged(const QString &)));
	row->addWidget(label);
	row->addWidget(m_lineeditSkullNearbyD);
	vboxGroup->addLayout(row);
	etcGroup->setLayout(vboxGroup);
	tab3->addWidget(etcGroup);

	widget->setLayout(tab3);
	//addTab(tab3, "Settings");
	return widget;
}

void GuiTrajectoryToolDialog::slotActiveTrajectoryChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	if (m_bSuppressUpdates) return;

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	std::string s(current->text().toStdString());
	if (!ptf->setActiveTrajectory(s))
	{
		QMessageBox::critical(this, "Active Trajectory", QString("Cannot set active trajectory - name not found (%1).").arg(s.c_str()),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	}
	else
	{
		refreshTrajectory();
//		updateItems();
		// re-draw surfaces
		theMainWindow->getBrainModelOpenGL()->updateAllGL();
	}

}

void GuiTrajectoryToolDialog::slotNewTrajectory()
{

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	bool ok;
	QString text = QInputDialog::getText(this, "New Trajectory", "Trajectory name:", QLineEdit::Normal, QString::null, &ok);
	if ( ok && !text.isEmpty() )
	{
		ElectrodeTrajectoryP ep(new ElectrodeTrajectory());
		ep->setName(std::string(text.toStdString()));
		if (ptf->getTrajectories().addTrajectory(ep))
		{
			if (ptf->getActiveTrajectory() == NULL)
			{
				// this will actually end up disabling almost all widgets, despite the name.
				enableWidgets();
			}
			ptf->setActiveTrajectory(ep->getName());
			QListWidgetItem *item =new QListWidgetItem(text, m_listwidgetTrajectories);
			m_listwidgetTrajectories->setCurrentItem(item);
			refreshTrajectory();

			// re-draw surfaces
			theMainWindow->getBrainModelOpenGL()->updateAllGL();
		}
		else
		{
			QMessageBox::warning(this, "New Trajectory", "Cannot add trajectory - duplicate name.",
				QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		}
	}
}

void GuiTrajectoryToolDialog::slotRemoveTrajectory()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (ep)	ptf->removeTrajectory(ep->getName());

	// Have to make another trajectory the active trajectory.
	ElectrodeTrajectoryMapConstIterator it = ptf->getTrajectories().begin();
	if (it == ptf->getTrajectories().end())
	{
		// No more trajectories - have to handle this@!!!!!!
		ptf->setActiveTrajectory("");
	}
	else
	{
		ptf->setActiveTrajectory(it->first);
	}
	refreshAll();
	//updateItems();
}

void GuiTrajectoryToolDialog::slotRenameTrajectory()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	bool ok;
	if (ep.get())
	{
		QString text = QInputDialog::getText(
				this,
				"Rename Trajectory",
				"Trajectory name:",
				QLineEdit::Normal,
				QString::null,
				&ok);
		if ( ok && !text.isEmpty() )
		{
			// remove the trajectory, change its name, then add it again.
			ptf->removeTrajectory(ep->getName());
			ep->setName(std::string(text.toStdString()));
			ptf->addTrajectory(ep);
			ptf->setActiveTrajectory(ep->getName());
			//updateItems();
			refreshTrajectory();
		}
	}
	else
	{
		QMessageBox::critical(this, "Rename Trajectory", "Cannot rename trajectory - not found!",
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	}
}


void GuiTrajectoryToolDialog::slotCylinderGridSelectionChanged(double x, double y)
{
	if (m_bSuppressUpdates) return;

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;

	ep->setGridEntry((float)x, (float)y);

	ptf->entryPointChanged(ep);

	refreshTrajectory();

	// Now change cursor position in volume/MRI images.
	updateSelectedVolumeSlices();

	// Mark the file modified
	ptf->setModified();

	// re-draw surfaces
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

}


void
GuiTrajectoryToolDialog::slotEditBrainSurfaceColor()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	QColor c = QColorDialog::getColor( ptf->getBrainSurfaceColor(), this );
  if ( c.isValid() )
  {
  	ptf->setBrainSurfaceColor(c);
		QPixmap pixmap(10, 10);
		pixmap.fill(c);
		m_labelBrainSurfaceColor->setPixmap(pixmap);

		// Mark the file modified
		ptf->setModified();
  }
}

void
GuiTrajectoryToolDialog::slotEditSkullSurfaceColor()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	QColor c = QColorDialog::getColor( ep->getSkullSurfaceColor(), this );
   if ( c.isValid() )
   {
   	ep->setSkullSurfaceColor(c);
		QPixmap pixmap(10, 10);
		pixmap.fill(c);
		m_labelSkullSurfaceColor->setPixmap(pixmap);

		// Mark the file modified
		ptf->setModified();
   }
}

void
GuiTrajectoryToolDialog::slotEditSkullSurfaceAnnulusColor()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	QColor c = QColorDialog::getColor( ep->getSkullSurfaceAnnulusColor(), this );
   if ( c.isValid() )
   {
   	ep->setSkullSurfaceAnnulusColor(c);
		QPixmap pixmap(10, 10);
		pixmap.fill(c);
		m_labelSkullSurfaceAnnulusColor->setPixmap(pixmap);

		// Mark the file modified
		ptf->setModified();
   }
}

void
GuiTrajectoryToolDialog::slotEditCylinderSidesColor()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	QColor c = QColorDialog::getColor( ep->getCylinderSidesColor(), this );
   if ( c.isValid() )
   {
   	ep->setCylinderSidesColor(c);
		QPixmap pixmap(10, 10);
		pixmap.fill(c);
		m_labelCylinderSidesColor->setPixmap(pixmap);

		// Mark the file modified
		ptf->setModified();
   }
}

void
GuiTrajectoryToolDialog::slotEditCylinderEndsColor()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	QColor c = QColorDialog::getColor( ep->getCylinderEndsColor(), this );
   if ( c.isValid() )
   {
   	ep->setCylinderEndsColor(c);
		QPixmap pixmap(10, 10);
		pixmap.fill(c);
		m_labelCylinderEndsColor->setPixmap(pixmap);

		// Mark the file modified
		ptf->setModified();
   }
}

void
GuiTrajectoryToolDialog::slotEditPathLineColor()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	QColor c = QColorDialog::getColor( ep->getPathLineColor(), this );
   if ( c.isValid() )
   {
   	ep->setPathLineColor(c);
		QPixmap pixmap(10, 10);
		pixmap.fill(c);
		m_labelPathLineColor->setPixmap(pixmap);

		// Mark the file modified
		ptf->setModified();
   }
}

void
GuiTrajectoryToolDialog::slotEditPathMarkersColor()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	QColor c = QColorDialog::getColor( ep->getPathMarkersColor(), this );
   if ( c.isValid() )
   {
   	ep->setPathMarkersColor(c);
		QPixmap pixmap(10, 10);
		pixmap.fill(c);
		m_labelPathMarkersColor->setPixmap(pixmap);

		// Mark the file modified
		ptf->setModified();
   }
}




void
GuiTrajectoryToolDialog::slotEditFlatMapNodesColor()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	QColor c = QColorDialog::getColor( ep->getFlatMapNodesColor(), this );
   if ( c.isValid() )
   {
   	ep->setFlatMapNodesColor(c);
		QPixmap pixmap(10, 10);
		pixmap.fill(c);
		m_labelFlatMapNodesColor->setPixmap(pixmap);

		// Mark the file modified
		ptf->setModified();
   }
}

void
GuiTrajectoryToolDialog::slotEditVolumeNodesColor()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	QColor c = QColorDialog::getColor( ep->getVolumeNodesColor(), this );
   if ( c.isValid() )
   {
   	ep->setVolumeNodesColor(c);
		QPixmap pixmap(10, 10);
		pixmap.fill(c);
		m_labelVolumeNodesColor->setPixmap(pixmap);

		// Mark the file modified
		ptf->setModified();
   }
}



/**
 * destructor.
 */
GuiTrajectoryToolDialog::~GuiTrajectoryToolDialog()
{
}












/**
 * called to display the dialog.
 */
#if 0
void
GuiTrajectoryToolDialog::show()
{
//	updateItems();
   QTabDialog::show();
}
#endif

/**
 * called to close the dialog.
 */
void
GuiTrajectoryToolDialog::close()
{
   QDialog::close();
}

void GuiTrajectoryToolDialog::slotCheckBoxToggled(bool on)
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	if (m_bSuppressUpdates) return;

	ep->setUseNodeID(on);

	// Mark the file modified
	ptf->setModified();
}



void GuiTrajectoryToolDialog::slotCheckBoxDisplayBrainToggled(bool on)
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	if (m_bSuppressUpdates) return;

	ep->setDisplayBrain(on);
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// Mark the file modified
	ptf->setModified();
}


void GuiTrajectoryToolDialog::slotCheckBoxDisplaySkullToggled(bool on)
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	if (m_bSuppressUpdates) return;

	ep->setDisplaySkull(on);
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// Mark the file modified
	ptf->setModified();
}

void GuiTrajectoryToolDialog::slotCheckBoxDisplayCylinderToggled(bool on)
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	if (m_bSuppressUpdates) return;

	ep->setDisplayCylinder(on);
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// Mark the file modified
	ptf->setModified();
}

void GuiTrajectoryToolDialog::slotCheckBoxDisplayTrajectoryToggled(bool on)
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	if (m_bSuppressUpdates) return;

	ep->setDisplayTrajectory(on);
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// Mark the file modified
	ptf->setModified();
}

void GuiTrajectoryToolDialog::slotCheckBoxUseDepthToolToggled(bool on)
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	if (m_bSuppressUpdates) return;

	ep->setUseDepthTool(on);
	refreshTrajectory();
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// Mark the file modified
	ptf->setModified();
}

/**
 * called when place cylinder push button is pushed. This will use the current target point and
 * find the point on the skull surface nearest to the target.
 */
void
GuiTrajectoryToolDialog::slotCylinderButtonPushed()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;

	ptf->targetPositionChanged(ep);
	refreshTrajectory();

	// re-draw surfaces
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// Mark the file modified
	ptf->setModified();
}

/**
 * called when place cylinder push button is pushed
 */
void
GuiTrajectoryToolDialog::slotLastSelectedButtonPushed()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;

	// Lets see if we can get the coords of the currently selected node.....
	GuiBrainModelOpenGL* brainModelOpenGL = theMainWindow->getBrainModelOpenGL();
	BrainModelOpenGLSelectedItem node = GuiBrainModelOpenGL::getOpenGLDrawing()->getSelectedNode();

	if (node.getItemIndex1() < 0)
	{
		WuQMessageBox::information(this, "No Node Selected", "You must select a target node.");
	}
	else
	{
		ep->setUseNodeID(true);
		ep->setNodeID(node.getItemIndex1());
		ptf->nodeIDChanged(ep);			// THIS triggers update chain

		// clear node ID symbols from GL display
		m_brainSet->clearNodeHighlightSymbols();

		// refresh stuff in dialog
		ptf->targetPositionChanged(ep);
		refreshTrajectory();

		// re-draw surfaces
		theMainWindow->getBrainModelOpenGL()->updateAllGL();

		// Mark the file modified
		ptf->setModified();
	}
}

void GuiTrajectoryToolDialog::slotDepthToolSliderValueChanged(int newValue)
{
	TrajectoryFile* ptf = NULL;
	ElectrodeTrajectoryP ep;

	if (m_bSuppressUpdates) return;

	ptf = m_brainSet->getTrajectoryFile();
	ep = ptf->getActiveTrajectory();
	if (!ep) return;
	ep->setDepthToolDepthPct(newValue);

	// Now change cursor position in volume/MRI images.
	updateSelectedVolumeSlices();

	// update traj dialog
	refreshTrajectory();

	// Now update graphics
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// Mark the file modified
	ptf->setModified();
}

void GuiTrajectoryToolDialog::updateSelectedVolumeSlices()
{
	int ijk[3];
	int i;
	TrajectoryFile* ptf = NULL;
	ElectrodeTrajectoryP ep;
	BrainModelVolume* bmv = NULL;
	ptf = m_brainSet->getTrajectoryFile();
	ep = ptf->getActiveTrajectory();
	if (!ep) return;

	// don't update volume cursor unless depth tool is active
	if (!ep->getUseDepthTool()) return;

	// Now change cursor position in volume/MRI images.
	bmv = m_brainSet->getBrainModelVolume(-1);
	if (bmv)
	{
		float origin[3];
		float spacing[3];
		double ppt[3];
		VolumeFile *vf = bmv->getUnderlayVolumeFile();
		vf->getOrigin(origin);
		vf->getSpacing(spacing);

		// Determine the location of the slider, then get the volume slices
		ep->getPenetrationPosition(ppt);
		for (i=0; i<3; i++)
		{
			ijk[i] = (int)((ppt[i] - origin[i])/spacing[i]);
		}

		for (i=0; i<BrainModel::NUMBER_OF_BRAIN_MODEL_VIEW_WINDOWS; i++)
		{
			bmv->setSelectedOrthogonalSlices(i, ijk);
			GuiToolBar::updateAllToolBars(false);
		}
	}
	return;
}


/**
 * called when target node text changes
 */
void GuiTrajectoryToolDialog::slotNodeIDTextChanged(const QString &t)
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	if (m_bSuppressUpdates) return;
	ep->setNodeID(t.toInt());

	// make necessary changes to trajectory
	ptf->nodeIDChanged(ep);

	// refresh
	refreshTrajectory();

	// Now update graphics
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// Mark the file modified
	ptf->setModified();
}


/**
 * called when target nodeID LineEdit widget loses focus
 */
void GuiTrajectoryToolDialog::slotNodeIDLostFocus()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	if (m_bSuppressUpdates) return;
	ep->setNodeID(m_lineeditNodeID->text().toInt());

	// make necessary changes to trajectory
	ptf->nodeIDChanged(ep);

	// refresh
	refreshTrajectory();

	// Now update graphics
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// Mark the file modified
	ptf->setModified();
}



/**
 * called when path radius text changes
 */

void GuiTrajectoryToolDialog::slotPathRadiusChanged(double val)
{
	if (m_bSuppressUpdates) return;

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;

	ep->setPathRadius(val);
	ptf->pathNodesChanged(ep);

	// re-draw surfaces
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// Mark the file modified
	ptf->setModified();
}

/*
 * Called when grid rotation spinbox changes
 */

void GuiTrajectoryToolDialog::slotGridRotationChanged(int value)
{
	if (m_bSuppressUpdates) return;

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;

	ep->setGridRotation((float)value/1000.0f);
	ptf->entryPointChanged(ep);

	// Now change cursor position in volume/MRI images.
	updateSelectedVolumeSlices();

	// Update views
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// refresh
	refreshTrajectory();

	// Mark the file modified
	ptf->setModified();
}


/**
 * called when depth tool tolerance text changes
 */

void GuiTrajectoryToolDialog::slotDepthToolToleranceChanged(double val)
{
	if (m_bSuppressUpdates) return;

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	ep->setDepthToolTolerance(val);
//	updateItems();

	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// Mark the file modified
	ptf->setModified();
}


/**
 * called when cylinderID text changes
 */

void GuiTrajectoryToolDialog::slotCylinderIDTextChanged(const QString &t)
{
	if (m_bSuppressUpdates) return;

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	ep->setCylinderID(t.toInt());
//	updateItems();

	ptf->cylinderPositionChanged(ep);

	refreshTrajectory();

	// Mark the file modified
	ptf->setModified();

	theMainWindow->getBrainModelOpenGL()->updateAllGL();
}

/**
 * called when cylinderOD text changes
 */

void GuiTrajectoryToolDialog::slotCylinderODTextChanged(const QString &t)
{
	if (m_bSuppressUpdates) return;

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	ep->setCylinderOD(t.toInt());
	//updateItems();

	ptf->cylinderPositionChanged(ep);

	refreshTrajectory();

	// Mark the file modified
	ptf->setModified();

	theMainWindow->getBrainModelOpenGL()->updateAllGL();
}

/**
 * called when cylinderH text changes
 */

void GuiTrajectoryToolDialog::slotCylinderHTextChanged(const QString &t)
{
	if (m_bSuppressUpdates) return;

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	ep->setCylinderH(t.toInt());
//	updateItems();

	// Mark the file modified
	ptf->setModified();

	theMainWindow->getBrainModelOpenGL()->updateAllGL();
}


void GuiTrajectoryToolDialog::slotMaxPenetrationDepthChanged(int val)
{
	if (m_bSuppressUpdates) return;

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	ep->setMaxPenetrationDepth(val);

	// Update GL. Affected nodes will be updated by the draw.
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// Mark the file modified
	ptf->setModified();
}

/**
 * called when SkullNearbyD text changes
 */

void GuiTrajectoryToolDialog::slotSkullNearbyDTextChanged(const QString &t)
{
	if (m_bSuppressUpdates) return;

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;
	ep->setSkullNearbyD(t.toDouble());
//	updateItems();

	ptf->cylinderPositionChanged(ep);

	refreshTrajectory();

	// Mark the file modified
	ptf->setModified();
	theMainWindow->getBrainModelOpenGL()->updateAllGL();
}


void GuiTrajectoryToolDialog::slotThetaValueChanged(int value)
{
	if (m_bSuppressUpdates) return;

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;

	ep->setTheta((float)value/1000.0f);
	ptf->trajectoryAnglesChanged(ep);
	updateSelectedVolumeSlices();
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// refresh
	refreshTrajectory();

	// Mark the file modified
	ptf->setModified();
}

void GuiTrajectoryToolDialog::slotPhiValueChanged(int value)
{
	if (m_bSuppressUpdates) return;

	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	if (!ep) return;

	ep->setPhi((float)value/1000.0f);

	ptf->trajectoryAnglesChanged(ep);
	updateSelectedVolumeSlices();
	theMainWindow->getBrainModelOpenGL()->updateAllGL();

	// refresh
	refreshTrajectory();

	// Mark the file modified
	ptf->setModified();
}



void GuiTrajectoryToolDialog::refreshColors()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();
	QColor color(0, 0, 0);

	QPixmap pixmap(10, 10);

	if (ep) color = ep->getSkullSurfaceColor();
	pixmap.fill(color);
	m_labelSkullSurfaceColor->setPixmap(pixmap);

	if (ep) color = ep->getSkullSurfaceAnnulusColor();
	pixmap.fill(color);
	m_labelSkullSurfaceAnnulusColor->setPixmap(pixmap);

	if (ep) color = ptf->getBrainSurfaceColor();
	pixmap.fill(color);
	m_labelBrainSurfaceColor->setPixmap(pixmap);

	if (ep) color = ep->getCylinderSidesColor();
	pixmap.fill(color);
	m_labelCylinderSidesColor->setPixmap(pixmap);

	if (ep) color = ep->getCylinderEndsColor();
	pixmap.fill(color);
	m_labelCylinderEndsColor->setPixmap(pixmap);

	if (ep) color = ep->getPathLineColor();
	pixmap.fill(color);
	m_labelPathLineColor->setPixmap(pixmap);

	if (ep) color = ep->getPathMarkersColor();
	pixmap.fill(color);
	m_labelPathMarkersColor->setPixmap(pixmap);

	if (ep) color = ep->getFlatMapNodesColor();
	pixmap.fill(color);
	m_labelFlatMapNodesColor->setPixmap(pixmap);

	if (ep) color = ep->getVolumeNodesColor();
	pixmap.fill(color);
	m_labelVolumeNodesColor->setPixmap(pixmap);

}

void GuiTrajectoryToolDialog::refreshTrajectory()
{
	int i;
	bool b;
	double xyz[3];
	double x, y;
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();

	m_bSuppressUpdates = true;

	refreshColors();

	// display checkboxes
	m_checkboxDisplaySkull->setChecked(ep->getDisplaySkull());
	m_checkboxDisplayBrain->setChecked(ep->getDisplayBrain());
	m_checkboxDisplayTrajectory->setChecked(ep->getDisplayTrajectory());
	m_checkboxDisplayCylinder->setChecked(ep->getDisplayCylinder());

	// node id
	i = ep->getNodeID();
	if (i >= 0)
	{
		m_lineeditNodeID->setText(QString("%1").arg(i));
	}

	// target xyz
	ep->getTarget(xyz);
	m_lineeditTargetX->setText(QString("%1").arg(xyz[0]));
	m_lineeditTargetY->setText(QString("%1").arg(xyz[1]));
	m_lineeditTargetZ->setText(QString("%1").arg(xyz[2]));

	b = ep->getUseNodeID();
	m_checkboxUseNodeID->setChecked(b);
	if (b)
	{
		m_lineeditNodeID->setReadOnly(false);
		m_lineeditTargetX->setReadOnly(true);
		m_lineeditTargetY->setReadOnly(true);
		m_lineeditTargetZ->setReadOnly(true);
	}
	else
	{
		m_lineeditNodeID->setReadOnly(true);
		m_lineeditTargetX->setReadOnly(false);
		m_lineeditTargetY->setReadOnly(false);
		m_lineeditTargetZ->setReadOnly(false);
	}

	// cylinder xyz
	ep->getCylinderCenter(xyz);
	m_lineeditCenterX->setText(QString("%1").arg(xyz[0]));
	m_lineeditCenterY->setText(QString("%1").arg(xyz[1]));
	m_lineeditCenterZ->setText(QString("%1").arg(xyz[2]));
	m_lineeditRestAngle->setText(QString("%1").arg(ep->getCylinderBaseAngle()));

	// and update the trajectory angle spinboxes...
	float theta = ep->getTheta();
	float phi = ep->getPhi();

	m_spinboxTheta->setValue((int)rintf(theta*1000));
	m_spinboxPhi->setValue((int)rintf(phi*1000));

	// path radius
	m_spinboxPathRadius->setValue((double)ep->getPathRadius());

	// depth tool
	m_sliderDepthTool->setEnabled(ep->getUseDepthTool());
	if (ep->getUseDepthTool())
	{
		m_labelDepth->setNum((double)ep->getPenetrationDepth());
	}
	else
	{
		m_labelDepth->setText("N/A");
	}

	// cylinder dimensions
	m_lineeditCylinderID->setText(QString("%1").arg(ep->getCylinderID()));
	m_cylinderGridFrame->setID(ep->getCylinderID());

	m_lineeditCylinderOD->setText(QString("%1").arg(ep->getCylinderOD()));
	m_cylinderGridFrame->setOD(ep->getCylinderOD());

	m_lineeditCylinderH->setText(QString("%1").arg(ep->getCylinderH()));

	m_spinboxMaxPenetrationDepth->setValue((int)ep->getMaxPenetrationDepth());

	m_lineeditSkullNearbyD->setText(QString("%1").arg(ep->getSkullNearbyD()));

	//
	ep->getGridEntry(&x, &y);
	m_gridEntryPoint->setText(QString("Electrode location (mm): (%1, %2)").arg(x).arg(y));
	m_cylinderGridFrame->setSelectedEntry(x, y);

	x = ep->getGridRotation();
	m_spinboxGridRotation->setValue((int)rintf(x*1000));

	enableWidgets();
	m_bSuppressUpdates = false;
}


void GuiTrajectoryToolDialog::refreshAll()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();

	if (ep)
		qDebug() << "Active trajectory : " << QString(ep->getName().c_str());
	else
		qDebug() << "Active trajectory : None";

	// refresh trajectory list box
	m_listwidgetTrajectories->clear();
	for (ElectrodeTrajectoryMapIterator it = ptf->getTrajectories().begin(); it!=ptf->getTrajectories().end(); it++)
	{
		QListWidgetItem *item =new QListWidgetItem(QString((it->second)->getName().c_str()), m_listwidgetTrajectories);
		if (ep && ep->getName() == it->second->getName())
		{
			m_listwidgetTrajectories->setCurrentItem(item);
		}
	}

	if (ep)
	{
		refreshTrajectory();
	}
	else
	{
		// refreshTrajectory calls this also. Call it here to make
		// sure things are appropriately disabled.
		enableWidgets();
	}
}

void GuiTrajectoryToolDialog::enableWidgets()
{
	TrajectoryFile* ptf = m_brainSet->getTrajectoryFile();
	ElectrodeTrajectoryP ep = ptf->getActiveTrajectory();

	// If there is no active trajectory, everything but the "New" button
	// should be disabled.
	// If there is an active trajectory, but no target has been selected,
	// then a limited set of widgets get enabled.
	// If there is an active trajectory, and a target has been selected,
	// then all widgets are enabled.

	if (!ep)
	{
		qDebug() << "enableWidgets: no active trajectory";
		m_targetDisplayGroup->setEnabled(false);
		m_targetNodeGroup->setEnabled(false);
		m_trajectoryGroup->setEnabled(false);
		m_pushbuttonNew->setEnabled(true);
		m_pushbuttonRemove->setEnabled(false);
		m_pushbuttonRename->setEnabled(false);
		m_tab2->setEnabled(false);
		m_tab3->setEnabled(false);
	}
	else
	{
		if (!ep->getHasTarget())
		{
			qDebug() << "enableWidgets: active trajectory has no target";
			m_targetDisplayGroup->setEnabled(false);
			m_targetNodeGroup->setEnabled(true);
			m_trajectoryGroup->setEnabled(false);
			m_pushbuttonNew->setEnabled(true);
			m_pushbuttonRemove->setEnabled(true);
			m_pushbuttonRename->setEnabled(true);
			m_tab2->setEnabled(false);
			m_tab3->setEnabled(false);
		}
		else
		{
			qDebug() << "enableWidgets: active trajectory has target";
			m_targetDisplayGroup->setEnabled(true);
			m_targetNodeGroup->setEnabled(true);
			m_trajectoryGroup->setEnabled(true);
			m_pushbuttonNew->setEnabled(true);
			m_pushbuttonRemove->setEnabled(true);
			m_pushbuttonRename->setEnabled(true);
			m_tab2->setEnabled(true);
			m_tab3->setEnabled(true);
		}
	}
}


void dumpvec(std::string s, float *v)
{
	std::cout << s << " " << v[0] << " , " << v[1] << " , " << v[2] << std::endl;
}
