// -*- C++ -*-
/*!
* @file  HOKUYO_LRF_MRPT.cpp
* @brief HOKUYO_LRF_MRPT
* @date $Date$
*
* $Id$
*/

#include "HOKUYO_LRF_MRPT.h"

// Module specification
// <rtc-template block="module_spec">
static const char* hokuyo_lrf_mrpt_spec[] =
{
	"implementation_id", "HOKUYO_LRF_MRPT",
	"type_name",         "HOKUYO_LRF_MRPT",
	"description",       "HOKUYO_LRF_MRPT",
	"version",           "1.0.0",
	"vendor",            "Nara-NCT Ueda lab. Masanori YOSHIMOTO",
	"category",          "Category",
	"activity_type",     "PERIODIC",
	"kind",              "DataFlowComponent",
	"max_instance",      "1",
	"language",          "C++",
	"lang_type",         "compile",
	// Configuration variables
	"conf.default.usb_or_ethernet", "usb",
	"conf.default.serial_port_name", "COM1",
	"conf.default.IP", "192.168.0.10",
	"conf.default.port", "10940",
	"conf.default.rightToLeft", "1",
	"conf.default.aperture", "270.0",
	// Widget
	"conf.__widget__.usb_or_ethernet", "text",
	"conf.__widget__.serial_port_name", "text",
	"conf.__widget__.IP", "text",
	"conf.__widget__.port", "text",
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
HOKUYO_LRF_MRPT::HOKUYO_LRF_MRPT(RTC::Manager* manager)
	// <rtc-template block="initializer">
	: RTC::DataFlowComponentBase(manager),
	m_dataOut("RangeData", m_data)

	// </rtc-template>
{
}

/*!
* @brief destructor
*/
HOKUYO_LRF_MRPT::~HOKUYO_LRF_MRPT()
{
}



RTC::ReturnCode_t HOKUYO_LRF_MRPT::onInitialize()
{
	// Registration: InPort/OutPort/Service
	// <rtc-template block="registration">
	// Set InPort buffers

	// Set OutPort buffer
	addOutPort("RangeData", m_dataOut);

	// Set service provider to Ports

	// Set service consumers to Ports

	// Set CORBA Service Ports

	// </rtc-template>

	// <rtc-template block="bind_config">
	// Bind variables and configuration variable
	bindParameter("usb_or_ethernet", m_usb_or_ethernet, "usb");
	bindParameter("serial_port_name", m_serial_port_name, "COM1");
	bindParameter("IP", m_IP, "192.168.0.10");
	bindParameter("port", m_port, "10940");
	bindParameter("rightToLeft", m_rightToLeft, "1");
	bindParameter("aperture", m_aperture, "270.0");

#if MRPT_HAS_WXWIDGETS
	win = CDisplayWindowPlots::Create("HOKUYO LRF");
#endif

	// </rtc-template>
	return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t HOKUYO_LRF_MRPT::onFinalize()
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t HOKUYO_LRF_MRPT::onStartup(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t HOKUYO_LRF_MRPT::onShutdown(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t HOKUYO_LRF_MRPT::onActivated(RTC::UniqueId ec_id)
{
	if ( m_usb_or_ethernet == "usb" )
		// Set the laser serial port:
		laser.setSerialPort( m_serial_port_name );
	else
		// Set the laser serial port:
		laser.setIPandPort( m_IP, m_port );

	std::cout << "Turning laser ON...\n" << std::endl;
	if (laser.turnOn())
		std::cout << "Initialization OK!\n" << std::endl;
	else
	{
		std::cout << "Initialization failed!\n" << std::endl;
		return RTC::RTC_ERROR;
	}
	tictac.Tic();

	return RTC::RTC_OK;
}


RTC::ReturnCode_t HOKUYO_LRF_MRPT::onDeactivated(RTC::UniqueId ec_id)
{
	laser.turnOff();

	return RTC::RTC_OK;
}


RTC::ReturnCode_t HOKUYO_LRF_MRPT::onExecute(RTC::UniqueId ec_id)
{
	bool						thereIsObservation, hardError;
	CObservation2DRangeScan		obs;

	laser.doProcessSimple( thereIsObservation, obs, hardError );

	if(m_rightToLeft == 1)
		obs.rightToLeft = true;
	else
		obs.rightToLeft = false;
	obs.aperture = m_aperture * M_PIf / 180.0;

	if (hardError)
		std::cout << "Hardware error=true!!!\n" << std::endl;

	if (thereIsObservation)
	{
		double FPS = 1.0 / tictac.Tac();


		printf("Scan received: %u ranges, FOV: %.02fdeg, %.03fHz: mid rang=%fm\n",
			(unsigned int)obs.scan.size(),
			RAD2DEG(obs.aperture),
			FPS,
			obs.scan[obs.scan.size()/2]);

		obs.sensorPose = CPose3D(0,0,0);

		mrpt::slam::CSimplePointsMap		theMap;
		theMap.insertionOptions.minDistBetweenLaserPoints	= 0;
		theMap.insertObservation( &obs );
		//map.save2D_to_text_file("_out_scan.txt");

		/*
		COpenGLScene			scene3D;
		opengl::CPointCloudPtr	points = opengl::CPointCloud::Create();
		points->loadFromPointsMap(&map);
		scene3D.insert(points);
		CFileStream("_out_point_cloud.3Dscene",fomWrite) << scene3D;
		*/

		// 書き出し
		m_data.ranges.length(obs.scan.size());
		for(int i=0; i<obs.scan.size(); i++) {
			m_data.ranges[i] = (double)(obs.scan[i]);
			m_data.config.frequency = FPS;
		}

		// データ送信
		m_dataOut.write();

#if MRPT_HAS_WXWIDGETS
		vector_float	xs, ys, zs;
		theMap.getAllPoints(xs, ys, zs);
		win->plot(xs, ys, ".b3");
		win->axis_equal();
#endif

		tictac.Tic();
	}

	mrpt::system::sleep(15);

	return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t HOKUYO_LRF_MRPT::onAborting(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t HOKUYO_LRF_MRPT::onError(RTC::UniqueId ec_id)
{
	laser.turnOff();

	return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t HOKUYO_LRF_MRPT::onReset(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t HOKUYO_LRF_MRPT::onStateUpdate(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t HOKUYO_LRF_MRPT::onRateChanged(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/



extern "C"
{

	void HOKUYO_LRF_MRPTInit(RTC::Manager* manager)
	{
		coil::Properties profile(hokuyo_lrf_mrpt_spec);
		manager->registerFactory(profile,
			RTC::Create<HOKUYO_LRF_MRPT>,
			RTC::Delete<HOKUYO_LRF_MRPT>);
	}

};


