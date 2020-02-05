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


#ifndef __TRAJECTORY_FILE_H__
#define __TRAJECTORY_FILE_H__

#include "AbstractFile.h"
#include "CoordinateFile.h"
#include "SpecFile.h"
#include <qcolor.h>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <ANN/ANN.h>

typedef
struct dfs_data_struct
{
	int nfaces;
	int *faces;
	double orientation[16];
	int nvertices;
	ANNcoord *vertices;
	float *vertexnormals;
	float *color;
	float *uv;
	ANNpointArray pa;
	ANNkd_tree *tree;
	std::multimap<int, int> mapFaces;
} DfsFileStruct;




class ElectrodeTrajectory
{
	private:

	std::string m_name;								///< trajectory name

	// skull points near entry
	ANNidxArray m_idxSkullNearby;			///< nn tree of points nearby the center point
	int m_nidxSkullNearby;						///< number of points in m_nidxSkullNearby
	ANNidxArray m_idxSkullAnnulus;		///< nn tree of points in projection of cylinder onto skull surface
	int m_nidxSkullAnnulus;						///< number of points in m_nidxSkullAnnulus

	// path radius, depth tool settings
	double m_dPathRadius;							///< radius (mm) used to search for nodes along trajectory
	bool m_bUseDepthTool;							///< if true, use depth tool to determine what nodes along path to display
	double m_dDepthToolTolerance;			///< when using depth tool, display nodes this distance above and below depth tool point
	int m_iDepthToolDepthPct;					///< value of depth tool slider (0<= m_iDepthToolDepthPct <= 100)
	std::map<double, int> m_mapPathNodes;		///< Nodes along trajectory path, mapped by their depth (measured from entry point)

	// Display-specific settings for this trajectory
	bool m_bUseNodeID;								///< if true, using node id for target position
	bool m_bDisplaySkull;							///< if true, displaying skull surface
	bool m_bDisplayBrain;							///< if true, displaying brain surface
	bool m_bDisplayCylinder;					///< if true, displaying cylinder
	bool m_bDisplayTrajectory;				///< if true, displaying trajectory (and nodes along it)

	// colors
	QColor m_colorSkullSurfaceAnnulus;///< color to use for skull surface annulus (projection of cylinder)
	QColor m_colorSkullSurface;				///< color to use for skull surface
	QColor m_colorCylinderSides;			///< color to use for sides of cylinder
	QColor m_colorCylinderEnds;				///< color to use for ends of cylinder
	QColor m_colorPathLine;						///< color to use for line indicating trajectory
	QColor m_colorPathMarkers;				///< color to use for markers on path
	QColor m_colorFlatMapNodes;				///< color to use for nearby nodes displayed on flat map
	QColor m_colorVolumeNodes;				///< color to use for nodes on volume map

	// cylinder position/fit values
	bool m_bHasTarget;								// this is false until a target is selected for the trajectory
	int m_iNodeID;										///< id of current target node (only valid if m_bUseNodeID==true)
	double m_dTarget[3];								///< location of target node
	double m_dCenter[3];								///< location of cylinder center intersection with skull surface
	double m_dEntry[3];								///< intersection of current trajectory and brain surface
	double m_theta;										///< angle between the trajectory's projection on the yz plane and z-hat
	double m_phi;											///< angle between trajectory and yz plane
	double m_dXCyl[3]; 								///< x unit vector (1st column) of cylinder rotation matrix
	double m_dYCyl[3]; 								///< y unit vector (2nd column) of cylinder rotation matrix
	double m_dZCyl[3]; 								///< z unit vector (3rd column) of cylinder rotation matrix
	double m_dZRest;										///< The cylinder base rests this distance (along trajectory) from fCenter
	double m_dCylinderID;							///< Cylinder inner diam (mm)
	double m_dCylinderOD;							///< cylinder outer diam (mm)
	double m_dCylinderH;								///< cylinder height (mm)
	double m_dMaxPenetrationDepth;				///< max penetration depth in mm
	double m_dSkullNearbyD;						///< when displaying skull, this is diameter for points to display
	double m_dCylFitCenter[3];					///< center point of cylinder when skull surface taken into account
	double m_dCylFitNormal[3];					///< normal vector of cylinder fit surface - indicates angle of cylinder base
	double m_dCylBaseAngle;						///< angle of cylinder base req'd for good fit on skull surface
	double m_dGridEntryX;							///< grid entry point offset (in cylinder X direction)
	double m_dGridEntryY;							///< grid entry point offset (in cylinder Y direction)
	double m_dGridRotation;						///< grid is rotated w/r/t cylinder coordinates (degrees)

	/// clear the list of nearby nodes
	void clearIdxNearby();

