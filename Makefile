# Compiler To Use
CC := gcc

# Rootfs Image
ROOTFSIMAGE := alpine-minirootfs-3.15.0-x86_64.tar.gz
ROOTFSIMAGEURL := https://dl-cdn.alpinelinux.org/alpine/v3.15/releases/x86_64/alpine-minirootfs-3.15.0-x86_64.tar.gz

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

# Compile Time Flags
CFLAGS := -Wall -Wextra -Werror=format-security -grecord-gcc-switches -fstack-clash-protection -pipe -g -O2 -D_GNU_SOURCE

#Defauilt Make
all: rootfs utils pidns mountns mountns userns networkns $(TARGET) 

rootfs:
	@$(RM) -rf $(ROOTFSIMAGE) $(ROOTFSDIR)
	@mkdir -p $(ROOTFSDIR)
	@echo "[+] Fetching Alpine rootfs image" && wget -q --show-progress $(ROOTFSIMAGEURL) -O $(ROOTFSIMAGE)
	@echo "[+] Extracting Rootfs" && tar -xzf $(ROOTFSIMAGE) -C $(ROOTFSDIR) && echo "Done!"
	@$(RM) -rf $(ROOTFSIMAGE)


utils: $(SRCDIR)/$(UTILS).c 
	@echo "[+] Compiling Program Utils"
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -I ${INCDIR} -c -o $(OBJDIR)/$(UTILS).o $(SRCDIR)/$(UTILS).c 


pidns: $(SRCDIR)/$(PIDNS).c 
	@mkdir -p $(OBJDIR)
	@echo "[+] Compiling PID Namespace Program"
	$(CC) $(CFLAGS) -I ${INCDIR} -c -o $(OBJDIR)/$(PIDNS).o $(SRCDIR)/$(PIDNS).c 


mountns: $(SRCDIR)/$(MOUNTNS).c
	@mkdir -p $(OBJDIR)
	@echo "[+] Compiling Mount Namespace Program"
	$(CC) $(CFLAGS) -I ${INCDIR} -c -o $(OBJDIR)/$(MOUNTNS).o $(SRCDIR)/$(MOUNTNS).c 

userns: $(SRCDIR)/$(USERNS).c 
	@mkdir -p $(OBJDIR)
	@echo "[+] Compiling User Namespace Program"
	$(CC) $(CFLAGS) -I ${INCDIR} -c -o $(OBJDIR)/$(USERNS).o $(SRCDIR)/$(USERNS).c 

networkns: $(SRCDIR)/$(NETNS).c 
	@mkdir -p $(OBJDIR)
	@echo "[+] Compiling Network Namespace Program"
	$(CC) $(CFLAGS) -I ${INCDIR} -c -o $(OBJDIR)/$(NETNS).o $(SRCDIR)/$(NETNS).c -lnetlink

$(TARGET): $(SRCDIR)/$(TARGET).c $(OBJDIR)/$(USERNS).o $(OBJDIR)/$(MOUNTNS).o $(OBJDIR)/$(UTILS).o
	@echo "[+] Compiling"
	$(CC) $(CFLAGS) -I ${INCDIR} -c -o $(OBJDIR)/$(TARGET).o $(SRCDIR)/$(TARGET).c 
	$(CC) $(CFLAGS) $(OBJDIR)/$(USERNS).o $(OBJDIR)/$(MOUNTNS).o $(OBJDIR)/$(TARGET).o $(OBJDIR)/$(UTILS).o $(OBJDIR)/$(PIDNS).o $(OBJDIR)/$(NETNS).o -o $(TARGET) 

#Non-File Targets
.PHONY: clean rootfs 

#Clean only Objecst
clean:
	@$(RM) -rf $(TARGET) $(ROOTFSDIR) $(OBJDIR)
