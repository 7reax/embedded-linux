/*
 * hello_param.c — Hello World Kernel-Modul mit Parameter
 *
 * Laden:   sudo insmod hello_param.ko name=Fulda
 * Prüfen:  dmesg | tail
 * Entfernen: sudo rmmod hello_param
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

static char *name = "World";
module_param(name, charp, 0444);
MODULE_PARM_DESC(name, "Name for the greeting (default: World)");

static int __init hello_init(void)
{
    pr_info("hello_param: Hello %s from the Kernel!\n", name);
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("hello_param: Goodbye %s!\n", name);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student Fulda");
MODULE_DESCRIPTION("Hello World Kernel Module with Parameter");
