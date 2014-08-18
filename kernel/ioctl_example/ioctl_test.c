#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/pagemap.h>

#define MY_MACIG 'G'
#define READ_IOCTL _IOR(MY_MACIG, 0, int)
#define WRITE_IOCTL _IOW(MY_MACIG, 1, int)
 
struct buffer_struct {
    void *addr;
    size_t size;
}; 
 
struct dma_page_info {
	unsigned long uaddr;
	unsigned long first;
	unsigned long last;
	unsigned int offset;
	unsigned int tail;
	int page_count;
};
 
void dma_get_page_info(struct dma_page_info *dma_page, unsigned long data, unsigned long size)
{
	dma_page->uaddr = data & PAGE_MASK;
	dma_page->offset = data & ~PAGE_MASK;
	dma_page->tail = 1 + ((data+size-1) & ~PAGE_MASK);
	dma_page->first = (data & PAGE_MASK) >> PAGE_SHIFT;
	dma_page->last = ((data+size-1) & PAGE_MASK) >> PAGE_SHIFT;
	dma_page->page_count = dma_page->last - dma_page->first + 1;
	if (dma_page->page_count == 1) dma_page->tail -= dma_page->offset;
} 
 
static int major; 
static char msg[200];

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
  	return simple_read_from_buffer(buffer, length, offset, msg, 200);
}


static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off)
{
	if (len > 199)
		return -EINVAL;
	copy_from_user(msg, buff, len);
	msg[len] = '\0';
	return len;
}

int num;

int device_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
	unsigned int __user *argp = (unsigned int __user *) arg;
	struct page *pages;
	struct mm_struct *mm = current->mm;
	unsigned long virtp;
	void *my_page_address;
	struct buffer_struct buf;
	struct dma_page_info user_dma;
	int res;
	
	switch(cmd) {
	case READ_IOCTL:	
		copy_to_user(argp, &num, sizeof(num));
		break;
	
	case WRITE_IOCTL:
		copy_from_user(&buf, argp, sizeof(buf));
		virtp = (unsigned long) buf.addr;
		printk("virt: %#llx", virtp);
		num = buf.size;
		
		dma_get_page_info(&user_dma, buf.addr, buf.size);
		num = user_dma.page_count;
		if (user_dma.page_count <= 0) {
			printk("dma_setup: Error %d page_count from %d bytes %d offset\n",
			user_dma.page_count, num, user_dma.offset);
			return -EINVAL;
		} 
		
		pages = kmalloc(user_dma.page_count * sizeof(struct page*),
						GFP_KERNEL);
		if (NULL == pages)
			return -ENOMEM;
		
		down_read(&current->mm->mmap_sem);
		res = get_user_pages(current, current->mm,
			user_dma.uaddr, user_dma.page_count, 1, 1, &pages, NULL);
		up_read(&current->mm->mmap_sem);
		if (res >= 1) {
			
        printk(KERN_INFO "Got %d pages\n", res);

        /* kmap
         * Takes a struct page from high memory and maps it into low 
         * memory. The address returned is the virtual address of 
         * the mapping
        */
        my_page_address = kmap(pages);
        kunmap(pages);
        
        /* Clean up */
    if (!PageReserved(pages)) {
        SetPageDirty(pages);
        }
	    page_cache_release(pages);
    } else {
        printk(KERN_INFO "Couldn't get pages :(\n");
    }
		break;

	default:
		return -ENOTTY;
	}
	return 0;

}
static struct file_operations fops = {
	.read = device_read, 
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
};

static int __init cdevexample_module_init(void)
{
	major = register_chrdev(0, "my_device", &fops);
	if (major < 0) {
     		printk ("Registering the character device failed with %d\n", major);
	     	return major;
	}
	printk("cdev example: assigned major: %d\n", major);
	printk("create node with mknod /dev/gnuradio c %d 0\n", major);
 	return 0;
}

static void __exit cdevexample_module_exit(void)
{
	unregister_chrdev(major, "my_device");
}  

module_init(cdevexample_module_init);
module_exit(cdevexample_module_exit);
MODULE_LICENSE("GPL");
