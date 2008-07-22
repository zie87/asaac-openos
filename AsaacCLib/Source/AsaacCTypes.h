/** 
 * @ingroup AsaacCLib 
 * 
 * $Revision: 2598 $
 * 
 * \author (C) Copyright 2008 ESG GmbH, Fürstenfeldbruck, Germany.
 *
 * \file  AsaacCTypes.h
 * 
 * \brief AsaacC global data types.
 * 
 * \details All data types of AsaacC are defined here. 
 * The data types in this file are implemented by AsaacC functions.   
 */

#ifndef ASAACCTYPES_H_
#define ASAACCTYPES_H_

#include <inttypes.h>

typedef float          float32_t;
typedef double         float64_t;
typedef long double    float128_t;

#include <sys/time.h>

/**
 ***********************************************************************
 * 	\brief Item for 32bit data.  
 ***********************************************************************
 */
typedef uint32_t Data32Item;

/**
 ***********************************************************************
 * 	\brief Array for 32bit data.  
 ***********************************************************************
 */
typedef Data32Item *Data32Array;

/**
 ***********************************************************************
 * 	\brief Item for 64bit keys.  
 ***********************************************************************
 */
typedef uint64_t Key64Item;

/**
 ***********************************************************************
 * 	\brief Array for 32bit key.  
 ***********************************************************************
 */
typedef Key64Item *Key64Array;

/**
 ***********************************************************************
 * 	\brief Item for a key of none predefined size.  
 ***********************************************************************
 */
typedef ASAAC_Address KeyItem;

/**
 ***********************************************************************
 * 	\brief Array for key of none predefined size.  
 ***********************************************************************
 */
typedef KeyItem KeyArray;

/**
 ***********************************************************************
 * 	\brief Item for data of none predefined size.  
 ***********************************************************************
 */
typedef ASAAC_Address DataItem;

/**
 ***********************************************************************
 * 	\brief Array for data of none predefined size.  
 ***********************************************************************
 */
typedef DataItem DataArray;

/**
 ***********************************************************************
 * 	\brief Item for adresses.  
 ***********************************************************************
 */
typedef ASAAC_Address AddressItem;

/**
 ***********************************************************************
 * 	\brief Array for adresses.  
 ***********************************************************************
 */
typedef AddressItem *AddressArray;

/**
 ***********************************************************************
 * 	\brief Item for 32bit indexes.  
 ***********************************************************************
 */
typedef uint32_t IndexItem;

/**
 ***********************************************************************
 * 	\brief Array for 32bit indexes.  
 ***********************************************************************
 */
typedef IndexItem *IndexArray;

/**
 ***********************************************************************
 * 	\brief Item for a pair of none predefined size of data and key.  
 ***********************************************************************
 */
typedef struct {
    DataItem data;
    KeyItem key;
} PairItem;

/**
 ***********************************************************************
 * 	\brief Array for pairs of none predefined size of data and key.  
 ***********************************************************************
 */
typedef PairItem *PairArray;

/**
 ***********************************************************************
 * 	\brief Item for a pair of none predefined size of data and a 64bit key.  
 ***********************************************************************
 */
typedef struct {
    DataItem data;
    Key64Item key;
} Pair64Item;

/**
 ***********************************************************************
 * 	\brief Array for pairs of none predefined size of data and a 64bit key.  
 ***********************************************************************
 */
typedef Pair64Item *Pair64Array;

/**
 ***********************************************************************
 * 	\brief Return status for comparing actions.  
 *  \details This status shall be returned by every comparing method:\n
 * - CRS_Lower: The compared parameter is lower than the comparing target.\n
 * - CRS_Equal: The compared parameter is equal to the comparing target.\n
 * - CRS_Greater: The compared parameter is greater than the comparing target.\n
 * - CRS_Undefined: The result of the comparing action is undefined.\n
 ***********************************************************************
 */
typedef enum {
    CRS_Lower,
    CRS_Equal,
    CRS_Greater,
    CRS_Undefined
} CompareReturnStatus;

/**
 ***********************************************************************
 * 	\brief Type to specify the number reprensation.  
 *  \details  The reprentation is stored in a (\ref ASAAC_CharacterSequence) format.\n
 * - NR_Binary: binary representation (i.e. 10101100111))\n
 * - NR_Octal: octal representation (i.e. o12345678)\n
 * - NR_UnsignedDecimal: unsigned decimal representation (i.e. 123456789)\n
 * - NR_SignedDecimal: signed decimal representation (i.e. +123456789, -123456789)\n
 * - NR_Hexadecimal: hexadecimal representation (i.e. 0x12345678)\n
 ***********************************************************************
 */
typedef enum {
     NR_Binary,
     NR_Octal,
     NR_UnsignedDecimal,
     NR_SignedDecimal,
     NR_Hexadecimal
} NumberRepresentation;

/**
 ***********************************************************************
 * 	\brief Type to specify a sample type.  
 *  \details The defined common sample type is used for regular expressions
 * processes processed in (\ref AsaacCharacterSequence.h).\n
 * - ST_Alnum: Alphanumeric characters [A-Za-z0-9]\n
 * - ST_Alpha: Alphabetic characters [A-Za-z]\n
 * - ST_Blank: Space and tab [ \\t]\n
 * - ST_Cntrl: Control characters [\\x00-\\x1F\\x7F]\n
 * - ST_Digit: Digits [0-9]\n
 * - ST_Graph: Visible characters [\\x21-\\x7E]\n
 * - ST_LoweR: Lowercase letters [a-z]\n
 * - ST_Print: Visible characters and spaces [\\x20-\\x7E]\n
 * - ST_Punct: Punctuation characters [-!"#$%&'()*+,./:;<=>?@[\\\\]_`{|}~]\n
 * - ST_Space: Whitespace characters [ \\t\\r\\n\\v\\f]\n
 * - ST_Upper: Uppercase letters [A-Z]\n
 * - ST_Xdigit: Hexadecimal digits [A-Fa-f0-9]\n
 ***********************************************************************
 */
typedef enum {
  ST_Alnum,
  ST_Alpha,
  ST_Blank,
  ST_Cntrl,
  ST_Digit,
  ST_Graph,
  ST_LoweR,
  ST_Print,
  ST_Punct,
  ST_Space,
  ST_Upper,
  ST_Xdigit
} RegExSampleType;


#endif /*ASAACCTYPES_H_*/
