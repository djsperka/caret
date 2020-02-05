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

#include <qglobal.h>
#ifdef Q_OS_WIN32
#define NOMINMAX
#endif

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>
#include <qdom.h>
#include <cstdlib>

#include "TrajectoryFile.h"
#include "FileUtilities.h"
#include "CoordinateFile.h"
#include "DebugControl.h"
#include "MathUtilities.h"
#include "BestFitPlane.h"

#include <QDataStream>
#include <QTextStream>
#include <QFile>
#include <QtDebug>

using namespace std;

ElectrodeTrajectories::ElectrodeTrajectories()
{
}

bool ElectrodeTrajectories::addTrajectory(ElectrodeTrajectoryP ep)
{
	bool bval = false;
	if (find(ep->getName()) == end())
	{
		insert(make_pair(ep->getName(), ep));
		bval = true;
	}
	return bval;
}


ElectrodeTrajectory::ElectrodeTrajectory()
{
	init();
}

ElectrodeTrajectory::~ElectrodeTrajectory()
{
	clearIdxNearby();
	clearIdxAnnulus();
	m_mapPathNodes.clear();
}


void ElectrodeTrajectory::init()
{
	m_colorSkullSurfaceAnnulus.setRgb(100, 150, 200);
	m_colorSkullSurface.setRgb(200, 200, 200);
	m_colorCylinderSides.setRgb(147, 147, 220);
	m_colorCylinderEnds.setRgb(127, 127, 200);
	m_colorPathLine.setRgb(255, 0, 0);
	m_colorPathMarkers.setRgb(0, 255, 0);
	m_colorFlatMapNodes.setRgb(200, 0, 0);
	m_colorVolumeNodes.setRgb(200, 0, 0);

	m_name = "New Trajectory";
	m_idxSkullNearby = NULL;
	m_nidxSkullNearby = 0;
	m_idxSkullAnnulus = NULL;
	m_nidxSkullAnnulus = 0;

	m_dPathRadius = 1.0;
	m_bUseNodeID=true;
	m_iNodeID=0;
	m_bUseDepthTool = false;
	m_dDepthToolTolerance = .5;
	m_iDepthToolDepthPct = 0;
	m_mapPathNodes.clear();

	// cylinder position/fit values
	m_bHasTarget = false;
	m_iNodeID= 0;
	m_dTarget[0]=m_dTarget[1]=m_dTarget[2]=0;
	m_dCenter[0]=m_dCenter[1]=0; m_dCenter[2]=10;
	m_dEntry[0] = m_dEntry[1] = m_dEntry[2] = 0;
	m_theta = 0.0;
	m_phi = 0.0;
	m_dXCyl[0] = 1.0; m_dXCyl[1] = m_dXCyl[2] = 0;
	m_dYCyl[1] = 1.0; m_dYCyl[0] = m_dYCyl[2] = 0;
	m_dZCyl[2] = 1.0; m_dZCyl[0] = m_dZCyl[1] = 0;
	m_dZRest = 0;
	m_dCylinderID = 19;
	m_dCylinderOD = 23;
	m_dSkullNearbyD = 40;
	m_dCylinderH = 10;
	m_dCylFitCenter[0] = m_dCylFitCenter[1] = m_dCylFitCenter[2] = 0;
	m_dCylFitNormal[0] = m_dCylFitNormal[1] = m_dCylFitNormal[2] = 0;
	m_dGridEntryX = 0;
	m_dGridEntryY = 0;
	m_dGridRotation = 0;
	m_dMaxPenetrationDepth = 5;
	m_bUseNodeID = false;
	m_bDisplaySkull = false;
	m_bDisplayBrain = false;
	m_bDisplayCylinder = false;
	m_bDisplayTrajectory = false;
}

ElectrodeTrajectory::ElectrodeTrajectory(const ElectrodeTrajectory& e)
{
	init(e);
}

void ElectrodeTrajectory::init(const ElectrodeTrajectory& e)
{
	setSkullSurfaceAnnulusColor(e.getSkullSurfaceAnnulusColor());
	setSkullSurfaceColor(e.getSkullSurfaceColor());
	setCylinderSidesColor(e.getCylinderSidesColor());
	setCylinderEndsColor(e.getCylinderEndsColor());
	setPathLineColor(e.getPathLineColor());
	setPathMarkersColor(e.getPathMarkersColor());
	setFlatMapNodesColor(e.getFlatMapNodesColor());
	setVolumeNodesColor(e.getVolumeNodesColor());

	m_name = e.getName();
	m_idxSkullNearby = NULL;
	m_nidxSkullNearby = 0;
	m_idxSkullAnnulus = NULL;
	m_nidxSkullAnnulus = 0;

	m_dPathRadius = 1.0;
	m_bUseNodeID=true;
	m_iNodeID=0;
	m_bHasTarget = false;
	m_bUseDepthTool = false;
	m_dDepthToolTolerance = .5;
	m_iDepthToolDepthPct = 0;
	m_mapPathNodes.clear();

	// cylinder position/fit values
	m_iNodeID= 0;
	m_dTarget[0]=m_dTarget[1]=m_dTarget[2]=0;
	m_dCenter[0]=m_dCenter[1]=0; m_dCenter[2]=10;
	m_dEntry[0] = m_dEntry[1] = m_dEntry[2] = 0;
	m_theta = 0.0;
	m_phi = 0.0;
	m_dXCyl[0] = 1.0; m_dXCyl[1] = m_dXCyl[2] = 0;
	m_dYCyl[1] = 1.0; m_dYCyl[0] = m_dYCyl[2] = 0;
	m_dZCyl[2] = 1.0; m_dZCyl[0] = m_dZCyl[1] = 0;
	m_dZRest = 0;
	m_dCylinderID = 19;
	m_dCylinderOD = 23;
	m_dSkullNearbyD = 40;
	m_dCylinderH = 10;
	m_dCylFitCenter[0] = m_dCylFitCenter[1] = m_dCylFitCenter[2] = 0;
	m_dCylFitNormal[0] = m_dCylFitNormal[1] = m_dCylFitNormal[2] = 0;
	m_dGridEntryX = 0;
	m_dGridEntryY = 0;
	m_dMaxPenetrationDepth = 5;
	m_bUseNodeID = false;
	m_bDisplaySkull = false;
	m_bDisplayCylinder = false;
	m_bDisplayTrajectory = false;
	m_bDisplayBrain = false;
}

/// set this trajectory's name
void ElectrodeTrajectory::setName(std::string sName)
{
	m_name = sName;
}

void ElectrodeTrajectory::clearIdxNearby()
{
	if (NULL != m_idxSkullNearby)
	{
		delete m_idxSkullNearby;
		m_nidxSkullNearby = 0;
	}
	m_nidxSkullNearby = 0;
}

void ElectrodeTrajectory::clearIdxAnnulus()
{
	if (NULL != m_idxSkullAnnulus)
	{
		delete m_idxSkullAnnulus;
		m_nidxSkullAnnulus = 0;
	}
	m_nidxSkullAnnulus = 0;
}




/// get the target node position
double* ElectrodeTrajectory::getTarget(double dTarget[3]) const
{
	for (int i=0; i<3; i++) dTarget[i] = m_dTarget[i];
	return dTarget;
}

/// set target node position
void ElectrodeTrajectory::setTarget(double dTarget[3])
{
	for (int i=0; i<3; i++) m_dTarget[i] = dTarget[i];
};

/// get the entry point
void ElectrodeTrajectory::getEntry(double dEntry[3]) const
{
	for (int i=0; i<3; i++) dEntry[i] = m_dEntry[i];
}

/// set entry point
void ElectrodeTrajectory::setEntry(double dEntry[3])
{
	for (int i=0; i<3; i++) m_dEntry[i] = dEntry[i];
};

/// set node id
void ElectrodeTrajectory::setNodeID(int i)
{
	m_iNodeID = i;
	setHasTarget(true);
};

/// get node id
int ElectrodeTrajectory::getNodeID() const
{
	return m_iNodeID;
};

void ElectrodeTrajectory::setHasTarget(bool hasTarget)
{
	m_bHasTarget = hasTarget;
}

bool ElectrodeTrajectory::getHasTarget()
{
	return m_bHasTarget;
}

/// set the path radius
void ElectrodeTrajectory::setPathRadius(double d)
{
	m_dPathRadius = d;
}

/// get the current path radius
double ElectrodeTrajectory::getPathRadius() const
{
	return m_dPathRadius;
};

/// set the current cylinder center point
void ElectrodeTrajectory::setCylinderCenter(double dCenter[3])
{
	for (int i=0; i<3; i++) m_dCenter[i] = dCenter[i];
};

/// get the current cylinder center
double* ElectrodeTrajectory::getCylinderCenter(double dCenter[3]) const
{
	for (int i=0; i<3; i++) dCenter[i] = m_dCenter[i];
	return dCenter;
}

/// set cylinder fit values
void ElectrodeTrajectory::setCylinderFitValues(double dCylFitCenter[3], double dCylFitNormal[3])
{
	m_dCylFitCenter[0] = dCylFitCenter[0];
	m_dCylFitCenter[1] = dCylFitCenter[1];
	m_dCylFitCenter[2] = dCylFitCenter[2];
	m_dCylFitNormal[0] = dCylFitNormal[0];
	m_dCylFitNormal[1] = dCylFitNormal[1];
	m_dCylFitNormal[2] = dCylFitNormal[2];
}

