/// @file NIITypes.h
/// @brief Contains the declaration of types, enums, structs and defines for use in NII module

// Created:   21.08.2004
// Authors:   S.Stratbuecker, N. Paluch
// Copyright (c)  ESG GmbH 2007.

#ifndef NII_TYPES_H
#define NII_TYPES_H

#include "ASAAC.h"
#include "NIIConstants.h"


/////////////////////////////////////////////
/// send or received Tc Data


typedef struct
{
	//Probably a header for data handling must be defined
} NetworkHeader;

typedef struct
{
	ASAAC_PublicId					tc_id;
} TcHeader;


typedef char TcData[NII_MAX_SIZE_OF_BUFFER];

typedef struct  
{
	NetworkHeader					network_header;
	TcHeader    	                tc_header;
	TcData		                    data;
} TcPacket;

typedef struct  
{
	TcPacket    	                packet;
	unsigned long                   data_length;
} TcPacketData;


/////////////////////////////////////////////
/// Internally used struct to associate a Network with an interface

typedef struct 
{
  ASAAC_PublicId                    port;                ///< Network Identifier
  ASAAC_InterfaceConfigurationData  configuration_data;  ///< Additional configuration data

  int                               fd;                  ///< Socket File Descriptor
  char								is_streaming;
  ASAAC_PublicId					tc_id_with_data;
} Network;


/////////////////////////////////////////////
/// Internally used struct to associate a Transfer Connection with a Network

typedef struct
{
  ASAAC_PublicId                    tc_id;        ///< TC Identifier
  ASAAC_NetworkDescriptor           network_id;   ///< Reference to associated Network
  ASAAC_TransferDirection           direction;    ///< Sender or Receiver TC
  ASAAC_TransferType                type;         ///< Message or Streaming TC

  ASAAC_Bool 						trigger_callback;
  ASAAC_PublicId                    callback_id;  ///< Identifier of Callback
  EventInfoData						event_info_data;
  
  ASAAC_PublicId                    buffer_id;
} TransferConnection;

#endif
