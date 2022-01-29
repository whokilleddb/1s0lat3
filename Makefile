# Compiler To Use
CC := gcc

# Rootfs Image
ROOTFSIMAGE := alpine-minirootfs-3.15.0-x86_64.tar.gz
ROOTFSIMAGEURL := https://dl-cdn.alpinelinux.org/alpine/v3.15/releases/x86_64/alpine-minirootfs-3.15.0-x86_64.tar.gz

# Target Binary
TARGET := isolate

# Directories
SRCDIR :=src
INCDIR := include
ROOTFSDIR := rootfs

# Compile Time Flags
CFLAGS := -Wall -Wextra -Werror=format-security -grecord-gcc-switches -fstack-clash-protection -pipe -g -O2 -D_GNU_SOURCE

#Defauilt Make
all: $(TARGET) rootfs

rootfs:
	@$(RM) -rf $(ROOTFSIMAGE) $(ROOTFSDIR)
	@mkdir -p $(ROOTFSDIR)
	@echo "Fetching Ubuntu rootfs image" && wget -q --show-progress $(ROOTFSIMAGEURL) -O $(ROOTFSIMAGE)
	@echo "Extracting Rootfs" && tar -xzf $(ROOTFSIMAGE) -C $(ROOTFSDIR) && echo "Done!"
	@$(RM) -rf $(ROOTFSIMAGE)

$(TARGET): $(SRCDIR)/$(TARGET).c
	$(CC) $(CFLAGS) -I $(INCDIR) -o $(TARGET)  $(SRCDIR)/$(TARGET).c

#Non-File Targets
.PHONY: clean rootfs

#Clean only Objecst
clean:
	@$(RM) -rf $(TARGET) $(ROOTFSDIR) 
