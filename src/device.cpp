/**
 * Copyright (C) 2009 Ubixum, Inc. 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/

#ifndef WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif

#include <bitset>
#include <string>
#include "stdio.h"
#include <map>
#include <memory>
#include <algorithm>

#ifdef DEBUG_DEV
#include <iostream>
#define dev_debug(x) cout << x << " (" << __FILE__ << ':' << __LINE__ << ')' << endl;
#else
#define dev_debug(x)
#endif

#ifdef DEBUG_MUTEX
#include <iostream>
#define dev_mutex(x) cout << x << " (" << __FILE__ << ':' << __LINE__ << ')' << endl;
#else
#define dev_mutex(x)
#endif

#include <nitro/device.h>
#include <nitro/error.h>
#include <nitro/node.h>


#include "hr_time.h"

using namespace std;


namespace Nitro {

#ifdef WIN32

class Mutex {
private:
    HANDLE m_mutex;
public:
    Mutex() {
		m_mutex = CreateMutex(0,FALSE,0);
		if (!m_mutex) {
            throw Exception ( DEVICE_MUTEX, "Failed to create mutex for device." );
        }
    }
    ~Mutex() {
        CloseHandle(m_mutex);
    }
    void lock() {
        WaitForSingleObject(m_mutex,INFINITE);
    }
    void unlock() {
        ReleaseMutex(m_mutex);
    }
};

#else

class Mutex {

    private:
        pthread_mutex_t m_mutex;
        pthread_mutexattr_t m_attrs;
        int m_lock_cnt;
    public:
        Mutex() : m_lock_cnt(0) {
            
            if(pthread_mutexattr_init(&m_attrs)) {
                throw Exception ( DEVICE_MUTEX, "Failed to create mutex attrs for device." );  
            }

            if(pthread_mutexattr_settype(&m_attrs, PTHREAD_MUTEX_RECURSIVE)){
                throw Exception ( DEVICE_MUTEX, "Failed to set mutex type." );
            }

            if(pthread_mutex_init(&m_mutex, &m_attrs)) {
                throw Exception ( DEVICE_MUTEX, "Failed to create mutex for device." );
            }
        }
        ~Mutex() {
            pthread_mutex_destroy(&m_mutex);
            pthread_mutexattr_destroy(&m_attrs);
        }
        void lock() {
            pthread_mutex_lock(&m_mutex);
            ++m_lock_cnt;
            dev_mutex ( "Lock Count: " << m_lock_cnt << " " << (uint64)this );
        }
        void unlock() {
            --m_lock_cnt;
            pthread_mutex_unlock(&m_mutex);
        }

};

#endif


class MutexLock {
    private:
        Mutex& m_mutex;
    public:
        MutexLock(Mutex& mutex) : m_mutex(mutex) { 
            m_mutex.lock(); 
            dev_mutex ( "Locked" );
            } 
        ~MutexLock() { 
            dev_mutex ( "Unlocking." );
            m_mutex.unlock(); 
            }
};

/**
 * Helper classes for get/set
 **/

struct AddressData {
        enum ADDR_TYPE {
            RAW=1,
            SINGLE,
            ARRAY,
            SUBREG
        };
        ADDR_TYPE type;
        vector<uint32> addrs; 
        NodeRef term_node; ///< only valid if type is != RAW
        NodeRef reg_node; ///< same
        NodeRef subreg_node; ///< only valid if type == SUBREG
};


struct Device::impl{
    uint32 m_timeout;
    Mutex m_mutex;
    NodeRef di;
    uint32 m_modes;
    map<uint32,uint32> m_term_modes;

    impl(): m_timeout(1000), di(DeviceInterface::create("di")), m_modes(STATUS_VERIFY)  {}

