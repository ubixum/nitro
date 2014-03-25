// Copyright (C) 2009 Ubixum, Inc. 
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef NITRO_DEVICE_H
#define NITRO_DEVICE_H

#include <map>

#include "types.h"
#include "error.h"


/**
 *  \defgroup devimpl Implementing Devices
 *
 *  To implement a new device, one must extend Nitro::Device and
 *  implement a set of core virtual functions.  _read, _write,  are called
 *  from the Device base class.  Classes extending Device should not 
 *  worry about thread safety.  The Nitro:Device::get etc lock an instance
 *  specific Mutex before making the call to implementing device.
 **/

/**
 * \defgroup dataac Data Acquisition
 *
 * The Nitrogen data acquisition system provides an abstraction layer for reading and
 * writing data to and from actual hardware.  Nitro::Device provides the basic interface
 * for communication.  Each Nitro::Device is an abstract class with virtual methods 
 * for actual device communication that must be implemented by a derived class.
 *
 * Nitro::Device is based on the concept of \e terminals and \e registers.  A terminal
 * represents a physical device, or part of a device, that is controlled by this Nitro::Device.
 * A terminal has any number of registers, which represent the different capabilites 
 * each terminal has.  A terminal may have 0 or more register addressed.  Terminals and
 * registers are identified by unique integer addresses.  The Nitro::Device API can be
 * used with raw integer addresses, but can also be configured to identify terminals and registers
 * by name.  The \ref devif "Device Interface" documentations explains this process further.
 *
 * Nitro::Device is used to get/set and/or read/write from register addresses.  The \ref hello.cpp "Hello" 
 * example provides a quick overview of how a device might be opened and used for a few
 * basie I/O operations.
 * 
 **/


namespace Nitro {


class DLL_API NodeRef;
// FUTURE typedef std::map<std::string,DataType> DataDict;

/**
 * \brief Base class for Nitrogen data acquisition.
 * \ingroup dataac
 * 
 * Device is the base class for any Nitrogen data acquisition device.  See \ref devimpl "implementing devices" 
 * for instructions on extending and implementing your own devices.
 *
 * The device class is thread-safe and wraps get/set and read/write calls with a mutex.
 *
 * If a device interface definition is loaded, this device implementation will use the loaded definitions
 * to translate string names of terminals and registers to integer addresses. \see Nitro::Node.
 **/
class DLL_API Device {
private:
    struct impl;
    impl* m_impl;
protected:
    /**
     *  \ingroup devimpl
     *  
     **/
    virtual void _read( uint32 terminal_addr, uint32 reg_addr, uint8* data, size_t length, uint32 timeout )=0 ;
    /**
     *  \ingroup devimpl
     *  
     **/
    virtual void _write( uint32 terminal_addr, uint32 reg_addr, const uint8* data, size_t length, uint32 timeout ) =0 ;
    /**
     *  \ingroup devimpl
     *  Device will not call _close() explicitly when it is deleted.  Implementing devices are free to 
     *  make this optimization however.
     **/
    virtual void _close()=0;

    /**
     * \ingroup devimpl
     * Optional ability to return a status code after a data transfer.
     * If implementing this method, return 0 for success and any other status
     * for error. Device only calls this method if DEV_MODE::VERIFY_STATUS=1
     **/
    virtual int _transfer_status() { return 0; };

    /**
     * \ingroup devimpl
     *
     * Optionally return a checksum of the last transfer.  This is for the
     * CHECKSUM_VERIFY mode.  CHECKSUM_VERIFY can't be enabled unless
     * this function is overridden properly.  The checksum is computed
     * by adding each 16 bit value and ignoring the carry.
     **/
    virtual uint16 _transfer_checksum() { return 0; }
public:

    /**
     * \brief modes for enable/disable_mode
     **/
    enum DEV_MODE {
        GETSET_VERIFY=1<<0, ///< After every set, performs a get and verifies the results match
        DOUBLEGET_VERIFY=1<<1, ///< Performs every get twice and verifies the result is the same for each get.
        STATUS_VERIFY=1<<2, ///< After any get/set/read/write, checks the transfer_status function for a successful transfer.
        CHECKSUM_VERIFY=1<<3, ///< After any get/set/read/write, check that the transfer_checksum is correct for the data.
        LOG_IO=1<<4, ///< Log get/set/read/wrote to stdout
        RETRY_ON_FAILURE=1<<31 ///< If a mode check failes and this is set, the transfer will be attempted again.
    };

