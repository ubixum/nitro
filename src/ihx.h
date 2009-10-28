
#ifndef IHX_H
#define IHX_H

#include <vector>
#include <string>
#include <nitro/types.h>
#include <nitro/error.h>

/**
 * A basic record for ihx files. 
 **/
struct IhxRecord {
   uint16 addr; 
   std::basic_string<uint8> bytes;
};

/**
 * An Ihx file is really just a collection of records
 **/
typedef std::vector<IhxRecord> IhxFile;

/**
 * Turn ihx file contents into a valid ihx file.
 * \param the .ihx file contents.
 * \param length number of bytes in the stream.
 * \throw Exception
 **/
IhxFile parse_ihx ( const char *bytes, size_t length ) ; 

#endif

