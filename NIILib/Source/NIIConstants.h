#ifndef NIICONSTANTS_H_
#define NIICONSTANTS_H_


#define NII_MAX_SIZE_OF_INTERFACECONFIGURATION		     64

#define NII_MAX_SIZE_OF_TCCONFIGURATION				     64

#define NII_MAX_SIZE_OF_TCMESSAGE		 			    (8*1024) 
//payload without header information

#define NII_MAX_LEN_OF_NWPORTSTATUS			 		     32

#define NII_MAX_SIZE_OF_STREAMBUFFER				     65536 
// Defines size of buffer in TransferConfigurationData for streaming TCs

#define NII_MAX_NUMBER_OF_TC_CONNECTIONS				 16  
// Maximal number of established transfer connections

#define NII_MAX_NUMBER_OF_NETWORKS						 16   
// Maximal number of established networks

#define NII_MAX_NUMBER_OF_RECEIVEBUFFERS				 32    
// Maximal number of receiver buffers

#define NII_MAX_SIZE_OF_RECEIVEBUFFER 			        (NII_MAX_SIZE_OF_TCMESSAGE + 8)
// Size of each receive data buffers including possible header information

#endif /*NIICONSTANTS_H_*/
