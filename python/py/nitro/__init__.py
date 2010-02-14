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
    log.debug("Locking dev for %s" % f)
    dev.lock()
    try:
        return f(dev,*args, **kw)
    finally:
        log.debug("Unlocking dev for %s" % f)
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


###############################################################################
class DevBase(object):
    """This is a wrapper class that lets you easily 'upgrade' a base
    nitro device to an application specific class. You extend this
    device and add your own custom methods. The user can then still
    call all the nitro device base methods, like get() and set(). You
    need to call the constructor of this device with the base device
    as the arguement. It will save the device reference off as
    self.dev. Then when the user makes a function call to your class,
    it will first check your class for the function, and if it does
    not exist it will then try to call the same function on self.dev.

    So suppose you had an imager plugged into the base nitro dev and
    wanted to add a capture() method to that device:

    class Imager(nitro.DevBase):
        def __init__(self, dev, myargs):
            nitro.DevBase.__init__(self, dev)

        def capture(self):
            pass


    Now the user can do the following to upgrade his device to your
    new Imager class:

    dev = nitro.USBDevice(0xFFFF, 0xAAAA)
    dev.open()
    dev = Imager(dev)
    dev.get("Imager", "version")
    img = dev.capture()
    dev.close()

    This is very useful when a generic device manager is used to find
    and open devices and you can't extend the device directly.
    """

    ############################################################################
    def __init__(self, dev):
        """Call this method after the parent method has initialized the DI. 
        This method will load the appropriate imager endpoints"""
        self.dev = dev

    ############################################################################
    # This function sets up this module to pass all attribute accesses
    # down to the the native dev so that this module appears to
    # overload a nitro device.  In other words, this allows the user
    # to perform get/set/read/write/etc on this class directly as if
    # it extended a nitro device directly. The reason to do it this
    # way is that the user can turn any generic nitro device into an
    # application specific class, as is the case when the generic
    # device manager returns a USB nitro device.
    def __getattribute__(self, name):
        try: # first check if this object has the requested method
            return object.__getattribute__(self, name)
        except AttributeError: # if not, try the self.dev
            return self.dev.__getattribute__(name)
