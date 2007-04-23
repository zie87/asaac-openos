#ifndef SIMPLECOMMANDINTERFACE_HH_
#define SIMPLECOMMANDINTERFACE_HH_

#include "OpenOSIncludes.hh"

#include "Common/Shared.hh"

#include "IPC/Semaphore.hh"
#include "IPC/Event.hh"


const unsigned long TERMINATE_HANDLER	= 0xffffffff;
const unsigned long INVALID_HANDLER		= 0xfffffffe;


typedef char CommandBuffer[ OS_SIZE_OF_SIMPLE_COMMANDBUFFER ];

typedef void(*CommandHandler)( CommandBuffer );

//! simple inter-process command interface by shared memory handshakes
/*! the SimpleCommandInterface makes use of a common shared memory block
 *  and synchronization objects to provide a simple interface that strictly
 *  works in the lines of REQUEST -> REPLY, this way forming a primitive interface
 *  for remote procedure calls.
 * 
 * It is employed in the command connection between GSM and applications, to convey
 * the fundamental configuration fonctionality, and can be expanded to provide additional
 * commands, too, such as those required for error handling services.
 */

class SimpleCommandInterface
{
public:
	//! default constructor
	SimpleCommandInterface();
	
	//! destructor
	virtual ~SimpleCommandInterface();
	
	
	//! explicit initialization
	void initialize( Allocator* ThisAllocator , bool IsMaster, bool IsServer );
	/*!< \param[in] ThisAllocator Reference to the Allocator that shall be used to 
	 *                            store all data and control structures of the SCI
	 * 
	 *   \param[in] IsMaster      Flag indicating whether this instance shall be
	 *                            responsible for the initialization and deinitialization
	 *                            of the synchronization objects required for communicaiton
	 * 
	 *   \param[in] IsServer      Flag indicating whether this instance shall be server, providing
	 *                            functions via this interface, or client, employing these functions.
	 *                            (Note: This only has implications on the execution of registered
	 *                            handlers on the server side, which can bypass the communication
	 *                            mechanisms and call the handler directly).
	 */
	
	//! explicit deinitialization
	void deinitialize();

	//! execute a command via the SCI	
	ASAAC_TimedReturnStatus sendCommand( unsigned long CommandIdentifier, CommandBuffer Buffer, const ASAAC_Time& Timeout = TimeInfinity, bool Cancelable = false );
	/*!< this function tries to process a request via the SCI. To this end, it claims control over the
	 *   SCI resources (or waits for them to become available, if necessary), and writes the data
	 *   provided by the caller into the communications buffer, then notifies the server of
	 *   the request. The server uses the supplied data to process the request with an installed
	 *   handler, which overwrites the communications buffer with the return data. The server then
	 *   signals to the client that a reply is available. The client copies the reply data back into the
	 *   caller-supplied buffer, and returns with an appropriate return code.
	 * 
	 *   \param[in] CommandIdentifier Value indicating the server operation to be requested.
	 *   \param[in] Buffer            Memory area storing the calling parameters, and returning the
	 *                                return values to the caller.
	 *   \param[in] Timeout           ASAAC_Time to wait for server reply before returning ASAAC_TM_TIMEOUT
	 * 
	 *   \returns                     ASAAC_TM_SUCCESS on successful transmission of the request and reception of a reply.
	 *                                This does not imply that the requested operation was performed without error,
	 *                                this kind of information shall be provided in the CommandBuffer.
	 *                                ASAAC_TM_TIMEOUT if the request could not be sent and/or the reply could not
	 *                                be received within the specified time frame.
	 *                                ASAAC_TM_ERROR if an error occurred during the communication, or
	 *                                if the respective CommandIdentifier did not match any installed handler
	 *                                on the server side.
	 * 
	 */
	
	
	//! install a new command handler in the SCI server
	ASAAC_ReturnStatus addCommandHandler( unsigned long CommandIdentifier, CommandHandler Handler );
	/*!< this function sets up a new handler function for the SCI server, to handle calls identified by the
	 *   CommandIdentifier as supplied by the caller. If a handler for the respective identifier already exists,
	 *   the function returns ASAAC_ERROR.
	 * 
	 * \param[in] CommandIdentifier identifier which to install a new handler for
	 * \param[in] Handler           reference to the function responsible for handling requests of the indicated
	 *                              identifier
	 * 
	 * \returns ASAAC_SUCCESS on successful operation. ASAAC_ERROR, if the command identifier is already assigned to a handler,
	 */
	
	//! remove a command handler from the SCI server
	ASAAC_ReturnStatus removeCommandHandler( unsigned long CommandIdentifier );
	/*!< remove the previously installed command handler from the list of registered handlers. If
	 *   no handler exists for the indicated CommandIdentifier, the function will return ASAAC_ERROR
	 * 
	 * \param[in] CommandIdentifier	Identifier which to remove the command handler for
	 * \returns   ASAAC_SUCCESS on successful operation. ASAAC_ERROR, if there is no handler installed for the
	 *            indicated command identifier.
	 */

	//! cause the server to handle a single SCI command
	ASAAC_ReturnStatus handleOneCommand( unsigned long& CommandIdentifier );
	/*!< wait for a single incoming command over the SCI. If a command handler is installed for the
	 *   received request, execute it and return the return values to the caller. Finally, return
	 *   the received CommandIdentifier.
	 * 
	 * \param[out] CommandIdentifier Command Identifier of the received request
	 * \returns    ASAAC_SUCCESS if a command was received and a handler was installed.
	 *             ASAAC_ERROR   if no handler was installed for the received command.
	 */


	//! dummy handler function, doing nothing
	static void	  EmptyHandler( CommandBuffer Buffer );


	//! start a dedicated thread for the handling of incoming requests
	void startHandlerThread();
	/*!< the started thread will handle incoming requests until it receives the
	 *   request TERMINATE_HANDLER
	 */
	
	
	//! stop the dedicated request-handling thread 	
	void stopHandlerThread();
	/*!< sends the TERMINAL_HANDLER request to the handler thread,
	 *   causing it to shut down.
	 */

	//! predict the amount of memory for control and data structures to be allocated via an allocator
	static size_t predictSize();
	
private:
	//! entry point for the dedicated handler thread
	static void* HandlerThread( void* Param );
	
	//! handler for the abortion of the dedicated handler thread
	static void  TerminationHandler( CommandBuffer Buffer );
	
	static ASAAC_CharacterSequence getCommandString( unsigned long CommandIdentifier );
	
	struct CommandHandlerMapping {
		
		unsigned long Identifier;
		CommandHandler  Handler;
		
	};
	
	struct CommandData {
		unsigned long Identifier;
		CommandBuffer	Buffer;
	};

	//! internal function for finding the corresponding handler of a given CommandIdentifier
	CommandHandlerMapping* getCommandHandler( unsigned long CommandIdentifier );

	bool					m_IsMaster;
	bool					m_IsServer;
	bool					m_HandlerThreadRunning;

	bool					m_IsInitialized;
		
	Semaphore				m_CommandSemaphore;
	Semaphore				m_AdministrationSemaphore;
	
	Event					m_SendReceiveEvent;
	
	Shared<CommandData>		m_CommandData;
	
	CommandHandlerMapping	m_Handler[ OS_MAX_COMMAND_HANDLERS ];
	
	oal_thread_t			m_HandlerThread;

};

#endif /*SIMPLECOMMANDINTERFACE_HH_*/
