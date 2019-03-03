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
import sys,os
from distutils.core import setup, Extension 
import subprocess
from subprocess import Popen

if sys.platform == 'win32':
    vprog=os.path.join('..','win32','x64','Release','nitro_version.exe')
else:
    vprog=os.path.join('..','build','usr','bin','nitro_version')

version=Popen(vprog,stdout=subprocess.PIPE).stdout.read().strip()


def get_ext():
    if sys.platform == 'win32':
        if 'AMD64' in sys.version:
           plat_library_dirs = [ '../win32/x64/Release' ]
        else:
           plat_library_dirs = ['../win32/DllRelease', '../win32/python_debug', '../win32/DllDebug']
        plat_extra_compile_args = ['/EHsc']
        plat_define_macros = [('WIN32',None)]
        plat_export_symbols = ['init_nitro_d']
    else:
        plat_library_dirs = ['../build/usr/lib', '../build/usr/lib64' ]
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

def get_scripts():
    scripts=['di','diconv']
    if sys.platform=='win32':
        scripts += ['di.bat','diconv.bat']
    return scripts

if __name__=='__main__':
    setup(
     name='nitro',
     version=version,
     packages=['nitro', 'nitro.wx'],
     package_dir={'nitro':'py/nitro'},
     package_data ={'nitro': [ 'include/python_nitro.h' ],
                    'nitro.wx': ['*.xrc']},
     scripts=get_scripts(),
     ext_modules=[get_ext()]
    )

