#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/sched.h> //for_each_process 
MODULE_LICENSE("BSD"); 
static int pid = -1; 
module_param(pid, int, S_IRUGO); 
static int killd_init(void) 
{ 
    printk("<1>hello\r\n");
    printk("S_IRUGO--%d\r\n",S_IRUGO); 
    return 0; 
} 
static void killd_exit(void) 
{ 
    printk("<1> killd: bye\r\n"); 
} 
module_init(killd_init); 
module_exit(killd_exit); 
