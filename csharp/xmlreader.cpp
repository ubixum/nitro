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



#include "xmlreader.h"
#include "util.h"
#include "error.h"

namespace Nitro {
namespace NET {

	XmlReader::XmlReader(const String^ path) {
		std::string p=to_string(path);
		m_reader=new Nitro::XmlReader(p);
	}
	void XmlReader::Read(Node ^node) {
		try {			
			m_reader->read( (Nitro::NodeRef)node );			
		} catch ( Nitro::Exception& e ) {
			throw static_cast<NitroException^>(e);
		}
	}
}}
