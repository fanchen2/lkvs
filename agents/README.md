# Case Migration Framework - Directory Structure

完全重构后的目录结构 (方案B)

```
agents/
├── agents/                                          ← Agent定义
│   └── case_migration_framework.agent.md           ← 主协调agent
│   └── avocado_case_runner.agent.md                ← 用例执行agent（先bootstrap，再改case名执行）
│
├── skills/                                          ← 可复用技能库
│   ├── VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL.md       ① 分析legacy case语义
│   ├── LKVS_PARAMETER_MAPPING_SKILL.md            ② 构建参数映射表
│   ├── NAMING_NORMALIZATION_SKILL.md              ③ 命名规范化
│   ├── LKVS_CFG_TRANSLATION_SKILL.md              ④ cfg配置翻译
│   └── LKVS_TEST_IMPLEMENTATION_SKILL.md          ⑤ Python测试实现/验证
│
└── guides/                                          ← 使用指南
    ├── CASE_MIGRATION_QUICK_START.md              ← 快速入门（推荐首读）
    └── CASE_MIGRATION_ARCHITECTURE.md             ← 架构详解
```

## 使用指南

### 快速开始
1. **首次使用** → 阅读 [Quick Start Guide](guides/CASE_MIGRATION_QUICK_START.md)
2. **理解架构** → 阅读 [Architecture Guide](guides/CASE_MIGRATION_ARCHITECTURE.md)
3. **运行agent** → 使用 [Main Agent](agents/case_migration_framework.agent.md)
4. **执行指定case** → 使用 [Avocado Case Runner](agents/avocado_case_runner.agent.md)

### 具体任务

| 任务 | 对应Skill |
|------|-----------|
| 理解legacy case的语义 | [VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL](skills/VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL.md) |
| 处理参数单位转换、映射 | [LKVS_PARAMETER_MAPPING_SKILL](skills/LKVS_PARAMETER_MAPPING_SKILL.md) |
| 统一case命名规范 | [NAMING_NORMALIZATION_SKILL](skills/NAMING_NORMALIZATION_SKILL.md) |
| 编写LKVS cfg配置 | [LKVS_CFG_TRANSLATION_SKILL](skills/LKVS_CFG_TRANSLATION_SKILL.md) |
| 实现或验证Python测试 | [LKVS_TEST_IMPLEMENTATION_SKILL](skills/LKVS_TEST_IMPLEMENTATION_SKILL.md) |

### 修改现有配置

- **Boot repeat相关** → 参考 [case_migration_framework.agent.md](agents/case_migration_framework.agent.md)
- **Memory sweep相关** → 参考 [case_migration_framework.agent.md](agents/case_migration_framework.agent.md)
- **按固定流程执行单个case（先 `avocado vt-bootstrap --vt-type qemu`）** → 参考 [avocado_case_runner.agent.md](agents/avocado_case_runner.agent.md)

## 关键特点

✅ **清晰的概念分离**
- Agents: 协调和工作流
- Skills: 专业化工具库
- Guides: 使用文档

✅ **完整的交叉引用**
- 所有文件都包含YAML frontmatter，指向相关资源
- 内部所有链接都使用相对路径

✅ **易于导航**
- 从任何文件都能快速跳转到相关资源
- 新人可以从Guides 快速上手

## 示例迁移流程

```
已知: 需要迁移legacy case "tdx_vmx2_from1024m_toall"

1️⃣  打开 guides/CASE_MIGRATION_QUICK_START.md
    ↓ (告诉你需要哪个skill)

2️⃣  运行 SKILL 1: VMM_TREE_LEGACY_CASE_ANALYSIS_SKILL
    ↓ (产出: legacy case语义)

3️⃣  运行 SKILL 2: LKVS_PARAMETER_MAPPING_SKILL
    ↓ (产出: 参数映射表)

4️⃣  运行 SKILL 3: NAMING_NORMALIZATION_SKILL
    ↓ (产出: LKVS命名)

5️⃣  运行 SKILL 4: LKVS_CFG_TRANSLATION_SKILL
    ↓ (产出: 可用的cfg)

6️⃣  运行 SKILL 5: LKVS_TEST_IMPLEMENTATION_SKILL
    ↓ (产出: 验证结果)

✓  提交!
```

## 常见问题

**Q: 我不知道应该从哪里开始？**
A: 打开 [Quick Start Guide](guides/CASE_MIGRATION_QUICK_START.md)

**Q: 我想了解整个框架的结构？**
A: 读 [Architecture Guide](guides/CASE_MIGRATION_ARCHITECTURE.md)

**Q: 我需要做参数映射，具体怎么做？**
A: 使用 [LKVS_PARAMETER_MAPPING_SKILL](skills/LKVS_PARAMETER_MAPPING_SKILL.md)

**Q: 现有的boot_repeat case需要调整，怎么做？**
A: 参考 [case_migration_framework.agent.md](agents/case_migration_framework.agent.md)

---

## 版本信息

- **Framework Version**: 1.0 (2024-03-25)
- **Structure Version**: 2 (方案B - 分离结构)
- **Last Updated**: 2026-03-25

所有文件都已交叉引用，完全可导航！🎉
