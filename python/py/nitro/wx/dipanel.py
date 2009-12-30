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
import wx
from wx import xrc
from xrcctrl import XrcPanel
import logging
import types

class DeviceInterfacePanel(XrcPanel):

    log=logging.getLogger ( __name__ )

    def __init__(self):
        XrcPanel.__init__(self)
        
    def _PostInit(self):

        self.log.debug ( "_PostInit" )
    
        self.info=dict()
        for id in ['TERMINAL', 'NAME', 'ADDR', 'INIT', 'MODE', 'TYPE', 'WIDTH', 'COMMENT', 'ARRAY' ]:
            ctrl = xrc.XRCCTRL ( self, "ID_STATIC_%s" % id )
            ctrl.SetLabel('') # 
            self.info[id.lower()] = ctrl
        self.values=dict()
        for id in ['BIN','HEX','DEC']:
            ctrl = xrc.XRCCTRL ( self, "ID_DEV_%sVAL" % id )
            self.values[id.lower()] = ctrl
            ctrl.Bind ( wx.EVT_KILL_FOCUS, self.OnText )
        
        self.Bind (wx.EVT_TEXT_ENTER, self.OnText )
        
        self.epTree = xrc.XRCCTRL( self, "ID_TREECTRL" )
        self.epTree.AddRoot("Terminals")
        
        self.getButton = xrc.XRCCTRL ( self, "ID_DEV_GET_BUTTON" )
        self.setButton = xrc.XRCCTRL ( self, "ID_DEV_SET_BUTTON" )


        self.Bind(wx.EVT_TREE_SEL_CHANGED, self.OnSelectionChanged,id=self.epTree.GetId())
        self.Bind(wx.EVT_BUTTON, self.OnGet, id=self.getButton.GetId())
        self.Bind(wx.EVT_BUTTON, self.OnSet, id=self.setButton.GetId())

        if hasattr(self,'initial_device'):
            self.set_device(self.initial_device)

        self.disable_getset()


    def set_device(self, dev):
        self.log.debug ( "Setting Device" )
        self.dev = dev

        if not hasattr(self,'epTree'):
            # in this case, _PostInit hasn't been called yet.
            self.initial_device = dev
            return

        root = self.epTree.GetRootItem()
        self.epTree.DeleteChildren(root) # clear any old data out

        # enumerate through endpoints and registers to create the device tree
        if self.dev is not None:
            di=self.dev.get_di()
            for ep in di: 
                node = self.epTree.AppendItem(root, ep)
                for reg in di[ep]:
                    reg_node=self.epTree.AppendItem(node, reg)
                    if di[ep][reg].num_children():
                        for subreg in di[ep][reg]:
                            self.epTree.AppendItem(reg_node,"%s.%s" % (reg,subreg))
                    elif di[ep][reg].array > 1:
                        for i in range(di[ep][reg].array):
                            self.epTree.AppendItem(reg_node,'%s[%d]' % ( reg, i ) ) 

        self.epTree.Expand(root)


    def disable_getset(self):
        for x in ['addr', "init", "mode", "type", "width", "comment"]:
            self.info[x].SetLabel("")
        for b in [self.getButton, self.setButton]:
            b.Disable()

    def OnSelectionChanged(self, event):
        self.disable_getset()


        node = self.epTree.GetSelection()
        name = self.epTree.GetItemText(node)
        parent = self.epTree.GetItemParent(node)
        root = self.epTree.GetRootItem()

        if(parent == root or node == root):
            # this is an end point
            pass
        elif self.epTree.GetItemParent(parent) == self.epTree.GetRootItem():
            # this is a register
            self.update(self.dev, self.epTree.GetItemText(parent), name)
        else:
            # this is a subregister or array

            term = self.epTree.GetItemParent(parent)
            reg = self.epTree.GetItemParent(node)
            self.update(self.dev, self.epTree.GetItemText(term), name )


    def update(self, dev, ep, name):
        self.info["terminal"].SetLabel(ep)
        self.info["name"].SetLabel(name)
        di=dev.get_di()
        term = di[str(ep)]
        rawregname = name
        if '.' in rawregname: rawregname = rawregname.split('.')[0]
        if '[' in rawregname: rawregname = rawregname.split('[')[0]
        reg = term[str(rawregname)]
        if len(name.split('.'))>1:
            reg = reg[str(name.split('.')[1])]
        for x in [ "addr", "init", "mode", "type", "width", "array" ]:
            try:
                y = getattr(reg,x)
                if(type(y) == str):
                    self.info[x].SetLabel(y)
                elif(type(y) == int):
                    self.info[x].SetLabel("%d (0x%x)" % (y, y))
                else:
                    self.info[x].SetLabel(str(y))
            except Exception,_inst:
#                log.exception(_inst)
                self.info[x].SetLabel("")
        c=getattr(reg,'comment')
        self.info['comment'].SetToolTipString(c)
        if len(c)>35: c=c[:35] + ' ...'
        self.info['comment'].SetLabel(str(c))
        self.getButton.Enable()
        self.setButton.Enable( self.info['mode'].GetLabel() != 'read' )

    def set_display_value(self,val):
       def binval(v):
           bin_str = ''
           val_tmp=v
           while val_tmp>0:
              bin_str = str(val_tmp & 1) + bin_str 
              val_tmp >>= 1
           return len(bin_str)>0 and bin_str or '0'
       
       if type(val) == types.ListType:
         self.values['bin'].ChangeValue ( "[%s]" % ", ".join( [ binval(v) for v in val] ) ) 
         self.values['hex'].ChangeValue ( "[%s]" % ", ".join( [ hex(v) for v in val] ) )  
         self.values['dec'].ChangeValue ( "[%s]" % ", ".join( [ str(v) for v in val] ) ) 
       else:
         self.values['bin'].ChangeValue(binval(val))
         self.values['hex'].ChangeValue(hex(val))
         self.values['dec'].ChangeValue(str(val))

    def get_display_value(self):
        return eval ( self.values['dec'].GetValue() ) 

    def OnGet(self, event):
        try:
            val = self.dev.get(str(self.info["terminal"].GetLabel()),
                                      str(self.info["name"].GetLabel()))

            self.set_display_value( val )

        except RuntimeError:
            pass
        
        
                            
    def OnSet(self, event):
        val = self.get_display_value()
        try:
            self.dev.set(str(self.info["terminal"].GetLabel()),
                                str(self.info["name"].GetLabel()),
                                val)
        except RuntimeError:
            pass

    def OnText(self,evt):
        ctrl = evt.GetEventObject()
        try:
            val_str=ctrl.GetValue()
            get_vals={self.values['bin'].GetId():lambda v: int(v,2),
                      self.values['hex'].GetId():lambda v: int(v,16),
                      self.values['dec'].GetId():lambda v: int(v)}

            if '[' in val_str:
                val_strs = [ s.strip() for s in val_str.replace('[','').replace(']','').split(',') ]
                self.set_display_value ( [get_vals[ctrl.GetId()](v) for v in val_strs] )
            else:
                self.set_display_value ( get_vals[ctrl.GetId()](val_str) )
                
        except Exception, _inst:
            self.log.exception(_inst)