    /**
     * \brief Retry Callback function.
     *
     * Function Class for Device::set_retry_func 
     * Having this as a class allows the user to save additional info across muliple
     * retry calls.  User is in charge of memory management for this class.  (User should
     * allocate and de-allocate it)
     *
     * 
     * */
    class RetryFunc {
     public:
         virtual ~RetryFunc(){}
         /**
          * \brief Implement this function in your callback.
          *
          * \param dev A reference to the device that had the error.
          * \param term_addr Terminal address
          * \param reg_addr Register address
          * \param count Number of retries already performed.
          * \param exc The Exception that was thrown on the last attempt.
          **/
         virtual bool operator()(Device& dev, uint32 term_addr, uint32 reg_addr, uint32 count, const Exception &exc )=0;
    };


    /**
     * Device is a pure virtual class and cannot be instantiated directly.
     **/
    Device();
    virtual ~Device() throw();

    /**
     * \ingroup devif
     * \brief Reference to the device interface tree.  If no device interface tree
     * has been set with set_di, This returns a new empty node.
     **/
    NodeRef get_di() const;

    /**
     * \ingroup devif
     * \brief Set the device interface definition for this Device to use.  
     *
     * A common usage:
     * \code
     *  USBDevice usbd;
     *  // initialize
     *  XmlReader reader("devif.xml");
     *  reader.read(usbd.get_di());
     * \endcode
     * 
     * \param di The new device interface tree to use.
     **/
    void set_di(const NodeRef& di);


    /**
     * \brief enable a mode or modes
     *
     * Multiple modes can be set by combining DEV_MODEs with the OR operator
     **/
    void enable_mode(uint32 modes);

    /**
     * \brief disable a mode or modes
     *
     * \see enable_mode
     **/
    void disable_mode(uint32 modes);

    /**
     * \brief get modes
     *
     * retreive the current modes
     **/
    uint32 get_modes() const ;

    /**
     * \brief enable mode for specific terminal only
     *
     * \see enable_mode
     * Enable mode(s) for a specific terminal.  Global settings for modes
     * override specific terminal settings.
     **/
    void enable_mode(const DataType &term, uint32 modes);

    /**
     * \brief disable modes for a specific terminal
     *
     * \see enable_mode
     **/
    void disable_mode(const DataType& term, uint32 modes);

    /**
     * \brief get modes for a specific terminal
     **/
    uint32 get_modes(const DataType &term ) const;

    /**
     * \brief set modes.
     *
     * Clears all modes and sets new modes accordingly.
     **/
    void set_modes(uint32 modes); 

    /**
     * \brief set modes for a specific terminal
     * \see set_modes(uint32)
     **/
    void set_modes(const DataType& term, uint32 modes); 


    /**
     * \brief Enable RETRY_ON_FAILURE function
     *
     * During RETRY_ON_FAILURE logic, the normal behavior is to retry once.  If
     * different logic is desired, one can create a retry callback and enable
     * it by passing it to this function.  Retry logic can be changed at any
     * time.  This function is thread safe with get/set/read/write.  Retry
     * logic can be disabled by passing NULL to this function. (But you still
     * have to disable RETRY_ON_FAILURE if you don't want the default logic to
     * be executed.)
     **/
    void set_retry_func( RetryFunc *func );

    /**
     * \ingroup devif
     * \brief Shortcut method to retrieve a node for a specific terminal.
     * If name is a string, this call is equivalent to:
     * \code get_di().get_child(name); \endcode
     * This function can also handle integer addresses.
     * \param name String terminal name or integer address for terminal to retreive.
     * \throw Nitro::Exception if the terminal does not exist in the device interface.
     **/
    NodeRef get_terminal ( const DataType& name ) const;

    /**
     * \ingroup devif
     * \brief Shortcut method to retrieve a register.  Same rules apply for register name
     * as in \ref get_terminal.  If terminal and reg are of string type, this
     * method is equivalent to: \code get_di().get_child(term).get_child(reg); \endcode
     * \param term String terminal name or integer address for terminal.
     * \param reg String register name or integer address for register.
     * \throw Nitro::Exception if the terminal or register does not exist in the device interface.
     **/
    NodeRef get_register ( const DataType& term , const DataType& reg ) const;

