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
	t->version = _vsn;						\
	t->leaf = _leaf;						\
	t->subleaf = _subleaf;						\
	t->regs._reg.mask = BIT(bnr);					\
	t->regs._reg.expect = BIT(bnr) * (_val);			\
	t->tdcs_td_ctl = _td_ctl;					\
	t->tdcs_feature_pv_ctl = _pv_ctl;				\
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
	t->tdcs_td_ctl = _td_ctl;					\
	t->tdcs_feature_pv_ctl = _pv_ctl;				\
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
	t->tdcs_td_ctl = _td_ctl;						\
	t->tdcs_feature_pv_ctl = _pv_ctl;					\
	list_add_tail(&t->list, &cpuid_list);					\
} while (0)

#ifdef AUTOGEN_CPUID
extern void initial_cpuid(void);
#else
void initial_cpuid(void)
{
        /* CPUID(0x0) */
        EXP_CPUID_BYTE(0x0, 0, eax, 0x00000021, VER1_0 | VER2_0, 0, 0); //"MaxIndex"
        EXP_CPUID_BYTE(0x0, 0, eax, 0x00000023, VER1_5, 0, 0);  //"MaxIndex"
        EXP_CPUID_BYTE(0x0, 0, ebx, 0x756e6547, VER1_5, 0, 0);  //"Genu"
        EXP_CPUID_BYTE(0x0, 0, ecx, 0x6c65746e, VER1_5, 0, 0);  //"ntel"
        EXP_CPUID_BYTE(0x0, 0, edx, 0x49656e69, VER1_5, 0, 0);  //"ineI"
								//
        /* CPUID(0x2) */
//        EXP_CPUID_BYTE(0x2, 0, eax, 0, VER1_5, 0, 0);
//        EXP_CPUID_BYTE(0x2, 0, eax, 0x00feff01, VER1_5, 4, 0);
//        EXP_CPUID_BYTE(0x2, 0, ebx, 0, VER1_5, 4, 0);
//        EXP_CPUID_BYTE(0x2, 0, ecx, 0, VER1_5, 4, 0);
//        EXP_CPUID_BYTE(0x2, 0, edx, 0, VER1_5, 4, 0);

        /* CPUID(0x1f) */
        EXP_CPUID_BYTE(0x1f, 0, eax, 0, VER1_5, 0, 0);
        EXP_CPUID_BYTE(0x1f, 0, ebx, 0, VER1_5, 0, 0);
        EXP_CPUID_BYTE(0x1f, 0, ecx, 0, VER1_5, 0, 0);
        EXP_CPUID_BYTE(0x1f, 0, edx, 0, VER1_5, 0, 0);
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
}
#endif
