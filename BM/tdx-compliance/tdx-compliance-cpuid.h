/* SPDX-License-Identifier: GPL-2.0-only */
#include "tdx-compliance.h"

#define DEF_CPUID_TEST(_leaf, _subleaf)		\
{						\
	.name = "CPUID_" #_leaf "_" #_subleaf,	\
	.leaf = _leaf,				\
	.subleaf = _subleaf,			\
}

#define EXP_CPUID_BIT(_leaf, _subleaf, _reg, _bit_nr, _val, _vsn, _td_ctl, _pv_ctl) do {	\
	struct test_cpuid *t;						\
	int bnr = _bit_nr;						\
	t = kzalloc(sizeof(struct test_cpuid), GFP_KERNEL);		\
	t->name = "CPUID(" #_leaf "," #_subleaf ")." #_reg "[" #_bit_nr "]";\
	t->version = (_vsn);						\
	t->leaf = (_leaf);						\
	t->subleaf = (_subleaf);					\
	t->regs._reg.mask = BIT(bnr);					\
	t->regs._reg.expect = BIT(bnr) * (_val);			\
        t->tdcs_td_ctl = (_td_ctl);                                       \
        t->tdcs_feature_pv_ctl = (_pv_ctl);                               \
	list_add_tail(&t->list, &cpuid_list);				\
} while (0)

#define EXP_CPUID_BYTE(_leaf, _subleaf, _reg, _val, _vsn, _td_ctl, _pv_ctl) do {		\
	struct test_cpuid *t;						\
	t = kzalloc(sizeof(struct test_cpuid), GFP_KERNEL);		\
	t->name = "CPUID(" #_leaf "," #_subleaf ")." #_reg;		\
	t->version = _vsn;						\
	t->leaf = _leaf;						\
	t->subleaf = _subleaf;						\
	t->regs._reg.mask = 0xffffffff;					\
	t->regs._reg.expect = (_val);					\
        t->tdcs_td_ctl = _td_ctl;                                       \
        t->tdcs_feature_pv_ctl = _pv_ctl;                               \
	list_add_tail(&t->list, &cpuid_list);				\
} while (0)

#define EXP_CPUID_RES_BITS(_leaf, _subleaf, _reg, _bit_s, _bit_e, _vsn, _td_ctl, _pv_ctl) do {	\
	int i = 0;								\
	struct test_cpuid *t;							\
	t = kzalloc(sizeof(struct test_cpuid), GFP_KERNEL);			\
	t->name = "CPUID(" #_leaf "," #_subleaf ")." #_reg "[" #_bit_e ":" #_bit_s "]";\
	t->version = _vsn;							\
	t->leaf = _leaf;							\
	t->subleaf = _subleaf;							\
	for (i = _bit_s; i <= (_bit_e); i++) {					\
		t->regs._reg.mask |= BIT(i);					\
	}									\
        t->tdcs_td_ctl = _td_ctl;                                               \
        t->tdcs_feature_pv_ctl = _pv_ctl;                                       \
	list_add_tail(&t->list, &cpuid_list);					\
} while (0)

