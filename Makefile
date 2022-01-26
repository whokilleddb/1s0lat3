#Compiler and Linker
CC          := gcc

#The Target Binary Program
TARGET      := isolate
ROOTFSIMAGE := ubuntu-bionic-rootfs.tar.gz

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := src
INCDIR      := include
BUILDDIR    := obj
TARGETDIR   := bin
SRCEXT      := c
DEPEXT      := d
OBJEXT      := o
ROOTFSDIR   := rootfs

#Flags, Libraries and Includes
CFLAGS      := -Wall -Werror -Wshadow -g -O3 -D_GNU_SOURCE
INC         := -I$(INCDIR) 
INCDEP      := -I$(INCDIR)

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------
SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

#Defauilt Make
all: directories rootfs $(TARGET) 

rootfs:
	@mkdir -p $(ROOTFSDIR)
	@echo "Fetching Ubuntu rootfs image" && wget -q --show-progress https://partner-images.canonical.com/oci/bionic/current/ubuntu-bionic-oci-amd64-root.tar.gz -O $(ROOTFSIMAGE)
	@echo "Extracting Rootfs" && tar -xzf $(ROOTFSIMAGE) -C $(ROOTFSDIR) && echo "Done!"
	@$(RM) -rf $(ROOTFSIMAGE)

#Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Link
$(TARGET): $(OBJECTS)
	$(CC) $^ $(LIB) $(CFLAGS) -o $(TARGETDIR)/$(TARGET) 

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

#Non-File Targets
.PHONY: all clean rootfs

#Clean only Objecst
clean:
	@$(RM) -rf $(BUILDDIR) $(TARGETDIR) $(ROOTFSDIR) 