/// get cylinder fit values
void ElectrodeTrajectory::getCylinderFitValues(double *dCylFitCenter, double *dCylFitNormal) const
{
	if (dCylFitCenter)
	{
		dCylFitCenter[0] = m_dCylFitCenter[0];
		dCylFitCenter[1] = m_dCylFitCenter[1];
		dCylFitCenter[2] = m_dCylFitCenter[2];
	}
	if (dCylFitNormal)
	{
		dCylFitNormal[0] = m_dCylFitNormal[0];
		dCylFitNormal[1] = m_dCylFitNormal[1];
		dCylFitNormal[2] = m_dCylFitNormal[2];
	}
}


/// set cylinder base angle
void ElectrodeTrajectory::setCylinderBaseAngle(double deg)
{
	m_dCylBaseAngle = deg;
}

/// get cylinder base angle
double ElectrodeTrajectory::getCylinderBaseAngle() const
{
	return m_dCylBaseAngle;
}


/// get use node id value
bool ElectrodeTrajectory::getUseNodeID() const
{
	return m_bUseNodeID;
};

/// get display skull value
bool ElectrodeTrajectory::getDisplaySkull() const
{
	return m_bDisplaySkull;
};

/// set display skull value
void ElectrodeTrajectory::setDisplaySkull(bool b)
{
	m_bDisplaySkull = b;
};

/// get display brain value
bool ElectrodeTrajectory::getDisplayBrain() const
{
	return m_bDisplayBrain;
};

/// set display brain value
void ElectrodeTrajectory::setDisplayBrain(bool b)
{
	m_bDisplayBrain = b;
};

/// get display cylinder value
bool ElectrodeTrajectory::getDisplayCylinder() const
{
	return m_bDisplayCylinder;
};

/// set display cylinder value
void ElectrodeTrajectory::setDisplayCylinder(bool b)
{
	m_bDisplayCylinder = b;
};

/// get display trajectory value
bool ElectrodeTrajectory::getDisplayTrajectory() const
{
	return m_bDisplayTrajectory;
};

/// set display trajectory value
void ElectrodeTrajectory::setDisplayTrajectory(bool b)
{
	m_bDisplayTrajectory = b;
};

/// get use depth tool value
bool ElectrodeTrajectory::getUseDepthTool() const
{
	return m_bUseDepthTool;
};

/// set use depth tool value
void ElectrodeTrajectory::setUseDepthTool(bool b)
{
	m_bUseDepthTool = b;
};

/// set depth tool tolerance
void ElectrodeTrajectory::setDepthToolTolerance(double d)
{
	m_dDepthToolTolerance = d;
};

/// get depth tool tolerance
double ElectrodeTrajectory::getDepthToolTolerance() const
{
	return m_dDepthToolTolerance;
};

/// set depth tool pct value
void ElectrodeTrajectory::setDepthToolDepthPct(int pct)
{
	m_iDepthToolDepthPct = pct;
};

/// get depth tool pct value
int ElectrodeTrajectory::getDepthToolDepthPct() const
{
	return m_iDepthToolDepthPct;
};

/// get trajectory angles
double ElectrodeTrajectory::getTheta() const
{
	return m_theta;
};

double ElectrodeTrajectory::getPhi() const
{
	return m_phi;
};

/// set trajectory angles
void ElectrodeTrajectory::setTheta(double theta)
{
	m_theta = theta;
};

void ElectrodeTrajectory::setPhi(double phi)
{
	m_phi = phi;
};

/// set use node id value
void ElectrodeTrajectory::setUseNodeID(bool bvalue)
{
	m_bUseNodeID = bvalue;
};

/// replace the map of nodes in the trajectory path
void ElectrodeTrajectory::setPathNodes(std::map<double, int> mapNodes)
{
	m_mapPathNodes.clear();
	m_mapPathNodes = mapNodes;
};

/// get the map of nodes in the traj path
const std::map<double, int>& ElectrodeTrajectory::getPathNodes() const
{
	return m_mapPathNodes;
};

/// get the list of nearby nodes
void ElectrodeTrajectory::getSkullNearbyIdx(ANNidxArray* pidx, int *pnidx) const
{
	*pidx = m_idxSkullNearby;
	*pnidx = m_nidxSkullNearby;
}

/// get the list of nodes in the annulus of the cylinder
void ElectrodeTrajectory::getSkullAnnulusIdx(ANNidxArray* pidx, int *pnidx) const
{
	*pidx = m_idxSkullAnnulus;
	*pnidx = m_nidxSkullAnnulus;
}

/// set the list of nearby nodes - those nearby the center point
void ElectrodeTrajectory::setSkullNearbyIdx(ANNidxArray idxSkullNearby, int nidx)
{
	clearIdxNearby();
	m_idxSkullNearby = idxSkullNearby;
	m_nidxSkullNearby = nidx;
}

/// set the list of nodes in the annulus of the cylinder
void ElectrodeTrajectory::setSkullAnnulusIdx(ANNidxArray idx, int nidx)
{
	clearIdxAnnulus();
	m_idxSkullAnnulus = idx;
	m_nidxSkullAnnulus = nidx;
}

/// get the orientation vectors and rest coordinate for the cylinder
void ElectrodeTrajectory::getCylinderCoords(double xhat[3], double yhat[3], double zhat[3], double *pzrest) const
{
	xhat[0] = m_dXCyl[0]; xhat[1] = m_dXCyl[1]; xhat[2] = m_dXCyl[2];
	yhat[0] = m_dYCyl[0]; yhat[1] = m_dYCyl[1]; yhat[2] = m_dYCyl[2];
	zhat[0] = m_dZCyl[0]; zhat[1] = m_dZCyl[1]; zhat[2] = m_dZCyl[2];
	if (pzrest) *pzrest = m_dZRest;
	return;
}

/// set the orientation vectors and rest coordinate for the cylinder
void ElectrodeTrajectory::setCylinderCoords(double xhat[3], double yhat[3], double zhat[3], double zrest)
{
	m_dXCyl[0] = xhat[0]; m_dXCyl[1] = xhat[1]; m_dXCyl[2] = xhat[2];
	m_dYCyl[0] = yhat[0]; m_dYCyl[1] = yhat[1]; m_dYCyl[2] = yhat[2];
	m_dZCyl[0] = zhat[0]; m_dZCyl[1] = zhat[1]; m_dZCyl[2] = zhat[2];
	m_dZRest = zrest;
	return;
}

/// get the offset values for the grid entry point
void ElectrodeTrajectory::getGridEntry(double *px, double *py) const
{
	*px = m_dGridEntryX;
	*py = m_dGridEntryY;
};

/// set the offset values for the grid entry point
void ElectrodeTrajectory::setGridEntry(double x, double y)
{
	m_dGridEntryX = x;
	m_dGridEntryY = y;
};

const QColor& ElectrodeTrajectory::getSkullSurfaceAnnulusColor() const
{
	return m_colorSkullSurfaceAnnulus;
};

void ElectrodeTrajectory::setSkullSurfaceAnnulusColor(const QColor& color)
{
	m_colorSkullSurfaceAnnulus = color;
};

const QColor& ElectrodeTrajectory::getSkullSurfaceColor() const
{
	return m_colorSkullSurface;
};

void ElectrodeTrajectory::setSkullSurfaceColor(const QColor& color)
{
	m_colorSkullSurface = color;
};

const QColor& ElectrodeTrajectory::getCylinderSidesColor() const
{
	return m_colorCylinderSides;
};

void ElectrodeTrajectory::setCylinderSidesColor(const QColor& color)
{
	m_colorCylinderSides = color;
};

const QColor& ElectrodeTrajectory::getCylinderEndsColor() const
{
	return m_colorCylinderEnds;
};

void ElectrodeTrajectory::setCylinderEndsColor(const QColor& color)
{
	m_colorCylinderEnds = color;
};

const QColor& ElectrodeTrajectory::getPathLineColor() const
{
	return m_colorPathLine;
};

void ElectrodeTrajectory::setPathLineColor(const QColor& color)
{
	m_colorPathLine = color;
};

const QColor& ElectrodeTrajectory::getPathMarkersColor() const
{
	return m_colorPathMarkers;
};

void ElectrodeTrajectory::setPathMarkersColor(const QColor& color)
{
	m_colorPathMarkers = color;
};

const QColor& ElectrodeTrajectory::getFlatMapNodesColor() const
{
	return m_colorFlatMapNodes;
};

void ElectrodeTrajectory::setFlatMapNodesColor(const QColor& color)
{
	m_colorFlatMapNodes = color;
};

const QColor& ElectrodeTrajectory::getVolumeNodesColor() const
{
	return m_colorVolumeNodes;
};

void ElectrodeTrajectory::setVolumeNodesColor(const QColor& color)
{
	m_colorVolumeNodes = color;
};

double ElectrodeTrajectory::getCylinderID() const
{
	return m_dCylinderID;
};

void ElectrodeTrajectory::setCylinderID(double id)
{
	m_dCylinderID = id;
};

double ElectrodeTrajectory::getCylinderOD() const
{
	return m_dCylinderOD;
};

void ElectrodeTrajectory::setCylinderOD(double od)
{
	m_dCylinderOD = od;
};

double ElectrodeTrajectory::getCylinderH() const
{
	return m_dCylinderH;
};

void ElectrodeTrajectory::setCylinderH(double h)
{
	m_dCylinderH = h;
};

double ElectrodeTrajectory::getMaxPenetrationDepth() const
{
	return m_dMaxPenetrationDepth;
}

