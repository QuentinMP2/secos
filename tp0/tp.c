/* GPLv2 (c) Airbus */
#include <debug.h>
#include <info.h>

extern info_t   *info;
extern uint32_t __kernel_start__;
extern uint32_t __kernel_end__;

string get_type_str(multiboot_uint32_t num) {
   string type_str = "";

   switch (num) {
      case 1:
         type_str = "MULTIBOOT_MEMORY_AVAILABLE";
         break;
      
      case 2:
         type_str = "MULTIBOOT_MEMORY_RESERVED";
         break;

      case 3:
         type_str = "MULTIBOOT_MEMORY_ACPI_RECLAIMABLE";
         break;

      case 4:
         type_str = "MULTIBOOT_MEMORY_NVS";
         break;

      default:
         type_str = "UNKNOWN_MULTIBOOT_TYPE";
         break;
      }

   return type_str;
}

void tp() {
   debug("kernel mem [%p - %p]\n", &__kernel_start__, &__kernel_end__);
   debug("MBI flags 0x%x\n", info->mbi->flags);

   multiboot_memory_map_t* entry = (multiboot_memory_map_t*)info->mbi->mmap_addr;
   while((uint32_t)entry < (info->mbi->mmap_addr + info->mbi->mmap_length)) {
      // TODO print "[start - end] type" for each entry
      debug("[0x%llu - 0x%llu] %s\n", (*entry).addr, (*entry).addr + (*entry).size, get_type_str((*entry).type));
      entry++;
   }

   // Test reading/writing in available and reserved memory
   int *ptr_in_available_mem;
   ptr_in_available_mem = (int*)0x0;
   debug("Available mem (0x0): before: 0x%x ", *ptr_in_available_mem); // read
   *ptr_in_available_mem = 0xaaaaaaaa;                           // write
   debug("after: 0x%x\n", *ptr_in_available_mem);                // check

   int *ptr_in_reserved_mem;
   ptr_in_reserved_mem = (int*)0xf0000;
   debug("Reserved mem (at: 0xf0000):  before: 0x%x ", *ptr_in_reserved_mem); // read
   *ptr_in_reserved_mem = 0xaaaaaaaa;                           // write
   debug("after: 0x%x\n", *ptr_in_reserved_mem);                // check

   // Test reading/writing in address out of the physical memory
   int *ptr_out_memory;
   ptr_out_memory = (int*)0x0fffffff;
   debug("Out of mem (at: 0x0fffffff): before: 0x%x ", *ptr_out_memory); // read
   *ptr_out_memory = 0xaaaaaaaa;                                // write
   debug("after: 0x%x\n", *ptr_in_available_mem);               // check
}
