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


#pragma once


#include <nitro.h>

#include "types.h"

using namespace System;
using namespace System::Collections;

namespace Nitro {
namespace NET {

	public ref class Node : public Object {

		private:
			Nitro::NodeRef* m_node;
		public:
			Node( Nitro::NodeRef& node );
			
			Node ( String^ name );
			Node ( const Node^ copy );
			~Node() { this->!Node(); }
			!Node();

			String^ GetName() { return gcnew String((*m_node)->get_name().c_str()); }


			Node^ GetChild(const String^ name);
			void AddChild( const Node^ child);

			bool HasChildren() { return (*m_node)->has_children(); }
			ArrayList^ ChildrenNames();

			const Nitro::NET::DataType^ GetAttr(const String^ name);
			void SetAttr(const String^ name, const DataType^ value );
			bool HasAttr(const String^ name) ;
            ArrayList^ AttrNames();

			static explicit operator Nitro::NodeRef ( const Node^ node ) { return *(node->m_node); }
            static explicit operator Node^ ( Nitro::NodeRef node ) { return gcnew Node(node ); }
	};



}} // end namespaces
