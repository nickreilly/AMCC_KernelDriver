#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
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
	{ 0x32e21920, "module_layout" },
	{ 0x59d743f9, "param_ops_long" },
	{ 0x233f929e, "pci_unregister_driver" },
	{ 0xf2bb7d9, "__pci_register_driver" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0xde80cd09, "ioremap" },
	{ 0x95b52781, "pci_request_regions" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0xb8b9f817, "kmalloc_order_trace" },
	{ 0xf35141b2, "kmem_cache_alloc_trace" },
	{ 0x26087692, "kmalloc_caches" },
	{ 0x36a97168, "device_create" },
	{ 0xaf919f03, "__class_create" },
	{ 0xd0abc829, "__register_chrdev" },
	{ 0x8d929026, "pci_enable_device" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0xa7bfbf2f, "current_task" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xb2fd5ceb, "__put_user_4" },
	{ 0xfb14ff3c, "pci_disable_device" },
	{ 0xf288fcdc, "pci_release_regions" },
	{ 0x999e8297, "vfree" },
	{ 0xedc03953, "iounmap" },
	{ 0x37a0cba, "kfree" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x92997ed8, "_printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("pci:v000010E8d00005920sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "AD4622688AA107AFB80597E");