    uint32 get_timeout(int32 timeout);
    uint32 term_addr( const DataType& term );
    NodeRef find_name_or_addr( NodeRef& parent, const DataType& find);
    uint32 reg_addr( const DataType& term, const DataType& reg );
    DataType valmap_or_const_val ( NodeRef node, const DataType& val );
    auto_ptr<AddressData> resolve_addrs ( const DataType& term, const DataType& reg ) ;
    void get_set_subreg ( bitset<1024> &bits, uint32 term_addr, uint32 reg_addr, bitset<1024> &value, uint32 offset, uint32 width, uint32 dwidth, vector<uint32> &clean_regs, int32 timeout , Device &dev);
    DataType do_get(Device &dev, uint32 term_addr, uint32 reg_addr, AddressData &a, int32 timeout, bool retry_ok=true); 
    void do_set(Device &dev, uint32 term_addr, uint32 reg_addr, DataType &value, AddressData &a, int32 timeout, bool retry_ok=true);
    void do_read(Device &dev, uint32 term_addr, uint32 reg_addr, uint8* data, size_t length, int32 timeout, bool retry_ok=true);
    void do_write(Device &dev, uint32 term_addr, uint32 reg_addr, const uint8* data, size_t length, int32 timeout, bool retry_ok=true);
    private:
    DataType raw_get( Device &dev, uint32 term_addr, uint32 reg_addr, AddressData &a, uint32 timeout ); // only called by do_get
    void raw_set ( Device &dev, uint32 term_addr, uint32 reg_addr, DataType &value, AddressData &a, uint32 timeout );
    void raw_write(Device &dev, uint32 term_addr, uint32 reg_addr, const uint8* data, size_t length, uint32 timeout);
    void raw_read(Device &dev, uint32 term_addr, uint32 reg_addr, uint8* data, size_t length, uint32 timeout);
    void check_status(Device &dev, uint32 term_addr);
    void check_checksum(Device &dev, uint32 term_addr, const uint8* data, size_t length );
};

uint32 Device::impl::get_timeout ( int32 timeout ) {
    return timeout < 0 ? m_timeout : timeout ;
}

uint32 Device::impl::term_addr(const DataType& term ) {
    if (STR_DATA != term.get_type()) return term;
    return find_name_or_addr( di, term)->get_attr("addr");
}
uint32 Device::impl::reg_addr( const DataType& term, const DataType& reg ) {
    if (STR_DATA != reg.get_type()) return reg;

    NodeRef pterm = find_name_or_addr( di, term );
    return find_name_or_addr( pterm, reg )->get_attr("addr"); 
}

NodeRef Device::impl::find_name_or_addr(NodeRef& parent, const DataType& find) {
    if (STR_DATA==find.get_type()) return parent->get_child(find);
    for ( DITreeIter itr=parent->child_begin();
          itr != parent->child_end();
          ++itr ) {
        if ((*itr)->get_attr("addr") == find ) return *itr;
    }
    throw Exception ( NODE_NOT_FOUND );
}

DataType Device::impl::valmap_or_const_val ( NodeRef node, const DataType& val ) {
   if ( val.get_type() != STR_DATA ) return val; // should cast as a uint32

   dev_debug ( "Attempt to find value from valuemap" << val );
   NodeRef valuemap = node->get_attr ( "valuemap" );
   return valuemap->get_attr(val); 
}

void Device::impl::check_status(Device &dev, uint32 term_addr) {
    if ( m_term_modes[term_addr] & STATUS_VERIFY  ||
        m_modes & STATUS_VERIFY ) {
        int status = dev._transfer_status();
        if (status) {
            throw Exception ( DEVICE_OP_ERROR, "Status error after get", status );
        } 
   }
}

void Device::impl::check_checksum(Device& dev, uint32 term_addr, const uint8* data, size_t length ) {
    if (m_term_modes[term_addr] & CHECKSUM_VERIFY ||
        m_modes & CHECKSUM_VERIFY) {
        uint16 checksum=0;
        dev_debug ( "Calculate checksum on data of length " << length );
        for ( uint32 i=0; i<length/2 ; ++i ) 
            checksum += *(uint16*)(data+i*2);
        if (length&1)
            checksum += data[length-1];

        if (checksum != dev._transfer_checksum()) {
            dev_debug ( "Checksum mismatch " << checksum << " expected: " << dev._transfer_checksum() );
            NodeRef err = Node::create("checksum error");
            err->set_attr("device checksum", dev._transfer_checksum());
            err->set_attr("calculated checksum", checksum );
            throw Exception ( DEVICE_OP_ERROR, "Checksum mismatch.", err );
        }

        dev_debug ( "Checksum ok: " << checksum );
    }
}

