#include "VcUpdateSignal.hh"

#include "OpenOSObject.hh"

#include "Allocator/AllocatedArea.hh"

VcUpdateSignal::VcUpdateSignal()
{
}

VcUpdateSignal::~VcUpdateSignal()
{
}


Trigger* VcUpdateSignal::getInstance()
{
	static Trigger ThisTrigger;
	
	return &ThisTrigger;
}


void VcUpdateSignal::initialize( bool IsMaster, Allocator *ParentAllocator )
{	
	if ( getInstance()->IsInitialized() == true )
		throw DoubleInitializationException(LOCATION);

	try 
	{
		getInstance()->initialize( ParentAllocator, IsMaster );
	}
	catch ( ASAAC_Exception& e )
	{
		e.addPath("Error initializing VcUpdateSignal", LOCATION);
		
		deinitialize();
		
		throw;
	}
}

void VcUpdateSignal::deinitialize()
{
	if ( getInstance()->IsInitialized() == false )
		return;
	
	try
	{
		getInstance()->deinitialize();
	}
	catch ( ASAAC_Exception &e )
	{
		e.addPath("Error deinitializing VcUpdateSignal", LOCATION);
		e.raiseError();
	}
}

size_t VcUpdateSignal::predictSize()
{
	return Trigger::predictSize();	
}
