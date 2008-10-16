#ifndef TIMEMANAGER_HH_
#define TIMEMANAGER_HH_

#include "OpenOSIncludes.hh"

class TimeManager
{
protected:
	// we do not want objects of this class to be instantiated.
	// this class only serves to gather time management related
	// functions at this time.
	TimeManager();
	
public:
	virtual ~TimeManager();
	
	static void getAbsoluteLocalTime( ASAAC_Time& absolute_local_time );
	static void getAbsoluteGlobalTime( ASAAC_Time& absolute_global_time );
	static void getRelativeLocalTime( ASAAC_Time& relative_local_time );

};

#endif /*TIMEManager_HH_*/