DataType Device::impl::raw_get ( Device &dev, uint32 term_addr, uint32 reg_addr, AddressData &a, uint32 timeout ) {
 
   DataType res = dev._get ( term_addr, reg_addr, timeout );
   check_status(dev,term_addr);
   if (res.get_type() == UINT_DATA ) { 
       uint32 val = res;
       check_checksum(dev,term_addr, (uint8*)&val, sizeof(val) );
   }

   if ( (m_term_modes[term_addr] & DOUBLEGET_VERIFY ||
         m_modes & DOUBLEGET_VERIFY) && 
        (a.type == AddressData::RAW || 
         a.reg_node->get_attr("mode") == "write" ) ) { 
         DataType res2 = dev._get ( term_addr, reg_addr, get_timeout ( timeout ) );
         if ( res != res2 ) { 
            NodeRef exc_info = Node::create("exc_info");
            exc_info->set_attr("term",term_addr);
            exc_info->set_attr("reg",reg_addr);
            exc_info->set_attr("1stval",res);
            exc_info->set_attr("2ndval",res2);
            throw Exception ( DEVICE_OP_ERROR, "Double Get Verify Failed", exc_info );
         }
    } 

    return res;
}

DataType Device::impl::do_get (Device &dev, uint32 term_addr, uint32 reg_addr, AddressData &a, int32 timeout, bool retry_ok) {

   try {
       return raw_get ( dev, term_addr, reg_addr, a, get_timeout(timeout) );
   } catch ( const Exception &e ) {
       if ( e.code() == DEVICE_OP_ERROR && (m_term_modes[term_addr] & RETRY_ON_FAILURE || 
            m_modes & RETRY_ON_FAILURE ) && retry_ok ) {
            dev_debug ( "do_get::Mode Failure " << term_addr << ", " << reg_addr << " - Retrying" );
            return do_get ( dev, term_addr, reg_addr, a, timeout, false ); // retry
       } else {
            throw e; 
       }
   } 
   
}

void Device::impl::raw_set( Device &dev, uint32 term_addr, uint32 reg_addr, DataType &value, AddressData &a, uint32 timeout ) {
    dev._set ( term_addr, reg_addr, value, timeout );
    check_status(dev,term_addr);
   if (value.get_type() == UINT_DATA ) { 
       uint32 val = value;
       check_checksum(dev,term_addr, (uint8*)&val, sizeof(val) );
   }
    if (
        (m_term_modes[term_addr] & GETSET_VERIFY || 
         m_modes & GETSET_VERIFY) && 
        (a.type == AddressData::RAW || 
         (a.reg_node->get_attr("type") != string("trigger") &&
          a.reg_node->get_attr("mode") == string("write" ))
        )
       ) {
         DataType v = raw_get ( dev, term_addr, reg_addr, a, timeout); 
         if (v != value ) {
            NodeRef exc_info = Node::create("exc_info");
            exc_info->set_attr("term",term_addr);
            exc_info->set_attr("reg",reg_addr);
            exc_info->set_attr("set_val",value);
            exc_info->set_attr("get_val",v);
            throw Exception ( DEVICE_OP_ERROR, "GETSET_VERIFY failed" , exc_info );
         }
    }

}

void Device::impl::do_set (Device &dev, uint32 term_addr, uint32 reg_addr, DataType &value, AddressData &a, int32 timeout, bool retry_ok) {

   try {
       raw_set ( dev, term_addr, reg_addr, value, a, get_timeout(timeout) );
   } catch ( const Exception &e ) {
       if ( e.code() == DEVICE_OP_ERROR && (m_term_modes[term_addr] & RETRY_ON_FAILURE || 
            m_modes & RETRY_ON_FAILURE ) && retry_ok ) {
            dev_debug ( "do_set::Mode Failure " << term_addr << ", " << reg_addr << " - Retrying" );
            do_set ( dev, term_addr, reg_addr, value, a, timeout, false ); // retry
       } else {
            throw e; 
       }
   } 
   
}

void Device::impl::raw_read(Device &dev, uint32 term_addr, uint32 reg_addr, uint8* data, size_t length, uint32 timeout ) {
   dev._read ( term_addr, reg_addr, data, length, timeout ); 
   check_status ( dev, term_addr );
   check_checksum ( dev, term_addr, data, length );
}