void ElectrodeTrajectory::setMaxPenetrationDepth(double d)
{
	m_dMaxPenetrationDepth = d;
}

double ElectrodeTrajectory::getPenetrationDepth() const
{
	return (double)getDepthToolDepthPct()/100.0d * getMaxPenetrationDepth();
}

/// Get position of current penetration, as defined by depth and trajectory
void ElectrodeTrajectory::getPenetrationPosition(double dpt[3]) const
{
	double dEntry[3];
	double xc[3], yc[3], zc[3];
	int i;
	// current entry point reflects the currently selected point on the grid.
	// Use the cylinder symmetry axis (zc) to find the penetration position.
	// Note that the direction of the axis vector is "up" relative to the cylinder,
	// hence the minus sign in computing the penetration position.
	getEntry(dEntry);
	getCylinderCoords(xc, yc, zc, NULL);
	for (i=0; i<3; i++)
	{
		dpt[i] = dEntry[i] - getPenetrationDepth() * zc[i];
	}
}

void ElectrodeTrajectory::getGridEntryOffset(double *offset) const
{
	double xc[3], yc[3], zc[3];
	double xgrid, ygrid;
	double xgridRotated, ygridRotated;

	// Modified 12-5-08 djs
	// The grid itself may be rotated an angle (gridRotation). Rotate xgrid and ygrid before computing
	// pathStart and pathEnd.
	// Modified 12-17-08 djs
	// Redefine the grid orientation angle so that rotation clockwise is positive.
	// This is opposite the customary definition for rotations; the sign on the
	// the sin() terms in the calculation of x/ygridRotated below reflects this change.

	getCylinderCoords(xc, yc, zc, NULL);
	getGridEntry(&xgrid, &ygrid);
	xgridRotated = xgrid * cos(getGridRotation() * MathUtilities::degreesToRadians()) + ygrid * sin(getGridRotation() * MathUtilities::degreesToRadians());
	ygridRotated = -xgrid * sin(getGridRotation() * MathUtilities::degreesToRadians()) + ygrid * cos(getGridRotation() * MathUtilities::degreesToRadians());

	for (int i=0; i<3; i++)
	{
		offset[i] = xgridRotated*xc[i] + ygridRotated*yc[i];
	}
}


double ElectrodeTrajectory::getSkullNearbyD() const
{
	return m_dSkullNearbyD;
};

void ElectrodeTrajectory::setSkullNearbyD(double d)
{
	m_dSkullNearbyD = d;
};


void ElectrodeTrajectory::setGridRotation(double r)
{
	m_dGridRotation = r;
};

double ElectrodeTrajectory::getGridRotation() const
{
	return m_dGridRotation;
};



//--------------------------------------------------------------------------------
/**
 * Constructor.
 */
TrajectoryFile::TrajectoryFile()
   : AbstractFile("Electrode trajectory file", SpecFile::getTrajectoryFileExtension(), true),
   m_kdtreeFiducial(NULL)
{
	m_volumeOrigin[0] = m_volumeOrigin[1] = m_volumeOrigin[2] = 0.0;
	m_dfsSkull.faces = NULL;
	m_dfsSkull.vertices = NULL;
	m_dfsSkull.vertexnormals = NULL;
	m_dfsSkull.uv = NULL;
	m_dfsSkull.color = NULL;
	m_dfsSkull.nfaces = 0;
	m_dfsSkull.nvertices = 0;


	m_dfsBrain.faces = NULL;
	m_dfsBrain.vertices = NULL;
	m_dfsBrain.vertexnormals = NULL;
	m_dfsBrain.uv = NULL;
	m_dfsBrain.color = NULL;
	m_dfsBrain.nfaces = 0;
	m_dfsBrain.nvertices = 0;

   clear();

	m_colorBrainSurface.setRgb(50, 100, 200);


   // Create a single trajectory and make it the current one
   _sActiveTrajectoryName = "new trajectory";
	_trajectories[_sActiveTrajectoryName] = ElectrodeTrajectoryP(new ElectrodeTrajectory());
}





/**
 * Destructor.
 */
TrajectoryFile::~TrajectoryFile()
{
   clear();
}



ElectrodeTrajectoryP TrajectoryFile::getActiveTrajectory() const
{
	return getTrajectory(_sActiveTrajectoryName);
}

bool TrajectoryFile::setActiveTrajectory(std::string name)
{
	bool bValue = true;
	ElectrodeTrajectoryMapIterator it = _trajectories.find(name);
	if (it != _trajectories.end())
	{
		_sActiveTrajectoryName = name;
	}
	else
	{
		bValue = false;
	}
	return bValue;
}


int TrajectoryFile::getTrajectoryCount() const
{
	return _trajectories.size();
}

ElectrodeTrajectoryP TrajectoryFile::getTrajectory(const std::string name) const
{
	ElectrodeTrajectoryP ep;
	ElectrodeTrajectoryMapConstIterator it = _trajectories.find(name);
	if (it != _trajectories.end())
	{
		ep = it->second;
	}
	return ep;
}

bool TrajectoryFile::addTrajectory(ElectrodeTrajectoryP& ep)
{
	bool bval = false;
	if (_trajectories.find(ep->getName()) == _trajectories.end())
	{
		_trajectories.insert(make_pair(ep->getName(), ep));
		bval = true;
	}
	return bval;
}

ElectrodeTrajectoryP TrajectoryFile::removeTrajectory(const std::string name)
{
	ElectrodeTrajectoryP ep;
	ElectrodeTrajectoryMapIterator it = _trajectories.find(name);
	ep = it->second;
	if (it != _trajectories.end())
	{
		_trajectories.erase(it);
	}
	return ep;
}


/**
 * Clear the skull file. TODO: DELETE ARRAYS ALLOCATED FOR VERTICES, ETC.
 */
void
TrajectoryFile::clear()
{
   clearAbstractFile();
   clearDfsStruct(&m_dfsSkull);
   clearDfsStruct(&m_dfsBrain);
   _trajectories.clear();
}

/// Get the trajectories map
ElectrodeTrajectories& TrajectoryFile::getTrajectories()
{
	return _trajectories;
}


void TrajectoryFile::readDfsFile(QFile& infile, QDataStream& binStream, DfsFileStruct *pdfs) throw (FileException)
{
   QString line;
   char magic[8];
   char version[4];
   int32_t hdrsize;
   int32_t mdoffset;
   int32_t pdoffset;
   int32_t stripSize;
   int32_t normals;
   int32_t uvStart;
   int32_t vcoffset;
   int32_t precision;
   int32_t nstrips;
   int i, j;

   // first 8 bytes are magic
   binStream.readRawData(magic, 8);
   if (!strcmp(magic,"DUFFSURF"))
   {
   	throw FileException(infile.fileName(), "This does not appear to be a binary FreeSurfer file!");
   }
   else
	{
		binStream.readRawData(version, 4); // the next 4 bytes from the version number of the current BrainSuite
		binStream.readRawData((char *)&hdrsize, 4);
		binStream.readRawData((char *)&mdoffset, 4);	// offset of the metadata in the file
		binStream.readRawData((char *)&pdoffset, 4);	// offset of the patientdata in the file
		binStream.readRawData((char *)&pdfs->nfaces, 4);		// number of triangles in the file
		binStream.readRawData((char *)&pdfs->nvertices, 4);	// number of vertices in the file
		binStream.readRawData((char *)&nstrips, 4);		// number of triangle strips in the file
		binStream.readRawData((char *)&stripSize, 4);	// size of the triangle strips
		binStream.readRawData((char *)&normals, 4);		// offset of the vertex normals
		binStream.readRawData((char *)&uvStart, 4);		// offset of the  surface parameterization data in the file
		binStream.readRawData((char *)&vcoffset, 4); 	// offset of the vertex color data in the file
		binStream.readRawData((char *)&precision, 4);	// Vertex Precision -- usually float32 or float64
		for (i=0; i<16; i++) binStream.readRawData((char *)&pdfs->orientation[i], sizeof(double));	// affine transformation to world coordinates


	   if (DebugControl::getDebugOn()) {
	   	std::cout << "hdrsize=" << hdrsize << std::endl;
	   	std::cout << "mdoffset=" << mdoffset << std::endl;
	   	std::cout << "pdoffset=" << pdoffset << std::endl;
	   	std::cout << "m_nfaces=" << pdfs->nfaces << std::endl;
	   	std::cout << "m_nvertices=" << pdfs->nvertices << std::endl;
	      std::cout << "There are " << pdfs->nvertices << " vertices and " << pdfs->nfaces
	      			 << " in " << infile.fileName().toStdString() << std::endl;
	   }


		//infile.at(hdrsize); 						// QT-equivalent of fseek(SEEK_SET) -- skip header
		infile.seek(hdrsize);	// Hoping that hdrsize is the offset from file beginning....
		pdfs->faces = new int[pdfs->nfaces*3]; // allocate memory for nTriangles*3 faces
		binStream.readRawData((char *)pdfs->faces, pdfs->nfaces*3*sizeof(int));			// read faces

		// read all coordinates and correct for volume origin.
		float *pvertices = new float[pdfs->nvertices*3];
		pdfs->vertices = new ANNcoord[pdfs->nvertices*3];
		binStream.readRawData((char *)pvertices, sizeof(float)*3*pdfs->nvertices);	// read vertices
		for (i=0; i<pdfs->nvertices; i++)
		{
			for (j=0; j<3; j++)
			{
				int index = i*3+j;
				pdfs->vertices[index] = pvertices[index] + m_volumeOrigin[j];
			}
		}

		// Form point array and kdtree
		pdfs->pa = new ANNpoint[pdfs->nvertices];
		for (i=0; i<pdfs->nvertices; i++)
		{
			pdfs->pa[i] = &(pdfs->vertices[i*3]);
		}
		pdfs->tree = new ANNkd_tree(pdfs->pa, pdfs->nvertices, 3);

	   if (DebugControl::getDebugOn()) {
			std::cout << "volume origin correction " << m_volumeOrigin[0] << "," << m_volumeOrigin[1] << "," << m_volumeOrigin[2] << std::endl;
	   }

		if(normals>0) // if vertex normals are stored in the file
		{
			pdfs->vertexnormals = new float[pdfs->nvertices*3];	// allocate memory for nVertices*3 normals for the vertex normals
			infile.seek(normals); 															// go the normals in the file
			binStream.readRawData((char *)pdfs->vertexnormals,sizeof(float)*pdfs->nvertices*3); // read normals
		} //if(normals>0)

		if(vcoffset>0) // if vertex colors are stored in the file
		{
			pdfs->color=new float[pdfs->nvertices*3]; // allocate memory for nVertices*3 (R G B) vertex colors
			infile.seek(vcoffset); 										// go to the colors in the file
			binStream.readRawData((char *)pdfs->color,sizeof(float)*pdfs->nvertices*3); // read colors
		} //if(vcoffset>0)

		if(uvStart>0) //if parameterization was stored
		{
			pdfs->uv = new float[pdfs->nvertices*2];	//allocate memory for nVertices u and v vectors
			infile.seek(uvStart); 											// go to parameterization in the files
			binStream.readRawData((char *)pdfs->uv,pdfs->nvertices*2*sizeof(float)); // read u and v
		}// if(uvStart>0)

		if (DebugControl::getDebugOn()) {
	    	std::cout	<< "Read " << pdfs->nvertices << " vertices and " << pdfs->nfaces
	      				<< " from " << infile.fileName().toStdString() << std::endl;
		}

		// Fill mapFaces
		for (i=0; i<pdfs->nfaces; i++)
		{
			for (j=0; j<3; j++)
			{
				int index = i*3+j;
				pdfs->mapFaces.insert(std::make_pair(pdfs->faces[index], i));
			}
		}

	} //if(strcmp(magic,"DUFFSURF"))
}



