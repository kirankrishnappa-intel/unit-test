#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const char ____versions[]
__used __section("__versions") =
	"\x18\x00\x00\x00\xda\x30\x09\x77"
	"dma_free_attrs\0\0"
	"\x10\x00\x00\x00\x53\x39\xc0\xed"
	"iounmap\0"
	"\x18\x00\x00\x00\x55\x88\x35\x77"
	"iomem_resource\0\0"
	"\x1c\x00\x00\x00\xc2\xc7\x35\x10"
	"__release_region\0\0\0\0"
	"\x14\x00\x00\x00\x3b\x4a\x51\xc1"
	"free_irq\0\0\0\0"
	"\x1c\x00\x00\x00\x1c\xda\x2a\xed"
	"pci_disable_device\0\0"
	"\x18\x00\x00\x00\x64\xbd\x8f\xba"
	"_raw_spin_lock\0\0"
	"\x1c\x00\x00\x00\x34\x4b\xb5\xb5"
	"_raw_spin_unlock\0\0\0\0"
	"\x10\x00\x00\x00\xba\x0c\x7a\x03"
	"kfree\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x20\x00\x00\x00\xca\x58\x91\xd1"
	"pci_unregister_driver\0\0\0"
	"\x1c\x00\x00\x00\x63\xa5\x03\x4c"
	"random_kmalloc_seed\0"
	"\x18\x00\x00\x00\x9e\xf5\x2e\xb2"
	"kmalloc_caches\0\0"
	"\x18\x00\x00\x00\x79\xb6\xdc\xd8"
	"kmalloc_trace\0\0\0"
	"\x1c\x00\x00\x00\x66\x06\xb4\x64"
	"pci_enable_device\0\0\0"
	"\x20\x00\x00\x00\x8e\x83\xd5\x92"
	"request_threaded_irq\0\0\0\0"
	"\x1c\x00\x00\x00\x08\x16\xbd\x85"
	"__request_region\0\0\0\0"
	"\x10\x00\x00\x00\x09\xcd\x80\xde"
	"ioremap\0"
	"\x18\x00\x00\x00\x53\xc1\x38\x9f"
	"dma_alloc_attrs\0"
	"\x18\x00\x00\x00\x0b\xe7\xd0\x88"
	"param_ops_int\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x20\x00\x00\x00\x86\x58\xe0\xc2"
	"__pci_register_driver\0\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x18\x00\x00\x00\xf8\x1a\x94\x92"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "");

MODULE_ALIAS("pci:v*d*sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "1EE35C518E43233C2AED8A4");