void Device::impl::do_read(Device &dev, uint32 term_addr, uint32 reg_addr, uint8* data, size_t length, int32 timeout, bool retry_ok) {
   try {
       raw_read ( dev, term_addr, reg_addr, data, length, get_timeout(timeout) );
   } catch ( const Exception &e ) {
       if ( e.code() == DEVICE_OP_ERROR && (m_term_modes[term_addr] & RETRY_ON_FAILURE || 
            m_modes & RETRY_ON_FAILURE ) && retry_ok ) {
            do_read ( dev, term_addr, reg_addr, data, length, timeout, false ); // retry
       } else {
            throw e; 
       }
   } 
}

void Device::impl::raw_write (Device &dev, uint32 term_addr, uint32 reg_addr, const uint8* data, size_t length, uint32 timeout ) {
   dev._write ( term_addr, reg_addr, data, length, timeout ); 
   check_status ( dev, term_addr );
   check_checksum ( dev, term_addr, data, length );
}

void Device::impl::do_write(Device &dev, uint32 term_addr, uint32 reg_addr, const uint8* data, size_t length, int32 timeout, bool retry_ok) {
   try {
       raw_write ( dev, term_addr, reg_addr, data, length, get_timeout(timeout) );
   } catch ( const Exception &e ) {
       if ( e.code() == DEVICE_OP_ERROR && (m_term_modes[term_addr] & RETRY_ON_FAILURE || 
            m_modes & RETRY_ON_FAILURE ) && retry_ok ) {
            do_write ( dev, term_addr, reg_addr, data, length, timeout, false ); // retry
       } else {
            throw e; 
       }
   } 
}

auto_ptr<AddressData> Device::impl::resolve_addrs ( const DataType& term, const DataType& reg ) {

    auto_ptr<AddressData> addrs ( new AddressData() ); 

    if ( STR_DATA != reg.get_type() ) {
        addrs->type = AddressData::RAW;
        addrs->addrs.push_back ( reg );
        return addrs;
    }
    
    // resolve str data 

    NodeRef term_node = find_name_or_addr ( di, term ); 
    uint32 term_data_width = term_node->get_attr("regDataWidth");
    NodeRef reg_node;

    // . = subregister
    // [ = array
    string reg_name = reg;
    size_t parse_idx=0;
    if ( (parse_idx = reg_name.find( "." )) != string::npos ) {
        // . 
        string subreg = reg_name.substr ( parse_idx+1 );
        reg_name = reg_name.substr( 0, parse_idx );
        reg_node = find_name_or_addr ( term_node, reg_name );
        NodeRef sub_node = reg_node->get_child(subreg);
         
        // depending on subreg width, could span multiple addresses

        uint32 addr = reg_node->get_attr("addr");
        uint32 offset = sub_node->get_attr("addr");
        addr += offset / term_data_width; // skip the number of registers before this subregister starts
        offset = offset % term_data_width;

        int32 width = sub_node->get_attr("width");
        do {
            // how many bits fit in this address
            uint32 cur_bits = term_data_width - offset;
            addrs->addrs.push_back(addr++);
            width -= cur_bits; 
            offset = 0;
        } while ( width > 0 );

        addrs->type = AddressData::SUBREG;
        addrs->subreg_node = sub_node;

    } else if ( (parse_idx = reg_name.find ( "[" ) ) != string::npos ) {
        // [
        size_t end = reg_name.find ( "]" );
        if ( end == string::npos || end < parse_idx+2 || end != reg_name.length()-1 ) throw Exception ( DEVICE_PARSE_ERROR, "Failed to parse array index.", reg_name ); 
        string idx = reg_name.substr ( parse_idx+1, end );
        reg_name = reg_name.substr ( 0, parse_idx ) ;
        reg_node = find_name_or_addr ( term_node, reg_name );

        
        uint32 nidx = 0;
        if ( sscanf ( idx.c_str(), "%u", &nidx ) != 1) {
            throw Exception ( DEVICE_PARSE_ERROR, "Failed to parse array index", idx );
        }
        
        if ( nidx > ((uint32) reg_node->get_attr("array")) - 1 ) {
            throw Exception ( DEVICE_PARSE_ERROR, "Invalid array index", nidx );
        }

        uint32 width = reg_node->get_attr("width");
        // width in registers, not bits
        uint32 item_width = (width / term_data_width) + 
                           (width % term_data_width > 0 ? 1 : 0);
        uint32 addr = reg_node->get_attr("addr");
        addr += nidx * item_width;
        addrs->type = AddressData::SINGLE;

        do {
            addrs->addrs.push_back(addr++);
        } while ( --item_width );
        
    } else {
        reg_node = find_name_or_addr ( term_node, reg_name );

        if ( (uint32)reg_node->get_attr("array") > 1) {
            addrs->type=AddressData::ARRAY;
            uint32 addr = reg_node->get_attr("addr");
            uint32 array_len = reg_node->get_attr("array");
            int32 width = reg_node->get_attr("width");  
            do {
               int32 awidth = width; 
               do {
                 addrs->addrs.push_back(addr++);
                 awidth -= term_data_width; 
               } while ( awidth > 0 );
            } while ( --array_len > 0 );
        } else {
            // logic for a subreg is the same is if it's a register that spans multiple gets
            uint32 addr = reg_node->get_attr("addr");
            int32 width = reg_node->get_attr("width");
            addrs->type = AddressData::SINGLE; 
            do {
                addrs->addrs.push_back(addr++);
                width -= term_data_width;
            } while ( width > 0 );
        }
        
    }
    
    addrs->term_node = term_node;
    addrs->reg_node = reg_node;
    return addrs;
}

