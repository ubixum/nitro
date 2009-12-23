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


#ifndef NITRO_NODE_H
#define NITRO_NODE_H

#include <string>
#include <vector>
#include <map>
#include <iosfwd>

#include "types.h"


namespace Nitro {

class DLL_API Node;
class DLL_API NodeRef;

 
typedef std::vector<NodeRef>::const_iterator DITreeIter;
typedef std::map<std::string,DataType>::const_iterator DIAttrIter;


/**
 * \defgroup devif Device Interface
 * 
 * It is possible to communicate with devices based on string names of 
 * terminals and regisers instead of hardware addresses.  The basic
 * Nitrogen data structure that holds mappings of hardware names to 
 * addresses is the Nitro::Node.  Each node simply contains a mapping
 * of attributes to Nitro::DataType objects and optionally a number
 * of children Nodes. The basic tree structure for this device interface
 * is:
 *
 * Root %Node (Each child node represents a terminal)<br>
 * &nbsp;&nbsp;-Terminal %Node(s)<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;Attributes<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;-name: Name of terminal<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;-addr: Terminal address<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;-regDataWidth: Width of data bus<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;-regAddrWidth: Width of address bus<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;Registers<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;-name: Name of register<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;-addr: Register address<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;-mode: read or write<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;-width: Width of register data<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;-init: Optional initial value<br>
 *
 * It is possible to programmatically construct a device interface but is
 * much more commont to use a previously saved text representation and 
 * then load the device interface with a Reader object.   XmlReader is 
 * provided to load a device interface from an xml file.
 *
 * Once a device interface is associated with a device, calls to get/set
 * and read/write can be parameterized with terminal and register names
 * instead of integer addresses.  Example:
 *
 * \code
 * XmlReader reader ("devif.xml")
 * // read the xml device interface and store the results
 * // in the tree used by the device.
 * reader.read ( dev.get_tree() ); 
 * dev.set("FPGA", "reset", 0);
 * dev.get("SENSOR", "start_addr" );
 * dev.read ( "SENSOR", 0, buf, 100 );
 * \endcode
 *
 *
 * 
 **/


/**
 * \ingroup devif
 * \brief Reference Counted Node
 *
 * Tracks references to Nodes in order to allow more convenient Node API usage.  Node won't be deleted
 * until all references to the node are no longer used.
 **/
class DLL_API NodeRef {

    friend DLL_API std::ostream& operator << ( std::ostream& , const NodeRef& );

    private:
        Node* m_node;
        void dec() throw();
    public:
        /**
         * \brief Construct a temporary NodeRef.
         *
         * NodeRef does not reference a Node object.
         **/
        NodeRef ( );

        /**
         *  \brief Constructs a NodeRef that references a new node.
         *
         *  Nodes cannot be created directly. Use a Nodes create method
         *  to obtain a new NodeRef.
         **/
        NodeRef ( Node* node );

        /**
         *  \brief Construct a NodeRef based on an existing NodeRef.
         *
         *  Both NodeRefs point to the same Node.
         **/
        NodeRef ( const NodeRef& copy );
        ~NodeRef () throw();

        /**
         * \brief Determine if this NodeRef points to a valid node or NULL
         **/
        bool is_null() const;

        /**
         *  \brief Obtain access to Node methods with this operator.
         **/
        Node* operator ->() const;

        /**
         *  \brief Obtain access to the referenced Node with this operator.
         **/
        Node& operator *() const;

        /**
         * \brief The assignment operator makes a shallow copy of the Node.
         * Both objects point to the same Node.
         **/
        NodeRef& operator=( const NodeRef & );

        /**
         * \brief return true if NodeRef points to the same node object.
         **/
        bool operator == ( const NodeRef & );

};

/**
 * \brief Write a NodeRef to an output stream
 **/
DLL_API std::ostream& operator << ( std::ostream& , const NodeRef & node );


// for clone to work properly, a virtual function must call the derived
// class create method.  Otherwise the base class create is called
// and the clone node has the incorrect type.
#define NODE_CALL_CREATE virtual NodeRef call_create(const std::string &name) const { return create(name); }


/**
 * \ingroup devif
 * \brief A device interface representation is stored in Nodes.  Each node
 * can have any number of children nodes.
 *
 * Children nodes are
 * indexed by name, but are stored in the order that they are added.
 * (The Nitro::DITreeIter will iterate the children nodes in the same order
 *  that they were added to the parent node.)
 * 
 * Each node also has a number of attributes.  Common node attributes are:
 * \li name
 * \li addr
 *
 **/
class DLL_API Node {

