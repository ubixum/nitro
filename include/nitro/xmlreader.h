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


#ifndef XMLREADER_H
#define XMLREADER_H


#include "types.h"
#include "reader.h"

namespace Nitro {

class DLL_API Node;
/**
 * \ingroup devif
 * \brief Construct device interface from xml.
 **/
class DLL_API XmlReader : public Reader {
   private:
        struct impl;
        impl* m_impl;
   public:
        /**
         * \param filepath Path to the xml file to read
         * \param validate Turn on schema validation.
         **/
        XmlReader ( const std::string& filepath, bool validate=false);
        ~XmlReader() throw();

        void read(NodeRef node);


};


} // end namespace

#endif