Device::Device() {
   m_impl=new impl(); 
}

Device::~Device() throw() {
    //close();
    // NOTE can't call virtual function in destructor...
    delete m_impl;
}


void Device::set_di( const NodeRef& node ) {
   MutexLock thread_safe_method(m_impl->m_mutex);
   m_impl->di = node; 
}
NodeRef Device::get_di( ) const {
    return m_impl->di;
}
NodeRef Device::get_terminal ( const DataType& name ) const {
    return m_impl->find_name_or_addr( m_impl->di, name );
}
NodeRef Device::get_register ( const DataType& term , const DataType& reg ) const {
    NodeRef t = m_impl->find_name_or_addr(m_impl->di, term);
    return m_impl->find_name_or_addr ( t , reg );
}

void Device::set_timeout ( uint32 timeout ) {
    MutexLock thread_safe_method(m_impl->m_mutex);
    m_impl->m_timeout=timeout;
}

uint32 Device::get_timeout () {
    MutexLock thread_safe_method(m_impl->m_mutex);
    return m_impl->m_timeout;
}

bitset<1024> to_bitset(const DataType& dt) {
    bitset<1024> res;
    if (dt.get_type() == BIGINT_DATA) {
        vector<DataType> ints = DataType::as_bigints(dt);
        while (ints.size()) {
            res <<= 32; 
            res |= (uint32) ints.back();
            ints.pop_back();
        }        
   } else {
    res |= (uint32) dt;
   }
   return res;
}

/**
 * split value into ints of width
 **/
void to_vector ( DataType &val, vector<DataType> &vals, uint32 n, uint32 width ) {
    bitset<1024> bits = to_bitset(val);
    bitset<1024> mask;
    for (uint32 i=0;i<width;++i) mask.set(i);
    do {
       vals.push_back ( (uint32)(bits & mask).to_ulong() ); 
       bits >>= width;
    } while (--n);
}


uint32 lsint ( bitset<1024> &bits ) {
   string resstr = bits.to_string().substr( 1024-32 );
   bitset<32> smaller ( resstr );
   uint32 res= smaller.to_ulong();
   return res;
}

DataType from_bitset(bitset<1024> &bits ) {

    // if dt can fit in an int, return it that way..
    string resstr = bits.to_string();
    size_t f = resstr.find ( "1" );
    if (f == string::npos) return 0;
    if (1024-f<=32) return lsint(bits);


    vector<DataType> ints;
    bitset<1024> mask = 0xffffffff;    
    do {
        uint32 lsd = lsint(bits);
        ints.push_back(lsd);
        bits >>= 32;
    } while (bits.any());
    return DataType::as_bigint_datatype(ints);
}

void Device::lock() {
    m_impl->m_mutex.lock();
}
void Device::unlock() {
    m_impl->m_mutex.unlock();
}

void Device::enable_mode(uint32 modes) {
    MutexLock thread_safe_method (m_impl->m_mutex);
    m_impl->m_modes |= modes;
}
void Device::set_modes(uint32 modes) {
    MutexLock thread_safe_method (m_impl->m_mutex);
    m_impl->m_modes = modes;
}