    friend class NodeRef;
    friend DLL_API std::ostream& operator << ( std::ostream&, const Node& );

    private:
       struct impl;
       impl* m_impl;
//       /**
//        * Deep copy.  New node does not contain references to 
//        * children of copy.
//        * \param copy node to copy.
//        **/
//        Unimplemented
	   Node(const Node& copy);
//       /**
//        * Assignment operator.  Functions same as copy constructor.
//        **/       
//        Unimplemented
	   Node& operator=(const Node&);

       NODE_CALL_CREATE;

    protected:


	    /**
	     * \param name Unique name for this node.  Within the concept
	     *			   of terminals/registers/subregisters, names
	     *			   are used as keys and must be unique.
	     **/
        Node(const std::string& name );
         	   
    public:

       enum NODE_TYPE {
         BASENODE,
         DEVIF,
         TERMINAL,
         REGISTER,
         SUBREGISTER,
         VALUEMAP
       };

       static NodeRef create ( const std::string& name );

       /**
        * \brief create a copy of the node
        *
        * The clone method returns a copy of this node and it's children.
        * Whether or not the node is currently a child of another node, the copy
        * is not a child of any other node.  Other than that, they should be the
        * same.
        *
        **/
       virtual NodeRef clone () const;

       virtual ~Node() throw();
       /**
        * \brief Unique identifier for derived node type.
        **/
       virtual NODE_TYPE get_type() const { return BASENODE; }

		// members
        /**
         * \return the node name.
         **/
		const std::string& get_name() const;

        /**
         * \brief Rename node
         **/
        void set_name( const std::string& name );

        
        // children methods
        /**
         * \return reference to child Node.  Reference is valid as long as the Parent nodes reference
         *  is valid.
         * \throw Nitro::Exception if node is not found.
         **/
        NodeRef get_child(const std::string& name) const;


        /**
         * \brief Add a child node.
         *
         * If needed, a derived class can override this method, add validation, and then call this method.
         *
         * \param node Node to add as a child.  Parent node copies child node.  Calling function no 
         * longer needs to maintain memory of child node.
         * \throw Nitro::Exception if child node does not have unique name.
         **/
        virtual void add_child(const NodeRef& node);
        /**
         * \return Children nodes can be iterated exactly according
         * to std::vector::iterator semantics.
         * \code
         * DITreeIter itr=n.child_begin()
         * while(itr!=n.child_end()) {
         *  Node* pn = *itr;
         *  // process node
         *  ++itr;
         * }
         * \endcode
         **/
        DITreeIter child_begin() const;
        /**
         * \copydoc child_begin
         **/
        DITreeIter child_end() const;


        /**
         * \return true if there are children nodes.
         **/
        bool has_children() const ;

        /**
         * \brief Determine if child exists
         *
         * \param name Name of child node to locate.
         * \return true if child node is present.
         **/
        bool has_child( const std::string& name ) const ; 


        /**
         *  \brief remove a child node from the tree. 
         *
         *  Removes the child node with the name supplied or throws
         *  an exception if the node is not removed or not found. 
         *
         *  \param name Remove the child node with this name
         **/
        void del_child( const std::string& name );

        /**
         * \brief Number of child nodes. 
         **/
        uint32 num_children() const;
        
        // attribute methods
        //
        /**
         * \param name The attribute name.
         * \throw Exception If the attribute name is not found.
         * \return DataType
         **/
        DataType get_attr(const std::string& name) const;


        /**
         * \brief Number of attributes
         **/
        uint32 num_attrs() const;

        /**
         * \return true if attribute exists.
         **/
        bool has_attr( const std::string& name) const;
        /**
         * Set attribute on node.  Function overwrites any existing
         * attribute with the same name.
         * \param name attribute name.
         * \param value attribute value.
         **/
        void set_attr(const std::string& name, const DataType& value);

        /**
         * \brief Remove node attribute.
         **/
        void del_attr(const std::string& name);


