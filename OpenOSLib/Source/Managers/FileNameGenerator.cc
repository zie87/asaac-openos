#include "FileNameGenerator.hh"

#include "OpenOSObject.hh"
#include "ProcessManagement/ProcessManager.hh"
#include "Communication/CommunicationManager.hh"


FileNameGenerator::FileNameGenerator()
{
}

FileNameGenerator::~FileNameGenerator()
{
}


//const char * subdir = "OpenOS/";
const char * subdir = "";


ASAAC_CharacterSequence FileNameGenerator::getGlobalVcName( ASAAC_PublicId GlobalVcId )
{
	long Index = CommunicationManager::getInstance()->getVirtualChannelIndex(GlobalVcId);
	
	if (Index == -1)
		throw OSException("GlobalVcId not found", LOCATION);
	
	CharacterSequence cs;
	
	cs << subdir << "GlobalVc_" << CharSeq(Index, signed_decimal);
	
	return cs.asaac_str();	
}


ASAAC_CharacterSequence FileNameGenerator::getOpenOSName()
{
	CharacterSequence cs;
	
	cs << subdir << "OpenOS";
	
	return cs.asaac_str();	
}


ASAAC_CharacterSequence FileNameGenerator::getProcessName( ASAAC_PublicId ProcessId )
{
	long Index = ProcessManager::getInstance()->getProcessIndex(ProcessId);

	if (Index == -1)
		throw OSException("ProcessId not found", LOCATION);

	CharacterSequence cs;
	
	cs << subdir << "Process_" << CharSeq(Index, signed_decimal);

	return cs.asaac_str();	
}


ASAAC_CharacterSequence FileNameGenerator::getProcessManagerName( ASAAC_PublicId CpuId )
{
	long Index = OpenOS::getInstance()->getCpuIndex(CpuId);

	if (Index == -1)
		throw OSException("CpuId not found", LOCATION);

	CharacterSequence cs;
	
	cs << subdir << "ProcessManager_" << CharSeq(Index, signed_decimal);
	
	return cs.asaac_str();	
}


ASAAC_CharacterSequence FileNameGenerator::getCommunicationManagerName()
{
	CharacterSequence cs;
	
	cs << subdir << "CommunicationManager";
	
	return cs.asaac_str();	
}


ASAAC_CharacterSequence FileNameGenerator::getRemoteInterfaceName( ASAAC_PublicId SlotId )
{
	CharacterSequence cs;
	
	cs << subdir << "RemoteInterface_" << CharSeq(SlotId);
	
	return cs.asaac_str();	
}

ASAAC_CharacterSequence FileNameGenerator::getLocalOliPath( ASAAC_PublicId ProcessId )
{
	CharacterSequence cs;
	
#ifdef _ELINOS_4_1_
	cs = "/bin";
#else
	cs = getenv( "ASAAC_OS" );
#endif
	
	cs += "/";	
	cs += OS_TEMP;	
	cs += "OliFile_";
	cs += CharSeq(ProcessId);
	
	return cs.asaac_str();
}

ASAAC_CharacterSequence FileNameGenerator::getAsaacPath( ASAAC_CharacterSequence path )
{

	CharacterSequence cs;
	
#ifdef _ELINOS_4_1_
	cs = "/bin";
#else
	cs = getenv( "ASAAC_OS" );
#endif
	
	cs += "/";
	cs += path;
    
	return cs.asaac_str();
}

ASAAC_CharacterSequence FileNameGenerator::getCurrentPath( ASAAC_CharacterSequence path )
{
	//TODO: implement this function
	
    ASAAC_CharacterSequence cs;
    return cs;
}
