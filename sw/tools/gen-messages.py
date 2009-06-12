#!/usr/bin/env python

import xmlobject
import gentools

import string
import optparse
import re
import os.path
import sys

LENGTHS = {
    "uint8"     :   1,
    "int8"      :   1,
    "uint16"    :   2,
    "int16"     :   2,
    "uint32"    :   4,
    "int32"     :   4,
    "float"     :   4,
}
ARRAY_LENGTH = re.compile("^([a-z0-9]+)\[(\d{1,2})\]$")

def get_field_length(_type):
    #check if it is an array
    m = ARRAY_LENGTH.match(_type)
    if m:
        _type, _len = m.groups()
        return LENGTHS[_type] * int(_len)
    else:
        return LENGTHS[_type]

class Message:
    def __init__(self, m):
        self.name = m.name
        if int(m.id) <= 255:
            self.id = int(m.id)
        else:
            raise Exception("Message IDs must be <= 255")
        try:
            if len(m.field):
                self.fields = m.field
            else:
                self.fields = [m.field]
        except:
            self.fields = []

        sizes = [get_field_length(f.type) for f in self.fields]

        self.size = sum(sizes)
        self.sizes = ["0"] + [str(s) for s in sizes]

class _Writer(object):

    def __init__(self, messages, note, messages_file):
        self.messages = messages
        self.note = note
        self.messages_path = messages_path

    def preamble(self):
        pass

    def body(self):
        raise NotImplementedError

    def postamble(self):
        pass

class _CWriter(_Writer):

    def _print_id(self, m):
        name = m.name.upper()
        print "#define MESSAGE_ID_%s %d" % (name, m.id)

    def _print_length(self, m):
        name = m.name.upper()
        print "#define MESSAGE_LENGTH_%s (%s)" % (name, "+".join(m.sizes))

    def preamble(self):
        gentools.print_header(
            "%s_H" % self.note.upper(),
            generatedfrom=self.messages_path)

        print "#include \"std.h\""
        print
        for m in self.messages:
            self._print_id(m)
        print
        for m in self.messages:
            self._print_length(m)
        print

    def postamble(self):
        gentools.print_footer("%s_H" % self.note.upper())

class MacroWriter(_CWriter):

    def _print_send_function(self, m):
        name = m.name.upper()
        print "#define MESSAGE_SEND_%s(" % name,
        print ", ".join([f.name for f in m.fields]), ") \\"
        print "{ \\"
        print "\tif (DownlinkCheckFreeSpace(%s)) { \\" % "+".join(m.sizes)
        print "\t\tDownlinkStartMessage(\"%s\", MESSAGE_ID_%s, MESSAGE_LENGTH_%s) \\" % (name, name, name)
        for f in m.fields:
            print "\t\t_Put%sByAddr((%s)) \\" % (f.type.title(), f.name)
        print "\t\tDownlinkEndMessage() \\"
        print "\t} else \\"
        print "\t\tDownlinkOverrun(); \\"
        print "}"
        print

    def _print_accessor(self, m):
        offset = 0
        name = m.name.upper()
        for f in m.fields:
            print "#define MESSAGE_%s_GET_FROM_BUFFER_%s(_payload)" % (name, f.name),
            if ARRAY_LENGTH.match(f.type):
                pass
            else:
                l = get_field_length(f.type)
                if l == 1:
                    print "(%s)(*((uint8_t*)_payload+%d))" % (f.type, offset)
                elif l == 2:
                    print "(%s)(*((uint8_t*)_payload+%d)|*((uint8_t*)_payload+%d+1)<<8)" % (f.type, offset, offset)
                elif l == 4:
                    if f.type == "float":
                        print "({ union { uint32_t u; float f; } _f; _f.u = (uint32_t)(*((uint8_t*)_payload+%d)|*((uint8_t*)_payload+%d+1)<<8|((uint32_t)*((uint8_t*)_payload+%d+2))<<16|((uint32_t)*((uint8_t*)_payload+%d+3))<<24); _f.f; })" % (offset, offset, offset, offset)
                    else:
                        print "(%s)(*((uint8_t*)_payload+%d)|*((uint8_t*)_payload+%d+1)<<8|((uint32_t)*((uint8_t*)_payload+%d+2))<<16|((uint32_t)*((uint8_t*)_payload+%d+3))<<24)" % (f.type, offset, offset, offset, offset)


    def preamble(self):
        _CWriter.preamble(self)
        print "#define _Put1ByteByAddr(_byte) {     \\"
        print "\tuint8_t _x = *(_byte);         \\"
        print "\tDownlinkPutUint8(_x);     \\"
        print "}"
        print "#define _Put2ByteByAddr(_byte) { \\"
        print "\t_Put1ByteByAddr(_byte);    \\"
        print "\t_Put1ByteByAddr((const uint8_t*)_byte+1);    \\"
        print "}"
        print "#define _Put4ByteByAddr(_byte) { \\"
        print "\t_Put2ByteByAddr(_byte);    \\"
        print "\t_Put2ByteByAddr((const uint8_t*)_byte+2);    \\"
        print "}"
        print "#define _PutInt8ByAddr(_x) _Put1ByteByAddr(_x)"
        print "#define _PutUint8ByAddr(_x) _Put1ByteByAddr((const uint8_t*)_x)"
        print "#define _PutInt16ByAddr(_x) _Put2ByteByAddr((const uint8_t*)_x)"
        print "#define _PutUint16ByAddr(_x) _Put2ByteByAddr((const uint8_t*)_x)"
        print "#define _PutInt32ByAddr(_x) _Put4ByteByAddr((const uint8_t*)_x)"
        print "#define _PutUint32ByAddr(_x) _Put4ByteByAddr((const uint8_t*)_x)"
        print "#define _PutFloatByAddr(_x) _Put4ByteByAddr((const uint8_t*)_x)"
        print

    def body(self):
        for m in self.messages:
            self._print_send_function(m)
        print
        for m in self.messages:
            self._print_accessor(m)

