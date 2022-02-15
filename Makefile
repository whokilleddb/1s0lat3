# Compiler To Use
CC := gcc

# Rootfs Image
ROOTFSIMAGE := alpine-minirootfs-3.15.0-x86_64.tar.gz
ROOTFSIMAGEURL := https://dl-cdn.alpinelinux.org/alpine/v3.15/releases/x86_64/alpine-minirootfs-3.15.0-x86_64.tar.gz

# Colors
NONE := \033[00m
RED := \033[01;31m
GREEN := \033[01;32m
YELLOW := \033[01;33m
BLUE := \033[01;34m
PURPLE := \033[01;35m
CYAN := \033[01;36m
WHITE := \033[01;37m
BOLD := \033[1m
BLINK := \033[5m
UNDERLINE := \033[4m

# Target Binary
TARGET := isolate
USERNS := userns
MOUNTNS := mountns
UTILS := utils
MOUNTNS := mountns
PIDNS := pidns
NETNS := networkns

# Directories
SRCDIR :=src
INCDIR := include
OBJDIR := obj
ROOTFSDIR := rootfs

# External libraries
LIBNL := /usr/include/libnl3/
LDFLAGS :=  -lnl-route-3 -lnl-3 

# Compile Time Flags
CFLAGS := -Wall -Wextra -Werror -grecord-gcc-switches -fstack-clash-protection -pipe -g -O2 -D_GNU_SOURCE

#Defauilt Make
all: rootfs utils pidns mountns mountns userns networkns $(TARGET) 

rootfs:
	@$(RM) -rf $(ROOTFSIMAGE) $(ROOTFSDIR)
	@mkdir -p $(ROOTFSDIR)
	@echo -e "[+] Fetching $(GREEN)Rootfs$(NONE) tarball" && wget -q --no-check-certificate --show-progress $(ROOTFSIMAGEURL) -O $(ROOTFSIMAGE)
	@echo "[+] Extracting $(CYAN)Rootfs$(NONE)" && tar -xzf $(ROOTFSIMAGE) -C $(ROOTFSDIR) && echo "Done!"
	@$(RM) -rf $(ROOTFSIMAGE)


utils: $(SRCDIR)/$(UTILS).c 
	@echo -e "[+] Compiling $(YELLOW)Program Utils$(NONE)"
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -I ${INCDIR} -c -o $(OBJDIR)/$(UTILS).o $(SRCDIR)/$(UTILS).c 


pidns: $(SRCDIR)/$(PIDNS).c 
	@mkdir -p $(OBJDIR)
	@echo -e "[+] Compiling $(RED)PID$(NONE) Namespace Program"
	$(CC) $(CFLAGS) -I ${INCDIR} -c -o $(OBJDIR)/$(PIDNS).o $(SRCDIR)/$(PIDNS).c 


mountns: $(SRCDIR)/$(MOUNTNS).c
	@mkdir -p $(OBJDIR)
	@echo -e "[+] Compiling $(PURPLE)MOUNT$(NONE) Namespace Program"
	$(CC) $(CFLAGS) -I ${INCDIR} -c -o $(OBJDIR)/$(MOUNTNS).o $(SRCDIR)/$(MOUNTNS).c 

userns: $(SRCDIR)/$(USERNS).c 
	@mkdir -p $(OBJDIR)
	@echo -e "[+] Compiling $(BLUE)USER$(NONE) Namespace Program"
	$(CC) $(CFLAGS) -I ${INCDIR} -c -o $(OBJDIR)/$(USERNS).o $(SRCDIR)/$(USERNS).c 

networkns: $(SRCDIR)/$(NETNS).c 
	@mkdir -p $(OBJDIR)
	@echo -e "[+] Compiling $(CYAN)NETWORK$(NONE) Namespace Program"
	$(CC) $(CFLAGS) -I ${INCDIR} -I ${LIBNL} -c -o $(OBJDIR)/$(NETNS).o $(SRCDIR)/$(NETNS).c $(LDFLAGS) 

$(TARGET): $(SRCDIR)/$(TARGET).c $(OBJDIR)/$(USERNS).o $(OBJDIR)/$(MOUNTNS).o $(OBJDIR)/$(UTILS).o
	@echo -e "[+] Compiling $(GREEN)$(UNDERLINE)$(TARGET)$(NONE) program" 
	$(CC) $(CFLAGS) -I ${INCDIR} -c -o $(OBJDIR)/$(TARGET).o $(SRCDIR)/$(TARGET).c 
	$(CC) $(CFLAGS) $(OBJDIR)/$(USERNS).o $(OBJDIR)/$(MOUNTNS).o $(OBJDIR)/$(TARGET).o $(OBJDIR)/$(UTILS).o $(OBJDIR)/$(PIDNS).o $(OBJDIR)/$(NETNS).o $(LDFLAGS)  -o $(TARGET) 

#Non-File Targets
.PHONY: clean rootfs 

#Clean only Objecst
clean:
	@$(RM) -rf $(TARGET) $(ROOTFSDIR) $(OBJDIR)
	@echo -e "[-] $(RED)$(BOLD)Cleanup Done$(NONE)!"
