#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "hello_proc"

// Parameter
static char *name = "World";
module_param(name, charp, 0644);
MODULE_PARM_DESC(name, "Name to greet");

// Proc-Eintrag
static struct proc_dir_entry *proc_entry;

// Read-Funktion für /proc
static ssize_t proc_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    char message[100];
    int len;

    len = snprintf(message, sizeof(message), "Hello %s from Kernel!\n", name);

    return simple_read_from_buffer(buf, count, pos, message, len);
}

// File Operations
static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
};

// Init-Funktion
static int __init hello_init(void)
{
    printk(KERN_INFO "Hello %s from Kernel Module!\n", name);

    proc_entry = proc_create(PROC_NAME, 0, NULL, &proc_fops);

    if (!proc_entry) {
        printk(KERN_ERR "Failed to create /proc entry\n");
        return -ENOMEM;
    }

    return 0;
}

// Exit-Funktion
static void __exit hello_exit(void)
{
    proc_remove(proc_entry);
    printk(KERN_INFO "Goodbye %s!\n", name);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student Fulda");
MODULE_DESCRIPTION("Hello World Kernel Module with Parameter");
