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
	{ 0x76a006e9, "module_layout" },
	{ 0x50c87208, "single_release" },
	{ 0xc4ebee06, "seq_lseek" },
	{ 0x92a11473, "seq_read" },
	{ 0x6863141c, "remove_proc_entry" },
	{ 0x811dc334, "usb_unregister_notify" },
	{ 0x3acce11f, "proc_create" },
	{ 0x89bbafc6, "usb_register_notify" },
	{ 0x8da6585d, "__stack_chk_fail" },
	{ 0x92997ed8, "_printk" },
	{ 0x953e1b9e, "ktime_get_real_seconds" },
	{ 0x9166fada, "strncpy" },
	{ 0x4829a47e, "memcpy" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0x3195d15d, "seq_printf" },
	{ 0x7696c2a6, "single_open" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "964ECDE1A98FD17DBBD1B88");
