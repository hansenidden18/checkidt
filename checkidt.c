#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/desc.h>
#include <linux/kallsyms.h>

#define IDT_ENTRIES 256
#define MAX_LINE_LENGTH 256


static gate_desc idt[IDT_ENTRIES];

static void print_idt_entry(int vector)
{
    gate_desc *entry;
    unsigned long address;
    unsigned short seg_selector;
    unsigned char dpl;
    const char *type;
    char handler_name[KSYM_NAME_LEN];
    
    entry = &idt[vector];
    // Extract the necessary information
#ifdef CONFIG_X86_64
    address = ((unsigned long)entry->offset_high << 32) |
              (entry->offset_middle << 16) |
              entry->offset_low;
#else
    address = entry->offset_low | (entry->offset_middle << 16);
#endif
    seg_selector = entry->segment;
    dpl = entry->bits.dpl;

    // Determine the type of the IDT entry
    if (entry->bits.type == GATE_INTERRUPT)
        type = "Interrupt gate";
    else if (entry->bits.type == GATE_TRAP)
        type = "Trap gate";
    else
        type = "Unknown";
    
    sprint_symbol(handler_name, address);
    if (handler_name[0]) {
   	pr_info("%-4d   0x%-10lx  %-11x   %-3d   %-16s   %s\n",
                vector, address, seg_selector, dpl, type, handler_name);
    } else {
        pr_info("%-4d   0x%-10lx  %-11x   %-3d   %-16s   Handler name not found\n",
                vector, address, seg_selector, dpl, type);
    }
//    pr_info("%-4d   0x%-10lx  %-11x   %-3d   %-16s\n",
//            vector, address, seg_selector, dpl, type);
}

static int __init mymodule_init(void)
{
    struct desc_ptr idtr;
    int vector;
    
    store_idt(&idtr);
    memcpy(idt, (gate_desc *)idtr.address, sizeof(gate_desc) * IDT_ENTRIES);

    pr_info("Int *** Stub Address * Segment *** DPL * Type  ***   Handler Name\n");
    pr_info("--------------------------------------------------\n");
    
    // Print the IDT table entries
    for (vector = 0; vector < IDT_ENTRIES; vector++)
    {
        print_idt_entry(vector);
    }

    return 0;
}

static void __exit mymodule_exit(void)
{
    printk(KERN_INFO "IDT module exited\n");
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hansen");
MODULE_DESCRIPTION("IDT Table Printer");
