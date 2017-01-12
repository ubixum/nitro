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
import types
import _nitro

__all__ = [ 'DeviceInterface', 'Terminal', 'Register', 'SubReg', 'Valuemap' ,
'printVerilogInstance', 'printVerilogDefs', 'printVerilogModule', 'printCDefs', ]


class EasyNode:
    child_kw_name = "put something here"
    def __init__(self, **kw ):
        for k in kw.keys():
            if k != self.child_kw_name:
                val=kw[k]
                if k=='valuemap' and type(val)==types.DictType:
                    val=Valuemap(**val)
                self.__setattr__(k,val)
        if kw.has_key(self.child_kw_name):
            for c in kw[self.child_kw_name]:
                self.add_child(c)
class DeviceInterface ( _nitro.DeviceInterface , EasyNode ):
    child_kw_name = 'terminal_list'
    def __init__( self, name, **kw ):
        _nitro.DeviceInterface.__init__(self,name)
        EasyNode.__init__(self,**kw)

class Terminal(_nitro.Terminal,EasyNode):
    child_kw_name = 'register_list'
    def __init__( self, name, **kw ):
        _nitro.Terminal.__init__(self,name)
        EasyNode.__init__(self, **kw)

class Register(_nitro.Register,EasyNode):
    child_kw_name = 'subregs'
    def __init__(self, name, **kw ):
        _nitro.Register.__init__(self,name)
        EasyNode.__init__(self,**kw)


class SubReg(_nitro.Subregister,EasyNode):
    def __init__(self,name,**kw):
        _nitro.Subregister.__init__(self,name)
        EasyNode.__init__(self,**kw)

class Valuemap(_nitro.Valuemap,EasyNode):
    def __init__(self,**kw):
        _nitro.Valuemap.__init__ ( self, "valuemap" )
        EasyNode.__init__(self,**kw)


def getRegs(ep, types=None, modes=None ):
    regs = []
    for reg in ep.values():
        if ((modes is None or reg.mode in modes) and
           (types is None or reg.type in types)): regs.append(reg)
    return regs


def getValueFromMap ( x, reg):
    if type(x) != types.StringType:
        return x
    if not hasattr(reg,'valuemap'):
        return x
    m=reg.valuemap
    if not hasattr(m,x):
        return x
    return reg.valuemap.__getattribute__( x )


