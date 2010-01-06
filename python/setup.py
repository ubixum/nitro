# Copyright (C) 2009 Ubixum, Inc. 
#
# This library is free software; you can redistribute it and/or
#
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USAimport struct
import sys
from distutils.core import setup, Extension 

words = open('python-nitro.spec').read().split()
version=words[words.index('Version:')+1]

def get_ext():
    if sys.platform == 'win32':
        plat_library_dirs = ['../win32/DllRelease', '../win32/python_debug', '../win32/DllDebug']
        plat_extra_compile_args = ['/EHsc']
        plat_define_macros = [('WIN32',None)]
        plat_export_symbols = ['init_nitro_d']
    else:
        plat_library_dirs = ['../build/usr/lib']
        plat_extra_compile_args =[]
        plat_define_macros = []
        plat_export_symbols = []

    return Extension('_nitro',
       include_dirs = [ 'py/nitro/include', 'include', '../include' ],
       libraries = [ '--debug' in sys.argv and 'nitro_d' or 'nitro'],
       library_dirs = plat_library_dirs,
       define_macros=plat_define_macros,
       export_symbols = plat_export_symbols,
       extra_compile_args = plat_extra_compile_args,
       sources = ['src/nitro.cpp', 'src/device.cpp', 'src/usb.cpp', 'src/userdevice.cpp', 'src/node.cpp', 'src/buffer.cpp', 'src/xml.cpp']
       )

if __name__=='__main__':
    setup(
     name='nitro',
     version=version,
     packages=['nitro', 'nitro.wx'],
     package_dir={'nitro':'py/nitro'},
     package_data ={'nitro': [ 'include/python_nitro.h' ],
                    'nitro.wx': ['*.xrc']},
     scripts=['di', 'diconv'],
     ext_modules=[get_ext()]
    )

