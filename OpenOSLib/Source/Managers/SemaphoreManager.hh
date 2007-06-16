#ifndef SEMAPHOREMANAGER_HH_
#define SEMAPHOREMANAGER_HH_

#include "OpenOSIncludes.hh"

#include "IPC/Semaphore.hh"

#include "Allocator/LocalMemory.hh"
#include "Allocator/AllocatedArea.hh"

class SemaphoreManager
{
public:
	static SemaphoreManager* getInstance();
	
	void initialize();
	void deinitialize();
	
	ASAAC_ResourceReturnStatus createSemaphore( const ASAAC_CharacterSequence& Name,
										  ASAAC_PrivateId& SemaphoreId,
										  unsigned long InitialValue,
										  unsigned long MaximumValue,
										  ASAAC_QueuingDiscipline Discipline );
										  
	ASAAC_ReturnStatus deleteSemaphore( ASAAC_PrivateId SemaphoreId );
	
	ASAAC_TimedReturnStatus waitForSemaphore( ASAAC_PrivateId SemaphoreId,
									    const ASAAC_Time& Timeout );
									    
	ASAAC_ReturnStatus postSemaphore( ASAAC_PrivateId SemaphoreId );
	
	ASAAC_ReturnStatus getSemaphoreStatus( ASAAC_PrivateId SemaphoreId,
									 unsigned long& CurrentValue,
									 unsigned long& WaitingCallers );
									 
	ASAAC_ReturnStatus getSemaphoreId( const ASAAC_CharacterSequence& Name, ASAAC_PrivateId& SemaphoreId );

	virtual ~SemaphoreManager();
	
protected:
	SemaphoreManager();
	
	long findSemaphoreByName( const ASAAC_CharacterSequence& SemaphoreName );
	long findSemaphoreByPrivateId( const ASAAC_PrivateId SemaphoreId );

	typedef struct {
		
		ASAAC_PrivateId          SemaphoreId;
		ASAAC_CharacterSequence  SemaphoreName;
		Semaphore*         SemaphoreObject;
		AllocatedArea      SemaphoreAllocator;
		ASAAC_QueuingDiscipline SemaphoreDiscipline;
		
	} SemaphoreData;

	LocalMemory		m_GlobalAllocator;

	bool			m_IsInitialized;
	Semaphore		m_AccessSemaphore;
	
	unsigned long	m_NumberOfSemaphores;
	SemaphoreData	m_Semaphores[ OS_MAX_NUMBER_OF_SEMAPHORES ];
	
};


#endif /*SEMAPHOREMANAGER_HH_*/
