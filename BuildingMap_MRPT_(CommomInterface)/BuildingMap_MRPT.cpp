// -*- C++ -*-
/*!
* @file  BuildingMap_MRPT.cpp
* @brief Output ""rawlog"" file and map building.
* @date $Date$
*
* $Id$
*/

#include "BuildingMap_MRPT.h"

// Module specification
// <rtc-template block="module_spec">
static const char* buildingmap_mrpt_spec[] =
{
	"implementation_id", "BuildingMap_MRPT",
	"type_name",         "BuildingMap_MRPT",
	"description",       "Output ""rawlog"" file and map building.",
	"version",           "1.1.0",
	"vendor",            "Nara-NCT Ueda Lab. MasanoriYOSHIMOTO",
	"category",          "Category",
	"activity_type",     "PERIODIC",
	"kind",              "DataFlowComponent",
	"max_instance",      "1",
	"language",          "C++",
	"lang_type",         "compile",
	// Configuration variables
	"conf.default.rightToLeft", "1",
	"conf.default.aperture", "270.0",
	// Widget
	"conf.__widget__.rightToLeft", "text",
	"conf.__widget__.aperture", "text",
	// Constraints
	""
};
// </rtc-template>

/*!
* @brief constructor
* @param manager Maneger Object
*/
BuildingMap_MRPT::BuildingMap_MRPT(RTC::Manager* manager)
	// <rtc-template block="initializer">
	: RTC::DataFlowComponentBase(manager),
	m_InOdometryPositionIn("InOdometryPosition", m_InOdometryPosition),
	m_rangeDataIn("RangeData", m_rangeData)

	// </rtc-template>
{
}

/*!
* @brief destructor
*/
BuildingMap_MRPT::‾BuildingMap_MRPT()
{
}