#ifdef AUTOGEN_CPUID
extern void initial_cpuid(void);
#else
void initial_cpuid(void)
{
        /* CPUID(0x1) */
        /* EST(est) */
        EXP_CPUID_BIT(0x1, 0, ecx, 7, 0x1, VER1_5, 0, 0); //Backward-Compatible, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, ecx, 7, 0x0, VER1_5, 0, 0); //Backward-Compatible, disable it by qemu
        EXP_CPUID_BIT(0x1, 0, ecx, 7, 0x0, VER1_5, 8, 0); //Reduced-#VE, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, ecx, 7, 0x1, VER1_5, 8, BIT(2)); //Reduced-#VE + Paravirtualization, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, ecx, 7, 0x0, VER1_5, 8, BIT(2)); //Reduced-#VE + Paravirtualization, disable it by qemu
        /* TSC_DEADLINE(tsc-deadline) */
        EXP_CPUID_BIT(0x1, 0, ecx, 24, 0x1, VER1_5, 0, 0); //Backward-Compatible, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, ecx, 24, 0x0, VER1_5, 0, 0); //Backward-Compatible, disable it by qemu
        EXP_CPUID_BIT(0x1, 0, ecx, 24, 0x0, VER1_5, 8, 0); //Reduced-#VE, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, ecx, 24, 0x1, VER1_5, 8, BIT(11)); //Reduced-#VE + Paravirtualization, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, ecx, 24, 0x0, VER1_5, 8, BIT(11)); //Reduced-#VE + Paravirtualization, disable it by qemu
        /* MCA(mce) */
        EXP_CPUID_BIT(0x1, 0, edx, 7, 0x1, VER1_5, 0, 0); //Backward-Compatible
        EXP_CPUID_BIT(0x1, 0, edx, 7, 0x0, VER1_5, 8, 0); //Reduced-#VE, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, edx, 7, 0x1, VER1_5, 8, BIT(3)); //Reduced-#VE + Paravirtualization, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, edx, 7, 0x0, VER1_5, 8, BIT(3)); //Reduced-#VE + Paravirtualization, disable it by qemu
        /* MTRR(mtrr) */
        EXP_CPUID_BIT(0x1, 0, edx, 12, 0x1, VER1_5, 0, 0); //Backward-Compatible
        EXP_CPUID_BIT(0x1, 0, edx, 12, 0x0, VER1_5, 8, 0); //Reduced-#VE, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, edx, 12, 0x1, VER1_5, 8, BIT(4)); //Reduced-#VE + Paravirtualization, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, edx, 12, 0x0, VER1_5, 8, BIT(4)); //Reduced-#VE + Paravirtualization, disable it by qemu
        /* MCA(mca) */
        EXP_CPUID_BIT(0x1, 0, edx, 14, 0x1, VER1_5, 0, 0); //Backward-Compatible, MCA(mca)
        EXP_CPUID_BIT(0x1, 0, edx, 14, 0x0, VER1_5, 8, 0); //Reduced-#VE, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, edx, 14, 0x1, VER1_5, 8, BIT(3)); //Reduced-#VE + Paravirtualization, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, edx, 14, 0x0, VER1_5, 8, BIT(3)); //Reduced-#VE + Paravirtualization, disable it by qemu
        /* TM(acpi) */
        EXP_CPUID_BIT(0x1, 0, edx, 22, 0x1, VER1_5, 0, 0); //Backward-Compatible, TM(acpi), enable it by qemu
        EXP_CPUID_BIT(0x1, 0, edx, 22, 0x0, VER1_5, 0, 0); //Backward-Compatible, disable it by qemu
        EXP_CPUID_BIT(0x1, 0, edx, 22, 0x0, VER1_5, 8, 0); //Reduced-#VE, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, edx, 22, 0x1, VER1_5, 8, BIT(8)); //Reduced-#VE + Paravirtualization, enable it by qemu
        EXP_CPUID_BIT(0x1, 0, edx, 22, 0x0, VER1_5, 8, BIT(8)); //Reduced-#VE + Paravirtualization, disable it by qemu

        /* CPUID(0x2) */
        EXP_CPUID_BYTE(0x2, 0, eax, 0, VER1_5, 0, 0); //Backward-Compatible, trigger #VE
        EXP_CPUID_BYTE(0x2, 0, eax, 0x00feff01, VER1_5, 4, 0); //VIRT_CPUID2
        EXP_CPUID_BYTE(0x2, 0, ebx, 0, VER1_5, 4, 0); //VIRT_CPUID2
        EXP_CPUID_BYTE(0x2, 0, ecx, 0, VER1_5, 4, 0); //VIRT_CPUID2
        EXP_CPUID_BYTE(0x2, 0, edx, 0, VER1_5, 4, 0); //VIRT_CPUID2
        EXP_CPUID_BYTE(0x2, 0, eax, 0x00feff01, VER1_5, 8, 0); //Reduced-#VE
        EXP_CPUID_BYTE(0x2, 0, ebx, 0, VER1_5, 8, 0); //Reduced-#VE
        EXP_CPUID_BYTE(0x2, 0, ecx, 0, VER1_5, 8, 0); //Reduced-#VE
        EXP_CPUID_BYTE(0x2, 0, edx, 0, VER1_5, 8, 0); //Reduced-#VE

        /* CPUID(0x6) */
        EXP_CPUID_BIT(0x6, 0, eax, 2, 0x0, VER1_5, 0, 0); //Backward-Compatible, trigger #VE
        EXP_CPUID_BIT(0x6, 0, eax, 2, 0x1, VER1_5, 8, 0); //Reduced-#VE
        EXP_CPUID_RES_BITS(0x6, 0, eax, 0, 1, 0x0, VER1_5, 8, 0); //Reduced-#VE
        EXP_CPUID_RES_BITS(0x6, 0, eax, 3, 31, 0x0, VER1_5, 8, 0); //Reduced-#VE
        EXP_CPUID_BYTE(0x6, 0, ebx, 0x0, VER1_5, 8, 0); //Reduced-#VE
        EXP_CPUID_BYTE(0x6, 0, ecx, 0x0, VER1_5, 8, 0); //Reduced-#VE
        EXP_CPUID_BYTE(0x6, 0, edx, 0x0, VER1_5, 8, 0); //Reduced-#VE

        /* CPUID(0x7) */
        /* CORE_CAPABILITIES(core-capability) */
        EXP_CPUID_BIT(0x7, 0, edx, 30, 0x1, VER1_5, 0, 0); //Backward-Compatible
        EXP_CPUID_BIT(0x7, 0, edx, 30, 0x0, VER1_5, 8, 0); //Reduced-#VE, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, edx, 30, 0x1, VER1_5, 8, BIT(0)); //Reduced-#VE + Paravirtualization, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, edx, 30, 0x0, VER1_5, 8, BIT(0)); //Reduced-#VE + Paravirtualization, disable it by qemu
        /* RDT_M(pqm) */
        EXP_CPUID_BIT(0x7, 0, ebx, 12, 0x1, VER1_5, 0, 0); //Backward-Compatible, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, ebx, 12, 0x0, VER1_5, 0, 0); //Backward-Compatible, disable it by qemu
        EXP_CPUID_BIT(0x7, 0, ebx, 12, 0x0, VER1_5, 8, 0); //Reduced-#VE, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, ebx, 12, 0x1, VER1_5, 8, BIT(7)); //Reduced-#VE + Paravirtualization, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, ebx, 12, 0x0, VER1_5, 8, BIT(7)); //Reduced-#VE + Paravirtualization, disable it by qemu
        /* RDT_A(rdta) */
        EXP_CPUID_BIT(0x7, 0, ebx, 15, 0x1, VER1_5, 0, 0); //Backward-Compatible, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, ebx, 15, 0x0, VER1_5, 0, 0); //Backward-Compatible, disable it by qemu
        EXP_CPUID_BIT(0x7, 0, ebx, 15, 0x0, VER1_5, 8, 0); //Reduced-#VE, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, ebx, 15, 0x1, VER1_5, 8, BIT(6)); //Reduced-#VE + Paravirtualization, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, ebx, 15, 0x0, VER1_5, 8, BIT(6)); //Reduced-#VE + Paravirtualization, disable it by qemu
        /* PCONFIG(pconfig) */
        EXP_CPUID_BIT(0x7, 0, edx, 18, 0x1, VER1_5, 0, 0); //Backward-Compatible, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, edx, 18, 0x0, VER1_5, 0, 0); //Backward-Compatible, disable it by qemu
        EXP_CPUID_BIT(0x7, 0, edx, 18, 0x0, VER1_5, 8, 0); //Reduced-#VE, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, edx, 18, 0x1, VER1_5, 8, BIT(5)); //Reduced-#VE + Paravirtualization, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, edx, 18, 0x0, VER1_5, 8, BIT(5)); //Reduced-#VE + Paravirtualization, disable it by qemu
        /* TME(tme) */
        EXP_CPUID_BIT(0x7, 0, ecx, 13, 0x1, VER1_5, 0, 0); //Backward-Compatible, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, ecx, 13, 0x0, VER1_5, 0, 0); //Backward-Compatible, disable it by qemu
        EXP_CPUID_BIT(0x7, 0, ecx, 13, 0x0, VER1_5, 8, 0); //Reduced-#VE, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, ecx, 13, 0x1, VER1_5, 8, BIT(10)); //Reduced-#VE + Paravirtualization, enable it by qemu
        EXP_CPUID_BIT(0x7, 0, ecx, 13, 0x0, VER1_5, 8, BIT(10)); //Reduced-#VE + Paravirtualization, disable it by qemu

        /* CPUID(0x9), enumerated by virtual CPUID(1).ECX[18] */
        /* DCA(dca) */
        EXP_CPUID_BIT(0x1, 0, ecx, 18, 0x1, VER1_5, 0, 0); //Backward-Compatible, enable it by qemu
        EXP_CPUID_BYTE(0x9, 0, eax, 0x0, VER1_5, 0, 0); //Backward-Compatible, VM_CTLS.CPUID_VE_DISABLE is false, trigger #VE

        EXP_CPUID_BIT(0x1, 0, ecx, 18, 0x0, VER1_5, 0, 0); //Backward-Compatible, disable it by qemu
        EXP_CPUID_BYTE(0x9, 0, eax, 0x0, VER1_5, 0, 0); //Backward-Compatible, VM_CTLS.CPUID_VE_DISABLE is false, trigger #VE

        EXP_CPUID_BIT(0x1, 0, ecx, 18, 0x0, VER1_5, 8, 0); //Reduced-#VE, enable it by qemu
        EXP_CPUID_BYTE(0x9, 0, eax, 0x0, VER1_5, 8, 0); //Reduced-#VE, virtual CPUID(1).ECX[18] == 0

        EXP_CPUID_BIT(0x1, 0, ecx, 18, 0x0, VER1_5, 8, BIT(1)); //Reduced-#VE + Paravirtualization, disable it by qemu
        EXP_CPUID_BYTE(0x9, 0, eax, 0x0, VER1_5, 8, BIT(1)); //Reduced-#VE + Paravirtualization, virtual CPUID(1).ECX[18] == 0
        EXP_CPUID_BYTE(0x9, 0, ebx, 0x0, VER1_5, 8, BIT(1)); //Reduced-#VE + Paravirtualization, virtual CPUID(1).ECX[18] == 0
        EXP_CPUID_BYTE(0x9, 0, ecx, 0x0, VER1_5, 8, BIT(1)); //Reduced-#VE + Paravirtualization, virtual CPUID(1).ECX[18] == 0
        EXP_CPUID_BYTE(0x9, 0, edx, 0x0, VER1_5, 8, BIT(1)); //Reduced-#VE + Paravirtualization, virtual CPUID(1).ECX[18] == 0

        EXP_CPUID_BIT(0x1, 0, ecx, 18, 0x1, VER1_5, 8, BIT(1)); //Reduced-#VE + Paravirtualization, enable it by qemu
        EXP_CPUID_BYTE(0x9, 0, eax, 0x0, VER1_5, 8, BIT(1)); //Reduced-#VE + Paravirtualization, virtual CPUID(1).ECX[18] != 0, trigger #VE

        /* CPUID(0xb), Per SDM, CPUID(0x1F, *) is a preferred superset to leaf CPUID(0xB,*) */
        /* -smp 8,threads=2,cores=4 cpu0 */
        EXP_CPUID_BYTE(0xb, 0, eax, 0, VER1_5, 0, 0); //Backward-Compatible, trigger #VE
        EXP_CPUID_BYTE(0xb, 0, eax, 0x1, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 0, ebx, 0x2, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 0, ecx, 0x0100, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 0, edx, 0, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 1, eax, 0x3, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 1, ebx, 0x8, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 1, ecx, 0x0201, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 1, edx, 0, VER1_5, 2, 0);

        EXP_CPUID_BYTE(0xb, 0, eax, 0x1, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 0, ebx, 0x2, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 0, ecx, 0x0100, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 0, edx, 0, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 1, eax, 0x3, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 1, ebx, 0x8, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 1, ecx, 0x0201, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 1, edx, 0, VER1_5, 8, 0);

        /* CPUID(0xb), Per SDM, CPUID(0x1F, *) is a preferred superset to leaf CPUID(0xB,*) */
        /* -smp 12,threads=3,cores=4 cpu11 */
        EXP_CPUID_BYTE(0xb, 0, eax, 0, VER1_5, 0, 0); //Backward-Compatible, trigger #VE
        EXP_CPUID_BYTE(0xb, 0, eax, 0x2, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 0, ebx, 0x3, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 0, ecx, 0x0100, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 0, edx, 0xe, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 1, eax, 0x4, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 1, ebx, 0xc, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 1, ecx, 0x0201, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0xb, 1, edx, 0xe, VER1_5, 2, 0);

        EXP_CPUID_BYTE(0xb, 0, eax, 0x2, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 0, ebx, 0x3, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 0, ecx, 0x0100, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 0, edx, 0xe, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 1, eax, 0x4, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 1, ebx, 0xc, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 1, ecx, 0x0201, VER1_5, 8, 0);
        EXP_CPUID_BYTE(0xb, 1, edx, 0xe, VER1_5, 8, 0);

        /* CPUID(0xc) Reserved */
        EXP_CPUID_BYTE(0xc, 0, eax, 0, VER1_5, 0, 0); //Backward-Compatible, trigger #VE
        EXP_CPUID_BYTE(0xc, 0, eax, 0x0, VER1_5, 8, 0); //Reduced-#VE
        EXP_CPUID_BYTE(0xc, 0, ebx, 0x0, VER1_5, 8, 0); //Reduced-#VE
        EXP_CPUID_BYTE(0xc, 0, ecx, 0x0, VER1_5, 8, 0); //Reduced-#VE
        EXP_CPUID_BYTE(0xc, 0, edx, 0x0, VER1_5, 8, 0); //Reduced-#VE

        /* CPUID(0x1f) -smp 8,threads=2,cores=4 cpu0 */
        EXP_CPUID_BYTE(0x1f, 0, eax, 0, VER1_5, 0, 0); //Backward-Compatible, trigger #VE
        EXP_CPUID_BYTE(0x1f, 0, eax, 0x1, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 0, ebx, 0x2, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 0, ecx, 0x0100, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 0, edx, 0, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 1, eax, 0x3, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 1, ebx, 0x8, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 1, ecx, 0x0201, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 1, edx, 0, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 2, eax, 0x3, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 2, ebx, 0x8, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 2, ecx, 0x0502, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 2, edx, 0, VER1_5, 2, 0);

        /* CPUID(0x1f) -smp 8,threads=3,cores=4 cpu11 */
        EXP_CPUID_BYTE(0x1f, 0, eax, 0, VER1_5, 0, 0); //Backward-Compatible, trigger #VE
        EXP_CPUID_BYTE(0x1f, 0, eax, 0x2, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 0, ebx, 0x3, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 0, ecx, 0x0100, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 0, edx, 0xe, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 1, eax, 0x4, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 1, ebx, 0xc, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 1, ecx, 0x0201, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 1, edx, 0xe, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 2, eax, 0x4, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 2, ebx, 0xc, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 2, ecx, 0x0502, VER1_5, 2, 0);
        EXP_CPUID_BYTE(0x1f, 2, edx, 0xe, VER1_5, 2, 0);
}
#endif