void TrajectoryFile::clearDfsStruct(DfsFileStruct *pdfs)
{
   if (NULL != pdfs->faces)
   {
   	delete[] pdfs->faces;
   	pdfs->faces = NULL;
   	pdfs->nfaces = 0;
   }
   if (NULL != pdfs->vertices)
   {
   	delete[] pdfs->vertices;
   	pdfs->vertices = NULL;
   }
   if (NULL != pdfs->vertexnormals)
   {
   	delete[] pdfs->vertexnormals;
   	pdfs->vertexnormals = NULL;
   }
   if (NULL != pdfs->uv)
   {
   	delete[] pdfs->uv;
   	pdfs->uv = NULL;
   }
   if (NULL != pdfs->color)
   {
   	delete[] pdfs->color;
   	pdfs->color = NULL;
   }

   pdfs->mapFaces.clear();
}

/**
 * Read the given trajectory file.  Will throw a FileException if an
 * error occurs while reading the file.
 */
void
TrajectoryFile::readSkullFiles(const QString& sFileSkull, const QString& sFileBrain) throw(FileException)
{
   // Note: filenameIn could possibly be "this's" filename so make a
   // copy of it before calling "clear()" to prevent it from being erased.
   double aabbMin[3];
   double aabbMax[3];

	// Open skull surface file and read it
	if (sFileSkull.length() == 0)
	{
	    throw FileException(getFileName(), "skull file name is empty!");
	}

	QFile fileSkull(sFileSkull);
	if (fileSkull.open(QIODevice::ReadOnly))
	{
		QDataStream binStream(&fileSkull);
		try
		{
			readDfsFile(fileSkull, binStream, &m_dfsSkull);

			// Generate the AABB for the skull surface
			aabbMin[0] = aabbMin[1] = aabbMin[2] = 999.9e99;
			aabbMax[0] = aabbMax[1] = aabbMax[2] = -999.9e99;
			for (int i=0; i<m_dfsSkull.nvertices; i++)
			{
				for (int j=0; j<3; j++)
				{
					int k = i*3 + j;
					if (m_dfsSkull.vertices[k] < aabbMin[j]) aabbMin[j] = m_dfsSkull.vertices[k];
					if (m_dfsSkull.vertices[k] > aabbMax[j]) aabbMax[j] = m_dfsSkull.vertices[k];
				}
			}
			setAABBMin(aabbMin);
			setAABBMax(aabbMax);
			qDebug() << "Read " << m_dfsSkull.nvertices << " vertices from skull file " << sFileSkull;
		}
		catch (FileException& e) {
        	fileSkull.close();
        	clearModified();
         	throw e;
		}

		fileSkull.close();
	}
	else {
    	throw FileException(QString(sFileSkull), "Failure trying to open: ");
	}

	// Find brain surface file tag -- this is the freesurfer surface file containing the
	// brain surface
	if (sFileBrain.length() == 0)
	{
	    throw FileException(getFileName(), "brain surface file name is empty!");
   }


	// Open brain surface file and read it

	QFile fileBrain(sFileBrain);
	if (fileBrain.open(QIODevice::ReadOnly))
	{
		QDataStream binStream(&fileBrain);
		try
		{
			readDfsFile(fileBrain, binStream, &m_dfsBrain);
		}
		catch (FileException& e) {
        	fileBrain.close();
        	clearModified();
         	throw e;
		}

		qDebug() << "Read " << m_dfsBrain.nvertices << " from brain surface file " << sFileBrain;
		fileBrain.close();
	}
	else {
    	throw FileException(sFileBrain, "Failure trying to open: ");
	}

}

/// Write the file's data (header has already been written)
void
TrajectoryFile::writeFileData(QTextStream& stream,
              QDataStream& ,
              QDomDocument& ,
              QDomElement& ) throw (FileException)
{
	int red, green, blue;
	double fxyz[3];
	double fX[3], fY[3], fZ[3];
	double ftemp, ftemp2;

	// Now dump the trajectories
	// First line contains number of trajectories and the current trajectory index.
	// These things are not implemented and are only here as placeholders.

	stream << _trajectories.size() << "\n";
	stream << _sActiveTrajectoryName.c_str() << "\n";
	for (ElectrodeTrajectoryMapIterator it = _trajectories.begin(); it != _trajectories.end(); it++)
	{
		ElectrodeTrajectoryP ep = it->second;
		stream << it->first.c_str() << "\n";
		stream << ep->getPathRadius() << " " << ep->getUseDepthTool() << " " << ep->getDepthToolTolerance() << " " << ep->getDepthToolDepthPct() << "\n";
		stream << ep->getUseNodeID() << " " << ep->getDisplaySkull() << " " << ep->getDisplayBrain() << " " << ep->getDisplayCylinder() << " " << ep->getDisplayTrajectory() << "\n";

		ep->getSkullSurfaceAnnulusColor().getRgb(&red, &green, &blue);
		stream	<< red << " " << green << " " << blue << "\n";
		ep->getSkullSurfaceColor().getRgb(&red, &green, &blue);
		stream	<< red << " " << green << " " << blue << "\n";
		ep->getCylinderEndsColor().getRgb(&red, &green, &blue);
		stream	<< red << " " << green << " " << blue << "\n";
		ep->getPathLineColor().getRgb(&red, &green, &blue);
		stream	<< red << " " << green << " " << blue << "\n";
		ep->getPathMarkersColor().getRgb(&red, &green, &blue);
		stream	<< red << " " << green << " " << blue << "\n";
		ep->getFlatMapNodesColor().getRgb(&red, &green, &blue);
		stream	<< red << " " << green << " " << blue << "\n";
		ep->getVolumeNodesColor().getRgb(&red, &green, &blue);
		stream	<< red << " " << green << " " << blue << "\n";

		stream << ep->getHasTarget() << "\n";
		stream << ep->getNodeID() << "\n";
		ep->getTarget(fxyz);
		stream << fxyz[0] << " " << fxyz[1] << " " << fxyz[2] << "\n";
		ep->getCylinderCenter(fxyz);
		stream << fxyz[0] << " " << fxyz[1] << " " << fxyz[2] << "\n";
		ep->getEntry(fxyz);
		stream << fxyz[0] << " " << fxyz[1] << " " << fxyz[2] << "\n";
		stream << ep->getTheta() << " " << ep->getPhi() << "\n";
		ep->getCylinderCoords(fX, fY, fZ, &ftemp);
		stream << fX[0] << " " << fX[1] << " " << fX[2] << "\n";
		stream << fY[0] << " " << fY[1] << " " << fY[2] << "\n";
		stream << fZ[0] << " " << fZ[1] << " " << fZ[2] << "\n";
		stream << ftemp << "\n";
		stream << ep->getCylinderID() << " " << ep->getCylinderOD() << " " << ep->getCylinderH() << " " << ep->getMaxPenetrationDepth() << " " << ep->getSkullNearbyD() << "\n";
		ep->getCylinderFitValues(fX, fY);
		stream << fX[0] << " " << fX[1] << " " << fX[2] << "\n";
		stream << fY[0] << " " << fY[1] << " " << fY[2] << "\n";
		ep->getGridEntry(&ftemp, &ftemp2);
		stream << ftemp << " " << ftemp2 << "\n";
		stream << ep->getGridRotation() << "\n";
	}

	return;
}


