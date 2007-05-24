#include "VcUpdateSignal.hh"

#include "OpenOS.hh"

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
	if ( getInstance()->IsInitialized() == false )
	{
		try {
			getInstance()->initialize( ParentAllocator, IsMaster );
		}
		catch ( ASAAC_Exception& e )
		{
			e.raiseError();
			throw;
		}
	}
}

void VcUpdateSignal::deinitialize()
{
	if ( getInstance()->IsInitialized() ) 
	{
		getInstance()->deinitialize();
	}
}

size_t VcUpdateSignal::predictSize()
{
	return Trigger::predictSize();	
}
