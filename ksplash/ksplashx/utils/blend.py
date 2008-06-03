#!/usr/bin/env python

import sys, os, getopt, Image, ImageFile

def generate(srcfile1,srcfile2,destfile):
    srcimg1 = Image.open(srcfile1)
    srcimg2 = Image.open(srcfile2)

    (width1,height1) = srcimg1.size
    (width2,height2) = srcimg2.size

    maxwidth = width1>width2 and width1 or width2
    maxheight = height1>height2 and height1 or height2

    if width1 != maxwidth or height1 != maxheight:
        srcimg1.resize( (maxwidth,maxheight) )
    if width2 != maxwidth or height2 != maxheight:
        srcimg2.resize( (maxwidth,maxheight) )

    destimg = Image.new( "RGBA", (maxwidth*10,maxheight*2) )
    for row in range(1,3):
        for col in range(1,11):
            if row==1:
                alpha = col*row/10.0
            else:
                alpha = 1.0 - (col*row/20.0)
            img = Image.blend(srcimg1, srcimg2, alpha)
            x = maxwidth * col - maxwidth
            y = maxheight * row - maxheight
            destimg.paste(img,(x,y))
    destimg.save(destfile)

def main(argv):
    def usage():
        print "Syntax: %s <inputimagefile1> <inputimagefile2> <outputimagefile>" % os.path.basename(argv[0])

    try:
        opts, args = getopt.getopt(sys.argv[1:], "ho:v", ["help"])
    except getopt.GetoptError, err:
        print str(err) # will print something like "option -a not recognized"
        usage()
        sys.exit(1)

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        else:
            assert False, "unhandled option"

    if len(args) < 3:
        usage()
        sys.exit(2)

    generate(args[0],args[1],args[2])

if __name__ == "__main__":
    main(sys.argv)