/**
 *
 */
void
TrajectoryFile::readFileData(QFile& infile,
             QTextStream& stream,
             QDataStream& ,
             QDomElement& ) throw (FileException)
{
	QString line;
	QString sActive, sName;
	int iNumTrajectories = 0;
	int red, green, blue;
	int m_ibDisplaySkull, m_ibDisplayBrain, m_ibDisplayCylinder, m_ibDisplayTrajectory, m_ibUseDepthTool, m_ibUseNodeID;
	int m_ibHasTarget;
	double dPathRadius;
	double dDepthToolTolerance;
	int iDepthToolDepthPct;
	int itemp;
	double dtemp;
	double dtemp3[3];
	double dx[3], dy[3], dz[3];
	std::string sSkullFileName, sBrainSurfaceFileName;

	// get skull and brain surface file tags, and read those files
	readSkullFiles(getHeaderTag("skull_surface_file"), getHeaderTag("brain_surface_file"));


	// Now read the trajectories
   readLine(stream, line);
   if (!line.isNull()) QTextStream(&line) >> iNumTrajectories;

   qDebug() << "TrajectoryFile: will read n trajectories: " << iNumTrajectories;

   if (iNumTrajectories > 0)
   {
		_trajectories.clear();
		readLine(stream, line);
		QTextStream(&line) >> sActive;

	   for (int i=0; i<iNumTrajectories; i++)
	   {
			ElectrodeTrajectoryP ep(new ElectrodeTrajectory);

			readLine(stream, line);
			QTextStream(&line) >> sName;
			ep->setName(sName.toStdString());

			readLine(stream, line);
			QTextStream(&line) >> dPathRadius >> m_ibUseDepthTool >> dDepthToolTolerance >> iDepthToolDepthPct;
			ep->setPathRadius(dPathRadius);
			ep->setUseDepthTool((bool)m_ibUseDepthTool);
			ep->setDepthToolTolerance(dDepthToolTolerance);
			ep->setDepthToolDepthPct(iDepthToolDepthPct);

			readLine(stream, line);
			QTextStream(&line) >> m_ibUseNodeID >> m_ibDisplaySkull >> m_ibDisplayBrain >> m_ibDisplayCylinder >> m_ibDisplayTrajectory;
			ep->setUseNodeID((bool)m_ibUseNodeID);
			ep->setDisplaySkull((bool)m_ibDisplaySkull);
			ep->setDisplayBrain((bool)m_ibDisplayBrain);
			ep->setDisplayCylinder((bool)m_ibDisplayCylinder);
			ep->setDisplayTrajectory((bool)m_ibDisplayTrajectory);

			readLine(stream, line);
			QTextStream(&line)	>> red >> green >> blue;
			ep->setSkullSurfaceAnnulusColor(QColor(red, green, blue));
			readLine(stream, line);
			QTextStream(&line)	>> red >> green >> blue;
			ep->setSkullSurfaceColor(QColor(red, green, blue));
			readLine(stream, line);
			QTextStream(&line)	>> red >> green >> blue;
			ep->setCylinderEndsColor(QColor(red, green, blue));
			readLine(stream, line);
			QTextStream(&line)	>> red >> green >> blue;
			ep->setPathLineColor(QColor(red, green, blue));
			readLine(stream, line);
			QTextStream(&line)	>> red >> green >> blue;
			ep->setPathMarkersColor(QColor(red, green, blue));
			readLine(stream, line);
			QTextStream(&line)	>> red >> green >> blue;
			ep->setFlatMapNodesColor(QColor(red, green, blue));
			readLine(stream, line);
			QTextStream(&line)	>> red >> green >> blue;
			ep->setVolumeNodesColor(QColor(red, green, blue));


			readLine(stream, line);
			QTextStream(&line) >> m_ibHasTarget;
			ep->setHasTarget((bool)m_ibHasTarget);
			readLine(stream, line);
			QTextStream(&line) >> itemp;
			ep->setNodeID(itemp);
			readLine(stream, line);
			QTextStream(&line) >> dtemp3[0] >> dtemp3[1] >> dtemp3[2];
			ep->setTarget(dtemp3);
			readLine(stream, line);
			QTextStream(&line) >> dtemp3[0] >> dtemp3[1] >> dtemp3[2];
			ep->setCylinderCenter(dtemp3);
			readLine(stream, line);
			QTextStream(&line) >> dtemp3[0] >> dtemp3[1] >> dtemp3[2];
			ep->setEntry(dtemp3);
			readLine(stream, line);
			QTextStream(&line) >> dtemp3[0] >> dtemp3[1];
			ep->setTheta(dtemp3[0]);
			ep->setPhi(dtemp3[1]);

			readLine(stream, line);
			QTextStream(&line) >> dx[0] >> dx[1] >> dx[2];
			readLine(stream, line);
			QTextStream(&line) >> dy[0] >> dy[1] >> dy[2];
			readLine(stream, line);
			QTextStream(&line) >> dz[0] >> dz[1] >> dz[2];
			readLine(stream, line);
			QTextStream(&line) >> dtemp;
			ep->setCylinderCoords(dx, dy, dz, dtemp);

			readLine(stream, line);
			QTextStream(&line) >> dtemp3[0] >> dtemp3[1] >> dtemp3[2] >> dx[0] >> dx[1];
			ep->setCylinderID(dtemp3[0]);
			ep->setCylinderOD(dtemp3[1]);
			ep->setCylinderH(dtemp3[2]);
			ep->setMaxPenetrationDepth(dx[0]);
			ep->setSkullNearbyD(dx[1]);

			readLine(stream, line);
			QTextStream(&line) >> dx[0] >> dx[1] >> dx[2];
			readLine(stream, line);
			QTextStream(&line) >> dy[0] >> dy[1] >> dy[2];
			ep->setCylinderFitValues(dx, dy);
			readLine(stream, line);
			QTextStream(&line) >> dy[0] >> dy[1];
			ep->setGridEntry(dy[0], dy[1]);
			readLine(stream, line);
			QTextStream(&line) >> dy[0];
			ep->setGridRotation(dy[0]);

			updateNearbySkullPoints(ep);
			searchPath(ep);


			if (!_trajectories.addTrajectory(ep))
			{
				throw FileException(infile.fileName(), "Cannot add trajectory.");
			}

	   }
	   // Make sure the active trajectory name is in the map
	   if (_trajectories.find(sActive.toStdString()) != _trajectories.end())
	   {
	   	_sActiveTrajectoryName = sActive.toStdString();
	   }
	   else
	   {
	   	std::cout << "Active trajectory name " << sActive.toStdString() << " is not in the current set of trajectories. Reassigning active trajectory." << std::endl;
	   	_sActiveTrajectoryName = _trajectories.begin()->first;
	   }
   }
   else
   {
   	_sActiveTrajectoryName = "";
//   	// No trajectories in file - create a dummy trajectory and make it active
//   	ElectrodeTrajectoryP ep(new ElectrodeTrajectory());
//   	ep->setName("trajectory_1");
//   	_trajectories.addTrajectory(ep);
//   	_sActiveTrajectoryName = ep->getName();
//   	std::cout << "Created new trajectory." << std::endl;
   }

	return;
}

bool TrajectoryFile::empty() const
{
	return (getNumSkullVertices() == 0);
}

/// Get the color assigned to the brain surface
const QColor& TrajectoryFile::getBrainSurfaceColor() const
{
	return m_colorBrainSurface;
};

/// Set the color assigned to the brain surface
void TrajectoryFile::setBrainSurfaceColor(const QColor& color)
{
	m_colorBrainSurface = color;
};

/// Returns number of vertices in skull
int TrajectoryFile::getNumSkullVertices() const
{
	return m_dfsSkull.nvertices;
};

/// Returns number of vertices in brain surface
int TrajectoryFile::getNumBrainVertices() const
{
	return m_dfsBrain.nvertices;
};

/// Returns a particular skull vertex. No bounds checking!
float* TrajectoryFile::getSkullVertex(int i, float *xyz) const
{
	xyz[0] = m_dfsSkull.vertices[i*3];
	xyz[1] = m_dfsSkull.vertices[i*3+1];
	xyz[2] = m_dfsSkull.vertices[i*3+2];
	return xyz;
};

double* TrajectoryFile::getSkullVertex(int i, double *xyz) const
{
	xyz[0] = (double)m_dfsSkull.vertices[i*3];
	xyz[1] = (double)m_dfsSkull.vertices[i*3+1];
	xyz[2] = (double)m_dfsSkull.vertices[i*3+2];
	return xyz;
};


/// Does the skull file have vertex normals?
bool TrajectoryFile::haveSkullVertexNormals() const
{
	return (m_dfsSkull.vertexnormals != NULL);
};

/// Returns a particular skull vertex normal. No bounds checking!
double* TrajectoryFile::getSkullVertexNormal(int i, double *xyz) const
{
	xyz[0] = m_dfsSkull.vertexnormals[i*3];
	xyz[1] = m_dfsSkull.vertices[i*3+1];
	xyz[2] = m_dfsSkull.vertices[i*3+2];
	return xyz;
};

/// Returns a particular brain vertex. No bounds checking!
double* TrajectoryFile::getBrainVertex(int i, double *xyz) const
{
	xyz[0] = m_dfsBrain.vertices[i*3];
	xyz[1] = m_dfsBrain.vertices[i*3+1];
	xyz[2] = m_dfsBrain.vertices[i*3+2];
	return xyz;
};

