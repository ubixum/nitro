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
import os
import wx.xrc

def create_dipanel(parent):
    import os
    import wx.xrc
    res = wx.xrc.XmlResource (
        "%s/dipanel.xrc" % os.path.dirname(os.path.abspath(__file__)))
    return res.LoadPanel ( parent, "ID_DI_PANEL" )


__all__ = ['create_dipanel']