	// clear the list of nodes in the cylinder annulus
	void clearIdxAnnulus();

	protected:

	// initialize a new trajectory
	void init();

	// initialize a new trajectory with values from another
	void init(const ElectrodeTrajectory& e);

	public:

	ElectrodeTrajectory();
	ElectrodeTrajectory(const ElectrodeTrajectory& et);
	virtual ~ElectrodeTrajectory();

	/// set this trajectory's name
	void setName(std::string sName);

	/// get this trajectory's name
	const std::string getName() const { return m_name; };

	/// get the target node position
	double *getTarget(double dTarget[3]) const;

	/// set target node position
	void setTarget(double dTarget[3]);

	/// get the entry point
	void getEntry(double dEntry[3]) const;

	/// set entry point
	void setEntry(double dEntry[3]);

	/// set node id
	void setNodeID(int i);

	/// get node id
	int getNodeID() const;

	/// We have a target.
	void setHasTarget(bool hasTarget);

	// do we have a target?
	bool getHasTarget();

	/// set the path radius
	void setPathRadius(double d);

	/// get the current path radius
	double getPathRadius() const;

	/// set the current cylinder center point
	void setCylinderCenter(double dCenter[3]);

	/// get the current cylinder center
	double* getCylinderCenter(double dCenter[3]) const;

	/// set cylinder fit values
	void setCylinderFitValues(double dCylFitCenter[3], double dCylFitNormal[3]);

	/// get cylinder fit values
	void getCylinderFitValues(double *dCylFitCenter, double *dCylFitNormal) const;

 	/// set cylinder base angle
	void setCylinderBaseAngle(double deg);

	/// get cylinder base angle
	double getCylinderBaseAngle() const;

	/// get display skull value
	bool getDisplaySkull() const;

	/// set display skull value
	void setDisplaySkull(bool b);

	/// get display brain value
	bool getDisplayBrain() const;

	/// set display brain value
	void setDisplayBrain(bool b);

	/// get display cylinder value
	bool getDisplayCylinder() const;

	/// set display cylinder value
	void setDisplayCylinder(bool b);

	/// get display trajectory value
	bool getDisplayTrajectory() const;

	/// set display trajectory value
	void setDisplayTrajectory(bool b);

	/// get use depth tool value
	bool getUseDepthTool() const;

	/// set use depth tool value
	void setUseDepthTool(bool b);

	/// set depth tool tolerance
	void setDepthToolTolerance(double d);

	/// get depth tool tolerance
	double getDepthToolTolerance() const;

	/// set depth tool pct value
	void setDepthToolDepthPct(int pct);

	/// get depth tool pct value
	int getDepthToolDepthPct() const;

	/// get trajectory angles
	double getTheta() const;
	double getPhi() const;

	/// set trajectory angles
	void setTheta(double theta);
	void setPhi(double phi);

	/// set use node id value
	void setUseNodeID(bool bvalue);

	/// get use node id value
	bool getUseNodeID() const;

	/// replace the map of nodes in the trajectory path
	void setPathNodes(std::map<double, int> mapNodes);

	/// get the map of nodes in the traj path
	const std::map<double, int>& getPathNodes() const;


	/// set the list of nearby nodes - those nearby the center point
	void setSkullNearbyIdx(ANNidxArray idxSkullNearby, int nidx);

	/// get the list of nearby nodes
	void getSkullNearbyIdx(ANNidxArray* pidx, int *pnidx) const;

	/// set the list of nodex in the annulus of the cylinder
	void setSkullAnnulusIdx(ANNidxArray idx, int nidx);

	/// get the list of nodes in the annulus of the cylinder
	void getSkullAnnulusIdx(ANNidxArray* pidx, int *pnidx) const;

	/// get the orientation vectors and rest coordinate for the cylinder
	void getCylinderCoords(double xhat[3], double yhat[3], double zhat[3], double *pzrest) const;

	/// set the orientation vectors and rest coordinate for the cylinder
	void setCylinderCoords(double xhat[3], double yhat[3], double zhat[3], double zrest);

	/// get the offset values for the grid entry point
	void getGridEntry(double *px, double *py) const;

	/// set the offset values for the grid entry point
	void setGridEntry(double x, double y);

	/// set rotation of grid w/r/to cylinder coords
	void setGridRotation(double r);

	/// get rotation of grid w/r/to cylinder coords
	double getGridRotation() const;

	const QColor& getSkullSurfaceAnnulusColor() const;
	void setSkullSurfaceAnnulusColor(const QColor& color);

	const QColor& getSkullSurfaceColor() const;
	void setSkullSurfaceColor(const QColor& color);

	const QColor& getCylinderSidesColor() const;
	void setCylinderSidesColor(const QColor& color);