/// returns the kdtree for the skull
ANNkd_tree* TrajectoryFile::getSkullTree() const
{
	return m_dfsSkull.tree;
};

/// returns the kdtree for the brain
ANNkd_tree* TrajectoryFile::getBrainTree() const
{
	return m_dfsBrain.tree;
};


/// gets a particular vertex from the fiducial surface (i.e. a node)
double* TrajectoryFile::getNodeVertex(int inode, double *xyz) const
{
	if (m_paFiducial)
	{
		for (int i=0; i<3; i++)
		{
			xyz[i] = m_paFiducial[inode][i];
		}
	}
	return xyz;
}



/// Set the origin to be used when reading coordinates
void TrajectoryFile::setVolumeOrigin(double xyzOrigin[3])
{
	m_volumeOrigin[0] = xyzOrigin[0];
	m_volumeOrigin[1] = xyzOrigin[1];
	m_volumeOrigin[2] = xyzOrigin[2];
};

/// Get the axis-aligned bounding box min corner
void TrajectoryFile::getAABBMin(double p[3]) const
{
	p[0] = m_aabbMin[0];
	p[1] = m_aabbMin[1];
	p[2] = m_aabbMin[2];
	return;
};

/// Get the axis-aligned bounding box max corner
void TrajectoryFile::getAABBMax(double p[3]) const
{
	p[0] = m_aabbMax[0];
	p[1] = m_aabbMax[1];
	p[2] = m_aabbMax[2];
	return;
};

/// Set the axis-aligned bounding box min corner
void TrajectoryFile::setAABBMin(double p[3])
{
	m_aabbMin[0] = p[0];
	m_aabbMin[1] = p[1];
	m_aabbMin[2] = p[2];
	return;
};

/// Set the axis-aligned bounding box max corner
void TrajectoryFile::setAABBMax(double p[3])
{
	m_aabbMax[0] = p[0];
	m_aabbMax[1] = p[1];
	m_aabbMax[2] = p[2];
	return;
};

void TrajectoryFile::getFace(int index, float *pfv1, float *pfv2, float *pfv3, float *pfn1, float *pfn2, float *pfn3) const
{
	int iface = index*3;
	int i;
	for (i=0; i<3; i++)
	{
		pfv1[i] = m_dfsSkull.vertices[m_dfsSkull.faces[iface]*3 + i];
		//(*pfv1) = m_dfsSkull.vertices + m_dfsSkull.faces[iface]*3;
		pfv2[i] = m_dfsSkull.vertices[m_dfsSkull.faces[iface+1]*3 + i];
		//(*pfv2) = m_dfsSkull.vertices + m_dfsSkull.faces[iface+1]*3;
		pfv3[i] = m_dfsSkull.vertices[m_dfsSkull.faces[iface+2]*3 + i];
		//(*pfv3) = m_dfsSkull.vertices + m_dfsSkull.faces[iface+2]*3;
		pfn1[i] = m_dfsSkull.vertexnormals[m_dfsSkull.faces[iface]*3 + i];
		//(*pfn1) = m_dfsSkull.vertexnormals + m_dfsSkull.faces[iface]*3;
		pfn2[i] = m_dfsSkull.vertexnormals[m_dfsSkull.faces[iface+1]*3 + i];
		//(*pfn2) = m_dfsSkull.vertexnormals + m_dfsSkull.faces[iface+1]*3;
		pfn3[i] = m_dfsSkull.vertexnormals[m_dfsSkull.faces[iface+2]*3 + i];
		//(*pfn3) = m_dfsSkull.vertexnormals + m_dfsSkull.faces[iface+2]*3;
	}
}

const std::multimap<int, int>& TrajectoryFile::getSkullFaceMap() const
{
	return m_dfsSkull.mapFaces;
};

bool TrajectoryFile::getSkullNearestPoint(double p[3], double fNearest[3])
{
	bool bValue = false;
	ANNidxArray			nnIdx;					// near neighbor indices
	ANNdistArray		dists;					// near neighbor distances

	// Do we have a kdtree for the skull?
	if (NULL != m_dfsSkull.tree)
	{
		nnIdx = new ANNidx[1];						// allocate near neigh indices
		dists = new ANNdist[1];						// allocate near neighbor dists
		nnIdx[0]=0;
		m_dfsSkull.tree->annkSearch(						// search
				p,									// query point
				1,									// number of near neighbors
				nnIdx,							// nearest neighbors (returned)
				dists							// distance (returned)
				);
		getSkullVertex(nnIdx[0], fNearest);
		bValue = true;
	}
	return bValue;
}

bool TrajectoryFile::getBrainNearestPoint(double p[3], double fNearest[3])
{
	bool bValue = false;
	ANNidxArray			nnIdx;					// near neighbor indices
	ANNdistArray		dists;					// near neighbor distances

	// Do we have a kdtree for the skull?
	if (NULL != m_dfsBrain.tree)
	{
		nnIdx = new ANNidx[1];						// allocate near neigh indices
		dists = new ANNdist[1];						// allocate near neighbor dists

		m_dfsBrain.tree->annkSearch(						// search
				p,									// query point
				1,									// number of near neighbors
				nnIdx,							// nearest neighbors (returned)
				dists							// distance (returned)
				);

		getBrainVertex(nnIdx[0], fNearest);
		bValue = true;
	}

	return bValue;
}

// Locates a section of skull near the point p, within radius.
int TrajectoryFile::findSkullNearbyPoints(double p[3], ANNidxArray* pIdx, double radius)
{
	int nidx;

	// first search gets the number of indices
	nidx = m_dfsSkull.tree->annkFRSearch(p, radius*radius, 0, NULL, NULL);

	if (nidx > 0)
	{
		*pIdx = new ANNidx[nidx];
		m_dfsSkull.tree->annkFRSearch(p, radius*radius, nidx, *pIdx, NULL);
	}
	return (nidx>=0 ? nidx : 0);
}


// Update the entry point. Call when the cylinder center has changed. This looks for the brain surface node
// closest to the path between the cylinder center and the target.
void TrajectoryFile::updateEntry(ElectrodeTrajectoryP& ep)
{
	double xyzTarget[3];
	double xyzCenter[3];
	double xyzEntry[3];
	double xc[3], yc[3], zc[3];
	double offset[3];
	double pathStart[3], pathEnd[3];

	ep->getTarget(xyzTarget);
	ep->getCylinderCenter(xyzCenter);
	ep->getCylinderCoords(xc, yc, zc, NULL);
	ep->getGridEntryOffset(offset);
	for (int i=0; i<3; i++)
	{
		pathStart[i] = xyzTarget[i] + offset[i];
		pathEnd[i] = xyzCenter[i] + offset[i];
	}
	getPointNearestPath(pathStart, pathEnd, m_dfsBrain.tree, 2.5, xyzEntry);
	ep->setEntry(xyzEntry);
}


// Create the kdtree for the fiducial surface.

bool TrajectoryFile::createFiducialTree(const CoordinateFile *cf)
{
	bool bval = false;
	if (NULL == m_kdtreeFiducial)
	{
		// Must get points, build tree
		int num = cf->getNumberOfCoordinates();
		if (num==0)
		{
			std::cerr << "ERROR: Coordinate file has 0 coordinates!" << std::endl;
			// issue a stern warning here
		}
		else
		{
			m_paFiducial = annAllocPts(num, 3);
			if (NULL == m_paFiducial)
			{
				std::cerr << "TrajectoryFile::createFiducialTree -- cannot allocate space for points!!!" << std::endl;
			}
			std::vector<float> vecFiducial;
			cf->getAllCoordinates(vecFiducial);
			for (int i=0; i<num; i++)
				for (int j=0; j<3; j++)
					m_paFiducial[i][j] = (double)vecFiducial[i*3+j];
			m_kdtreeFiducial = new ANNkd_tree(m_paFiducial, num, 3);
			if (NULL == m_kdtreeFiducial)
			{
				std::cerr << "TrajectoryFile::createFiducialTree -- cannot allocate space for kdtree!!!" << std::endl;
			}
			bval = true;
		}
	}
	return bval;
}

bool TrajectoryFile::insideAABB(double p[3])
{
	bool bvalue = true;
	for (int i=0; i<3; i++)
	{
		if (p[i] < m_aabbMin[i] || p[i] > m_aabbMax[i]) bvalue = false;
	}
	return bvalue;
}