class FunctionWriter(_CWriter):

    def _print_pack_function(self, m):
        name = m.name.lower()
        print "static inline void message_send_%s(" % name,
        print ", ".join(["%s %s" % (f.type, f.name) for f in m.fields]), ")"
        print "{"
        print "}"

    def preamble(self):
        _CWriter.preamble(self)
        print "static inline void message_start(uint8_t id, uint8_t len)\n{\n}"

#    comm_send_ch(chan, STX);
#    comm_status[chan].ck_a = len;
#    comm_status[chan].ck_b = len;
#    COMM_SEND_CH(chan, len+4);
#    COMM_SEND_CH(chan, ACID);
#    COMM_SEND_CH(chan, id);
#}

#void
#comm_end_message ( CommChannel_t chan )
#{
#    comm_send_ch(chan, comm_status[chan].ck_a);
#    comm_send_ch(chan, comm_status[chan].ck_a);

    def body(self):
        for m in self.messages:
            self._print_pack_function(m)

class RSTWriter(_Writer):

    TABLE_COL_W  = 10
    TABLE_GAP_W  = 1
    TABLE_HEADER = '='*TABLE_COL_W
    HEADING_LEVELS = ('=','-','^','"')

    def _write_header(self, name, level=0):
        print name
        print self.HEADING_LEVELS[level]*len(name)
        
    def _write_table(self, m, indent=None):
        def _print_field(name, _type, center=False, gap=" "):
            if center:
                f = string.center
            else:
                f = string.ljust

            if indent:
                print indent,
            print "%s%s%s" % ( f(name,self.TABLE_COL_W), gap*self.TABLE_GAP_W, f(_type, self.TABLE_COL_W))

        def _print_header():
            _print_field(self.TABLE_HEADER, self.TABLE_HEADER, center=True)

        def _print_title(name):
            title_w = 2*self.TABLE_COL_W + self.TABLE_GAP_W
            title_ul = "-"*self.TABLE_COL_W

            if indent:
                print indent,
            print string.center(name, title_w)
            _print_field(title_ul, title_ul, gap="-")

        _print_header()
        _print_title("Payload")
        _print_field("name", "type", center=True)
        _print_header()
        
        for f in m.fields:
            _print_field(f.name, f.type)
        
        _print_header()

    def preamble(self):
        self._write_header("Messages")
        print
        self._write_header("Message Definitions", level=2)
        print

    def body(self):
        for m in self.messages:

            print " * **%s**" % m.name
            print 
            print "   *ID:* %s" % m.id
            print
            print "   *Payload Length:* %s" % m.size
            print
            self._write_table(m, indent="  ")
            print

        

if __name__ == "__main__":
    OUTPUT_MODES = {
        "macro"     :   MacroWriter,
        "function"  :   FunctionWriter,
        "rst"          :   RSTWriter,
    }
    OUTPUT_MODES_DEFAULT = "macro"
    OUTPUT_MODES_LIST = ", ".join(OUTPUT_MODES)

    parser = optparse.OptionParser()
    parser.add_option("-m", "--messages",
                    default="messages.xml",
                    help="messages xml file", metavar="FILE")
    parser.add_option("-f", "--format",
                    default=OUTPUT_MODES_DEFAULT,
                    help="output format: %s [default: %s]" % (OUTPUT_MODES_LIST, OUTPUT_MODES_DEFAULT))

    options, args = parser.parse_args()

    if not os.path.exists(options.messages):
        parser.error("could not find messages.xml")

    try:
        klass = OUTPUT_MODES[options.format]
    except KeyError:
        parser.error("output mode must be one of %s" % OUTPUT_MODES_LIST)

    try:
        messages_path = os.path.abspath(options.messages)
        x = xmlobject.XMLFile(path=messages_path)
        messages = [Message(m) for m in x.root.message]
    except:
        import traceback
        parser.error("invalid xml\n%s" % traceback.format_exc())

    writer = klass(messages, "messages", messages_path)
    writer.preamble()
    writer.body()
    writer.postamble()



