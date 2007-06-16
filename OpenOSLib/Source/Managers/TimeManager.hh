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
	
	static ASAAC_ReturnStatus getAbsoluteLocalTime( ASAAC_Time& absolute_local_time );
	static ASAAC_ReturnStatus getAbsoluteGlobalTime( ASAAC_Time& absolute_global_time );
	static ASAAC_ReturnStatus getRelativeLocalTime( ASAAC_Time& relative_local_time );

};

#endif /*TIMEManager_HH_*/
