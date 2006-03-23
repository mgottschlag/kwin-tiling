kde4_header()
include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )
add_subdirectory(devices)
add_subdirectory(dma)
add_subdirectory(interrupts)
add_subdirectory(ioports)
add_subdirectory(memory)
add_subdirectory(nics)
add_subdirectory(opengl)
add_subdirectory(partitions)
add_subdirectory(pci)
add_subdirectory(pcmcia)
add_subdirectory(processor)
add_subdirectory(protocols)
add_subdirectory(samba)
add_subdirectory(scsi)
add_subdirectory(sound)
add_subdirectory(usb)
add_subdirectory(xserver)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#SUBDIRS = $(AUTODIRS)
#
#KDE_LANG= en
#KDE_DOCS = AUTO
#
