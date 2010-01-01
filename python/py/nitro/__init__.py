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
from _nitro import Device, USBDevice, UserDevice, XmlReader, XmlWriter, _NITRO_API , Exception, Buffer, \
    GETSET_VERIFY, DOUBLEGET_VERIFY, STATUS_VERIFY, RETRY_ON_FAILURE, \
    version, str_version, load_di
from di import * 

import logging
log = logging.getLogger(__name__)


###############################################################################
# The below set of functions implement an atomic decorator. Use this to
# wrap a function whose first argument is a nitro.Device with a lock()
# and unlock() mechanism. This decorator can be used on a class that extends
# nitro.Device or on a function whose first argument is a nitro.Device.
#
# Example:
#
# @nitro.atomic
# def myfunction(dev, **kw):
#     pass
#
def _lock(f, dev,*args,**kw):
    log.debug("Locking dev")
    dev.lock()
    try:
        return f(dev,*args, **kw)
    finally:
        log.debug("Unlocking dev")
        dev.unlock()

try:
    """
    Try to use the decorator package if possible.  This way the 
    function parameters etc are preserved
    """
    from decorator import decorator
    @decorator
    def atomic(f,dev,*args,**kw):
        return _lock(f, dev, *args, **kw)

except ImportError:
    def atomic(f):
        def wrapped(dev, *args, **kw):
            return _lock(f, dev, *args, **kw)
        wrapped.__doc__ = f.__doc__
        return wrapped
    print "HERE2"
###############################################################################

