# Makefile for the smartpa devkit drivers.
#

# Each configuration option enables a list of files.
ifdef CONFIG_HUAWEI_SMARTPAKIT_AUDIO
ccflags-y += -Idrivers/devkit/audiokit/
endif
obj-$(CONFIG_HUAWEI_SMARTPAKIT_AUDIO) += smartpakit.o
obj-$(CONFIG_HUAWEI_SMARTPAKIT_AUDIO) += smartpakit_i2c.o
