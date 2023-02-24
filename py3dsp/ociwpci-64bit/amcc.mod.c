#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

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


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x92997ed8, "_printk" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x37a0cba, "kfree" },
	{ 0xedc03953, "iounmap" },
	{ 0x999e8297, "vfree" },
	{ 0x36ca193e, "pci_release_regions" },
	{ 0x350790, "pci_disable_device" },
	{ 0xb2fd5ceb, "__put_user_4" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x9a994cf7, "current_task" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xd494bc92, "pci_enable_device" },
	{ 0xc1352057, "__register_chrdev" },
	{ 0xaee657ee, "__class_create" },
	{ 0x3818fa5c, "device_create" },
	{ 0xf301d0c, "kmalloc_caches" },
	{ 0x35789eee, "kmem_cache_alloc_trace" },
	{ 0xb8b9f817, "kmalloc_order_trace" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x308dbbb7, "pci_request_regions" },
	{ 0xde80cd09, "ioremap" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x7e2d339e, "__pci_register_driver" },
	{ 0x86bb6b5a, "pci_unregister_driver" },
	{ 0xfd4cb24f, "param_ops_long" },
	{ 0x541a6db8, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("pci:v000010E8d00005920sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "9F89D27E3E7A1B05671BFD5");