    /**
     * \ingroup dataac
     * \brief Lock the device mutex.
     *
     * Calls to get/set and read/write are already protected by the internal device mutex. 
     * Sometimes however, you may wish to make a serious of calls to the device before another
     * thread is allowed to access the device.  Call this method to obtain a lock on the
     * device for the current threads.  This call can be made repeatedly in the same thread
     * as long as unlock is called for each lock call.
     **/
    void lock ();

    /**
     * \ingroup dataac
     * \brief Unlock the device mutex.
     *
     * unlock must be called for each lock call.
     **/
    void unlock();

    /** \ingroup dataac
     * \brief Set the default timeout for get/set and read/write calls.
     * 
     * If calls to IO methods use a timeout value of -1, then the default timeout will be 
     * used for the underlying IO call.  Any timeout >= 0 passed to the IO methods overrides
     * the default timeout.
     *
     * \param timeout Timeout in milliseconds.  Use 0 for no timeout.
     *
     **/
    void set_timeout ( uint32 timeout );

    /**
     * \ingroup dataac
     *
     * \brief Get the current default timeout for IO calls.
     *
     **/
    uint32 get_timeout ();

    /**
     * \ingroup dataac
     * \brief Thread-safe set.
     * \param term Terminal name or address.
     * \param reg Register name or address.      
     * \param value Value to set to the register.
     * \param data_width Used for hint on number of bytes to set when set is called without
     *                   a device interface.  If data_width==0 and term/reg are integer data.
     *                   The default 2 byte width will be used. If a device interface is present
     *                   the width from the register or terminal is used.
     * \param timeout Timeout for call in milliseconds. -1 = use the default timeout.  0 = no timeout.  
     * \throw Nitro::Exception 
     **/
    void set ( const DataType& term, const DataType& reg, const DataType& value, int32 timeout=-1, uint32 data_width=0 );


    /*
     *  \param dict Dictionary of values.  When a dictionary is used.  The register being set
     *              must have subregisters.  Each key in the dictionary is the subregister name.
     *              Each value is the value for that specific subregister.
     **/

    // NOTE future
    // void set ( const DataType& term, const DataType& reg, const DataDict& dict, uint32 timeout );

    /**
     * \ingroup dataac
     * \brief Thread-safe get.
     * \param term String or integer address for terminal.
     * \param reg String or integer address for register.
     * \param data_width Hint for number of bytes to get if term/reg are integer data.
     *                   Leaving the default value of 0 will assume a 2 byte get or will
     *                   use the width from the device interface.
     * \param timeout Timeout in milliseconds. -1 = use the default timeout.  0 = no timeout.
     * \return Nitro::DataType currently, only integer data types are returned.
     * \throw Nitro::Exception on communication error.
     **/
    DataType get( const DataType& term, const DataType& reg, int32 timeout=-1, uint32 data_width=0 ) ;

    /**
     * \ingroup dataac
     * \brief Get subregister values
     * 
     * This method calls get, but returns the values of the register as a dictionary of 
     * subregister values.  There must be a device interface set to use this or an exception 
     * will be thrown
     *
     * \param term String or integer address for terminal.
     * \param reg String or integer address for register.
     * \param timeout Timeout in milliseconds.  -1=use the default timeout. 0 = no timeout.
     * \return Nitro::NodeRef  The attributes of the node are the subregister names/values.
     * \throw Nitro::Exception on communication or invalid device interface registers.
     **/
    NodeRef get_subregs ( const DataType& term, const DataType& reg , int32 timeout=-1 );

    /**
     * \ingroup dataac
     * \brief Thread-safe read.
     * \param term String or integer address for terminal.
     * \param reg String or integer address for register.
     * \param data raw data buffer for data to be stored in.
     * \param length size of data buffer.
     * \param timeout Timeout in milliseconds. -1 = use the default timeout.  0 = no timeout.
     * \throw Nitro::Exception on communication error.
     **/
    void read(const DataType& term, const DataType& reg, uint8* data, size_t length, int32 timeout=-1) ;
     /**
     * \ingroup dataac
     * \brief Thread-safe write.
     * \param term String or integer address for terminal.
     * \param reg String or integer address for register.
     * \param data raw data buffer for data to be stored in.
     * \param length size of data buffer.
     * \param timeout Timeout in milliseconds. -1 = use the default timeout.  0 = no timeout.
     * \throw Nitro::Exception on communication error.
     **/
    void write(const DataType& term, const DataType& reg, const uint8* data, size_t length, int32 timeout=-1) ;

    /**
     * \brief Exclicitly close a device.
     **/
    void close();

};

} // end namespace

#endif
