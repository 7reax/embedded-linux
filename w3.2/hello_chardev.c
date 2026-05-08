/*
 * hello_chardev.c — Hello World als Char-Device-Treiber
 *
 * Erzeugt automatisch /dev/hello via udev.
 *
 * Laden:      sudo insmod hello_chardev.ko name=Fulda
 * Lesen:      cat /dev/hello
 * Schreiben:  echo "Linux" | sudo tee /dev/hello
 * Entfernen:  sudo rmmod hello_chardev
 *
 * Kernel >= 5.6 (proc_ops), class_create-API ab 6.4 wird abgedeckt.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/string.h>

#define DEVICE_NAME "hello"
#define CLASS_NAME  "hello_class"
#define BUF_SIZE    64

/* ---- Parameter ---------------------------------------------------------- */

static char *name = "World";
module_param(name, charp, 0444);
MODULE_PARM_DESC(name, "Greeting name loaded at insmod (default: World)");

/* ---- Interner Zustand ---------------------------------------------------- */

static dev_t      dev_num;
static struct cdev      hello_cdev;
static struct class    *hello_class;
static struct device   *hello_device;

/* Schreibbarer Puffer — wird via /dev/hello aktualisiert */
static char name_buf[BUF_SIZE] = "World";

/* ---- File Operations ----------------------------------------------------- */

static int hello_open(struct inode *inode, struct file *file)
{
    pr_info("hello_chardev: device opened\n");
    return 0;
}

static int hello_release(struct inode *inode, struct file *file)
{
    pr_info("hello_chardev: device closed\n");
    return 0;
}

/*
 * read: Gibt "Hello <name> from Kernel!\n" zurück.
 * simple_read_from_buffer kümmert sich um pos-Tracking (EOF-Erkennung).
 */
static ssize_t hello_read(struct file *file, char __user *buf,
                           size_t count, loff_t *pos)
{
    char msg[BUF_SIZE + 32];
    int  len;

    len = snprintf(msg, sizeof(msg), "Hello %s from Kernel!\n", name_buf);
    return simple_read_from_buffer(buf, count, pos, msg, len);
}

/*
 * write: Übernimmt den neuen Namen aus dem Userspace.
 * echo "Fulda" > /dev/hello  →  name_buf = "Fulda"
 */
static ssize_t hello_write(struct file *file, const char __user *buf,
                            size_t count, loff_t *pos)
{
    size_t len = min(count, (size_t)(BUF_SIZE - 1));

    if (copy_from_user(name_buf, buf, len))
        return -EFAULT;

    name_buf[len] = '\0';

    /* Abschließendes Newline entfernen */
    if (len > 0 && name_buf[len - 1] == '\n')
        name_buf[len - 1] = '\0';

    pr_info("hello_chardev: name updated to '%s'\n", name_buf);
    return count;
}

static const struct file_operations hello_fops = {
    .owner   = THIS_MODULE,
    .open    = hello_open,
    .release = hello_release,
    .read    = hello_read,
    .write   = hello_write,
};

/* ---- Init / Exit --------------------------------------------------------- */

static int __init hello_init(void)
{
    int ret;

    /* Anfangswert aus Modul-Parameter übernehmen */
    strncpy(name_buf, name, BUF_SIZE - 1);
    name_buf[BUF_SIZE - 1] = '\0';

    /* 1) Dynamische Major-/Minor-Nummer anfordern */
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("hello_chardev: alloc_chrdev_region failed (%d)\n", ret);
        return ret;
    }

    /* 2) cdev initialisieren und registrieren */
    cdev_init(&hello_cdev, &hello_fops);
    hello_cdev.owner = THIS_MODULE;

    ret = cdev_add(&hello_cdev, dev_num, 1);
    if (ret < 0) {
        pr_err("hello_chardev: cdev_add failed (%d)\n", ret);
        goto err_unreg;
    }

    /* 3) Geräteklasse anlegen (sichtbar unter /sys/class/hello_class/) */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    hello_class = class_create(CLASS_NAME);
#else
    hello_class = class_create(THIS_MODULE, CLASS_NAME);
#endif
    if (IS_ERR(hello_class)) {
        ret = PTR_ERR(hello_class);
        pr_err("hello_chardev: class_create failed (%d)\n", ret);
        goto err_cdev;
    }

    /* 4) Device-Node anlegen → udev erzeugt /dev/hello */
    hello_device = device_create(hello_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(hello_device)) {
        ret = PTR_ERR(hello_device);
        pr_err("hello_chardev: device_create failed (%d)\n", ret);
        goto err_class;
    }

    pr_info("hello_chardev: loaded  Major=%d Minor=%d  /dev/%s\n",
            MAJOR(dev_num), MINOR(dev_num), DEVICE_NAME);
    pr_info("hello_chardev: Hello %s from Kernel!\n", name_buf);
    return 0;

err_class:
    class_destroy(hello_class);
err_cdev:
    cdev_del(&hello_cdev);
err_unreg:
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

static void __exit hello_exit(void)
{
    device_destroy(hello_class, dev_num);
    class_destroy(hello_class);
    cdev_del(&hello_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("hello_chardev: unloaded — Goodbye %s!\n", name_buf);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student Fulda");
MODULE_DESCRIPTION("Hello World Char Device Driver with Parameter");