	const QColor& getCylinderEndsColor() const;
	void setCylinderEndsColor(const QColor& color);

	const QColor& getPathLineColor() const;
	void setPathLineColor(const QColor& color);

	const QColor& getPathMarkersColor() const;
	void setPathMarkersColor(const QColor& color);

	const QColor& getFlatMapNodesColor() const;
	void setFlatMapNodesColor(const QColor& color);

	const QColor& getVolumeNodesColor() const;
	void setVolumeNodesColor(const QColor& color);

	double getCylinderID() const;
	void setCylinderID(double id);

	double getCylinderOD() const;
	void setCylinderOD(double od);

	double getCylinderH() const;
	void setCylinderH(double h);

	double getMaxPenetrationDepth() const;
	void setMaxPenetrationDepth(double d);

	/// Get depth of penetration, as defined by the max penetration depth and the current slider value.
	double getPenetrationDepth() const;

	/// Get position of current penetration, as defined by depth and trajectory
	void getPenetrationPosition(double dpt[3]) const;

	/// Get offset vector for current grid position and rotation.
	/// The returned vector is parallel to the plane of the grid.
	void getGridEntryOffset(double *offset) const;

	double getSkullNearbyD() const;
	void setSkullNearbyD(double d);


};

typedef boost::shared_ptr< ElectrodeTrajectory > ElectrodeTrajectoryP;
typedef std::map<std::string, ElectrodeTrajectoryP> ElectrodeTrajectoryMap;
typedef ElectrodeTrajectoryMap::iterator ElectrodeTrajectoryMapIterator;
typedef ElectrodeTrajectoryMap::const_iterator ElectrodeTrajectoryMapConstIterator;


class ElectrodeTrajectories: public std::map<std::string, ElectrodeTrajectoryP>
{
	private:

	public:

	ElectrodeTrajectories();
	virtual ~ElectrodeTrajectories() {};

	bool addTrajectory(ElectrodeTrajectoryP e);

};




/// class for storing data associated with the electrode trajectory tool(s)
class TrajectoryFile : public AbstractFile
{
   private:

	std::string m_sSkullFileName;					///< file used for skull surface data
	std::string m_sBrainFileName;					///< file used for brain surface data
	DfsFileStruct m_dfsSkull;					///< data from skull surface file
	DfsFileStruct m_dfsBrain;					///< data from brain surface file
	double m_volumeOrigin[3];					///< origin of volume file - added to skull and brain surface coords
	double m_aabbMin[3];								///< min corner of axis-aligned bounding box for skull surface
	double m_aabbMax[3];								///< max corner of axis-aligned bounding box for skull surface
	ANNkd_tree *m_kdtreeFiducial;				///< nn tree for fiducial surface
	ANNpointArray m_paFiducial;				/// point storage for fiducial storage nn tree

	QColor m_colorBrainSurface;				///< color to use for brain surface

	std::string _sActiveTrajectoryName;
	ElectrodeTrajectories _trajectories;

#if 0
	bool getHeaderBool(const std::string& name, bool& b);
	bool getHeaderFloat(const std::string& name, float& f);
	bool getHeaderInt(const std::string& name, int& i);
	bool getHeaderXyz(const std::string& name, float xyz[3]);
	bool getHeaderColor(const std::string& name, QColor& color);
	void setHeaderBool(const std::string& name, const bool& b);
	void setHeaderFloat(const std::string& name, const float& f);
	void setHeaderInt(const std::string& name, const int& i);
	void setHeaderXyz(const std::string& name, const float xyz[3]);
	void setHeaderColor(const std::string& name, const QColor& color);
#endif

	/// read a data file
	void readDfsFile(QFile& infile, QDataStream& binStream, DfsFileStruct *pfile) throw (FileException);

	/// free the memory allocated in the data file struct
	void clearDfsStruct(DfsFileStruct *pdfs);

	/// check whether a point is inside the axis-aligned bounding box (AABB) of the skull surface
	bool insideAABB(double p[3]);

   public:

   /// constructor
   TrajectoryFile();

	void readSkullFiles(const QString& skullSurfaceFile, const QString& brainSurfaceFile) throw (FileException);


      /// Read the contents of the file (header has already been read)
   virtual void readFileData(QFile& file,
                             QTextStream& stream,
                             QDataStream& binStream,
                             QDomElement& rootElement) throw (FileException);

   /// Write the file's data (header has already been written)
   virtual void writeFileData(QTextStream& stream,
                              QDataStream& binStream,
                              QDomDocument& xmlDoc,
                              QDomElement& rootElement) throw (FileException);


   /// destructor
   virtual ~TrajectoryFile();