        /**
         * Attributes can be iterated exactly according to 
         * std::map::iterator semantics.
         * \code
         *  DIAttrIter ai = n.attrs_begin();
         *  while ( ai != n.attrs_end() ) {
         *   std::cout << "Name: " << (*ia).first << " value: " << (*ai).second << std::endl;
         *   ++ai;
         *  }
         * \endcode
         **/
        DIAttrIter attrs_begin() const;
        /**
         * \copydoc attrs_begin
         **/
        DIAttrIter attrs_end() const;

};


/**
 * \brief Write a node to an output stream.
 **/
DLL_API std::ostream& operator << ( std::ostream&, const Node& node );


/**
 * \ingroup devif
 * \brief Root node for Device Interface.
 **/
class DLL_API DeviceInterface : public Node {
   private:
        DeviceInterface( const std::string& name) : Node(name) {}
        NODE_CALL_CREATE; 
   public:
        ~DeviceInterface() throw() {} 
        static NodeRef create ( const std::string& name );
        virtual NODE_TYPE get_type () const { return DEVIF; }
        void add_child ( const NodeRef& node );
};

/**
 * \ingroup devif
 * \brief A Terminal is a representation of a specific hardware component or 
 * a specific interface of a hardware componenet.  Terminals are made up of a
 * collection of 0 or more registers.
 *
 *
 **/
class DLL_API Terminal : public Node {
    private:
        Terminal ( const std::string& name , uint32 regAddrWidth=16, uint32 regDataWidth=16 ) : Node ( name ) {
            set_attr("regAddrWidth", regAddrWidth );
            set_attr("regDataWidth", regDataWidth );
        }
        NODE_CALL_CREATE; 

    public:
        ~Terminal() throw() {}
        static NodeRef create ( const std::string& name );
        virtual NODE_TYPE get_type() const { return TERMINAL; }
        void add_child ( const NodeRef& node );
};

/**
 * \ingroup devif
 * \brief A Register is a specific hardware address on a terminal that can
 * be read from or written to.
 *
 **/
class DLL_API Register: public Node {
    private:
        Register ( const std::string& name, const std::string& type = "int" ) : Node ( name ) {
            set_attr("type", type );
        }
        NODE_CALL_CREATE; 
    public:
        ~Register() throw() {}
        static NodeRef create ( const std::string& name );
        virtual NODE_TYPE get_type() const { return REGISTER; }
        void add_child ( const NodeRef& node );
};

/**
 * \ingroup devif
 * \brief A subregister represents specific bits on a register.
 *
 * Sometimes registers have different functionality in different bits.  These can
 * be represented with a subregister.  In the case of a register with subregisters, 
 * the subregisters can be written to or read from without worrying about the
 * entire register value.
 **/
class DLL_API Subregister: public Node {
    private:
        Subregister ( const std::string& name ) : Node ( name ) {}
        NODE_CALL_CREATE; 
    public:
        ~Subregister() throw() {}
        static NodeRef create ( const std::string& name );
        virtual NODE_TYPE get_type() const { return SUBREGISTER; }
};


/**
 * \ingroup devif
 * \brief A Valuemap holds string names to integer values as attributes.
 * 
 * Valuemaps are supported by Registers and Subregisters.
 **/
class DLL_API Valuemap : public Node {
    private:
        Valuemap ( const std::string& name ) : Node ( name ) {}
        NODE_CALL_CREATE; 
    public:
        ~Valuemap () throw() {}
        static NodeRef create ( const std::string & name );
        virtual NODE_TYPE get_type() const { return VALUEMAP; }
};

/**
 * \ingroup devif
 * \brief Load a Device interface from a file.
 *
 *  This function first checks the absolute or relative path specified by
 *  filepath.  If that doesn't exist, it looks for the NITRO_DI_PATH environment
 *  variable and checks the filepath appended to each of the colon separated
 *  paths in the environment variable for a match.  It loads the first one it 
 *  finds.
 *
 * \param filepath relative or absolute path to a file.
 * \param dst Optional NodeRef destination.  If dst is passed as a parameter, the di
 *            is loaded into dst.  
 * \return DeviceInterface If dst is passed as a parameter, this function retuns dst.
 * \throw Exception if the file is not found or if the specified file does not 
 *                  load a device interface properly.
 **/
DLL_API NodeRef load_di( const std::string &filepath, NodeRef dst = DeviceInterface::create("di") );

} // end namespace

#endif
