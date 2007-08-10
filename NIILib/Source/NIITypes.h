/// @file nii_types.h
/// @brief Contains the declaration of types, enums, structs and defines for use in NII module

// Created:   21.08.2004
// Authors:   S.Stratbuecker, N. Paluch
// Copyright (c)  ESG GmbH 2007.

#ifndef NII_TYPES_H
#define NII_TYPES_H

#include <pthread.h>

#include "ASAAC.h"
#include "NIIConstants.h"


/////////////////////////////////////////////
/// Additional Data for TC configuration. Especially buffer for streaming transfer connections

typedef struct 
{
    long* data_length;
    char* stream_buffer;
} TransferConfigurationData;


/////////////////////////////////////////////
/// Internally used struct to associate a Network with an interface

typedef struct 
{
  char                              valid;        ///< Valid flag
  
  ASAAC_NetworkDescriptor           id;           ///< Network Identifier
  ASAAC_InterfaceConfigurationData  config_data;  ///< Additional configuration data
  unsigned long                     open_tcs;     ///< Number of open Transfer Connections on Network
} NwData;


/////////////////////////////////////////////
/// Internally used struct to associate a Transfer Connection with a Network

typedef struct
{
  char                              valid;        ///< Valid flag
  
  ASAAC_PublicId                    id;           ///< TC Identifier
  ASAAC_TransferDirection           direction;    ///< Sender or Receiver TC
  ASAAC_TransferType                type;         ///< Message or Streaming TC
  TransferConfigurationData         config_data;  ///< Additional configuration data, such as streaming buffers

  int                               fd;           ///< Socket File Descriptor
  NwData*                           nw;           ///< Reference to associated Network
  
  ASAAC_Bool 						trigger_callback;
  ASAAC_PublicId                    callback_id;  ///< Identifier of Callback
  EventInfoData						event_info_data;
  
  char								has_data;     ///< Indicates if, new data is available
} TcData;

#endif
