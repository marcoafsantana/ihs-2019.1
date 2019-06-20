#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Driver for Discover the Word Project");
MODULE_AUTHOR("Bianca Iz Sarabia, Mateus Cavalcanti, Lucas Dias, Pedro Henrique and Marco Santana");

#define DISPLAY_1    1
#define DISPLAY_2    2
#define DISPLAY_3    3
#define DISPLAY_4    4
#define SWITCHES     5
#define BUTTONS      6
#define GREENLEDS    7
#define REDLEDS      8

//--Control variables.
static uint32_t old_buttons   = 0;
static uint32_t old_switches  = 0;
static uint32_t message_read  = 0; 
static uint32_t message_write = 0; 
static int   access_count     = 0;
static int changed            = 0;

//-- Hardware Handles
static void *display1;  
static void *display2; 
static void *display3;
static void *display4;
static void *switches;
static void *buttons; 
static void *greenleds;
static void *redleds;

//-- Char Driver Interface.
static int   MAJOR_NUMBER = 91;

static int     char_device_open    ( struct inode * , struct file *);
static int     char_device_release ( struct inode * , struct file *);
static ssize_t char_device_read    ( struct file * , char *,       size_t , loff_t *);
static ssize_t char_device_write   ( struct file * , const char *, size_t , loff_t *);

static struct file_operations file_opts = {
  .read = char_device_read,
  .open = char_device_open,
  .write = char_device_write,
  .release = char_device_release
};

static int char_device_open(struct inode *inodep, struct file *filep) {
  access_count++;
  printk(KERN_ALERT "Altera_driver: opened %d time(s)\n", access_count);
  return 0;
}

static int char_device_release(struct inode *inodep, struct file *filep) {
  printk(KERN_ALERT "Altera_driver: device closed.\n");
  return 0;
}

static ssize_t char_device_read(struct file *filep, char *buf, size_t opt, loff_t *off) {
  message_read = 0;
  switch(opt){
    case SWITCHES:
      message_read = ioread32(switches);
      if(message_read != old_switches){
        changed = 1;
        old_switches = message_read;
      }
      break;
    case BUTTONS:
      message_read = ioread32(buttons);
      if(message_read == 15) message_read = 0;
      if(message_read != old_buttons){
        changed = 1;
        old_buttons = message_read;
      }
      break;
    default:
      printk("Invalid option!\n");
      return -3; 
      break;
  }
  printk("Ola gente: valor de message_read eh %u", message_read);
  if(changed == 1){
    copy_to_user(buf, &message_read, sizeof(uint32_t));
    changed = 0;
    return 4;
  }
  else{
    return 0;
  }
}


static ssize_t char_device_write(struct file *filep, const char *buf, size_t opt, loff_t *off) {
  message_write = 0;
  copy_from_user(&message_write, buf, sizeof(uint32_t));
  
  switch(opt){
    case DISPLAY_1:
      iowrite32(message_write, display1);
      break;
    case DISPLAY_2:
      iowrite32(message_write, display2);
      break;
    case DISPLAY_3: 
      iowrite32(message_write, display3);
      break;
    case DISPLAY_4:
      iowrite32(message_write, display4);
      break;
    case REDLEDS:
      iowrite32(message_write, redleds);
      break;
    case GREENLEDS:
      printk("Chegou aqui!\n");
      iowrite32(message_write, greenleds);
      break;
    default:
      printk("Invalid option!");
      break;
  }
  printk("message_write sent: %u\n", message_write);
  return 4;
}

//-- PCI Device Interface
static struct pci_device_id pci_ids[] = {
  { PCI_DEVICE(0x1172, 0x0004), },
  { 0, }
};
MODULE_DEVICE_TABLE(pci, pci_ids);

static int pci_probe(struct pci_dev *dev, const struct pci_device_id *id);
static void pci_remove(struct pci_dev *dev);

static struct pci_driver pci_driver = {
  .name     = "alterahello",
  .id_table = pci_ids,
  .probe    = pci_probe,
  .remove   = pci_remove,
};

static unsigned char pci_get_revision(struct pci_dev *dev) {
  u8 revision;
  pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
  return revision;
}

static int pci_probe(struct pci_dev *dev, const struct pci_device_id *id) {
  int vendor;
  int retval;
  unsigned long resource;
  retval = pci_enable_device(dev);
  
  if (pci_get_revision(dev) != 0x01) {
    printk(KERN_ALERT "altera_driver: cannot find pci device\n");
    return -ENODEV;
  }

  pci_read_config_dword(dev, 0, &vendor);
  printk(KERN_ALERT "altera_driver: Found Vendor id: %x\n", vendor);

  resource = pci_resource_start(dev, 0);
  printk(KERN_ALERT "altera_driver: Resource start at bar 0: %lx\n", resource);

  display1 = ioremap_nocache(resource + 0xC040, 0x20);
  display2 = ioremap_nocache(resource + 0xC080, 0x20);
  display3 = ioremap_nocache(resource + 0XC0C0, 0x20);
  display4 = ioremap_nocache(resource + 0XC100, 0x20);
  switches = ioremap_nocache(resource + 0XC1C0, 0x20);
  buttons = ioremap_nocache(resource + 0XC000, 0x20);
  greenleds = ioremap_nocache(resource + 0XC140, 0x20);
  redleds = ioremap_nocache(resource + 0XC180, 0x20);

  return 0;
}

static void pci_remove(struct pci_dev *dev) {
  iounmap(display1);
  iounmap(display2);
  iounmap(display3);
  iounmap(display4);
  iounmap(switches);
  iounmap(buttons);
  iounmap(greenleds);
  iounmap(redleds);
}


//-- Global module registration
static int __init altera_driver_init(void) {
   int t = register_chrdev(MAJOR_NUMBER, "de2i150_altera", &file_opts);
   t = t | pci_register_driver(&pci_driver);

   if(t<0)
      printk(KERN_ALERT "altera_driver: error: cannot register char or pci.\n");
   else
     printk(KERN_ALERT "altera_driver: char+pci drivers registered.\n");

   return t;
}

static void __exit altera_driver_exit(void) {
  printk(KERN_ALERT "Goodbye from de2i150_altera.\n");

  unregister_chrdev(MAJOR_NUMBER, "de2i150_altera");
  pci_unregister_driver(&pci_driver);
}

module_init(altera_driver_init);
module_exit(altera_driver_exit);