def printVerilogModule(ep, module, filename):
    shadowed=[]
    for reg in ep.values():
        if reg.shadowed: shadowed.append(reg)

    f = open(filename,"w");

    f.write("// This file is auto-generated. Do not edit.\n");
    f.write("module " + module + "(\n")
    f.write("  input clk,\n")
    f.write("  input resetb,\n")
    f.write("  input we,\n")
    f.write("  input [" + str(ep.regAddrWidth-1) + ":0] addr,\n")
    f.write("  input [" + str(ep.regDataWidth-1) + ":0] datai,\n\n")
    if shadowed:
        f.write("  input shadow_sync, // pulse high to transfer data to shadow register\n")
    
    for reg in ep.values():
        if(reg.num_children()>0):
            # subreg
            for subreg in reg.values():
                if reg.mode =='read':
                    f.write("  input      ")
                else:
                    f.write("  output     ");
                if subreg.width > 1:
                    f.write("["+str(subreg.width-1)+":0] ")
                f.write("%s,\n" % subreg.vlog_name )

            if(reg.mode != "read"):
                f.write("  output reg ");
                
                if(reg.width > 1 or reg.array > 1):
                    f.write("[" + str(reg.array*reg.width-1) + ":0] ");
                f.write(reg.name + ",\n");

        else:
            if(reg.mode == "read"):
                f.write("  input      ");
            else:
                f.write("  output reg ");
                
            if(reg.width > 1 or reg.array > 1):
                f.write("[" + str(reg.array*reg.width-1) + ":0] ");
    
            f.write(reg.name + ",\n");

    f.write("\n  output reg[" + str(ep.regDataWidth-1)+":0] datao\n");
    f.write(");\n\n");

    # subreg combinations
    for reg in ep.values():
       if reg.num_children()>0:
            if reg.mode == 'write':
                #f.write("reg [%d:0] %s;\n" % ( reg.width-1, reg.name ))
                for subreg in reg.values():
                    f.write("assign %s = %s" % ( subreg.vlog_name , reg.name ))
                    if reg.width > 1:
                        f.write("[")
                        if subreg.width>1:
                            f.write("%d:" % (subreg.addr+subreg.width-1))
                        f.write("%d]" % ( subreg.addr))
                    f.write(";\n")
            else:
                f.write("wire [%d:0] %s = {" % ( reg.width-1, reg.name))
                subregs = reg.values()
                subregs.reverse()
                f.write ( ", ".join ( [ s.vlog_name for s in subregs ] ) ) 
                f.write ( "};\n" )

    def init_str(width, init):
        return "%d'h%x" % (width, init)

    # create shadow registers
    if shadowed:
        for reg in shadowed:
            f.write("reg ")
            if(reg.width > 1 or reg.array > 1):
                f.write("[%d:0] " % (reg.array*reg.width-1, ))
            f.write(reg.name + "_internal_;\n")

        f.write("//shadow registers\n")
        f.write("always @(posedge clk or negedge resetb) begin\n")
        f.write("  if(!resetb) begin\n")
        for reg in shadowed:
            f.write("    %s <= %s;\n" % (reg.name, init_str(reg.width, getValueFromMap(reg.init, reg)))) #  str(getValueFromMap(reg.init, reg))))
        f.write("  end else if(shadow_sync) begin\n")
        for reg in shadowed:
            f.write("    %s <= %s_internal_;\n" % (reg.name, reg.name,))
            reg.name += "_internal_" # rename the register for the remainder of the file

        f.write("  end\n")
        f.write("end\n\n")

  
    #Create triggers
    triggers = getRegs(ep, types=["trigger"], modes=["write"])
    if len(triggers):
        f.write("// Create triggers\n")
        f.write("always @(posedge clk or negedge resetb) begin\n");
        f.write("   if(!resetb) begin\n");
        for reg in triggers:
            f.write("      " + reg.name + " <= 0" + ";\n");
        
        f.write("   end else begin\n");
        for reg in triggers:
            f.write("      %s <= {%d{we & (addr == %d'd%d)}} & datai[%d:0];\n" % (reg.name, reg.width, ep.regAddrWidth, reg.addr, reg.width-1));
        f.write("   end\n");
        f.write("end\n\n");
    #End triggers

    #Writable registers:
    writable = getRegs(ep, types=["int"], modes=["write"])


    if len(writable):
        f.write("// Create writable static registers\n")
        f.write("always @(posedge clk or negedge resetb) begin\n");
        f.write("  if(!resetb) begin\n");
        for reg in writable:
            if type(reg.init) in [ list, tuple ]:
                if len(reg.init) != reg.array:
                    raise Exception(str(reg) + " init sequence is not the same length as the array")
                for i in range(reg.array):
                    f.write("     " + reg.name + "[%d:%d] <= " %((i+1)*reg.width-1, i*reg.width) + init_str(reg.width, getValueFromMap(reg.init[i], reg)) + ";\n");
            else:
                if(reg.array > 1):
                    for i in range(reg.array):
                        f.write("     " + reg.name + "[%d:%d] <= %s;\n" % ((i+1)*reg.width-1, i*reg.width, init_str(reg.width, getValueFromMap(reg.init, reg))))
                else:
                    f.write("     " + reg.name + " <= %s;\n" % (init_str(reg.width, getValueFromMap(reg.init, reg)),))
                    
        f.write("  end else if(we) begin\n");
        f.write("    case(addr)\n");
        for reg in writable:
            w = (reg.width-1)/ep.regDataWidth
            for array in range(reg.array):
                n = w
                i = w
                while(i>=0):
                    curAddr = i
                    arrayOffset = reg.addr+array*(w+1)
                    if ep.endian=='big':
                        curAddr = w-i
                    f.write("      " + str(curAddr+arrayOffset) + ": " + reg.name);
                    if(reg.width > 1 or reg.array > 1):
                        f.write("[")
                        if(i==n):
                            f.write(str(reg.width*(array+1)-1))
                            m = reg.width-ep.regDataWidth*i-1
                        else:
                            f.write(str(ep.regDataWidth*(i+1)-1+array*reg.width))
                            m = ep.regDataWidth-1
                        f.write(":" + str(ep.regDataWidth*i+array*reg.width) + "]");
                        f.write(" <= datai[%d:0];\n" % (m, ));
                    else:
                        f.write(" <= datai[0];\n");
                        
                    i=i-1
        f.write("    endcase\n");
        f.write("  end\n");
        f.write("end\n\n");
    #End writable registers
    
    #Readable registers
    readable = getRegs(ep, types=["int"])
    f.write("// Create readable registers\n")
    f.write("always @(posedge clk or negedge resetb) begin\n");
    f.write(" if (!resetb) begin\n" )
    f.write("  datao <= 0;\n" )
    f.write(" end else begin\n" )


