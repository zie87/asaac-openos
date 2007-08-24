#ifndef FILENAMEGENERATOR_HH_
#define FILENAMEGENERATOR_HH_

#include "OpenOSIncludes.hh"

using namespace std;

class FileNameGenerator
{
public:
	static ASAAC_CharacterSequence getGlobalVcName( ASAAC_PublicId GlobalVcIdx );
	static ASAAC_CharacterSequence getOpenOSName();
	static ASAAC_CharacterSequence getProcessName( ASAAC_PublicId ProcessId );
	static ASAAC_CharacterSequence getProcessManagerName( ASAAC_PublicId CpuId );
	static ASAAC_CharacterSequence getCommunicationManagerName();
	static ASAAC_CharacterSequence getRemoteInterfaceName( ASAAC_PublicId SlotId );
	static ASAAC_CharacterSequence getLocalOliPath( ASAAC_PublicId ProcessId );
    static ASAAC_CharacterSequence getAsaacPath( ASAAC_CharacterSequence path );
    static ASAAC_CharacterSequence getCurrentPath( ASAAC_CharacterSequence path );

	virtual ~FileNameGenerator();
	

private:
	FileNameGenerator();

};

#endif /*FILENAMEGENERATOR_HH_*/
