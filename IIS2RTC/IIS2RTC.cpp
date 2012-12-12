// -*- C++ -*-
/*!
* @file  IIS2RTC.cpp
* @brief ModuleDescription
* @date $Date$
*
* $Id$
*/

#include "IIS2RTC.h"

// Module specification
// <rtc-template block="module_spec">
static const char* iis2rtc_spec[] =
{
	"implementation_id", "IIS2RTC",
	"type_name",         "IIS2RTC",
	"description",       "ModuleDescription",
	"version",           "1.0.0",
	"vendor",            "Masanori YOSHIMOTO",
	"category",          "Category",
	"activity_type",     "PERIODIC",
	"kind",              "DataFlowComponent",
	"max_instance",      "1",
	"language",          "C++",
	"lang_type",         "compile",
	""
};
// </rtc-template>

/*!
* @brief constructor
* @param manager Maneger Object
*/
IIS2RTC::IIS2RTC(RTC::Manager* manager)
	// <rtc-template block="initializer">
	: RTC::DataFlowComponentBase(manager),
	m_iisTimedPose2DIn("iisTimedPose2D", m_iisTimedPose2D),
	m_rtcTimedPose2DOut("rtcTimedPose2D", m_rtcTimedPose2D)

	// </rtc-template>
{
}

/*!
* @brief destructor
*/
IIS2RTC::~IIS2RTC()
{
}



RTC::ReturnCode_t IIS2RTC::onInitialize()
{
	// Registration: InPort/OutPort/Service
	// <rtc-template block="registration">
	// Set InPort buffers
	addInPort("iisTimedPose2D", m_iisTimedPose2DIn);

	// Set OutPort buffer
	addOutPort("rtcTimedPose2D", m_rtcTimedPose2DOut);

	// Set service provider to Ports

	// Set service consumers to Ports

	// Set CORBA Service Ports

	// </rtc-template>

	std::cout << "IIS::TimedPose2D to RTC::TimedPose2D" << std::endl;

	return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t IIS2RTC::onFinalize()
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t IIS2RTC::onStartup(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t IIS2RTC::onShutdown(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t IIS2RTC::onActivated(RTC::UniqueId ec_id)
{
	return RTC::RTC_OK;
}


RTC::ReturnCode_t IIS2RTC::onDeactivated(RTC::UniqueId ec_id)
{
	return RTC::RTC_OK;
}


RTC::ReturnCode_t IIS2RTC::onExecute(RTC::UniqueId ec_id)
{
	if(m_iisTimedPose2DIn.isNew()) {
		m_iisTimedPose2DIn.read();
		m_rtcTimedPose2D.data.heading = m_iisTimedPose2D.data.heading;
		m_rtcTimedPose2D.data.position.x = m_iisTimedPose2D.data.position.x;
		m_rtcTimedPose2D.data.position.y = m_iisTimedPose2D.data.position.y;
		m_rtcTimedPose2D.tm = m_iisTimedPose2D.tm;

		m_rtcTimedPose2DOut.write();
	}

	return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t IIS2RTC::onAborting(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t IIS2RTC::onError(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t IIS2RTC::onReset(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t IIS2RTC::onStateUpdate(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t IIS2RTC::onRateChanged(RTC::UniqueId ec_id)
{
return RTC::RTC_OK;
}
*/



extern "C"
{

	void IIS2RTCInit(RTC::Manager* manager)
	{
		coil::Properties profile(iis2rtc_spec);
		manager->registerFactory(profile,
			RTC::Create<IIS2RTC>,
			RTC::Delete<IIS2RTC>);
	}

};