#    f.write("always @(addr");
#    for reg in readable:
#        f.write(" or " + reg.name);
            
#    f.write(") begin\n");
    f.write("  case(addr)\n");
    for reg in readable:
        w = (reg.width-1)/ep.regDataWidth
        for array in range(reg.array):
            n = w
            i = w
            while(i>=0):
                curAddr = i
                arrayOffset = reg.addr+array*(w+1)
                if ep.endian=='big':
                    curAddr = w-i
                f.write("    " + str(curAddr+arrayOffset) + ": datao <= ")
                if((reg.width > 1) or (reg.array > 1)):
                    if(i==n and ep.regDataWidth-(reg.width-ep.regDataWidth*i) != 0):
                        f.write("{ %d'b0, %s[%d:%d] }" % (ep.regDataWidth-(reg.width-ep.regDataWidth*i), reg.name, reg.width-1+(array*reg.width), ep.regDataWidth*i+(array*reg.width), ))
                    else:
                        f.write("%s[%d:%d]" % (reg.name, ep.regDataWidth*(i+1)+(array*reg.width)-1, ep.regDataWidth*i+(array*reg.width),))
                else:
                    f.write("{ %d'b0, %s }" %(ep.regDataWidth-1, reg.name,));
                    
                f.write(";\n");
                i=i-1

    f.write("    default: datao <= 0;\n")
    f.write("  endcase\n")
    f.write(" end\n" )
    f.write("end\n\n")
    
    f.write("endmodule\n");
    f.close();

    # rename shadow registers back
    for reg in shadowed:
        reg.name = reg.name[:-10]


def printVerilogInstance(ep, module, filename, clk="di_clk", resetb="resetb", we="di_write", taddr="di_term_addr", addr="di_reg_addr",  datai="di_reg_datai", shadow_sync="shadow_sync"):
    shadowed=False
    for reg in ep.values():
        if reg.shadowed: shadowed = True

    f = open(filename, "w")
    for reg in ep.values():
        if reg.mode=='write':
            if reg.num_children()>0:
                for subreg in reg.values():
                    if subreg.width > 1:
                        f.write("  wire [%d:0] %s;\n" % (subreg.width-1, subreg.vlog_name))
                    else:
                        f.write("  wire %s;\n" % (subreg.vlog_name))
            f.write("  wire ");
            if(reg.width > 1 or reg.array > 1):
                f.write("[" + str(reg.width*reg.array-1) + ":0] ");
            f.write(reg.name + ";\n");
            if(reg.array > 1 and reg.width > 1):
                for array in range(reg.array):
                    f.write("  wire [%d:0] %s%d = %s[%d:%d];\n" % (reg.width-1, reg.name, array, reg.name, reg.width*(array+1)-1,reg.width*array, ))


    f.write("  wire [%d:0] %s_reg_datao;\n" % ( ep.regDataWidth-1, module ) )
    f.write("  wire %s_%s = %s && (%s == %d);\n" % ( we, module, we, taddr, ep.addr ) )
    
    f.write("  " + module + " " + module + "(\n");
    f.write("     .clk(%s),\n"    % clk);
    f.write("     .resetb(%s),\n" % resetb);
    if(shadowed):
        f.write("     .shadow_sync(%s),\n" % (shadow_sync))
    f.write("     .we(%s_%s),\n"%  (we, module) )
    f.write("     .addr(%s[%d:0]),\n"   % (addr, ep.regAddrWidth-1))
    f.write("     .datai(%s),\n"  % datai)
    f.write("     .datao(%s_reg_datao),\n\n" % module);
    
    for i,reg in enumerate(ep.values()):
        if reg.num_children()>0:
            for j,r in enumerate(reg.values()):
                f.write("     ." + r.vlog_name+ "(" + r.vlog_name + ")");
                if j+1 != len(reg) or i+1 != len(ep) or reg.mode=="write":
                    f.write(',')
                f.write('\n')
            if reg.mode == "write":
                f.write("     ." + reg.name+ "(" + reg.name + ")");
                if(i+1 != len(ep)):
                    f.write(",");
                f.write("\n");
        else:
            f.write("     ." + reg.name+ "(" + reg.name + ")");
            if(i+1 != len(ep)):
                f.write(",");
            f.write("\n");

    f.write("     );\n\n");
    f.close();