void Device::disable_mode(uint32 modes) {
    MutexLock thread_safe_method (m_impl->m_mutex);
    m_impl->m_modes &= ~modes;
}
uint32 Device::get_modes() const {
    MutexLock thread_safe_method(m_impl->m_mutex);
    return m_impl->m_modes;
}

void Device::enable_mode(const DataType &term, uint32 modes) {
    MutexLock thread_safe_method (m_impl->m_mutex);
    uint32 addr = m_impl->term_addr(term);
    m_impl->m_term_modes[addr] |= modes; 
}
void Device::set_modes(const DataType &term, uint32 modes) {
    MutexLock thread_safe_method (m_impl->m_mutex);
    uint32 addr = m_impl->term_addr(term);
    m_impl->m_term_modes[addr] = modes; 
}
void Device::disable_mode(const DataType &term, uint32 modes) {
    MutexLock thread_safe_method (m_impl->m_mutex);
    uint32 addr = m_impl->term_addr(term);
    m_impl->m_term_modes[addr] &= ~modes;
}
uint32 Device::get_modes(const DataType &term) const {
    MutexLock thread_safe_method(m_impl->m_mutex);
    return m_impl->m_term_modes[m_impl->term_addr(term)];
}


/**
 * get orig register if part of register is dirty. 
 **/
void Device::impl::get_set_subreg ( bitset<1024> &bits, uint32 term_addr, uint32 reg_addr, bitset<1024> &value, uint32 offset, uint32 width, uint32 dwidth, vector<uint32> &clean_regs, int32 timeout, Device &dev) {
    uint32 subreg_start = reg_addr + (offset / dwidth);
    uint32 subreg_end = subreg_start;
    uint32 subreg_offset = offset % dwidth;
    int32 subreg_width = (int32) width; 
    do {
        ++subreg_end;
        subreg_width -= dwidth - subreg_offset;
        subreg_offset=0;
    } while ( subreg_width > 0 );

    for (uint32 addr = subreg_start; addr < subreg_end; ++addr ) { 
        if ( count ( clean_regs.begin(), clean_regs.end(), addr ) == 0 ) {
            bitset<1024> dirty_reg ( (uint32) dev.get ( term_addr, addr, timeout ) );
            dirty_reg <<= (addr-subreg_start + offset/dwidth) * dwidth;
            bits |= dirty_reg;
            clean_regs.push_back(addr);
        }
    }

    bitset<1024> mask;
    for (uint32 i=0;i<width;++i) mask.set(i);
    mask <<= offset;
    bits &= ~mask;
    value <<= offset;
    bits |= value ;

}