int TrajectoryFile::searchPath(ElectrodeTrajectoryP& ep)
{
	double start[3];
	double end[3];
	double radius;
	double fEntry[3];				// entry point on brain surface
	double xc[3], yc[3], zc[3];
	double fSqSearchRadius;		// radius used for searching.
	double fSqRadius;				// the path radius squared
	int isteps;						// count index - # times search has been done
	double f;							// distance along path to search at.
	double v[3];
	double p[3];
	double fstepsize;
	int i;
	int nidx;						// number of points found in each search
	ANNidxArray indices;					// dynamically allocated array of indices
	ANNdistArray distances;			// dynamically allocated array of distances
	std::map<double, int> mapPathNodes;

	// Get info from traj file
	ep->getEntry(fEntry);
	radius = ep->getPathRadius();
	ep->getCylinderCoords(xc, yc, zc, NULL);

	// The entry point takes the grid entry offset and grid rotation into account (see
	// TrajectoryFile::updateEntry()).
	// Start and end points of search are along the direction of the trajectory (same as zc)
	// from the currently selected grid entry point.
	// To get the endpoint of the search path use zc (cylinder z-axis) and point back towards
	// the target.
	for (i=0; i<3; i++)
	{
		end[i] = fEntry[i] - zc[i];
		start[i] = fEntry[i];
	}

	//
	// Search radius is twice the path radius
	fstepsize = 3.4641016 * radius;		// that's 2 * sqrt(3) * radius
	fSqSearchRadius = radius*radius*4;
	fSqRadius = radius*radius;

	// Get path unit vector.
	for (i=0; i<3; i++) v[i] = end[i]-start[i];
	MathUtilities::normalize(v);

	// Search loop
	f=0;
	isteps = 0;
	p[0] = start[0];
	p[1] = start[1];
	p[2] = start[2];
	while (insideAABB(p))
	{
		f = isteps * fstepsize;
		for (i=0; i<3; i++) p[i] = start[i] + f*v[i];

		// first search gets the number of indices
		nidx = m_kdtreeFiducial->annkFRSearch(p, fSqSearchRadius, 0, NULL, NULL);

		if (nidx > 0)
		{
			indices = new ANNidx[nidx];
			distances = new ANNdist[nidx];
			m_kdtreeFiducial->annkFRSearch(p, fSqSearchRadius, nidx, indices, distances);

			// Now save only those indices within the actual radius
			for (i=0; i<nidx; i++)
			{
				double cp[3];	// candidate point
				double pp[3];	// perpendicular part
				double dp;		// dot product
				double dist;
				if (indices[i] == ANN_NULL_IDX) break;

				dist = 0;
				dp = 0;

				// cp is the vector from the start search point to a point on the fiducial surface that came back in this
				// search. We have to test it to see if its within 'radius' of the line from 'start' to 'end'. So, we form
				// the vector cp and take its dot product with 'v', the unit vector along the line. We use the dot product to
				// subtract off the component of 'cp' parallel to the line and the remaining vector 'pp' is the part of that
				// vector which is perpendicular to the line. The length of that vector is the distance from the candidate point
				// to the line.

				for (int j=0; j<3; j++) cp[j] = m_paFiducial[indices[i]][j] - start[j];
				for (int j=0; j<3; j++) dp += v[j]*cp[j];
				for (int j=0; j<3; j++) pp[j] = cp[j] - dp*v[j];

				dist = pp[0]*pp[0] + pp[1]*pp[1] + pp[2]*pp[2];
				if (dist < fSqRadius)
				{
					// map the points by their depth, as measured from brain entry point
					mapPathNodes.insert(std::pair<double, int>(dp, indices[i]));
				}
			}

			// free memory
			delete[] indices;
			delete[] distances;
		}
		isteps++;
	}
	ep->setPathNodes(mapPathNodes);

	return mapPathNodes.size();
}


void TrajectoryFile::getPointNearestPath(double start[3], double end[3], ANNkd_tree* tree, double radius, double result[3])
{
	double v[3];
	double p[3];
	double fstepsize;
	double fSqSearchRadius;		// radius used for searching.
	int isteps=0;						// count index - # times search has been done
	double f=0;							// distance along path to search at.
	int i;
	int nidx;						// number of points found in each search
	ANNidxArray indices;					// dynamically allocated array of indices
	ANNdistArray distances;			// dynamically allocated array of distances
	double fDpClosest = 0.0;
	double fClosest = 999999.0;

	// Search radius is twice the path radius
	fstepsize = 3.4641016 * radius;		// that's 2 * sqrt(3) * radius
	fSqSearchRadius = radius*radius*4;

	// Get path distance and unit vector
	for (i=0; i<3; i++) v[i] = end[i]-start[i];
	MathUtilities::normalize(v);

	// Search loop. The point p[] denotes the point we are searching at.
	// The search continues until that point travels outside the bounding box for the
	// skull. Admittedly that's beyond where we need to search, but we want to ensure that
	// the search travels the entire brain.
	p[0] = start[0];
	p[1] = start[1];
	p[2] = start[2];
	while (insideAABB(p))
	{
		f = isteps * fstepsize;
		for (i=0; i<3; i++) p[i] = start[i] + f*v[i];

		// first search gets the number of indices
		nidx = tree->annkFRSearch(p, fSqSearchRadius, 0, NULL, NULL);

		if (nidx > 0)
		{
			indices = new ANNidx[nidx];
			distances = new ANNdist[nidx];
			tree->annkFRSearch(p, fSqSearchRadius, nidx, indices, distances);

			// Now get the distance away from the line, save the closest
			for (i=0; i<nidx; i++)
			{
				double cp[3];	// candidate point
				double pp[3];	// perpendicular part
				double dp;		// dot product
				double *pbp;	   // candidate nearest point
				double dist;
				if (indices[i] == ANN_NULL_IDX) break;

				dist = 0;
				dp = 0;

				// cp is the vector from the start search point to a point on the surface
				// that came back in this search. We have to test it to see if its within
				// 'radius' of the line from 'start' to 'end'. So, we form the vector cp
				// and take its dot product with 'v', the unit vector along the line.
				// We use the dot product to subtract off the component of 'cp' parallel
				// to the line and the remaining vector 'pp' is the part of that vector
				// which is perpendicular to the line. The length of that vector is the
				// distance from the candidate point to the line. Note: I'm using the distance
				// SQUARED as the comparison.

				pbp = tree->thePoints()[indices[i]];
				for (int j=0; j<3; j++) cp[j] = pbp[j] - start[j];
				for (int j=0; j<3; j++) dp += v[j]*cp[j];
				for (int j=0; j<3; j++) pp[j] = cp[j] - dp*v[j];

				dist = pp[0]*pp[0] + pp[1]*pp[1] + pp[2]*pp[2];
				if (dist < fClosest)
				{
					fClosest = dist;	// we're saving the closest distance squared....
					fDpClosest = dp;
				}
			}

			// free memory
			delete[] indices;
			delete[] distances;
		}
		isteps++;
	}

	// Get result. This assumes that the 'end' point is the target!
	for (int i=0; i<3; i++) result[i] = start[i] + fDpClosest*v[i];
}

/// updateNearbySkullPoints - finds orientation of cylinder (i.e. angle it will rest at) given
/// the current cylinder center and target point. Also finds a set of points on the skull which
/// lie near the cylinder (for displaying) and a ring of points which lie in the projection of
/// the cylinder onto the skull surface.

