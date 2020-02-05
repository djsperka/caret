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


#ifndef __GUI_TRAJECTORY_TOOL_DIALOG_H__
#define __GUI_TRAJECTORY_TOOL_DIALOG_H__

#include <vector>

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDoubleSpinBox>
#include <string>
#include "QtAngleSpinBox.h"
#include "QtCylinderGridFrame.h"
#include <ANN/ANN.h>

class QGroupBox;
class QCheckBox;
class QListBox;
class QSlider;
class QSpinBox;
class BrainSet;
class QPushButton;

/// Dialog for performing electrode trajectory functions
class GuiTrajectoryToolDialog : public QDialog {
	Q_OBJECT

	public:
	/// constructor
	GuiTrajectoryToolDialog(QWidget* parent, BrainSet* brain);

	/// destructor
	~GuiTrajectoryToolDialog();

	/// called to close the dialog
	virtual void close();

	/// Update displayed items in dialog. This means bringing the state of the item
	/// (in other words, what it displays) into alignment with the underlying model
	/// value (e.g. the current values in the active trajectory)
	void updateItems();

	// refresh all displayable trajectory things. Presumably the update chain in
	// TrajectoryFile has been called, and now the various things need to be updated.
	void refreshTrajectory();
	void refreshColors();
	void refreshAll();

	protected slots:

	private slots:

	/// called when last selected node button pushed
	void slotLastSelectedButtonPushed();
	void slotCheckBoxToggled(bool on);
	void slotCylinderButtonPushed();
	void slotCheckBoxDisplayBrainToggled(bool on);
	void slotCheckBoxDisplaySkullToggled(bool on);
	void slotCheckBoxDisplayCylinderToggled(bool on);
	void slotCheckBoxDisplayTrajectoryToggled(bool on);
	void slotCheckBoxUseDepthToolToggled(bool on);
	void slotDepthToolToleranceChanged(double v);
	void slotDepthToolSliderValueChanged( int val);

	void slotNodeIDTextChanged(const QString &t);
	void slotNodeIDLostFocus();
	void slotPathRadiusChanged(double v);
	void slotGridRotationChanged(int value);
	void slotThetaValueChanged(int value);
	void slotPhiValueChanged(int value);
	void slotEditBrainSurfaceColor();
	void slotEditSkullSurfaceAnnulusColor();
	void slotEditSkullSurfaceColor();
	void slotEditCylinderSidesColor();
	void slotEditCylinderEndsColor();
	void slotEditPathLineColor();
	void slotEditPathMarkersColor();
	void slotEditFlatMapNodesColor();
	void slotEditVolumeNodesColor();
	void slotCylinderIDTextChanged(const QString &);
	void slotCylinderODTextChanged(const QString &);
	void slotCylinderHTextChanged(const QString &);
	void slotMaxPenetrationDepthChanged(int val);
	void slotSkullNearbyDTextChanged(const QString &);
	void slotCylinderGridSelectionChanged(double x, double y);
	void slotNewTrajectory();
	void slotRemoveTrajectory();
	void slotRenameTrajectory();
	void slotActiveTrajectoryChanged(QListWidgetItem* current, QListWidgetItem* previous);

	private:

	BrainSet *m_brainSet;
	std::string m_szTargetNode;
	QGroupBox *m_targetDisplayGroup;
	QGroupBox *m_targetNodeGroup;
	QGroupBox *m_trajectoryGroup;
	QPushButton *m_pushbuttonNew;
	QPushButton *m_pushbuttonRemove;
	QPushButton *m_pushbuttonRename;
	QWidget *m_tab1;
	QWidget *m_tab2;
	QWidget *m_tab3;
	QListWidget* m_listwidgetTrajectories;
	QLineEdit* m_lineeditNodeID;
	QLineEdit* m_lineeditTargetX;
	QLineEdit* m_lineeditTargetY;
	QLineEdit* m_lineeditTargetZ;
	QLineEdit* m_lineeditCenterX;
	QLineEdit* m_lineeditCenterY;
	QLineEdit* m_lineeditCenterZ;
	QLineEdit* m_lineeditRestAngle;
	QtAngleSpinBox*  m_spinboxTheta;
	QtAngleSpinBox*  m_spinboxPhi;
	QCheckBox* m_checkboxUseNodeID;
	QCheckBox* m_checkboxDisplaySkull;
	QCheckBox* m_checkboxDisplayBrain;
	QCheckBox* m_checkboxDisplayCylinder;
	QCheckBox* m_checkboxDisplayTrajectory;
	QCheckBox* m_checkboxShowNodesAlongPath;
	QtAngleSpinBox*  m_spinboxGridRotation;
	QLabel* m_gridEntryPoint;
	QDoubleSpinBox* m_spinboxPathRadius;
	QCheckBox* m_checkboxUseDepthTool;
	QDoubleSpinBox* m_spinboxDepthToolTolerance;
	QtCylinderGridFrame* m_cylinderGridFrame;
	QSlider*   m_sliderDepthTool;
	QLabel*    m_labelDepth;
	QLabel*    m_labelBrainSurfaceColor;
	QLabel*    m_labelSkullSurfaceAnnulusColor;
	QLabel*    m_labelSkullSurfaceColor;
	QLabel*    m_labelCylinderSidesColor;
	QLabel*    m_labelCylinderEndsColor;
	QLabel*    m_labelPathLineColor;
	QLabel*    m_labelPathMarkersColor;
	QLabel*    m_labelFlatMapNodesColor;
	QLabel*    m_labelVolumeNodesColor;
	QLineEdit* m_lineeditCylinderID;
	QLineEdit* m_lineeditCylinderOD;
	QLineEdit* m_lineeditCylinderH;
	QSpinBox*  m_spinboxMaxPenetrationDepth;
	QLineEdit* m_lineeditSkullNearbyD;
	bool 		  m_bSuppressUpdates;

	QWidget* setupTab1();
	QWidget* setupTab2();
	QWidget* setupTab3();
	void enableWidgets();
	void updateGridEntryPointLabel();

	// Call this to update the slices shown in a volume view to reflect
	// the current electrode penetration depth. Should be followed by
	// a call to update GL windows to actually show the changes.
	void updateSelectedVolumeSlices();
};

#endif // __GUI_TRAJECTORY_TOOL_DIALOG_H__

