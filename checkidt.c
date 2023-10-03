#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/desc.h>
#include <linux/kallsyms.h>
#include <linux/kprobes.h>
#include <linux/irq.h>
#include <linux/irqdesc.h>
#include <linux/interrupt.h>

#define IDT_ENTRIES 256
#define MAX_LINE_LENGTH 256


static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};



static gate_desc idt[IDT_ENTRIES];

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
kallsyms_lookup_name_t my_kallsyms_lookup_name;

typedef struct irq_desc *(*irq_to_desc_t)(unsigned int irq);
irq_to_desc_t my_irq_to_desc;

//my_irq_to_desc = (irq_to_desc_t)kallsyms_lookup_name("irq_to_desc");

void print_irq_handler_name(unsigned int irq) {
    struct irq_desc *desc = my_irq_to_desc(irq);
    struct irqaction *action;
    char handler_name[KSYM_NAME_LEN];

    if (!desc) {
        pr_info("IRQ %d: Not found\n", irq);
        return;
    }

    raw_spin_lock(&desc->lock);
    action = desc->action;
    if (action) {
        sprint_symbol_no_offset(handler_name, (unsigned long)action->handler);
        if (handler_name[0]){
            pr_info("IRQ %d: *** Interrupt Name: %s  ***  Handler Name: %s\n",
                        irq, action->name, handler_name);
        }
    } else {
        pr_info("IRQ %d: No handler\n", irq);
    }
    raw_spin_unlock(&desc->lock);
}

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
    /*
    typedef unsigned long (*symbol_lookup_t)(long unsigned int);
    symbol_lookup_t symbol_lookup;
    register_kprobe(&kp);
    symbol_lookup = (symbol_lookup_t) kp.addr;
    unregister_kprobe(&kp);
    */

    sprint_symbol_no_offset(handler_name, address);
    if (handler_name[0]) {
        pr_info("%-4d   0x%-10lx  %-11x   %-3d   %-16s   %s\n",
                vector, address, seg_selector, dpl, type, handler_name);
    } else {
        pr_info("%-4d   0x%-10lx  %-11x   %-3d   %-16s   Handler name not found\n",
                vector, address, seg_selector, dpl, type);
    }
}

static int __init mymodule_init(void)
{
    struct desc_ptr idtr;
    int vector;

    register_kprobe(&kp);
    my_kallsyms_lookup_name = (kallsyms_lookup_name_t) kp.addr;
    my_irq_to_desc = (irq_to_desc_t)my_kallsyms_lookup_name("irq_to_desc");
    unregister_kprobe(&kp);

    store_idt(&idtr);
    memcpy(idt, (gate_desc *)idtr.address, sizeof(gate_desc) * IDT_ENTRIES);

    pr_info("Int *** Stub Address * Segment *** DPL * Type  ***   Handler Stub\n");
    pr_info("-----------------------------------------------------------------\n");

    // Print the IDT table entries
    for (vector = 0; vector < IDT_ENTRIES; vector++)
    {
        print_idt_entry(vector);
        print_irq_handler_name(vector);
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
