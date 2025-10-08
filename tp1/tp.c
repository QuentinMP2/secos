/* GPLv2 (c) Airbus */
#include <debug.h>
#include <segmem.h>
#include <string.h>

void userland() {
   asm volatile ("mov %eax, %cr0");
}

void print_gdt_content(gdt_reg_t gdtr_ptr) {
    seg_desc_t* gdt_ptr;
    gdt_ptr = (seg_desc_t*)(gdtr_ptr.addr);
    int i=0;
    while ((uint32_t)gdt_ptr < ((gdtr_ptr.addr) + gdtr_ptr.limit)) {
        uint32_t start = gdt_ptr->base_3<<24 | gdt_ptr->base_2<<16 | gdt_ptr->base_1;
        uint32_t end;
        if (gdt_ptr->g) {
            end = start + ( (gdt_ptr->limit_2<<16 | gdt_ptr->limit_1) <<12) + 4095;
        } else {
            end = start + (gdt_ptr->limit_2<<16 | gdt_ptr->limit_1);
        }
        debug("%d ", i);
        debug("[0x%x ", start);
        debug("- 0x%x] ", end);
        debug("seg_t: 0x%x ", gdt_ptr->type);
        debug("desc_t: %d ", gdt_ptr->s);
        debug("priv: %d ", gdt_ptr->dpl);
        debug("present: %d ", gdt_ptr->p);
        debug("avl: %d ", gdt_ptr->avl);
        debug("longmode: %d ", gdt_ptr->l);
        debug("default: %d ", gdt_ptr->d);
        debug("gran: %d ", gdt_ptr->g);
        debug("\n");
        gdt_ptr++;
        i++;
    }
}


void tp() {
    gdt_reg_t gdtr;
	get_gdtr(gdtr);
    print_gdt_content(gdtr);

    debug("CS: %d, ", get_cs());
    debug("SS: %d, ", get_ss());
    debug("DS: %d, ", get_ds());
    debug("ES: %d, ", get_es());
    debug("FS: %d, ", get_fs());
    debug("GS: %d\n", get_gs());

    // ========
    // NEW GDTR
    // ========
    seg_desc_t descs[6];

    // desc0 = NULL
    descs[0].raw = 0;

    // Code, 32 bits RX, flat, ring 0
    descs[1].limit_1 = 0xFFFF;
    descs[1].base_1 = 0x0000;
    descs[1].base_2 = 0x00;
    descs[1].type = SEG_DESC_CODE_XR;
    descs[1].s = 1;
    descs[1].dpl = 0;
    descs[1].p = 1;
    descs[1].limit_2 = 0xF;
    descs[1].avl = 0;
    descs[1].l = 0;
    descs[1].d = 1;
    descs[1].g = 1;
    descs[1].base_3 = 0x00;
    // seg_sel_t seg1 = {.index = 1, .rpl = 0, .ti = 0};

    // Données, 32 bits RW, ring 0
    descs[2].limit_1 = 0xFFFF;
    descs[2].base_1 = 0x0000;
    descs[2].base_2 = 0x00;
    descs[2].type = SEG_DESC_DATA_RW;
    descs[2].s = 1;
    descs[2].dpl = 0;
    descs[2].p = 1;
    descs[2].limit_2 = 0xF;
    descs[2].avl = 0;
    descs[2].l = 0;
    descs[2].d = 1;
    descs[2].g = 1;
    descs[2].base_3 = 0x00;
    seg_sel_t seg2 = {.index = 2, .rpl = 0, .ti = 0};

    // Données, 32 bits RW, base 0x600000, limite 32 octets, ring 0
    descs[3].limit_1 = 0x20;
    descs[3].base_1 = 0x0000;
    descs[3].base_2 = 0x60;
    descs[3].type = SEG_DESC_DATA_RW;
    descs[3].s = 1;
    descs[3].dpl = 0;
    descs[3].p = 1;
    descs[3].limit_2 = 0x0;
    descs[3].avl = 0;
    descs[3].l = 0;
    descs[3].d = 1;
    descs[3].g = 0;
    descs[3].base_3 = 0x00;
    seg_sel_t seg3 = {.index = 3, .rpl = 0, .ti = 0};

    // Code, 32 bits RX, ring 3, flat
    descs[4].limit_1 = 0xFFFF;
    descs[4].base_1 = 0x0000;
    descs[4].base_2 = 0x00;
    descs[4].type = SEG_DESC_CODE_XR;
    descs[4].s = 1;
    descs[4].dpl = 3;
    descs[4].p = 1;
    descs[4].limit_2 = 0xF;
    descs[4].avl = 0;
    descs[4].l = 0;
    descs[4].d = 1;
    descs[4].g = 1;
    descs[4].base_3 = 0x00;
    seg_sel_t seg4 = {.index = 4, .rpl = 3, .ti = 0};

    // Data, 32 bits RW, ring 3, flat
    descs[5].limit_1 = 0xFFFF;
    descs[5].base_1 = 0x0000;
    descs[5].base_2 = 0x00;
    descs[5].type = SEG_DESC_DATA_RW;
    descs[5].s = 1;
    descs[5].dpl = 3;
    descs[5].p = 1;
    descs[5].limit_2 = 0xF;
    descs[5].avl = 0;
    descs[5].l = 0;
    descs[5].d = 1;
    descs[5].g = 1;
    descs[5].base_3 = 0x00;
    seg_sel_t seg5 = {.index = 5, .rpl = 3, .ti = 0};

    gdt_reg_t new_gdtr;
    new_gdtr.desc = descs;
    new_gdtr.limit = sizeof(descs);

    set_gdtr(new_gdtr);

    get_gdtr(gdtr);
    print_gdt_content(gdtr);

    set_ds(seg2);
    set_ss(seg2);
    set_es(seg3);
    set_fs(seg2);
    set_gs(seg2);
    // set_cs(0x1 << 3);

    // Copie du sujet
    char  src[64];
    char *dst = 0;
    // memset(src, 0xff, 32);
    _memcpy8(dst, src, 32);

    set_ds(seg5);
    // set_ss(seg5);
    set_es(seg3);
    set_fs(seg5);
    set_gs(seg5);
    
    fptr32_t farjump = {.offset = (uint32_t)&userland, .segment = seg4.raw};
    farjump(farjump);
    // set_cs(0x4 << 3);
}