void TrajectoryFile::updateNearbySkullPoints(ElectrodeTrajectoryP& ep)
{
	ANNidxArray idx;
	int nidx;
	double xyzCenter[3];
	double xyzTarget[3];
	double zprime[3], xprime[3], yprime[3];
	int i, j;
	double xhat[3] = {1, 0, 0};

	// Get target point. Use it later.
	ep->getTarget(xyzTarget);

	// fetch a set of skull points close to the cylinder center. We use the nearby diameter here - this will
	// yield a superset of the points we really want.
	ep->getCylinderCenter(xyzCenter);

	nidx = findSkullNearbyPoints(xyzCenter, &idx, ep->getSkullNearbyD()/2);

	// Find annulus of points which correspond
	// to the projection of the cylinder (with inner and outer diameter fID and fOD). Finds the resting point
	// of the cylinder on the skull surface, and ultimately generates a physical placement of the cylinder.

	double fID = ep->getCylinderID();
	double fOD = ep->getCylinderOD();	// in mm - these should be configured via dialog
	double fND = ep->getSkullNearbyD();

	// trajectory path will be the z axis...zprime
	// xprime and yprime round out the coord system of the cylinder.
	// yprime is formed by the cross product of zprime and xhat - the x unit
	// vector in the brain coord system. Thus yprime will be parallel to
	// the yz plane (the sagittal plane). In the special case where zprime
	// is parallel (antiparallel) to xhat we set yprime equal to yhat (-yhat).
	// xprime is formed by taking yprime cross zprime.
	for (i=0; i<3; i++) zprime[i]=xyzCenter[i]-xyzTarget[i];
	MathUtilities::normalize(zprime);
	MathUtilities::crossProduct(zprime, xhat, yprime);
	if (MathUtilities::normalize(yprime)==0)
	{
		if (MathUtilities::dotProduct(zprime, xhat)>0)
		{
			yprime[1] = 1;
			yprime[0]=yprime[2]=0;
		}
		else
		{
			yprime[1] = -1;
			yprime[0]=yprime[2]=0;
		}

	}
	MathUtilities::crossProduct(yprime, zprime, xprime);
	MathUtilities::normalize(xprime);

	// Form the transformation matrix. We want to transform the skull points
	// to the cylinder coordinates, which are defined by the three normalized
	// vectors xprime, yprime, and zprime. The rotation matrix is formed by
	// putting those unit vectors in the rows of a 3x3 matrix. Each skull point
	// is transformed by first subtracting off the origin (the cylinder center),
	// and then multiplying (right-multiplication) the matrix by the point.
	// The x and y coordinates tell us the distance from the trajectory.

	double T[3][3];
	for (i=0; i<3; i++)
	{
		T[0][i] = xprime[i];
		T[1][i] = yprime[i];
		T[2][i] = zprime[i];
	}

	// Now look at each point in idx. First subtract off the cylinder center, then transform to the prime
	// system. Check the x'y' distance from the origin, make sure its within the annulus region. If it is,
	// then look at the z coord. We are interested in the largest z coord - that's the resting point.

	double dsqAnnulusMin = fID*fID/4;
	double dsqAnnulusMax = fOD*fOD/4;
	double dsqNearbyMax = fND*fND/4;
	double dsq;
	double sk[3];		// skull point
	double skp[3];		// skull point, relative to center
	double skprime[3];	// skull point in cylinder coords
	double skn[3];		// vertex normal
	double zprimeMax=-99999999;
	ANNidxArray idxAnnulus;
	int nidxAnnulus=0;
	ANNidxArray idxNearby;
	int nidxNearby=0;
	double annulusSum[3] = { 0, 0, 0 };
	double annulusNormalSum[3] = { 0, 0, 0 };

	idxAnnulus = new ANNidx[nidx];	// This array will not be filled - but this way we know there's enough room.
	idxNearby = new ANNidx[nidx];	// This array will not be filled - but this way we know there's enough room.

	for (i=0; i<nidx; i++)
	{
		getSkullVertex(idx[i], sk);
		for (j=0; j<3; j++) skp[j]=sk[j]-xyzCenter[j];

		// the Multiply3x3 function is gone. Since this is the only place it gets used,
		// just code a dumb step here. Ugh.
		//MathUtilities::Multiply3x3(T, skp, skprime);
		for (j=0; j<3; j++)
			skprime[j] = T[j][0]*skp[0] + T[j][1]*skp[1] + T[j][2]*skp[2];


		// Is this point within the annulus region
		dsq = skprime[0]*skprime[0] + skprime[1]*skprime[1];

		if (dsq >= dsqAnnulusMin && dsq <= dsqNearbyMax)
		{
			if (dsq <= dsqAnnulusMax)
			{
				idxAnnulus[nidxAnnulus++] = idx[i];

				// this point is in the annulus region. Check its z coord against the max, save if necessary
				if (skprime[2] > zprimeMax) zprimeMax = skprime[2];

				// Add the point to the running sum...
				for (j=0; j<3; j++) annulusSum[j] += sk[j];

				// fetch the vertex normal and add it to the "normal sum"
				if (haveSkullVertexNormals())
				{
					getSkullVertexNormal(idx[i], skn);
					for (j=0; j<3; j++) annulusNormalSum[j] += skn[j];
				}
			}
			else
			{
				idxNearby[nidxNearby++] = idx[i];
			}
		}
	}

	// Allocate a double array for the plane fit.
	double *skullNearbyPoints;
	double plane[4];
	double fplane[3];
	double intersectpt[3];
	double t;
	double ang;
	double angdot;

	skullNearbyPoints = new double[nidxAnnulus*3];
	for (j=0; j<nidxAnnulus; j++)
	{
		getSkullVertex(idxAnnulus[j], skullNearbyPoints+j*3);
	}

	// Do the fit
	getBestFitPlane(nidxAnnulus, skullNearbyPoints, 3*sizeof(double), (double *)NULL, 0, plane);

	// We can save this plane vector. Convert to float because I've been using float
	// for everything.
	// plane[0], plane[1], plane[2] are comprise the normal vector.
	// plane[3] is the (-1)*distance from the origin to the plane.
	// Get intersection of electrode trajectory and the plane.
	fplane[0] = plane[0];
	fplane[1] = plane[1];
	fplane[2] = plane[2];

	t = (-plane[3] - MathUtilities::dotProduct(xyzTarget, fplane))/(MathUtilities::dotProduct(xyzCenter, fplane) - MathUtilities::dotProduct(xyzTarget, fplane));
	for (i=0; i<3; i++) intersectpt[i] = xyzTarget[i] + t*(xyzCenter[i]-xyzTarget[i]);

	// compute resting angle. Since acos returns values between 0 and pi, we end up with values close to pi if the dot
	// product is negative. Test the dot product and invert its sign if its negative.
	angdot = MathUtilities::dotProduct(zprime, fplane);
	if (angdot < 0) angdot = angdot * -1;
	ang = acos(angdot) * MathUtilities::radiansToDegrees();

	ep->setCylinderFitValues(intersectpt, fplane);
	ep->setCylinderBaseAngle(ang);

	delete[] skullNearbyPoints;

	ep->setSkullNearbyIdx(idxNearby, nidxNearby);
	ep->setSkullAnnulusIdx(idxAnnulus, nidxAnnulus);
	ep->setCylinderCoords(xprime, yprime, zprime, zprimeMax);

}


// Called to update the cylinder center point after one of the angle spinboxes is changed.
#if 0
void TrajectoryFile::updateCylinderCenter(ElectrodeTrajectoryP& ep)
{
	float xyzTarget[3];
	float xyzCenter[3];
	int i;
	float end[3];
	float v[3];
	float phi;
	float theta;
	float t_theta;
	float t_phi;

	ep->getTarget(xyzTarget);
	// construct new trajectory vector.
	phi = ep->getPhi() * MathUtilities::degreesToRadians();
	theta = ep->getTheta() * MathUtilities::degreesToRadians();
	t_theta = tan(theta);
	t_phi = tan(phi);
	v[2] = sqrt(1/(1+t_theta*t_theta + t_phi*t_phi));
	v[0] = v[2]*t_phi;
	v[1] = v[2]*t_theta;
	for (i=0; i<3; i++) end[i] = xyzTarget[i] + v[i];
	getPointNearestPath(xyzTarget, end, m_dfsSkull.tree, 2.5, xyzCenter);
	ep->setCylinderCenter(xyzCenter);
	updateNearbySkullPoints(ep);
	updateEntry(ep);
	searchPath(ep);
}
#endif


// Update the trajectory angles. This is called when the cylinder center has changed -- i.e. after the cylinder
// button is pushed -- and the values of theta and phi must be updated. The values of the spinbox items will
// be updated, so we suppress updates in those items' slots.
void TrajectoryFile::updateTrajectoryAngles(ElectrodeTrajectoryP& ep)
{
	double fCenter[3];
	double fTarget[3];
	double t[3];
	double theta, phi;
	ep->getCylinderCenter(fCenter);
	ep->getTarget(fTarget);
	for (int i=0; i<3; i++) t[i] = fCenter[i] - fTarget[i];
	MathUtilities::normalize(t);

	// djs New method for computing trajectory using two euler rotations.
	// This means a diff't method for figuring out theta and phi.
	theta = std::asin(t[1]);

	if (t[2]!=0)
	{
		phi = std::atan(t[0]/t[2]);
	}
	else
	{
		// trajectory lies in xy plane. Phi is either +90 or -90 or 0.
		if (t[0]==0) phi = 0;
		else
			if (t[0]>0) phi = 90 * MathUtilities::degreesToRadians();
			else phi = -90 * MathUtilities::degreesToRadians();
	}

	ep->setTheta(theta * MathUtilities::radiansToDegrees());
	ep->setPhi(phi * MathUtilities::radiansToDegrees());
}





// update the target position when the nodeID changes.
void TrajectoryFile::nodeIDChanged(ElectrodeTrajectoryP& ep)
{
	double xyz[3];
	ep->setTarget(getNodeVertex(ep->getNodeID(), xyz));
	targetPositionChanged(ep);
}

// The target position has changed. Re-find the cylinder center by locating the
// nearest skull point.
void TrajectoryFile::targetPositionChanged(ElectrodeTrajectoryP& ep)
{
	double xyz[3];
	double fCenter[3];

	// The target is the point in the brain the trajectory should pass through.
	// The 'center' is the location on the skull where the trajectory passes through.
	ep->getTarget(xyz);
	if (getSkullNearestPoint(xyz, fCenter))
	{
		ep->setCylinderCenter(fCenter);
		updateTrajectoryAngles(ep);
		cylinderPositionChanged(ep);
	}
	else
	{
		std::cerr << "getSkullNearestPoint failed!" << std::endl;
	}
}

// update the cylinder center when trajectory angles change. The target point itself has
// not changed -- but the user has adjusted the trajectory angles. That means the cylinder
// center must be recomputed using the trajectory intersection with the skull
void TrajectoryFile::trajectoryAnglesChanged(ElectrodeTrajectoryP& ep)
{
	double fTarget[3];
	double fCenter[3];
	int i;
	double end[3];
	double v[3];
	double phi;
	double theta;
	double s_theta, s_phi, c_theta, c_phi;

	ep->getTarget(fTarget);
	phi = ep->getPhi() * MathUtilities::degreesToRadians();
	theta = ep->getTheta() * MathUtilities::degreesToRadians();
	c_theta = cos(theta);
	s_theta = sin(theta);
	c_phi = cos(phi);
	s_phi = sin(phi);
	v[0] = s_phi * c_theta;
	v[1] = s_theta;
	v[2] = c_phi * c_theta;
	for (i=0; i<3; i++) end[i] = fTarget[i] + v[i];
	getPointNearestPath(fTarget, end, m_dfsSkull.tree, 2.5, fCenter);
	ep->setCylinderCenter(fCenter);
	cylinderPositionChanged(ep);
}

// Update the cylinder position/fit values when cylinder center point has changed
void TrajectoryFile::cylinderPositionChanged(ElectrodeTrajectoryP& ep)
{
	updateNearbySkullPoints(ep);
	entryPointChanged(ep);
}

// electrode entry point has changed - recalculate it based on trajectory and offset
void TrajectoryFile::entryPointChanged(ElectrodeTrajectoryP& ep)
{
	updateEntry(ep);
	pathNodesChanged(ep);
}

// set of nodes along path need to be redone
void TrajectoryFile::pathNodesChanged(ElectrodeTrajectoryP& ep)
{
	searchPath(ep);
}