void Device::set ( const DataType& term, const DataType& reg, const DataType& value, int32 timeout ) {
    MutexLock thread_safe_method(m_impl->m_mutex);

    dev_debug ( "Set: " << term << " " << reg << ": " << value );

    auto_ptr<AddressData> addrs = m_impl->resolve_addrs( term, reg );

    vector<DataType> set_vals;
    // val setters
    vector<uint32> dirty_regs; // for dict/subreg
    // if a reg addr is in the clean_regs, it means we set data in that reg.
    switch ( addrs->type ) {
        case AddressData::RAW:
            set_vals.push_back(value);
            break;
        case AddressData::ARRAY:
            {
                if ( LIST_DATA != value.get_type()) throw Exception ( DEVICE_OP_ERROR, "Expected array data for setting array register.", value );
                vector<DataType> val_array = value; 
                uint32 asize = addrs->reg_node->get_attr("array");
                if (val_array.size() != asize ) throw Exception ( DEVICE_OP_ERROR, "Array data must be same length as register array." ); 

                uint32 dwidth = addrs->term_node->get_attr("regDataWidth");
                uint32 awidth = addrs->reg_node->get_attr("width");
                uint32 regs = (awidth / dwidth ) +
                              (awidth % dwidth > 0 ? 1 : 0);
                for (vector<DataType>::iterator itr = val_array.begin();
                     itr != val_array.end(); ++itr ) {
                     DataType val = m_impl->valmap_or_const_val( addrs->reg_node , *itr);
                     to_vector ( val, set_vals, regs, dwidth );
                }
            }
            break;
        case AddressData::SINGLE:
            {
                uint32 dwidth = addrs->term_node->get_attr("regDataWidth");
                
                if ( NODE_DATA == value.get_type() ) {


                    bitset<1024> new_value;
                    NodeRef val_map = value;
                    uint32 term_addr = addrs->term_node->get_attr("addr");
                    uint32 reg_addr = addrs->reg_node->get_attr("addr");
                    for ( DIAttrIter itr = val_map->attrs_begin();
                          itr != val_map->attrs_end();
                          ++itr ) {
                          NodeRef subreg = addrs->reg_node->get_child ( itr->first );
                          uint32 offset = subreg->get_attr("addr");
                          uint32 width = subreg->get_attr("width");

                          bitset<1024> set_bits = to_bitset(m_impl->valmap_or_const_val ( subreg, itr->second ) );
                          m_impl->get_set_subreg ( 
                            new_value,
                            term_addr,
                            reg_addr,
                            set_bits,
                            offset,
                            width,
                            dwidth,
                            dirty_regs,
                            timeout, *this );
                    }
                    DataType set_val = from_bitset(new_value);
                    to_vector ( set_val, set_vals, addrs->addrs.size(), dwidth ); 
                } else {
                    DataType tmp = m_impl->valmap_or_const_val( addrs->reg_node, value );
                    to_vector ( tmp, set_vals, addrs->addrs.size(), dwidth );
                }
            }
            break;
        case AddressData::SUBREG:
            {
                    
                uint32 dwidth = addrs->term_node->get_attr("regDataWidth");
                uint32 offset = addrs->subreg_node->get_attr("addr");

                bitset<1024> vals;

                bitset<1024> set_bits = to_bitset( m_impl->valmap_or_const_val ( addrs->subreg_node , value ) );
                m_impl->get_set_subreg ( 
                    vals,
                    (uint32) addrs->term_node->get_attr("addr"),
                    (uint32) addrs->reg_node->get_attr("addr"),
                    set_bits,
                    offset,
                    (uint32) addrs->subreg_node->get_attr("width"),
                    dwidth,
                    dirty_regs,
                    timeout,
                    *this);

                vals >>= (offset / dwidth) * dwidth;
                DataType set_val = from_bitset(vals);
                to_vector ( set_val, set_vals, addrs->addrs.size(), dwidth);

            }
            break;
    };
    
    if ( set_vals.size() != addrs->addrs.size() ) throw Exception ( DEVICE_OP_ERROR , "Internal Logic Error" );

    for (uint32 i=0;i<set_vals.size();++i) {
        DataType set = set_vals.at(i);
        uint32 addr = addrs->addrs.at(i);

        if ( addrs->type == AddressData::SUBREG || 
             (addrs->type == AddressData::SINGLE && value.get_type()==NODE_DATA) ) {
             // in this case, some of the reg addrs may not be dirty 
             if ( count ( dirty_regs.begin(), dirty_regs.end(), addr ) == 0 ) {
                continue; // in this case, the register was not fetched for a set, therefore, no need to reset 
                // (and there isn't data to reset anyway)
             }
        }


        uint32 term_addr = addrs->type == AddressData::RAW ? m_impl->term_addr(term) : (uint32) addrs->term_node->get_attr("addr");
        m_impl->do_set ( *this, term_addr, addr, set, *addrs, timeout); 
    }
}



/**
 * Basic Get 
 **/
