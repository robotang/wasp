import sys

def print_header(name, generatedfrom=None, generatedby=None, outfile=sys.stdout):
    if generatedfrom:
        print >> outfile, "/* This outfile has been generated from %s */" % generatedfrom
    if generatedby:
        print >> outfile, "/* This outfile has been generated by %s */" % generatedby
    print >> outfile, "/* Please DO NOT EDIT */"
    print >> outfile
    print >> outfile, "#ifndef ", name
    print >> outfile, "#define ", name

def print_footer(name, outfile=sys.stdout):
    print >> outfile, "\n#endif /* %s */" % name

def define_string(name, val, maxwidth=0, outfile=sys.stdout):
    val = val.strip()
    #take one off the maxwidth
    maxwidth -= 1
    if maxwidth and len(val) != maxwidth:
        val = val.ljust(maxwidth)[0:maxwidth]
    print >> outfile, '#define %s "%s"' % (name.upper(), val)

def define_int(name, val, outfile=sys.stdout):
    print >> outfile, "#define %s %d" % (name.upper(), val)
