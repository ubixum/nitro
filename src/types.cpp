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


#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include <nitro/types.h>
#include <nitro/error.h>
#include <nitro/node.h>
#include <nitro/device.h>


namespace Nitro {



template<typename T>
std::string set_str(const T& val) {
    std::stringstream io;
    std::string str;
    io << val;    
    return io.rdbuf()->str();
}

DataType::DataType(int32 val) : m_type(INT_DATA), m_int(val), m_node(NULL), m_list(NULL), m_dev(NULL){
    m_str=set_str(val);
}
DataType::DataType(uint32 val) : m_type(UINT_DATA), m_uint(val), m_node(NULL), m_list(NULL), m_dev(NULL){
    m_str=set_str(val);
}

DataType::DataType(double val) : m_type(FLOAT_DATA), m_float(val), m_node(NULL), m_list(NULL), m_dev(NULL) {
    m_str=set_str(val);
}

DataType::DataType(const char* value): m_type(STR_DATA), m_str(value), m_node(NULL), m_list(NULL), m_dev(NULL) {}
DataType::DataType(const std::string& val): m_type(STR_DATA), m_str(val), m_node(NULL), m_list(NULL), m_dev(NULL) {}

DataType::DataType(const NodeRef& node): m_type(NODE_DATA), m_node(new NodeRef()), m_list(NULL), m_dev(NULL) {
    *m_node = node;
    m_str=set_str ( *m_node );
}

DataType::DataType(const std::vector<DataType>& list) : m_type(LIST_DATA), m_node(NULL), m_list(new std::vector<DataType>), m_dev(NULL) {
   *m_list = list; 
   m_str = set_str ( *m_list );
}

DataType::DataType(Device& dev) : m_type(DEV_DATA), m_node(NULL), m_list(NULL), m_dev(&dev) {
  m_str = "Nitro::Device"; 
}

DataType DataType::as_datatype(uint8* buf, uint32 len) {
	DataType b(len);
	b.m_buf = buf;
	b.m_type = BUF_DATA;
	return b;
}

uint8* DataType::as_buffer(const DataType &d){
	if (d.m_type != BUF_DATA) {
		throw Exception ( INVALID_TYPE );
	}
	return d.m_buf;
}

DataType DataType::as_bigint_datatype ( const std::vector<DataType> &ints, const std::string str ) {
    DataType bi(0);
    bi.m_list = new std::vector<DataType>(ints);
    bi.m_type = BIGINT_DATA;
    bi.m_str = str;

    return bi;
}

std::vector<DataType> DataType::as_bigints ( const DataType& d ) {
    if (d.m_type != BIGINT_DATA) {
        throw Exception ( INVALID_TYPE );
    }
    return *d.m_list;
}


void DataType::copy(const DataType& other) {
	m_type=other.m_type;
	m_str=other.m_str;
    if (other.m_type == FLOAT_DATA)
        m_float=other.m_float;
    else
	    m_int=other.m_int; // m_uint the same   

    if (m_node) delete m_node;
    if (other.m_node) {
        m_node = new NodeRef ( *other.m_node );
    } else {
        m_node = NULL;
    }

    if (m_list) delete m_list;
    if (other.m_list) {
        m_list = new std::vector<DataType> ( *other.m_list );
    } else {
        m_list = NULL;
    }
    m_dev = other.m_dev;
	m_buf = other.m_buf;
}

DataType::DataType(const DataType &other) : m_node(NULL), m_list(NULL) {
	copy(other);
}

DataType::~DataType() throw() {
    if (m_node) delete m_node;
    if (m_list) delete m_list;
    // not currently managing m_dev memory
}


DataType& DataType::operator=(const DataType& other) {
	copy(other);
	return *this;	
}

const std::string& DataType::str_value() const {
    return m_str;
}

const std::string& DataType::str_type() const {
    const static std::string int_str="int";
    const static std::string uint_str="uint";
    const static std::string str_str="str";
    const static std::string node_str="node";
    const static std::string list_str="list";
    const static std::string dev_str="device";
    const static std::string buf_str="buffer";
    const static std::string bi_str="bigint";
    const static std::string float_str="float";
    switch(m_type) {
        case INT_DATA:
            return int_str;
        case UINT_DATA:
            return uint_str;
        case STR_DATA:
            return str_str;
        case NODE_DATA:
            return node_str;
        case LIST_DATA:
            return list_str;
        case DEV_DATA:
            return dev_str;
        case BUF_DATA:
            return buf_str;
        case BIGINT_DATA:
            return bi_str;
        case FLOAT_DATA:
            return float_str;
        default:
            throw Exception(INVALID_TYPE);
    }
}

DATA_TYPE DataType::get_type() const {
    return m_type;
}

DataType::operator uint32() const {
    return m_type == FLOAT_DATA ? (uint32) m_float : m_uint;
}
DataType::operator int32() const {
    return m_type == FLOAT_DATA ? (int32) m_float : m_int;
}

DataType::operator double() const {
    return m_type == INT_DATA ? (double) m_int :
           m_type == UINT_DATA ? (double) m_uint :
           m_float;
}

DataType::operator std::string() const {
    return m_str;
} 

DataType::operator NodeRef() const {
    if (NODE_DATA != m_type) throw Exception ( INVALID_CAST, "Unsupported Cast to NodeRef" );
    return *m_node;
}


DataType::operator std::vector<DataType> () const {
    if (LIST_DATA != m_type) throw Exception ( INVALID_CAST, "Unsupported cast to list." );
    return *m_list;
}

DataType::operator Device& () const {
    if (DEV_DATA != m_type) throw Exception ( INVALID_CAST, "Unsupported cast to device." );
    return *m_dev;
}

bool DataType::operator==(const DataType& other) const {
    switch ( m_type ) {
        // can compare int/uint
        case INT_DATA:
        case UINT_DATA:
            return m_int==other.m_int;
        case FLOAT_DATA:
            return m_float == other.m_float;
        case STR_DATA:
            return other.m_type==STR_DATA && m_str == other.m_str;
        case NODE_DATA:
            if (other.m_type != NODE_DATA) return false; // eliminates NULL for other node 
            return *m_node == *other.m_node;
        case LIST_DATA:
            if (other.m_type != LIST_DATA) return false;
            return *m_list == *other.m_list;
        default:
            throw Exception ( INVALID_TYPE, "Unsupported Data Type" );
    }
}
bool DataType::operator!=(const DataType& other) const {
    return !this->operator==(other);
}



std::ostream& operator << ( std::ostream& stream, const DataType& data ) {

    stream << data.str_value();
    return stream;
    
}

std::ostream& operator << ( std::ostream& out , const std::vector<DataType> &list ) {
   if ( list.size() > 0 ) {
       std::copy ( list.begin(), list.end()-1, std::ostream_iterator<DataType> ( out, ", " ) );
       out << list.back();
   } 
   return out;
}


} // end namespace