   /// clear a trajectory file
   void clear();

   /// returns true if there is no brain or skull file.. not the best, but it'll do
   bool empty() const;

	ElectrodeTrajectoryP getActiveTrajectory() const;
	bool setActiveTrajectory(std::string name);

	int getTrajectoryCount() const;
	ElectrodeTrajectoryP getTrajectory(const std::string name) const;

	/// Get the trajectories map
	ElectrodeTrajectories& getTrajectories();

	/// Add a new trajectory.
	bool addTrajectory(ElectrodeTrajectoryP& ep);

	/// remove a trajectory. Returns pointer to trajectory (will be null if no trajectory with
	/// that name exists).
	ElectrodeTrajectoryP removeTrajectory(const std::string name);

	// pass existing arrays like float fv1[3] to this function.
	// ugh, vertices stored as double but normals as float.
	void getFace(int index, float *pfv1, float *pfv2, float *pfv3, float *pfn1, float *pfn2, float *pfn3) const;

	const std::multimap<int, int>& getSkullFaceMap() const;

	/// Get the color assigned to the brain surface
	const QColor& getBrainSurfaceColor() const;

	/// Set the color assigned to the brain surface
	void setBrainSurfaceColor(const QColor& color);

	/// Returns number of vertices in skull
	int getNumSkullVertices() const;

	/// Returns number of vertices in brain surface
	int getNumBrainVertices() const;

	/// Returns a particular skull vertex. No bounds checking!
	float *getSkullVertex(int i, float *xyz) const;

	/// Returns a particular skull vertex. No bounds checking!
	double *getSkullVertex(int i, double *xyz) const;

	/// gets a particular vertex from the fiducial surface (i.e. a node)
	float *getNodeVertex(int inode, float *xyz) const;

	/// gets a particular vertex from the fiducial surface (i.e. a node)
	double *getNodeVertex(int inode, double *xyz) const;

	/// Does the skull file have vertex normals?
	bool haveSkullVertexNormals() const;

	/// Returns a particular skull vertex normal. No bounds checking!
	double *getSkullVertexNormal(int i, double *xyz) const;

	/// Returns a particular brain vertex. No bounds checking!
	double *getBrainVertex(int i, double *xyz) const;

	/// returns the kdtree for the skull
	ANNkd_tree *getSkullTree() const;

	/// returns the kdtree for the brain
	ANNkd_tree *getBrainTree() const;

	/// Set the origin to be used when reading coordinates
	void setVolumeOrigin(double xyzOrigin[3]);

	/// Get the axis-aligned bounding box min corner
	void getAABBMin(double p[3]) const;

	/// Get the axis-aligned bounding box max corner
	void getAABBMax(double p[3]) const;

	/// Set the axis-aligned bounding box min corner
	void setAABBMin(double p[3]);

	/// Set the axis-aligned bounding box max corner
	void setAABBMax(double p[3]);


	bool getSkullNearestPoint(double p[3], double dNearest[3]);

	bool getBrainNearestPoint(double p[3], double dNearest[3]);

	int findSkullNearbyPoints(double p[3], ANNidxArray* pIdx, double radius);

	void updateEntry(ElectrodeTrajectoryP& ep);

	int searchPath(ElectrodeTrajectoryP& ep);

	void getPointNearestPath(double start[3], double end[3], ANNkd_tree* tree, double radius, double result[3]);

	void updateNearbySkullPoints(ElectrodeTrajectoryP& ep);

	bool createFiducialTree(const CoordinateFile *cf);

	void updateCylinderCenter(ElectrodeTrajectoryP& ep);

	void updateTrajectoryAngles(ElectrodeTrajectoryP& ep);

	// Update the target location.
	void updateTarget(ElectrodeTrajectoryP& ep);



	// update the target position when the nodeID changes.
	void nodeIDChanged(ElectrodeTrajectoryP& ep);

	// The target position has changed. Re-find the cylinder center by locating the
	// nearest skull point.
	void targetPositionChanged(ElectrodeTrajectoryP& ep);

	// update the cylinder center when trajectory angles change. The target point itself has
	// not changed -- but the user has adjusted the trajectory angles. That means the cylinder
	// center must be recomputed using the trajectory intersection with the skull
	void trajectoryAnglesChanged(ElectrodeTrajectoryP& ep);

	// Update the cylinder position/fit values when cylinder center point has changed
	void cylinderPositionChanged(ElectrodeTrajectoryP& ep);

	// electrode entry point has changed - recalculate it based on trajectory and offset
	void entryPointChanged(ElectrodeTrajectoryP& ep);

	// set of nodes along path need to be redone
	void pathNodesChanged(ElectrodeTrajectoryP& ep);





};

#endif