def printVerilogDefs(di, module, filename):
    f = open(filename, "w");
    f.write("// This file is auto-generated. Do not edit.\n");
    f.write("`ifndef _%s_DEFS_\n" % module)
    f.write("`define _%s_DEFS_\n\n" % module)

    for term in di.values():
        f.write(("/"*75) + "\n")
        f.write("`define TERM_" + term.name + " " + str(term.addr) + "\n");
        f.write("`define   TERM_" + term.name + "_ADDR_WIDTH " + str(term.regAddrWidth) + "\n")
        for reg in term.values():
            w = (reg.width-1)/term.regDataWidth
            for array in range(reg.array):
                n = w
                i = w
                while(i>=0):
                    if(reg.array > 1):
                        f.write("`define   "+ term.name + "_" + reg.name + str(array))
                        if(w > 0):
                            f.write("_" + str(i))
                        f.write(" " + str(i+reg.addr+(w+1)*array) + "\n")
                    else:
                        f.write("`define   " + term.name + "_" + reg.name)
                        if(w>0):
                            f.write("_" + str(i))
                        f.write(" " + str(i+reg.addr) + "\n")
                        if(w>0 and i==0):
                            f.write("`define   " + term.name + "_" + reg.name)
                            f.write(" " + str(i+reg.addr) + "\n")
                    i=i-1
            f.write("`define      WIDTH_"+term.name+"_"+reg.name+" %d\n" % reg.width)
            if(reg.array > 1):
                f.write("`define      ARRAY_SIZE_"+term.name+"_"+reg.name+" %d\n" % reg.array)
            if hasattr(reg, "valuemap"):
                for k,v in reg.valuemap.attr_items():
                    f.write("`define       "+term.name+"_"+reg.name+"_"+k+" " + str(v)+"\n")
            for subreg in reg.values():
                f.write("`define     "+term.name+"_"+reg.name+"_"+subreg.name+ " %d:%d\n" % (subreg.addr+subreg.width-1,subreg.addr))
        f.write("\n\n")
    f.write("`endif\n")
    f.close()


def printCDefs(di, filename, registers_only=False):
    f = open(filename, "w");
    f.write("// This file is auto-generated. Do not edit.\n");
    defname = "_" + filename.upper().replace(".","_").replace('\\','_').replace('/','_') + "_"
    f.write("#ifndef %s\n" % defname)
    f.write("#define %s\n\n" % defname)

    for term in di.values():
        term_name = term.name.upper()
        f.write(("/"*75) + "\n")
        if not registers_only:
            f.write("#define TERM_" + term_name + " " + str(term.addr) + "\n");
            f.write("#define   TERM_" + term_name + "_ADDR_WIDTH " + str(term.regAddrWidth) + "\n")
        for reg in term.values():
            reg_name = term_name + "_" + reg.name.upper()
           
            w = (reg.width-1)/term.regDataWidth
            for array in range(reg.array):
                n = w
                i = w
                while(i>=0):
                    if(reg.array > 1):
                        f.write("#define   " + reg_name + str(array))
                        if(w > 0):
                            f.write("_" + str(i))
                        f.write(" " + str(i+reg.addr+(w+1)*array) + "\n")
                    else:
                        f.write("#define   " + reg_name)
                        if(w>0):
                            f.write("_" + str(i))
                        f.write(" " + str(i+reg.addr) + "\n")
                        if(w>0 and i==0):
                            f.write("#define   " + reg_name)
                            f.write(" " + str(i+reg.addr) + "\n")
                        for s in reg.values():
                            s_name = reg_name + "_" + s.name.upper()
                            f.write ( "#define      " + s_name + " " + str(s.addr) + "\n" ) 
                    i=i-1
            if hasattr(reg, "valuemap"):
                for k,v in reg.valuemap.attr_items():
                    f.write("#define       "+term.name.upper()+"_"+reg.name.upper()+"_"+k.upper()+" " + str(v)+"\n")
#                f.write("#define    WIDTH_"+term_name+"_"+reg.name+" %d\n" % reg.width)

#            for subreg in reg.values():
#                f.write("#define     "+term_name+"_"+reg.name+"_"+subreg.name+ " %d:%d\n" % (subreg.addr+subreg.width-1,subreg.addr))
        f.write("\n\n")
    f.write("#endif\n")
    f.close()