RTC::ReturnCode_t BuildingMap_MRPT::onInitialize()
{
	// Registration: InPort/OutPort/Service
	// <rtc-template block="registration">
	// Set InPort buffers
	addInPort("InOdometryPosition", m_InOdometryPositionIn);
	addInPort("RangeData", m_rangeDataIn);

	// Set OutPort buffer

	// Set service provider to Ports

	// Set service consumers to Ports

	// Set CORBA Service Ports

	// </rtc-template>

	// <rtc-template block="bind_config">
	// Bind variables and configuration variable
	bindParameter("rightToLeft", m_rightToLeft, "1");
	bindParameter("aperture", m_aperture, "270.0");

	now_x = 0.0; now_y = 0.0; now_theta = 0.0;
	past_x = 0.0; past_y = 0.0; past_theta = 0.0;

	// -----------------------------------------------------------------------------------
	//            MRPT
	// -----------------------------------------------------------------------------------
	CConfigFile				iniFile("icp-slam.ini");

	// ------------------------------------------
	//			Load config from file:
	// ------------------------------------------
	rawlog_offset		 = iniFile.read_int("MappingApplication","rawlog_offset",0,  /*Force existence:*/ true);
	OUT_DIR_STD			 = iniFile.read_string("MappingApplication","logOutput_dir","log_out",  /*Force existence:*/ true);
	LOG_FREQUENCY		 = iniFile.read_int("MappingApplication","LOG_FREQUENCY",5,  /*Force existence:*/ true);
	SAVE_POSE_LOG		 = iniFile.read_bool("MappingApplication","SAVE_POSE_LOG", false,  /*Force existence:*/ true);
	SAVE_3D_SCENE        = iniFile.read_bool("MappingApplication","SAVE_3D_SCENE", false,  /*Force existence:*/ true);
	CAMERA_3DSCENE_FOLLOWS_ROBOT = iniFile.read_bool("MappingApplication","CAMERA_3DSCENE_FOLLOWS_ROBOT", true,  /*Force existence:*/ true);

	SHOW_PROGRESS_3D_REAL_TIME = false;
	SHOW_PROGRESS_3D_REAL_TIME_DELAY_MS = 0;

	MRPT_LOAD_CONFIG_VAR( SHOW_PROGRESS_3D_REAL_TIME, bool,  iniFile, "MappingApplication");
	MRPT_LOAD_CONFIG_VAR( SHOW_PROGRESS_3D_REAL_TIME_DELAY_MS, int, iniFile, "MappingApplication");

	OUT_DIR = OUT_DIR_STD.c_str();

	// ------------------------------------
	//		Constructor of ICP-SLAM object
	// ------------------------------------
	mapBuilder.ICP_options.loadFromConfigFile( iniFile, "MappingApplication");
	mapBuilder.ICP_params.loadFromConfigFile ( iniFile, "ICP");

	// Construct the maps with the loaded configuration.
	mapBuilder.initialize();


	// ---------------------------------
	//   CMetricMapBuilder::TOptions
	// ---------------------------------
	mapBuilder.options.verbose = true;
	mapBuilder.options.alwaysInsertByClass.fromString( iniFile.read_string("MappingApplication","alwaysInsertByClass","") );


	// Print params:
	printf("Running with the following parameters:¥n");
	printf(" Output directory:¥t¥t¥t'%s'¥n",OUT_DIR);
	printf(" matchAgainstTheGrid:¥t¥t¥t%c¥n", mapBuilder.ICP_options.matchAgainstTheGrid ? 'Y':'N');
	printf(" Log record freq:¥t¥t¥t%u¥n",LOG_FREQUENCY);
	printf("  SAVE_3D_SCENE:¥t¥t¥t%c¥n", SAVE_3D_SCENE ? 'Y':'N');
	printf("  SAVE_POSE_LOG:¥t¥t¥t%c¥n", SAVE_POSE_LOG ? 'Y':'N');
	printf("  CAMERA_3DSCENE_FOLLOWS_ROBOT:¥t%c¥n",CAMERA_3DSCENE_FOLLOWS_ROBOT ? 'Y':'N');

	printf("¥n");

	mapBuilder.ICP_params.dumpToConsole();
	mapBuilder.ICP_options.dumpToConsole();

	// Prepare output directory:
	// --------------------------------
	deleteFilesInDirectory(OUT_DIR);
	createDirectory(OUT_DIR);

	// Checks:
	step = 0;

	// Open log files:
	// ----------------------------------
	f_log.open(format("%s/log_times.txt",OUT_DIR));
	f_path.open(format("%s/log_estimated_path.txt",OUT_DIR));
	f_pathOdo.open(format("%s/log_odometry_path.txt",OUT_DIR));

	// Create 3D window if requested:
#if MRPT_HAS_WXWIDGETS
	if (SHOW_PROGRESS_3D_REAL_TIME)
	{
		win3D = CDisplayWindow3D::Create("ICP-SLAM @ MRPT C++ Library", 600, 500);
		win3D->setCameraZoom(20);
		win3D->setCameraAzimuthDeg(-45);
	}
#endif

	// </rtc-template>
	return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t BuildingMap_MRPT::onFinalize()
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t BuildingMap_MRPT::onStartup(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t BuildingMap_MRPT::onShutdown(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t BuildingMap_MRPT::onActivated(RTC::UniqueId ec_id)
{
	// Rawlog
	outputFile.open(format("%s/dataset.rawlog",OUT_DIR));
	firstloop = true;

	return RTC::RTC_OK;
}


RTC::ReturnCode_t BuildingMap_MRPT::onDeactivated(RTC::UniqueId ec_id)
{
	outputFile.close();
	f_log.close();
	f_path.close();
	f_pathOdo.close();

	// Save map:
	mapBuilder.getCurrentlyBuiltMap(finalMap);

	str = format("%s/_finalmap_.simplemap",OUT_DIR);
	printf("Dumping final map in binary format to: %s¥n", str.c_str() );
	mapBuilder.saveCurrentMapToFile(str);

	CMultiMetricMap  *finalPointsMap = mapBuilder.getCurrentlyBuiltMetricMap();
	str = format("%s/_finalmaps_.txt",OUT_DIR);
	printf("Dumping final metric maps to %s_XXX¥n", str.c_str() );
	finalPointsMap->saveMetricMapRepresentationToFile( str );

	mapBuilder.clear();

	//if (win3D)
	//	win3D->waitForKey();

	return RTC::RTC_OK;
}


RTC::ReturnCode_t BuildingMap_MRPT::onExecute(RTC::UniqueId ec_id)
{
	// 休憩
	Sleep(300);

	// ------------------------------------------------------------------------
	//                           Rawlog
	// ------------------------------------------------------------------------
	CActionCollection actions;
	CSensoryFrame  SF;


	// Odometry
	//while(!m_InOdometryPositionIn.isEmpty()){
	if(m_InOdometryPositionIn.isNew()){
		m_InOdometryPositionIn.read();
		now_x = m_InOdometryPosition.data.position.x;
		now_y = m_InOdometryPosition.data.position.y;
		now_theta = m_InOdometryPosition.data.heading;

		if(firstloop == true){
			past_x = now_x; past_y = now_y; past_theta = now_theta;
			firstloop = false;
		}

		double x = (now_x-past_x)*cos(-past_theta)-(now_y-past_y)*sin(-past_theta);
		double y = (now_x-past_x)*sin(-past_theta)+(now_y-past_y)*cos(-past_theta);
		double theta = now_theta-past_theta;

		CPose2D  Pos(x, y, theta);
		CActionRobotMovement2D action;  // 2D odometry
		CActionRobotMovement2D::TMotionModelOptions	options;
		action.computeFromOdometry(Pos, options);
		mrpt::system::TTimeStamp AtD0	=  mrpt::system::getCurrentTime();
		action.timestamp = AtD0;
		actions.insert(action);
	}

	// LRF
	//while(!m_InRangeDataIn.isEmpty()){
	if(m_rangeDataIn.isNew()){
		m_rangeDataIn.read();
		CObservation2DRangeScanPtr myObs = CObservation2DRangeScan::Create();
		int SCANS_SIZE = m_rangeData.ranges.length();
		myObs->aperture = m_aperture * M_PIf / 180.0;
		if(m_rightToLeft == 1)
			myObs->rightToLeft = true;
		else
			myObs->rightToLeft = false;
		//myObs->maxRange = 40;
		myObs->validRange.resize(SCANS_SIZE);
		myObs->scan.resize(SCANS_SIZE);
		mrpt::system::TTimeStamp AtD1	=  mrpt::system::getCurrentTime();
		myObs->timestamp = AtD1;
		myObs->scan.resize(SCANS_SIZE);
		for(int i=0; i<SCANS_SIZE; i++) {
			myObs->scan[i] = (float)m_rangeData.ranges[i];
			myObs->validRange[i] = 1;
		}
		SF.insert(myObs);  // memory of "myObs" will be automatically freed.
	}

	past_x = now_x;
	past_y = now_y;
	past_theta = now_theta;

	outputFile << actions << SF;

	// ----------------------------------------------------------
	//						Map Building
	// ----------------------------------------------------------
	CPose2D	odoPose(0,0,0);

	// Execute:
	// ----------------------------------------
	mapBuilder.processActionObservation( actions, SF );

	// Info log:
	// -----------
	f_log.printf("%f %i¥n",1000.0f*t_exec,mapBuilder.getCurrentlyBuiltMapSize() );

	const CMultiMetricMap* mostLikMap =  mapBuilder.getCurrentlyBuiltMetricMap();

	if (0==(step % LOG_FREQUENCY))
	{
		// Pose log:
		// -------------
		if (SAVE_POSE_LOG)
		{
			printf("Saving pose log information...");
			mapBuilder.getCurrentPoseEstimation()->saveToTextFile( format("%s/mapbuild_posepdf_%03u.txt",OUT_DIR,step) );
			printf("Ok¥n");
		}
	}

	// Save a 3D scene view of the mapping process:
	if (0==(step % LOG_FREQUENCY) || (SAVE_3D_SCENE || win3D.present()))
	{
		CPose3D robotPose;
		mapBuilder.getCurrentPoseEstimation()->getMean(robotPose);

		COpenGLScenePtr		scene = COpenGLScene::Create();

		COpenGLViewportPtr view=scene->getViewport("main");
		ASSERT_(view);

		COpenGLViewportPtr view_map = scene->createViewport("mini-map");
		view_map->setBorderSize(2);
		view_map->setViewportPosition(0.01,0.01,0.35,0.35);
		view_map->setTransparent(false);

		{
			mrpt::opengl::CCamera &cam = view_map->getCamera();
			cam.setAzimuthDegrees(-90);
			cam.setElevationDegrees(90);
			cam.setPointingAt(robotPose);
			cam.setZoomDistance(20);
			cam.setOrthogonal();
		}

		// The ground:
		mrpt::opengl::CGridPlaneXYPtr groundPlane = mrpt::opengl::CGridPlaneXY::Create(-200,200,-200,200,0,5);
		groundPlane->setColor(0.4,0.4,0.4);
		view->insert( groundPlane );
		view_map->insert( CRenderizablePtr( groundPlane) ); // A copy

		// The camera pointing to the current robot pose:
		if (CAMERA_3DSCENE_FOLLOWS_ROBOT)
		{
			scene->enableFollowCamera(true);

			mrpt::opengl::CCamera &cam = view_map->getCamera();
			cam.setAzimuthDegrees(-45);
			cam.setElevationDegrees(45);
			cam.setPointingAt(robotPose);
		}

		// The maps:
		{
			opengl::CSetOfObjectsPtr obj = opengl::CSetOfObjects::Create();
			mostLikMap->getAs3DObject( obj );
			view->insert(obj);

			// Only the point map:
			opengl::CSetOfObjectsPtr ptsMap = opengl::CSetOfObjects::Create();
			if (mostLikMap->m_pointsMaps.size())
			{
				mostLikMap->m_pointsMaps[0]->getAs3DObject(ptsMap);
				view_map->insert( ptsMap );
			}
		}

		// Draw the robot path:
		CPose3DPDFPtr posePDF =  mapBuilder.getCurrentPoseEstimation();
		CPose3D  curRobotPose;
		posePDF->getMean(curRobotPose);
		{
			opengl::CSetOfObjectsPtr obj = opengl::stock_objects::RobotPioneer();
			obj->setPose( curRobotPose );
			view->insert(obj);
		}
		{
			opengl::CSetOfObjectsPtr obj = opengl::stock_objects::RobotPioneer();
			obj->setPose( curRobotPose );
			view_map->insert( obj );
		}

		// Save as file:
		if (0==(step % LOG_FREQUENCY) && SAVE_3D_SCENE)
		{
			CFileGZOutputStream	f( format( "%s/buildingmap_%05u.3Dscene",OUT_DIR,step ));
			f << *scene;
		}

		// Show 3D?
		if (win3D)
		{
			opengl::COpenGLScenePtr &ptrScene = win3D->get3DSceneAndLock();
			ptrScene = scene;

			win3D->unlockAccess3DScene();

			// Move camera:
			win3D->setCameraPointingToPoint( curRobotPose.x(),curRobotPose.y(),curRobotPose.z() );

			// Update:
			win3D->forceRepaint();

			sleep( SHOW_PROGRESS_3D_REAL_TIME_DELAY_MS );
		}
	}

	// Save the memory usage:
	// ------------------------------------------------------------------
	{
		printf("Saving memory usage...");
		unsigned long	memUsage = getMemoryUsage();
		FILE		*f=os::fopen( format("%s/log_MemoryUsage.txt",OUT_DIR).c_str() ,"at");
		if (f)
		{
			os::fprintf(f,"%u¥t%lu¥n",step,memUsage);
			os::fclose(f);
		}
		printf("Ok! (%.04fMb)¥n", ((float)memUsage)/(1024*1024) );
	}

	// Save the robot estimated pose for each step:
	f_path.printf("%i %f %f %f¥n",
		step,
		mapBuilder.getCurrentPoseEstimation()->getMeanVal().x(),
		mapBuilder.getCurrentPoseEstimation()->getMeanVal().y(),
		mapBuilder.getCurrentPoseEstimation()->getMeanVal().yaw() );


	f_pathOdo.printf("%i %f %f %f¥n",step,odoPose.x(),odoPose.y(),odoPose.phi());

	step++;

	return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t BuildingMap_MRPT::onAborting(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t BuildingMap_MRPT::onError(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t BuildingMap_MRPT::onReset(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t BuildingMap_MRPT::onStateUpdate(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t BuildingMap_MRPT::onRateChanged(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/



extern "C"
{

	void BuildingMap_MRPTInit(RTC::Manager* manager)
	{
		coil::Properties profile(buildingmap_mrpt_spec);
		manager->registerFactory(profile,
			RTC::Create<BuildingMap_MRPT>,
			RTC::Delete<BuildingMap_MRPT>);
	}

};


