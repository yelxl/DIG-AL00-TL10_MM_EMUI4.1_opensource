# For NXP Smart Audio.

# include path defination.
LOCAL_PATH := sound/soc/codecs/tfa98xx
EXTRA_CFLAGS += -I$(LOCAL_PATH)
# debugging support (also enables trace_printk)
EXTRA_CFLAGS += -DDEBUG

snd-soc-tfa98xx-objs := tfa98xx.o tfa_container.o tfa_dsp.o
obj-$(CONFIG_SND_SOC_TFA98XX)	+= snd-soc-tfa98xx.o
