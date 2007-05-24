/// @file nii_types.h
/// @brief Contains the declaration of types, enums, structs and defines for use in NII module

// Created:   21.08.2004
// Authors:   S.Stratbuecker
// Copyright (c)  ESG GmbH 2006.

#ifndef NII_TYPES_H
#define NII_TYPES_H

#include <pthread.h>

#include "ASAAC.h"

#include "OpenOS.hh"

typedef ASAAC_PublicId PublicId;

typedef ASAAC_Time Time;

typedef ASAAC_PublicId NetworkId; ///< Defines a multicast group or an IP address

typedef ASAAC_NiiReturnStatus NiiReturnStatus;

typedef ASAAC_NetworkDescriptor NetworkDescriptor;

typedef ASAAC_TC_ConfigurationData TC_ConfigurationData;

typedef ASAAC_InterfaceConfigurationData InterfaceConfigurationData;

struct TransferConfigurationData {
    long* data_length;
    char* stream_buffer;
};

typedef ASAAC_TransferDirection TransferDirection;

typedef ASAAC_TransferType TransferType;

typedef PublicId Port;

typedef ASAAC_Length Length;

typedef unsigned long Position;

typedef unsigned long Index; ///< Array Index

const Length STREAM_BUFFER_SIZE = 65536; ///< Defines size of buffer in TransferConfigurationData for streaming TCs

const Index MAX_TC_DATA = 16;  ///< Maximal number of established transfer connections

const Index MAX_NW_DATA = 16;   ///< Maximal number of established networks

const Index NO_RECEIVE_BUFFER = 32;    ///< Maximal number of receiver buffers

const unsigned long NII_RECEIVE_BUFFER_SIZE = TC_MAX_MESSAGE_SIZE + 8; ///< Size of each receive data buffers including possible header information



/////////////////////////////////////////////
/// Defines possible Network Interfaces to configure

enum
{
  NII_IF_ETHERNET = 8023
};


/////////////////////////////////////////////
/// Additional Data for TC configuration. Especially buffer for streaming transfer connections



/////////////////////////////////////////////
/// Internally used struct to associate a Network with an interface

struct NwData 
{
  int                         valid;        ///< Valid flag
  NetworkDescriptor           id;           ///< Network Identifier
  PublicId                    interface;    ///< Interface Identifier
  InterfaceConfigurationData  config_data;  ///< Additional configuration data
  int                         open_tcs;     ///< Number of open Transfer Connections on Network
};

/////////////////////////////////////////////
/// Internally used struct to associate a Transfer Connection with a Network

struct TcData
{
  int                         valid;        ///< Valid flag
  PublicId                    id;           ///< TC Identifier
  int                         fd;           ///< Socket File Descriptor
  NwData*                             nw;           ///< Reference to associated Network
  TransferDirection                direction;    ///< Sender or Receiver TC
  TransferType                       type;         ///< Message or Streaming TC
  TransferConfigurationData  config_data;  ///< Additional configuration data, such as streaming buffers
  PublicId                    callback_id;  ///< Identifier of Callback
  pthread_t                   th;           ///< Thread handle whenever streaming is used
};

#endif
