
#ifndef XDIRNAME_H
#define XDIRNAME_H

#include <string>

std::string xdirname ( const std::string &path );
std::string xjoin ( const std::string &path, const std::string &file );
std::string xgetcwd ( );
#ifdef WIN32
#include <Windows.h>
std::string get_inst_dir ();
#endif

#endif
