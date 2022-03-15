#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/interrupt.h> 

#define SHARED_IRQ 1 

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Nguyensanghso@gmail.com");

static int my_dev_id;
char tasklet_data[] = "This is tasklet data"; 

void tasklet_handler(unsigned long data);

DECLARE_TASKLET(my_tasklet, tasklet_handler, (unsigned long) &tasklet_data);

void tasklet_handler(unsigned long data) 
{
	printk(KERN_INFO "[TL] Tasklet: state - %ld, count - %d, data - %s\n",
		my_tasklet.state, my_tasklet.count, my_tasklet.data);
}


static irqreturn_t my_interrupt(int irq, void *dev_id) 
{
	if (irq == SHARED_IRQ) 
	{
		printk(KERN_INFO "[TL] Tasklet scheduled\n"); 
		tasklet_schedule(&my_tasklet); 
		return IRQ_HANDLED;
	} 
	else 
		return IRQ_NONE;
}

static int __init my_tasklet_init(void)
{
	if (request_irq(SHARED_IRQ, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id))
	{
		printk(KERN_ERR "[TL] Error on request_irq\n"); 
		return -1;
	}
 
	printk(KERN_INFO "[TL] Module loaded!\n");
	return 0;
}

static void __exit my_tasklet_exit(void) 
{
	tasklet_kill(&my_tasklet);
	free_irq(SHARED_IRQ, &my_dev_id);
	printk(KERN_INFO "[TL] Module unloaded!\n");
}

module_init(my_tasklet_init);
module_exit(my_tasklet_exit);
