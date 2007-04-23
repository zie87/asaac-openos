#ifndef CALLBACK_HH_
#define CALLBACK_HH_

//!< callback interface

/*!< The Callback interface provides a facility to handle events
 * by calling a pre-initialized class inheriting this interface,
 * which will carry the instructions and data required to handle
 * said event
 */

class Callback {
	
public:
		virtual void call( void* Data ) = 0;
		//!< callback handler
		/*!< \param[in] Data  ASAAC_Address of data block to be handed over to the callback handler.
		 *                    The size and nature of data at the indicated address depends on
		 *                    the actual use of the function.
		 */
		
		virtual ~Callback() {};

};


#endif /*CALLBACK_HH_*/