DataType Device::get( const DataType& term, const DataType& reg, int32 timeout  ) {
    MutexLock thread_safe_method(m_impl->m_mutex);

    auto_ptr<AddressData> addrs = m_impl->resolve_addrs( term, reg );

    vector<DataType> results;
    for ( vector<uint32>::iterator itr = addrs->addrs.begin();
          itr != addrs->addrs.end(); ++itr ) {
          uint32 addr = *itr;
          uint32 term_addr = addrs->type == AddressData::RAW ? m_impl->term_addr(term) : (uint32) addrs->term_node->get_attr("addr");
          DataType res = m_impl->do_get ( *this, term_addr, addr, *addrs, timeout ); 
          results.push_back(res);
    }

    // val builder
    switch ( addrs->type ) {
        case AddressData::RAW:
            return results.front(); // only one in this case
        case AddressData::SINGLE:
            {
                uint32 width = addrs->term_node->get_attr("regDataWidth");
                bitset<1024> bits;
                while (results.size()) {
                    bits <<= width;
                    bits |= (uint32) results.back();
                    results.pop_back();
                }
                return from_bitset(bits);
            }
        case AddressData::ARRAY:
            {
                uint32 dwidth = addrs->term_node->get_attr("regDataWidth");
                uint32 awidth = addrs->reg_node->get_attr("width");
                uint32 array = addrs->reg_node->get_attr("array");
                vector<DataType> ret;
                while (array--) {
                    uint32 width = (awidth / dwidth) +
                                   (awidth % dwidth > 0 ? 1 : 0);
                    if (results.size() < width ) throw Exception ( DEVICE_OP_ERROR, "Internal logic error." );
                    vector<DataType> cur_array;
                    for (uint32 i=0;i<width;++i) {
                        cur_array.push_back(results.front());
                        results.erase(results.begin());
                    }

                    bitset<1024> bits;
                    while ( cur_array.size() ) {
                       bits <<= dwidth; 
                       bits |= (uint32)cur_array.back();
                       cur_array.pop_back();
                    }
                    ret.push_back(from_bitset(bits));
                }
                return ret;
            }
        case AddressData::SUBREG:
            {
                uint32 dwidth = addrs->term_node->get_attr("regDataWidth");
                uint32 start_addr = addrs->reg_node->get_attr("addr");
                bitset<1024> reg_data;
                while ( results.size() ) {
                    reg_data <<= dwidth;
                    reg_data |= (uint32) results.back();
                    results.pop_back();
                }

                while ( start_addr < addrs->addrs.front() ) {
                    ++start_addr;
                    reg_data <<= dwidth;
                }

                uint32 swidth = addrs->subreg_node->get_attr("width");
                uint32 soffset = addrs->subreg_node->get_attr("addr");
                reg_data >>= soffset;
                bitset<1024> mask;
                for (uint32 i=0;i<swidth;++i) mask.set(i);
                reg_data &= mask;
                return from_bitset(reg_data);
            }

    }

    throw Exception ( DEVICE_OP_ERROR, "Unhandled data type" );

}

NodeRef Device::get_subregs ( const DataType& term, const DataType& reg , int32 timeout ) {

    MutexLock thread_safe_method(m_impl->m_mutex);
    NodeRef tnode = m_impl->find_name_or_addr( m_impl->di , term );
    NodeRef rnode = m_impl->find_name_or_addr( tnode, reg ); 
    if ( !rnode->has_children()) {
        throw Exception ( DEVICE_OP_ERROR, "Register must have subregisters to use this method.", reg );
    }
    DataType ret = get ( term, reg, timeout );
    bitset<1024> data = to_bitset(ret); 
    NodeRef subreg_vals = Node::create(rnode->get_name());
    for (DITreeIter itr = rnode->child_begin(); itr != rnode->child_end(); ++itr ) {
        NodeRef subreg = *itr;
        uint32 swidth = subreg->get_attr("width");
        bitset<1024> mask;
        for (uint32 i=0;i<swidth;++i) mask.set(i);
        bitset<1024> subreg_val = data & mask; 
        data >>= swidth;
        subreg_vals->set_attr( subreg->get_name(), from_bitset(subreg_val) );
    }
    return subreg_vals;
}


/**
 * Set the register by subregister values.
 **/
void Device::read(const DataType& term, const DataType& reg, uint8* data, size_t length, int32 timeout) {    
    dev_debug ( "Read " << length << " bytes from " << term << ", " << reg );
    MutexLock thread_safe_method(m_impl->m_mutex);    
    dev_debug ( "Mem addr " << (uint64)data );
    m_impl->do_read(*this, m_impl->term_addr(term), m_impl->reg_addr(term,reg), data, length, timeout);
}
void Device::write(const DataType& term, const DataType& reg, const uint8* data, size_t length, int32 timeout) {
    dev_debug ( "Write " << length << " bytes to " << term << ", " << reg );
    MutexLock thread_safe_method(m_impl->m_mutex);    
    m_impl->do_write(*this, m_impl->term_addr(term),m_impl->reg_addr(term,reg),data,length,timeout);
};

void Device::close() {
    MutexLock thread_safe_method(m_impl->m_mutex);
    _close();
}

} // end namespace
