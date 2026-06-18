# Synera: Synergy Auto-Arena

基于 Qt6 与 C++17 的单机 PVE 自走棋游戏，南京大学高级程序设计课程 PA 项目。

---

## 1. 基本信息

| 项目 | 内容 |
|------|------|
| 姓名 | *** |
| 学号 | 2518***** |
| 项目名称 | Synera: Synergy Auto-Arena |
| 开发环境 | Qt 6.8.3 + MSVC 2022 + C++17 + CMake 3.16+ |

---

## 2. 文件树结构

```
Synera_starter/
├── README.md                         # 本文档
├── CMakeLists.txt                    # CMake 构建配置 (Qt6, C++17)
├── PA说明文档.pdf                     # 课程 PA 要求文档
├── assets/                           # 美术资源（精灵图、序列帧）
│   ├── black_e/normal/               #   精灵图
│   ├── white_e/normal/               #   
│   ├── gugugaga/normal/              #   
│   ├── hihihihi/normal/              #   
│   ├── kafuka/                       #   
│   ├── craftpix-reaper-man-*/        #   
│   ├── craftpix-satyr-*/             #   
│   └── equipments/                   #   装备图标
├── data/                             # 游戏数据配置 (JSON)
│   ├── units.json                    #   单位模板：基础属性、特质、技能绑定
│   ├── skills.json                   #   技能定义：策略类型、目标规则、数值参数
│   ├── synergies.json                #   羁绊规则：种族/职业的阈值与加成
│   ├── stages.json                   #   关卡配置：敌方波次的单位数量与星级分布
│   ├── equipments.json               #   装备属性：基础数值加成
│   └── bufftypes.json                #   Buff 类型：控制/Dot/属性修正的分类与叠加规则
├── build/                            # CMake 构建输出（提交时剔除）
│   └── Debug/
│       ├── Synera_Starter.exe        #   可执行文件
│       └── saves/                    #   存档目录（运行时生成）
└── src/                              # 源代码
    ├── main.cpp                      #   程序入口：创建 QApplication 与 GameWindow
    ├── core/                         #   核心逻辑层：游戏主控、战斗、寻路、经济
    │   ├── game.h / game.cpp         #     游戏主控制器（状态机、拖拽、场景同步）
    │   ├── board.h / board.cpp       #     棋盘 + 备战席数据结构与占位规则
    │   ├── battlesystem.h / .cpp     #     自动战斗引擎（QTimer 驱动，50ms/tick）
    │   ├── pathfinder.h / .cpp       #     六边形网格寻路（A* 算法）
    │   ├── store.h / store.cpp       #     商店系统（5 槽位刷新与购买）
    │   ├── enemyspawner.h / .cpp     #     敌方波次生成器（读取 stages.json）
    │   └── savemanager.h / .cpp      #     存档/读档管理器（8 槽位，JSON 序列化）
    ├── entity/                       #   实体/数据层：单位、技能、羁绊、装备、Buff 的定义
    │   ├── unit.h / unit.cpp         #     单位实体：属性、状态、Buff、装备、星级
    │   ├── unitdata.h / .cpp         #     单位模板注册表（单例，加载 units.json）
    │   ├── player.h / player.cpp     #     玩家状态（金币、血量、等级、经验）
    │   ├── skill.h                   #     技能抽象基类（纯虚接口）
    │   ├── skillregistry.h / .cpp    #     技能注册表 + 工厂（按 strategy 字段创建）
    │   ├── synergyregistry.h / .cpp  #     羁绊规则注册表（阈值匹配与加成计算）
    │   ├── equipment.h               #     装备数据结构（值对象）
    │   ├── equipmentdata.h / .cpp    #     装备注册表（单例，加载 equipments.json）
    │   ├── buffregistry.h / .cpp     #     Buff 类型注册表（分类与叠加规则）
    │   └── skills/                   #     具体技能实现
    │       ├── SingleTargetedSkill   #       单体目标技能（伤害/治疗）
    │       ├── MultiTargetedSkill    #       多目标 / AoE 技能（支持溅射）
    │       └── BuffSingleTargetSkill #       带 Buff 附着的单体技能（眩晕、dot，增益减益）
    └── gui/                           #   Qt 渲染/交互层
        ├── gamewindow.h / .cpp       #     主窗口 (QMainWindow, 1400x850)
        ├── griditem.h / .cpp         #     六边形格子图元 (QGraphicsObject)
        ├── unititem.h / .cpp         #     单位图元（精灵渲染、HP/蓝条、拖拽交互）
        ├── storeslotitem.h / .cpp    #     商店槽位图元（点击购买）
        ├── storerefreshbutton.h/.cpp #     商店刷新按钮
        ├── sellzoneitem.h / .cpp     #     出售区域（拖拽单位至此出售）
        ├── equipbar.h / .cpp         #     装备栏管理器
        ├── equipslotitem.h / .cpp    #     装备槽图元（拖拽穿戴/卸下）
        ├── lootdialog.h / .cpp       #     战利品选择弹窗（装备溢出时取舍）
        ├── settlementdialog.h / .cpp #     战斗结算弹窗
        └── saveloaddialog.h / .cpp   #     存档/读档界面（8 槽位选择）
```

### 核心文件夹说明

| 目录 | 说明 |
|------|------|
| `src/core/` | 游戏核心逻辑。包含状态机流转、战斗 tick、寻路、商店、敌方生成和存档等所有运行时逻辑，不依赖 Qt GUI 可独立测试 |
| `src/entity/` | 数据与领域模型。定义 Unit、Skill、Equipment、Synergy、Buff 等数据结构及其 JSON 注册表，负责"是什么"而非"怎么运行" |
| `src/gui/` | Qt 图形界面层。负责所有视觉呈现与鼠标交互，通过信号槽与 core 层解耦 |
| `data/` | JSON 配置数据。所有单位模板、技能参数、羁绊规则、关卡配置等均由此加载，修改数据无需重新编译 |
| `assets/` | 静态美术资源。单位精灵图、装备图标等，运行时按需懒加载 |

---

## 3. 核心类和数据结构

### 3.1 棋盘与单位管理

| 类名 | 文件 | 主要功能 |
|------|------|----------|
| `BoardANDBench` | `core/board.h` | 棋盘 + 备战席数据结构。管理 8x8 棋盘和 1x10 备战区的单位占位，维护 `QVector<Unit*>` 存储数组和 `QHash<Unit*, QPoint>` 反向索引。提供增删、移动、查询、合法性校验等接口 |
| `Unit` | `entity/unit.h` | 单位实体类。包含基础属性（HP/ATK/Range/MaxMana/Mana/Speed）、星级（影响属性倍率：1 星=1.0, 2 星=1.8, 3 星=3.24）、特质标签(traits)、归属(owner)、Buff 列表、装备列表、技能冷却等。所有单位（我方/敌方）统一使用此类，仅通过 `owner` 区分 |
| `UnitData` | `entity/unitdata.h` | 单位模板注册表（单例）。加载 `data/units.json`，提供 `createUnit(key, owner, star)` 工厂方法按模板生成单位实例 |

### 3.2 战斗系统

| 类名                      | 文件                       | 主要功能                                                                                                                                                   |
| ----------------------- | ------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `BattleSystem`          | `core/battlesystem.h`    | 自动战斗引擎。基于 QTimer（50ms/帧）循环驱动，每 tick 依次处理：冷却更新、目标搜索、移动/攻击/技能决策、伤害结算、死亡清理、胜负判定。战斗结束后发射 `battleFinished(BattleResult)` 信号                                 |
| `Pathfinder`            | `core/pathfinder.h`      | 六边形网格 BFS 寻路（静态工具类）。使用立方体坐标系统（x+y+z=0）进行六边形距离计算和邻居遍历，返回最短路径供单位移动                                                                                       |
| `Skill`                 | `entity/skill.h`         | 技能抽象基类。定义纯虚接口：`targetType()`（敌方/友方/自身）、`selectType()`（最近/最低血量/最高攻击）、`castRange()`、`selectTargets()`、`calculateValue()`、`execute()`。每个具体技能重写这些方法实现差异化行为 |
| `SkillRegistry`         | `entity/skillregistry.h` | 技能注册表（单例）。加载 `data/skills.json`，根据 `strategy` 字段工厂创建 `SingleTargetedSkill` / `MultiTargetedSkill` / `BuffSingleTargetSkill` 实例                         |
| `SingleTargetedSkill`   | `entity/skills/`         | 单体目标技能。选择一个目标单位，计算伤害/治疗值后执行                                                                                                                            |
| `MultiTargetedSkill`    | `entity/skills/`         | 多目标/AoE 技能。以施法者或目标为范围中心，对范围内所有合法目标造成伤害（支持溅射比例 `splashRatio`）                                                                                           |
| `BuffSingleTargetSkill` | `entity/skills/`         | 附带 Buff 的单体技能。在造成伤害/治疗后，向目标附加 JSON 配置中定义的 Buff（眩晕/Dot/属性修正等）                                                                                           |
| `EnemySpawner`          | `core/enemyspawner.h`    | 敌方波次生成器（单例）。读取 `data/stages.json`，按回合索引生成包含单位 key、星级、位置、装备的 `EnemySpawnPlan` 列表                                                                        |

### 3.3 经济与商店

| 类名 | 文件 | 主要功能 |
|------|------|----------|
| `Player` | `entity/player.h` | 玩家状态。维护金币、血量、等级（最高 7 级）、经验值与升级经验曲线（`xpToNext = 4 + (level-1)*2`） |
| `Store` | `core/store.h` | 商店系统。管理 5 个招募槽位的 `std::unique_ptr<Unit>`，支持随机刷新（消耗 2 金币）和购买（消耗对应金币，落位至备战区） |

### 3.4 羁绊系统

| 类名 | 文件 | 主要功能 |
|------|------|----------|
| `SynergyRegistry` | `entity/synergyregistry.h` | 羁绊规则注册表（单例）。加载 `data/synergies.json`，按 trait 名称维护阈值列表。`getBonus(trait, count)` 根据场上同 trait 单位数量取最高满足阈值的 `SynergyBonus`（含 ATK/HP/Mana/Speed/DotIntervalReduction/MoveSpeed 加成），多 trait 单位可叠加多个加成 |
| `SynergyBonus` | `entity/synergyregistry.h` | 羁绊加成数据结构。包含各项战斗属性的加值，重载 `operator+=` 支持多 trait 加成叠加 |
| `SynergyThreshold` | `entity/synergyregistry.h` | 单个羁绊阈值。包含所需单位数量 `count` 和对应的 `SynergyBonus` |

### 3.5 装备系统

| 类名 | 文件 | 主要功能 |
|------|------|----------|
| `Equipment` | `entity/equipment.h` | 装备值对象。包含 key、名称、属性加成（Atk/MaxHp/MaxMana/Speed/AttackSpeed/MoveSpeed）、精灵路径 |
| `EquipmentRegistry` | `entity/equipmentdata.h` | 装备注册表（单例）。加载 `data/equipments.json`，提供 `createEquipment(key)` 工厂方法 |
| `EquipBar` | `gui/equipbar.h` | 装备栏管理器。管理场景中的 `EquipSlotItem` 组件，提供增删查改与溢出处理 |
| `EquipSlotItem` | `gui/equipslotitem.h` | 装备槽图元（QGraphicsObject）。支持拖拽装备到单位和从单位卸下装备 |

### 3.6 Buff 系统

| 类名 | 文件 | 主要功能 |
|------|------|----------|
| `BuffRegistry` | `entity/buffregistry.h` | Buff 类型注册表（单例）。加载 `data/bufftypes.json`，管理 `BuffCategory`（Control/Dot/StatMod）和 `BuffStackRule`（Refresh/UniquePerSource/Independent），决定 Buff 的叠加行为和 tick 逻辑 |

### 3.7 存档系统

| 类名 | 文件 | 主要功能 |
|------|------|----------|
| `SaveManager` | `core/savemanager.h` | 存档管理器（静态类）。提供 8 个存档槽位，将玩家状态、场上/备战区单位、商店、装备栏等信息序列化为 `QJsonObject` 写入文件，并支持完整反序列化恢复 |

### 3.8 GUI 渲染层

| 类名 | 文件 | 主要功能 |
|------|------|----------|
| `GameWindow` | `gui/gamewindow.h` | 主窗口 (QMainWindow, 1400x850)。左侧 QGraphicsView 承载游戏场景，右侧面板展示玩家/敌方信息、轮次、按钮（战斗/刷新/升人口/存档/读档/重开） |
| `GridItem` | `gui/griditem.h` | 六边形格子图元 (QGraphicsObject)。渲染单个六边形地块，支持 hover 高亮和 drop 高亮两种视觉反馈 |
| `UnitItem` | `gui/unititem.h` | 单位图元 (QGraphicsObject)。懒加载 PNG 精灵图，失败时回退占位绘制；显示 HP 条、蓝条、星级标记、装备图标；处理鼠标拖拽事件并发射信号 |
| `StoreSlotItem` | `gui/storeslotitem.h` | 商店槽位图元。六边形样式，显示单位精灵、名称、价格，支持点击购买 |
| `StoreRefreshButton` | `gui/storerefreshbutton.h` | 刷新按钮图元。圆角矩形，显示"Refresh (2G)"，金币不足时变暗 |
| `SellZoneItem` | `gui/sellzoneitem.h` | 出售区域图元。红色矩形区域，拖拽单位至此时高亮反馈 |
| `SettlementDialog` | `gui/settlementdialog.h` | 战斗结算弹窗 (QDialog)。展示胜负结果、HP 变化、金币变化 |
| `LootDialog` | `gui/lootdialog.h` | 战利品选择弹窗 (QDialog)。装备栏满时弹出，展示新获得装备与现有装备，玩家选择保留哪些 |
| `SaveLoadDialog` | `gui/saveloaddialog.h` | 存档/读档界面 (QDialog)。4x2 网格展示 8 个存档槽，显示时间戳/轮次/等级/HP/金币等元数据 |

---

## 4. 数据驱动的词条组合系统

本章的核心目标不是罗列"目前已实现的内容"，而是展示**"通过正交词条的自由组合，可以立即产生多少种新内容"**。所有实体（装备、技能、羁绊、Buff）均由 `data/` 目录下的 JSON 定义；新增一种装备或技能只需在 JSON 中选择词条组合，无需修改 C++ 代码。

### 4.1 装备词条

装备由 6 个独立数值词条自由组合而成（[entity/equipment.h](src/entity/equipment.h) 的 `Equipment` 结构体）。任选若干词条赋非零值，其余保持默认 0，即定义了一件新装备。

**可组合的词条维度：**

| 词条 | 类型 | 含义 | 运行时效果 |
|------|------|------|-----------|
| `bonusAtk` | int | 攻击力加值 | 累加到 `Unit::atk()` |
| `bonusMaxHp` | int | 最大生命值加值 | 累加到 `Unit::maxHp()` |
| `bonusMaxMana` | int | 法力上限修正 | 累加到 `Unit::maxMana()`（负数 = 蓝条缩短 = 更易放技能） |
| `bonusSpeed` | int | 速度加值 | 累加到 `Unit::speed()`，影响移动冲突优先级 |
| `bonusAttackSpeed` | float | 攻速系数 | `effectiveAttackCooldown() = max(1, baseCD / (1 + bonus))` |
| `bonusMoveSpeed` | float | 移速系数 | `effectiveMoveCooldown() = max(1, baseCD / (1 + bonus))` |

每个词条互不依赖——`Unit` 的各属性 getter 中，装备加成（`m_equipBonus*`）与羁绊加成（`m_bonus*`）和 Buff 修正（`statModSum`）三者各自独立累加，最终求和。6 个词条各可为零或非零，理论组合空间为 2⁶ = 64 种。

**已实现的 5 种装备（作为词条组合的示例）：**

| key | 名称 | ATK | HP | Mana | Speed | AtkSpd | MovSpd | 组合说明 |
|-----|------|-----|----|------|-------|--------|--------|----------|
| `bf_sword` | 暴风大剑 | +8 | | | | | | 单维：纯 ATK |
| `giant_belt` | 巨人腰带 | | +70 | | | | | 单维：纯 HP |
| `tear` | 女神之泪 | | | −5 | | | | 单维：纯 Mana |
| `recurve_bow` | 反曲之弓 | +5 | | | | +0.2 | | 二维：ATK + AtkSpd |
| `chain_vest` | 锁子甲 | +5 | +50 | | | | | 二维：ATK + HP |

> **扩展示例**：若想做一件"疾风之靴"（纯移速），只需在 `equipments.json` 中添加：
> ```json
> { "key": "swift_boots", "name": "疾风之靴", "bonusMoveSpeed": 0.3 }
> ```
> ——其余 5 个字段默认为 0，`EquipmentRegistry` 自动解析，C++ 零改动。

### 4.2 技能词条

技能系统是词条组合最丰富的部分。一个技能由 **策略 × 目标 × 数值** 三层正交维度组合而成，Buff 类技能还额外叠加一层 Buff 维度。所有维度均由枚举定义于 [entity/skill.h](src/entity/skill.h)，各枚举值互不耦合——任取一种策略、一种目标阵营、一种选择方式、一种数值公式，即可构成一个有效技能。

#### 4.2.1 第一层：策略（Strategy）—— 决定执行流程

三种策略对应三种 C++ 子类，它们各自的 `execute()` 流程不同（详见 [6. 技能系统设计](#6-技能系统设计)）：

| 策略 | 类 | 执行模式 |
|------|-----|----------|
| `SingleTargeted` | `SingleTargetedSkill` | 选择 1 个目标 → 计算数值 → 立即结算伤害/治疗 |
| `MultiTargeted` | `MultiTargetedSkill` | 选择 1 个主目标 → 以施法者或目标为中心圈定范围 → 范围内所有合法目标各自结算（支持溅射/均摊） |
| `BuffSingleTargeted` | `BuffSingleTargetSkill` | 选择 1 个目标 → 施加 Buff（Buff 自身可带独立数值计算）；**可在 `buffs` 数组中挂载多个 Buff，每个 Buff 独立配置** |

#### 4.2.2 第二层：目标词条（TargetType × SelectType）

这两个枚举决定"技能作用于谁"以及"在候选者中如何挑选"：

| 词条 | 枚举值 | 含义 |
|------|--------|------|
| `TargetType` | `Enemy` / `Ally` / `Self` | 目标阵营：敌方 / 友方 / 仅自身 |
| `SelectType` | `Nearest` / `LowestHp` / `HighestAtk` | 筛选策略：最近 / 最低血量 / 最高攻击力 |

`TargetType` 与 `SelectType` 独立组合。例如 `Enemy × HighestAtk` = "选敌方攻击最高者"（处决）；`Ally × Nearest` = "选最近友方"（dot奶）；`Self × Nearest` = "仅自身"（强化）。3×3 = 9 种组合，各有明确的战术含义。

#### 4.2.3 第三层：数值词条（ValueType）

决定伤害/治疗量的计算方式：

| ValueType | 适用策略 | 计算公式 | 典型用途 |
|-----------|----------|----------|----------|
| `fixed` | 全部 | `value`（固定值） | 固定伤害/治疗，不依赖属性 |
| `AtkRatio` | 全部 | `caster.atk() × ratio` | 依赖施法者攻击力的技能 |
| `HpRatio` | 全部 | `target.maxHp() × ratio` | 斩杀类（对高 HP 目标伤害高）、百分比治疗 |
| `split` | 仅 `MultiTargeted` | `value ÷ N`（固定值均摊到 N 个目标） | AoE 总伤害固定，人多则每人少 |
| `splash` | 仅 `MultiTargeted` | 主目标 = 全额，次要目标 = 全额 × `splashRatio` | 主目标吃满伤害，溅射伤害递减 |

`MultiTargeted` 策略额外支持两个词条：

| 词条 | 含义 |
|------|------|
| `areaCenter` | 范围中心：`Target`（以目标为中心）/ `Caster`（以施法者为中心） |
| `areaRange` | 范围半径（六边形距离），范围内所有合法目标被纳入结算 |
| `splashRatio` | 溅射比例（仅 `splash`），次要目标受到的伤害比例 |

#### 4.2.4 组合矩阵：策略 × 目标 × 数值

以下矩阵展示了三种策略与目标阵营、数值公式的合法组合。"✓"表示已实现，"○"表示完全可行但暂未实装：

**对单即时（`SingleTargeted`）：**

| TargetType | fixed | AtkRatio | HpRatio |
|------------|-------|----------|---------|
| Enemy | ○ 固定伤害 | ○ ATK 比例伤害 | ✓ `execute`（处决） |
| Ally | ○ 固定治疗 | ○ ATK 比例治疗 | ○ HP 比例治疗 |
| Self | ○ 自身固定治疗 | ○ 自身 ATK 比例治疗 | — |

**对群（`MultiTargeted`）：**

| TargetType | fixed | AtkRatio | HpRatio | split | splash |
|------------|-------|----------|---------|-------|--------|
| Enemy | ○ 固定 AoE | ○ ATK 比例 AoE | ○ HP 比例 AoE | ○ 均摊 AoE | ✓ `cleave`（火球） |
| Ally | ○ 固定群奶 | ○ ATK 比例群奶 | ✓ `groupHeal` | ○ 均摊群奶 | — |

**Buff 附着（`BuffSingleTargeted`）：**

由于 Buff 自身可在 `buffs` 数组中配置独立的数值计算（见 [4.2.5](#425-第四层buff-挂载词条)），此策略的下层词条聚焦于"Buff 施加给谁"：

| TargetType | 典型场景 |
|------------|----------|
| Enemy | 眩晕（`stunStrike`）、Dot（`poisonBlade`）、削弱（`atk_affect` 负值） |
| Ally | Dot奶（`dotHeal`）、增益（`atk_affect` 正值） |
| Self | 自身强化（`selfEnhance`）、自身回蓝（`mana_affect` 负值） |

SelectType 在三类策略中均可自由替换。例如将 `stunStrike` 的 `Nearest` 改为 `HighestAtk`，即可得到"优先眩晕敌方最高攻击力者"——克制刺客的辅助型控制技能，仅改一行 JSON。

#### 4.2.5 第四层：Buff 挂载词条

`BuffSingleTargeted` 策略的核心特征：技能选择目标后，不是立即结算伤害/治疗，而是向目标挂载一组 Buff。每个 Buff 是一个独立配置单元，包含以下词条：

| 词条 | 含义 |
|------|------|
| `buffKey` | 引用的 Buff 类型 key（详见 [4.4](#44-buff-类型词条)） |
| `duration` | 持续 tick 数 |
| `valueType` | Buff 效果的数值计算方式（同技能：`fixed` / `AtkRatio` / `HpRatio`） |
| `value` / `ratio` | 数值参数（分别对应 `fixed` 和 `AtkRatio`/`HpRatio`） |
| `damageInterval` | Dot 触发间隔（仅 Dot 类有效；正数=伤害间隔，负数=治疗间隔） |

Buff 的 `valueType` 与技能本体的数值计算**完全独立**——一个 BuffSingleTargeted 技能可以没有即时伤害（如纯眩晕 `stunStrike`），也可以附带多个 Buff（如同时施加 Dot + 削弱），每个 Buff 各用自己的 valueType 和参数。`buffs` 是一个 JSON 数组，可在一次技能中挂载任意多个 Buff 实例。

#### 4.2.6 已实现技能一览

下表列出当前 `skills.json` 中的 7 个技能，每个都是上述词条空间中的一个具体坐标：

| key | 名称 | Strategy | Target | Select | ValueType | Buff | 技能特征 |
|-----|------|----------|--------|--------|-----------|------|----------|
| `execute` | 处决 | `SingleTargeted` | Enemy | HighestAtk | HpRatio (0.6) | — | 对敌方最高攻击者造成 60% 最大生命伤害 |
| `cleave` | 火球 | `MultiTargeted` | Enemy | Nearest | splash (10, 0.5) | — | 对最近敌人及其周围造成溅射伤害 |
| `groupHeal` | 群体治疗 | `MultiTargeted` | Ally | — | HpRatio (0.3) | — | 回复自身周围 2 格友方 30% 最大生命 |
| `stunStrike` | 震荡猛击 | `BuffSingleTargeted` | Enemy | Nearest | — | stun (12t) | 眩晕最近敌人 12 tick |
| `poisonBlade` | 毒刃 | `BuffSingleTargeted` | Enemy | Nearest | — | dot (18t, int=3, fixed=4) | 对最近敌人施加固定 4 点/3tick 的 Dot |
| `dotHeal` | dot奶 | `BuffSingleTargeted` | Ally | Nearest | — | dot (18t, int=−3, fixed=3) | 对最近友方施加每 3tick 回复 3 点的 Hot |
| `selfEnhance` | 强化 | `BuffSingleTargeted` | Self | Nearest | — | atk_affect (32t, AtkRatio=0.5) | 自身攻击力提升 50%，持续 32 tick |

> **注**：`stunStrike` 目前未被任何单位绑定，预留为后续单位或关卡敌人的技能池扩展。

**未实装但一条 JSON 即可完成的技能示例：**

- **斩杀**：`SingleTargeted × Enemy × LowestHp × fixed(999)` — 对血量最低敌人造成 999 固定伤害
- **刺客克星**：`BuffSingleTargeted × Enemy × HighestAtk` + `stun(12t)` — 优先眩晕敌方最高攻击者
- **均摊 AoE**：`MultiTargeted × Enemy × Nearest × split(30)` — 30 点总伤害均摊给范围内的所有敌人

### 4.3 羁绊词条

羁绊由 **trait 标签 + 阈值人数 + 加成字段组合** 定义（[entity/synergyregistry.h](src/entity/synergyregistry.h) 的 `SynergyBonus`）。一个 trait 可有多个阈值档位，达到更高人数时获得更强加成。

**可组合的加成词条：**

| 词条 | 类型 | 含义 |
|------|------|------|
| `bonusAtk` | int | 攻击力加值 |
| `bonusMaxHp` | int | 最大生命值加值 |
| `bonusMaxMana` | int | 法力上限修正（负数 = 蓝耗降低） |
| `bonusSpeed` | int | 速度加值（移动冲突优先级） |
| `bonusMoveSpeed` | float | 移速系数 |
| `dotIntervalReduction` | int | Dot 触发间隔缩减（tick），取团队最大值 |

每个阈值的加成 = 上述词条的任意非零子集。当前 4 个 trait 各定义了 1-2 档阈值：

| trait | 第 1 档 | 第 2 档 |
|-------|---------|---------|
| **神** | 2 人：ATK +10 | 4 人：ATK +15, HP +10, MoveSpeed +0.2 |
| **人** | 2 人：ATK +5, HP +10 | 3 人：ATK +5, HP +20, Mana −10 |
| **区** | 1 人：Mana −5 | 2 人：Mana −10, ATK +5 |
| **dot女子** | 1 人：Mana −5 | 2 人：DotIntervalReduction +1, Mana −5, MoveSpeed +0.2 |

> **扩展示例**：若想新增"兽"trait——3 人 +20 Speed +0.3 MoveSpeed，只需在 `synergies.json` 新增一个条目并定义阈值，给对应单位打上 `"兽"` 标签。`SynergyRegistry` 和 `Game::recalculateSynergies()` 通用于所有 trait，无需改动。

### 4.4 Buff 类型词条

Buff 类型由 **三维正交分类** 定义（[entity/buffregistry.h](src/entity/buffregistry.h) 的 `BuffDef`）：类别 × 叠加规则 × 影响属性。三维的每种合法组合即定义一个 Buff 类型。

| 维度 | 枚举值 | 语义 |
|------|--------|------|
| `category` | `Control` / `Dot` / `StatMod` | **类别**：决定每 tick 的行为模式 |
| `stackRule` | `Refresh` / `UniquePerSource` / `Independent` | **叠加规则**：同 key Buff 再次施加时的行为 |
| `stat` | `None` / `ATK` / `MaxMana` | **影响属性**：仅 `StatMod` 类有效，指定修改哪个属性 |

**Category 的 tick 行为：**

| Category | 每 tick 做什么 |
|----------|---------------|
| `Control` | 阻止行动：`isDisabled()` 返回 true → `decideAction()` 跳过该单位 |
| `Dot` | `damageIntervalCounter` 归零时触发一次伤害（正值）或治疗（负值），然后重置计数器 |
| `StatMod` | 无主动行为；在被查询时通过 `statModSum(stat)` 实时累加修正值 |

**叠加规则：**

| 规则 | 同 key + 同来源 | 同 key + 不同来源 |
|------|----------------|-------------------|
| `Refresh` | 刷新：持续时间取 max，效果取 max | 同左 |
| `UniquePerSource` | 刷新 | 各自独立共存 |
| `Independent` | 各自独立共存 | 各自独立共存 |

**已实现的 4 种 Buff 类型（作为三维组合示例）：**

| key | Category | StackRule | Stat | 说明 |
|-----|----------|-----------|------|------|
| `stun` | Control | Refresh | — | 眩晕：阻止行动，重复施加只刷新不叠加 |
| `dot` | Dot | UniquePerSource | — | 持续伤害/治疗：同源刷新，异源共存 |
| `atk_affect` | StatMod | Refresh | ATK | 攻击力修正：正值=增益，负值=削弱 |
| `mana_affect` | StatMod | Refresh | MaxMana | 法力修正：负值=蓝耗降低，正值=蓝耗增加 |

> **扩展示例**：若想新增一个"不可刷新的独立眩晕"——`{ "key": "stun_independent", "category": "Control", "stackRule": "Independent" }` ——不同施法者施加的眩晕各自独立计时，互不干扰。仅需在 `bufftypes.json` 中添加一行，`BuffRegistry` 自动加载。

### 4.5 单位词条

单位是上述所有词条系统的**汇合点**——一个单位 = 基础属性 + 技能绑定 + trait 标签，其中技能和 trait 将单位接入技能组合系统和羁绊组合系统。

[data/units.json](data/units.json) 定义所有单位模板。以下列出 7 个已实现单位的 1 星基础属性（星级倍率：1 星 ×1.0 / 2 星 ×1.8 / 3 星 ×3.24）：

| key | 名称 | HP | ATK | Rng | Mana | SPD | AtkCD | MovCD | Cost | Traits | Type | Skill |
|-----|------|----|-----|-----|------|-----|-------|-------|------|--------|------|-------|
| `white_e` | 企厄 | 45 | 5 | 1 | 30 | 20 | 12 | 6 | 3 | 神 | 战士 | `cleave` |
| `black_e` | 黑厄 | 70 | 6 | 1 | 90 | 25 | 6 | 4 | 4 | 神 | 刺客 | `execute` |
| `lingsha` | 灵砂 | 90 | 1 | 1 | 50 | 10 | 18 | 6 | 5 | 神, 区 | 辅助 | `groupHeal` |
| `gugugaga` | 咕咕嘎嘎 | 300 | 2 | 1 | 50 | 30 | 12 | 8 | 6 | 人 | 坦克 | `cleave` |
| `hihihihi` | 爱音斯坦 | 40 | 9 | 3 | 30 | 40 | 12 | 4 | 2 | 人 | 射手 | `selfEnhance` |
| `kafuka` | 拉琴女子 | 50 | 2 | 2 | 20 | 40 | 4 | 4 | 2 | dot女子, 人 | 法师 | `poisonBlade` |
| `lao_e` | 鹅 | 80 | 1 | 1 | 20 | 40 | 4 | 4 | 2 | dot女子, 区 | 辅助 | `dotHeal` |

单位属性字段（Speed、Range、AtkCD、MovCD、Price、Type）的说明见 [3.1 棋盘与单位管理](#31-棋盘与单位管理) 中的 `Unit` 类描述。Skill 和 Traits 字段将单位接入上述 [4.2](#42-技能词条) 和 [4.3](#43-羁绊词条) 的词条组合系统。

---

## 5. 算法描述

### 5.1 六边形网格寻路 (BFS)

寻路是本项目最核心的算法，用于战斗阶段单位自动向目标移动时的路径规划。由于棋盘采用六边形网格（错列排布），不能直接使用四方向或八方向的曼哈顿网格寻路。

**坐标系统**

六边形网格使用两套坐标：偏移坐标 (offset coordinates) 用于存储和渲染（`row, col`），立方体坐标 (cube coordinates, `x, y, z` 满足 `x+y+z=0`）用于距离计算和邻居遍历。`Pathfinder` 类提供 `offsetToCube()` 和 `cubeToOffset()` 进行双向转换。

偏移坐标到立方体坐标的转换公式（even-r 布局）：

```
x = col - (row + (row & 1)) / 2
z = row
y = -x - z
```

**六边形距离**

两点之间的最短步数通过立方体坐标计算：

```
hexDistance(a, b) = (|a.x - b.x| + |a.y - b.y| + |a.z - b.z|) / 2
```

对六边形网格而言此公式等价于 `max(|dx|, |dy|, |dz|)`，两者均可使用。

**邻居遍历**

每个六边形格子有 6 个邻居。为避免偏移坐标下偶数行/奇数行需要不同偏移量的繁琐处理，算法统一在立方体坐标下使用 6 个固定方向向量计算邻居：

```
directions[6] = {
    {+1, -1,  0}, {+1,  0, -1}, { 0, +1, -1},
    {-1, +1,  0}, {-1,  0, +1}, { 0, -1, +1}
}
```

**BFS 主体流程**

[pathfinder.cpp:50-111](src/core/pathfinder.cpp#L50) 中 `findPath()` 的实现：

1. 若 `start == target`，无需移动，返回空路径
2. 若 `hexDistance(start, target) <= attackRange`，单位已在攻击范围内，返回仅含 `start` 的路径（表示无需移动即可攻击）
3. 初始化 `QQueue<QPoint>` 队列，`visited[row][col]` 访问标记，`parent[row][col]` 回溯数组
4. BFS 主循环：从队首取节点，若该节点与 `target` 的六边形距离 `<= attackRange`，则找到目标，终止搜索；否则遍历 6 个邻居：跳过越界、已访问、被占用的格子，将合法邻居入队并记录父节点
5. 若 BFS 结束仍未找到可达节点，返回空路径（无路可走）
6. 通过 `parent` 数组回溯路径，`std::reverse` 后返回 `QList<QPoint>`

为何选择 BFS 而非 A*：六边形网格中每条边的代价相等（均为 1 步），BFS 保证首次到达目标即为最短路径，且无需设计启发函数和优先队列。棋盘规模仅为 8x8（64 格），性能满足实时战斗需求。

### 5.2 羁绊加成计算

[src/core/game.cpp](src/core/game.cpp) 中 `recalculateSynergies()` 负责每帧/每次布阵变动后重算羁绊加成。

**流程**：

1. **统计**：遍历玩家场上所有存活单位，按 `traits` 标签分组计数（如 "神": 2, "人": 3, "dot女子": 2）
2. **查表**：对每个 trait，调用 `SynergyRegistry::getBonus(trait, count)`，在预加载的阈值列表中匹配最高满足档位。例如 "神" trait 阈值为 `[{count:2, bonus:+10ATK}, {count:4, bonus:+25ATK}]`，当前 2 个单位命中第一档，返回 `+10ATK`
3. **叠加**：遍历场上所有单位，将其所有 trait 对应的 `SynergyBonus` 通过 `operator+=` 累加到单位属性上。同一单位拥有多个 trait 时，加成叠加
4. **特殊机制**：除属性光环外，羁绊可包含机制改变类加成（如 Dot 触发间隔缩减 `dotIntervalReduction`），在战斗 tick 中被 `BattleSystem` 读取使用

### 5.3 战斗逻辑：决策-执行分离

战斗系统是本项目最核心的子系统，由 `BattleSystem` 类（[core/battlesystem.h](src/core/battlesystem.h)）驱动。战斗以 **tick** 为单位推进——QTimer 每 50ms 触发一次 `onBattleTick()`，每一 tick 内完成"决策 → 执行 → 结算"的完整循环。

#### 5.3.1 问题：战斗执行顺序为何重要

在任何逐单位遍历执行的动作系统中，执行顺序都会影响公平性。考虑两种朴素的实现方案及其缺陷：

**方案 A：按 Speed 遍历，立即执行**

最直观的做法是按 speed 降序遍历所有单位，每个单位立即完成"决策→移动→攻击→造成伤害→判定死亡"全流程，然后处理下一个单位。

问题：
- **先手优势**。高速单位先动，可能直接击杀低速单位，使后者完全没有出手机会。这等价于将 speed 同时用作"行动顺序"和"先手权"，低速单位在机制层面被双重惩罚。
- **连锁效应**。先手单位击杀敌人后，后手单位的寻路目标消失、攻击目标变更，整个战局被先手单位的行动结果重塑。

**方案 B：按 Speed 遍历，延迟死亡结算**

在方案 A 的基础上改进：伤害仍然每次立即计算并扣减 HP，但死亡标记延迟到 tick 末尾统一处理。这解决了"击杀剥夺出手权"的问题——即使 HP 被扣到 0，单位仍能完成本 tick 的行动。

遗留问题：
- **控制/Dot 等状态类效果无法延迟**。如果高速单位在本 tick 内对低速单位施加了眩晕，后者在轮到自身时已经被控制，无法出手。这意味着双方无法"同时"释放控制技能——先手方永远占据优势。
- **"未来视"问题**（见下文 4.3.2）。高速单位先移动并占据格子，低速单位在寻路时已经看到了移动后的棋盘状态，等价于低速单位获得了预知高速单位行动的能力。例如在一个tick开始前两个单位在彼此攻击范围外，高速单位先寻路并移动，然后低速单位在决策时发现敌方已经进入了攻击范围，便可以开始攻击，此时低速单位反而占据了优势。

#### 5.3.2 方案：基于快照的决策-执行分离

本项目的设计选择是：**将"决策"与"执行"解耦**。所有单位在同一时刻、基于同一份游戏状态快照做出决策；决策全部收集完毕后，再分阶段执行；伤害和死亡在 tick 末尾批量结算。

核心原则：

| 原则 | 说明 |
|------|------|
| 决策基于快照 | tick 开始时遍历所有单位做决策，`decideAction()` 只读状态不写状态 |
| 执行分阶段 | 移动、技能、普攻各自独立结算，互不穿插 |
| 伤害批量结算 | 所有伤害/治疗事件累积到 `m_pendingDamageEvents`，tick 末尾统一应用 |
| 死亡批量结算 | 所有 HP≤0 的单位在 `resolveDeaths()` 中统一标记为 Dead |

这带来了几个关键优势：

**1. 消除"未来视"。** 高速单位先移动 → 低速单位看到新位置再寻路 → 低速单位实际上获得了"预知未来"的能力。在当前方案中，所有单位的移动目标在决策阶段已经确定（基于 tick 开始时的棋盘状态），移动结算时各单位的去向互不影响寻路决策——你的目标不会因为别人先移动而改变。

**2. 控制技能公平。** 状态类 Buff 在技能执行阶段（`skillAction()`）才挂载到目标，而决策阶段早已完成。这意味着：即使 A 在本 tick 对 B 施加了眩晕，B 在本 tick 已经做出的决策（攻击/移动/技能）不受影响，控制效果从下一 tick 才开始生效。双方的控制技能真正做到了"同时释放"。

**3. Speed 的职责清晰。** Speed 不再影响"谁先看到世界状态"（决策顺序），只影响：
- **移动冲突优先级**：多个单位争抢同一目标格时，speed 高者优先（[battlesystem.cpp:13-21](src/core/battlesystem.cpp#L13) 的 `moveActionLess`）
- **攻击/技能顺序**（如需要）：同 tick 内的多目标结算顺序
- Speed 的作用从"全局行动权"收窄为"物理碰撞优先权"，语义更加明确。

**4. 高度可扩展。** 执行流程分为多个独立阶段（移动→技能→普攻→伤害→死亡），新增机制只需插入新阶段或扩展现有阶段，不影响决策逻辑。

#### 5.3.3 逐帧流程

`BattleSystem::onBattleTick()` 每一 tick 的完整流程（[battlesystem.cpp:111-134](src/core/battlesystem.cpp#L111)）：

| 步骤 | 方法 | 阶段 | 说明 |
|------|------|------|------|
| 1 | `processBuffsPreActions()` | 预处理 | 遍历所有单位：Dot 类 Buff 按 `damageInterval` 周期累伤；所有 Buff 的 `remainingTicks` 递减 1 |
| 2 | `decideAction()` × N | **决策** | 遍历所有存活单位，各自基于当前快照决定行动。决策结果收集为 `QVector<PlannedAction>`，不修改游戏状态 |
| 3a | `moveAction()` | 执行·移动 | 筛选 `Moving` 状态的行动，按 speed 降序排列后依次结算移动（含碰撞回退） |
| 3b | `skillAction()` | 执行·技能 | 筛选 `Casting` 状态的行动，依次施放技能。产生伤害事件（入队 `m_pendingDamageEvents`）和 Buff 挂载（直接写入目标） |
| 3c | `attackAction()` | 执行·普攻 | 筛选 `Attacking` 状态的行动，依次造成伤害。伤害入队 `m_pendingDamageEvents`，攻击方获得 5 点法力值 |
| 4 | `resolveDamage()` | **批量结算** | 统一遍历 `m_pendingDamageEvents`，对每个目标调用 `takeDamage()`，清空事件队列 |
| 5 | `processBuffsPostActions()` | 清理 | 移除所有 `remainingTicks <= 0` 的过期 Buff |
| 6 | `updateUnits()` | 冷却 | 所有存活单位的攻击冷却和移动冷却各递减 1 |
| 7 | `resolveDeaths()` | **死亡结算** | 遍历所有单位，将 HP≤0 者标记为 `Dead` |
| 8 | `checkEndCondition()` | 胜负判定 | 检查双方是否仍有存活单位，决定继续战斗或发射 `battleFinished()` 信号 |

三步"批量"操作是公平性的关键保障：

- **决策全部完成，再开始执行**（步骤 2 → 步骤 3）：所有单位看到的世界完全一致。
- **伤害全部入队，再批量结算**（步骤 3 → 步骤 4）：一个 tick 内没有任何单位"先受伤"。
- **死亡在所有伤害结算后才判定**（步骤 4 → 步骤 7）：即使 DoT 在步骤 1 就累积了致命伤害，该伤害在步骤 4 才实际扣血，步骤 7 才判定死亡——受害单位仍然完成了步骤 2 的决策和步骤 3 的执行。

#### 5.3.4 决策逻辑详解

`decideAction()`（[battlesystem.cpp:136-185](src/core/battlesystem.cpp#L136)）决定了每个单位在本 tick 的行动。决策按以下优先级逐级判断：

| 优先级 | 条件 | 行动 | 说明 |
|--------|------|------|------|
| — | 单位在备战席上 | `Idle` | 非场上单位不参与战斗 |
| — | 被控制（`isDisabled()` 检查 Control 类 Buff） | `Idle` | 眩晕等单位本 tick 跳过行动 |
| 1 | 拥有技能 + 法力满 + 冷却就绪 + 技能有合法目标 | `Casting` | 技能优先于普攻，"有蓝就放" |
| 2 | 攻击范围内有敌人 + 普攻冷却就绪 | `Attacking` | 若不能放技能则尝试普攻 |
| 3 | 敌人不在攻击范围内 + 移动冷却就绪 + BFS 有路径 | `Moving` | 向最近敌人移动一步 |
| — | 以上均不满足 | `Idle` | 等待冷却或目标出现 |

**目标选择规则**（[battlesystem.cpp:85-95](src/core/battlesystem.cpp#L85) 的 `selectTarget()` + `targetLess()`）：

从敌对阵营存活单位中按四级级联排序选出最优目标：
1. 六边形距离最近（`hexDistance` 最小）
2. 距离相同时，HP 最低（集火残血单位）
3. HP 相同时，Y 坐标最小
4. Y 相同时，X 坐标最小

后两级平局规则保证选择结果的确定性，避免同一距离/血量下不同 tick 随机切换目标。

#### 5.3.5 移动冲突解决

移动结算是整个 tick 中**唯一一个单位顺序会影响结果的步骤**。当多个单位的目标格重叠时，需要决定谁获得优先权。

`moveAction()`（[battlesystem.cpp:193-241](src/core/battlesystem.cpp#L193)）的处理流程：

1. **预留非移动单位的位置**。所有不移动的单位，其当前格子加入 `actualOccupied` 集合——移动单位不能"挤开"静止单位。
2. **按优先级排序**。移动单位按 `moveActionLess` 排序（[battlesystem.cpp:13-21](src/core/battlesystem.cpp#L13)）：speed 降序 → ID 升序 → Y 降序 → X 降序。Speed 高者优先选择落脚点。
3. **依次结算**。对每个移动单位：
   - 若目标格空闲 → 直接移动，占位
   - 若目标格已被（更高优先级的单位）占据 → 搜索 6 个六边形邻居 + 原地不动共 7 个候选格，按到目标格的六边形距离排序，取最近的空闲格子
   - 若只能原地不动 → 设为 `Idle`
4. **每步占位**。单位移动后立即将新位置加入 `actualOccupied`，确保后续单位不会与之冲突。

排序中的 ID 升序、Y/X 降序是确定性平局规则：当两个单位 speed 相同时，不依赖 `QList` 的遍历顺序（遍历顺序可能不稳定），而是以自身固有属性确定唯一顺序。

#### 5.3.6 设计权衡

任何同步回合制设计都面临"真实同时性"与"离散计算"之间的根本矛盾。当前方案的选择及其代价：

**注定死亡的单位仍能"临死一击"。** 假设单位 A 身上挂有本 tick 即将触发的致命 Dot。步骤 1（`processBuffsPreActions`）将该 Dot 伤害入队，但步骤 2 时 A 的 HP 尚未扣除——A 仍然存活，照常决策和行动。只有在步骤 4（`resolveDamage`）中 Dot 伤害才实际扣减 HP，步骤 7（`resolveDeaths`）才标记死亡。A 在"已知将死"的本 tick 内仍能完成最后一次攻击或技能释放。这是刻意为之——它等价于"所有单位同时行动"的抽象：既然双方同时出手，就不应该有人的攻击被"未来的"死亡取消。

**控制效果延迟一 tick 生效。** 步骤 3b 中 A 对 B 施加了眩晕 Buff，但 B 在步骤 2 已经做出了本 tick 的行动决策。因此 B 在本 tick 不受眩晕影响，控制效果从下一 tick 的 `decideAction()` 开始才阻止 B 行动。这保证了双方的控制技能处于平等地位——不存在"谁先出手谁就压制对方"的情况。

**Speed 不做"先手权"。** 在本设计中，speed 不论多高也不会让单位在"信息层面"优于对手——所有单位看到的是同一帧画面。Speed 的价值体现在更稳定的移动落点选择权（冲突时优先选格），以及通过装备/羁绊赋予的间接收益，而非"我是全场最快所以我先看完所有人动完再决定"的特权。

**缺点**：所有单位的寻路无法达到全局最优，必须依赖 fallback 情况。

---

## 6. 技能系统设计

技能系统采用"策略模式 + 工厂模式"架构，目标是在不修改 C++ 代码的前提下，仅通过 JSON 配置即可定义新技能。

### 6.1 抽象接口设计

[entity/skill.h](src/entity/skill.h) 定义抽象基类 `Skill`，所有技能必须实现以下纯虚函数：

| 接口                               | 作用                                                    |
| -------------------------------- | ----------------------------------------------------- |
| `targetType()`                   | 决定目标阵营：敌方 / 友方 / 自身                                   |
| `selectType()`                   | 决定选择策略：最近优先 / 最低血量优先 / 最高攻击优先                         |
| `valueType()`                    | 决定数值计算方式：固定值 / 基于施法者攻击力比例 / 基于目标最大生命值比例 / 均摊伤害 / 溅射伤害 |
| `castRange()`                    | 施法范围（六边形距离），超出此范围的单位不可被选为目标                           |
| `selectTargets(units)`           | 从候选单位中按 targetType + selectType + castRange 筛选目标      |
| `calculateValue(caster, target)` | 计算最终伤害/治疗数值（应用 valueType 和 ratio 参数）                  |
| `execute(caster, targets)`       | 执行技能：计算数值 -> 应用伤害/治疗 -> 附着 Buff（子类重写）                 |

这种接口设计将"选谁"（targetType / selectType）、"怎么算"（valueType）、"怎么执行"（execute）三阶段解耦，枚举组合即可产生大量技能变体。

### 6.2 JSON 配置驱动

[data/skills.json](data/skills.json) 中的每个技能定义包含 `strategy` 字段，`SkillRegistry` 据此工厂创建具体类：

| strategy 值 | 对应类 | 行为特征 |
|-------------|--------|----------|
| `SingleTargeted` | `SingleTargetedSkill` | 选择单个目标，直接造成伤害/治疗 |
| `MultiTargeted` | `MultiTargetedSkill` | 以施法者或目标为中心，对范围内所有合法目标造成伤害（支持溅射比例） |
| `BuffSingleTargeted` | `BuffSingleTargetSkill` | 在单体技能基础上，附加 JSON 中定义的 Buff（眩晕/Dot/属性修正） |

新增技能的典型流程：在 `skills.json` 中添加条目，指定 strategy 和参数，无需修改 C++ 代码。这使得技能数值调整和新增对非程序员友好。

### 6.3 具体技能执行流程差异

以 `BuffSingleTargetSkill::execute()` 为例，其流程为：

1. 调用父类 `Skill::execute()` 完成目标选择与伤害结算
2. 遍历 JSON 中 `buffApplications` 数组
3. 对每个 buff 条目，从 `BuffRegistry` 获取 `BuffType` 定义（类别、叠加规则、tick 行为）
4. 构造 `BuffInstance` 附加到目标单位的 `buffs` 列表中
5. 后续战斗 tick 中，`BattleSystem` 根据 `BuffType` 定义自动处理眩晕跳过回合、Dot 周期性伤害、属性修正衰减等

---

## 7. 辅助函数

以下列出项目中关键的全局/静态工具函数，多数集中于 `Game` 和 `Pathfinder` 类中。

### 坐标转换

| 函数 | 所在文件 | 作用 |
|------|----------|------|
| `gridToWorld(row, col)` | `core/game.cpp` | 六边形网格坐标转 Qt 场景世界坐标（像素位置），考虑偶数行水平偏移 |
| `worldToGrid(worldPos)` | `core/game.cpp` | 鼠标场景坐标反向查找最近的网格坐标，用于拖拽时定位目标格 |
| `offsetToCube(hex)` | `core/pathfinder.cpp` | 偏移坐标转立方体坐标（even-r 布局） |
| `cubeToOffset(cube)` | `core/pathfinder.cpp` | 立方体坐标转偏移坐标 |
| `hexDistance(a, b)` | `core/pathfinder.cpp` | 计算两个六边形格子的最短步数距离（偏移坐标和立方体坐标两个重载） |
| `hexNeighbors(hex)` | `core/pathfinder.cpp` | 获取某格子的 6 个六边形邻居坐标 |

### 游戏逻辑

| 函数 | 所在文件 | 作用 |
|------|----------|------|
| `findEmptyBenchSlot()` | `core/game.cpp` | 在备战区 1x10 中寻找第一个空槽位，购买单位时使用 |
| `countFieldUnits(owner)` | `core/game.cpp` | 统计场上某方（我方/敌方）存活单位数量 |
| `randomStarByLevel(level)` | `core/game.cpp` | 根据玩家等级按概率生成 1-3 星单位，商店刷新时使用 |
| `tryMergeStar(unit)` | `core/game.cpp` | 检查备战区+场上是否有 3 个同名同星单位，触发自动升星合并（3 合 1） |
| `dotIntervalReduction(owner)` | `core/game.cpp` | 累加某方羁绊提供的 Dot 触发间隔缩减总量 |
| `isOverSellZone(scenePos)` | `core/game.cpp` | 判断场景坐标是否落在出售区域内 |
| `recalculateSynergies()` | `core/game.cpp` | 遍历场上单位按 trait 分组统计，查 SynergyRegistry 取最高满足阈值，叠加加成

---

## 8. 设计哲学

### 8.1 JSON 驱动的数据与逻辑分离

本项目所有单位模板、技能参数、羁绊规则、关卡配置、装备属性和 Buff 类型均存储在 `data/` 目录的 JSON 文件中，C++ 代码仅负责读取和运行时执行，不硬编码任何具体数值或机制。

这一设计的核心动机：

**Mod 与自定义（单机场景）。** 玩家可以直接编辑 JSON 文件修改单位属性、技能效果、羁绊加成，甚至新增单位或技能，无需重新编译。数据层与逻辑层分离使得"修改游戏"对非程序员同样可行，可以视作 Mod 玩法的一部分。

**服务器端校验（线上扩展）。** 如果将项目扩展为线上对战，所有 JSON 数据可存放于服务器端，客户端仅做渲染和输入转发。对战双方的伤害计算、技能触发、羁绊加成均在服务器以唯一数据源完成，杜绝客户端篡改作弊。

**RL 实验框架的潜力（研究扩展）。** JSON 的高度可配置性使本框架或许可以作为自走棋类强化学习实验平台——研究者无需修改 C++ 源码即可自定义环境参数（单位池、技能集、羁绊规则、关卡难度曲线），快速迭代实验配置。

### 8.2 继承与多态的设计选择

**Skill 体系（继承 + 多态）。** 技能之间在"执行模式"上存在本质差异：单体即时结算、范围溅射结算、附带持续 Buff 的结算。三种模式的 `execute()` 流程不可互相替代——它们的目标筛选范围不同、结算次数不同、是否有后置 Buff 附着步骤不同。因此定义抽象基类 `Skill`（统一接口），派生出 `SingleTargetedSkill`、`MultiTargetedSkill`、`BuffSingleTargetSkill` 三个子类各自重写 `execute()`，是继承和多态的典型应用场景。

在此基础上，通过 `SkillRegistry` 工厂和 JSON 的 `strategy` 字段实现了数据驱动：新增一个技能（如新的单体伤害技能）只需在 JSON 中选择 `SingleTargeted` 策略并填入参数，无需新增 C++ 类。只有需要全新执行模式时才新增子类。继承负责"行为框架"，数据负责"参数填充"，两者互补。

**Unit 体系（数据驱动优先）。** 不同英雄之间的差异主要体现在数值（HP/ATK/Mana 等）和标签（traits/羁绊）层面，由 JSON 模板和运行时字段承载。在行为层面，所有单位的移动、普攻、释放技能、死亡流程共享同一套逻辑。本项目选择将英雄间的"行为差异"通过 Skill 系统和 Buff 系统来表达：一个英雄的特殊机制（如中毒、眩晕）由其所绑定的技能和 Buff 类型决定，而非由 Unit 的子类决定。这种设计将行为变化的"责任"集中到了 Skill 和 Buff 体系中，避免了两处同时管理行为逻辑的复杂性。

**整体原则。** 项目中继承的使用遵循一个简单判断：当一个实体需要不同的执行流程时，使用继承（如 Skill）；当差异可以归约到数据和可组合的模块时，优先使用数据驱动和组合（如 Unit + Skill + Buff）。

#### 以 Buff 类技能的挂载方式为例

技能系统的数据驱动特性在 Buff 类技能中体现得最为完整。整个链条从 JSON 配置出发，经过三层抽象，最终在 Unit 端落地为具体效果。

**第一层：Buff 类型定义 (`bufftypes.json` + `BuffRegistry`)**

所有 Buff 的行为由三个维度抽象定义，无需为每种 Buff 单独编写逻辑：

| 维度 | 对应枚举 | 含义 |
|------|----------|------|
| 类别 | `BuffCategory` | `Control`（控制，阻止行动）/ `Dot`（持续伤害）/ `StatMod`（属性修正） |
| 影响属性 | `BuffStat` | `ATK` / `MaxMana` / `None`（仅 StatMod 类有效，Control 和 Dot 不需要） |
| 叠加规则 | `BuffStackRule` | `Refresh`（刷新时间取 max，效果取 max）/ `UniquePerSource`（同源刷新，异源独立）/ `Independent`（每次施加都是新实例） |

三种维度的组合即可完整区分所有 Buff 类型。例如 `data/bufftypes.json` 中的定义：`stun` = Control + Refresh，`dot` = Dot + UniquePerSource，`atk_affect` = StatMod(ATK) + Refresh，`mana_affect` = StatMod(MaxMana) + Refresh。新增一种 Buff 类型只需在 JSON 中添加一个条目，C++ 端不做任何修改。

**第二层：技能配置 (`skills.json` + `BuffSingleTargetSkill`)**

`BuffSingleTargetSkill` 是三组枚举的组合器，每组枚举负责一个正交的决策维度：

| 枚举           | 可选值                                   | 决策内容           |
| ------------ | ------------------------------------- | -------------- |
| `TargetType` | `Enemy` / `Ally` / `Self`             | 技能作用于谁         |
| `SelectType` | `Nearest` / `LowestHp` / `HighestAtk` | 在候选者中如何选择      |
| `ValueType`  | `fixed` / `AtkRatio` / `HpRatio`      | Buff 效果的数值如何计算 |

三组枚举自由组合即可产生大量技能变体，无需新增 C++ 代码。例如：

- **增伤 Buff**（如 `selfEnhance`）：`atk_affect` + `Self` + `Nearest` + `AtkRatio(0.5)` — 对自身施加，攻击力提升 50%
- **眩晕技能**（如 `stunStrike`）：`stun` + `Enemy` + `Nearest` — 对最近的敌人施加眩晕（Control 类无需 ValueType，magnitude 无意义）
- **毒刃**（如 `poisonBlade`）：`dot` + `Enemy` + `Nearest` + `fixed(4)` — 对最近敌人施加每 3 tick 造成 4 点伤害的 Dot
- **群体治疗**（如 `dotHeal`）：`dot` + `Ally` + `Nearest` + `fixed(3)` + 负 `damageInterval` — 对最近友方施加每 3 tick 恢复 3 点的治疗效果

还可以组合出一些有特定意义的功能的技能，如：

- 辅助保护后排用的眩晕技能：`stun` + `Enemy` + `HighestAtk` — 对刺客施加眩晕
- 针对坦克的百分比dot伤害：`dot` + `Enemy` + `Nearest` + `HpRatio(0.05)` — 对最近的敌人施加每次结算最大生命上限5%的dot伤害

所有以上技能均在 JSON 中定义，由 `SkillRegistry` 根据 `strategy: "BuffSingleTargeted"` 工厂创建对应的 `BuffSingleTargetSkill` 实例。

**第三层：Unit 端的效果落地**

技能执行后，`BuffSingleTargetSkill::execute()` 生成 `BuffApplication` 列表（含 buffKey、duration、magnitude 等字段）写入 `SkillResult`。`BattleSystem` 读取后调用 `Unit::addBuff()` 将 `BuffInstance` 挂载到目标单位。之后的每个战斗 tick：

- `processBuffsPreAction()`：遍历所有 Dot 类 Buff，按 `damageInterval` 周期累计伤害/治疗
- `isDisabled()`：检查是否存在 Control 类 Buff，有则本 tick 跳过行动
- `statModSum(stat)`：实时累加所有 StatMod 类 Buff 的属性修正值，供 `atk()` / `maxMana()` 等 getter 使用
- `addBuff()` 内的叠加逻辑：根据 `BuffDef::stackRule` 决定新 Buff 是刷新旧 Buff、作为独立实例共存、还是按来源去重

整个链条 "JSON 类型定义 -> 技能枚举组合 -> Unit 运行时落地" 使得 Buff/Skill 的扩展和数值调整完全在数据层闭环，无需触及 C++ 逻辑代码。

## 9. AI 使用说明

### 9.1 skill部分

#### 9.1.1 项目规划：技能系统从"完全数据驱动"到"机制性继承"的演变

技能系统的设计不是一步到位的，而是经历了四个阶段的推演，每个阶段都在上一轮的基础上发现问题、调整方向，最终收敛到当前方案。整个过程中，AI（Deepseek 接入 Claude Code）充当了讨论对手和瓶颈诊断者的角色。

##### 阶段一：Skill 绑定而非 Unit 继承

项目从一开始就确定 Unit 采用数据驱动：`units.json` 定义所有单位的属性模板，`UnitData` 工厂按模板创建实例，所有单位共享同一个 `Unit` 类，靠数据字段区分。这套设计运行良好——新增一个单位就是新增一条 JSON 条目，不需要改 C++。

所以在考虑技能系统时，首先排除的就是"为每个英雄写一个 Unit 子类"这个方向。如果为了技能而打破 Unit 的数据驱动，让每个英雄变成 Unit 的派生类，那么单位本身和技能就都锁死在 C++ 里了。保持 Unit 不变、技能作为外部组件绑定上去，这个方向从一开始就明确了。

##### 阶段二：试图用"一个 Skill 类 + 全 JSON 描述"走到底

受 Unit 数据驱动思路的影响，一开始想把技能也做成完全数据驱动的：只写一个 `Skill` 类，所有行为差异全在 JSON 中描述。大致设想是用大量字段覆盖所有可能——`type` 区分伤害/治疗/控制，`target` 区分敌方/友方/自身，`select` 区分最近/最低血/最高攻，`scope` 区分单体/范围/溅射，`valueCalc` 区分固定值/ATK比例/HP比例，再加上可选的 buff 字段。

和 AI 讨论这个方案时，AI 阅读了已有代码库后指出两个问题：

1. **单类内部无用字段太多**。同一个 `Skill` 对象里，`scope` 是 `single` 时 `blastRadius` 和 `splashRatio` 毫无意义；`hasBuff` 是 `false` 时 `buffKey`、`duration`、`damageInterval` 全是废字段。所有技能共享同一个类意味着这个类必须承载所有可能的能力，大量字段在多数技能上是冗余的，读代码时看不出一眼"这个技能用到了哪些参数"。

2. **嵌套 switch 不可维护**。`execute()` 里需要 `switch (type)` 判断伤害还是治疗，`switch (scope)` 判断单体还是范围，`switch (valueCalc)` 判断怎么算数值，`switch (hasBuff)` 判断要不要挂 Buff——核心逻辑会变成一张难以阅读的真值表，加一种新机制就要在多处 switch 里找位置插新 case。

此时意识到：完全数据驱动对 Unit 行得通，是因为所有 Unit 共享完全相同的属性结构和行为流程——HP 就是 HP，ATK 就是 ATK，普攻就是普攻，不存在"有的 Unit 有 ATK 有的没有"的情况。但技能之间的行为流程存在质的差异（即时结算 vs 范围溅射 vs 附 Buff 持续结算），试图用一个类消化所有差异，代价是类内部膨胀到不可维护。

##### 阶段三：走到反面——"一个技能一个子类"

既然一个类消化不了所有差异，自然想到为每个具体技能写一个 Skill 子类。但和 AI 进一步讨论后，发现了对称的问题：

1. **子类数量随技能数线性增长**。7 个技能 = 7 个子类，20 个技能 = 20 个子类。而且很多技能之间只有数值参数不同——比如一个单体伤害 50、另一个单体伤害 80，为了一个数字就要新建类并重新编译，这和 Unit 继承的类爆炸是同构的问题。

2. **JSON 的数据驱动优势被削弱**。Unit 数据驱动最大的便利是"纯靠 JSON 创造一个新单位"。如果技能变成"一个子类一个技能"，创造新技能仍然要写 C++ 类——技能层丧失了 Unit 层已经做到的能力，JSON 退化为参数微调表。

此时设计卡在一个两难的境地：阶段二太泛（一个类兜底但要写海量 switch），阶段三太专（结构清晰但丧失数据驱动）。

##### 阶段四：收敛——"机制性继承"与正交枚举的组合

在和 AI 的多轮讨论中，逐一拆解了每个现有技能的执行流程，逐步骤判断"这是机制不同（必须走不同代码路径）还是参数不同（可以归约到数值选择）"：

- **选择目标的方式不同**？不是机制不同——选最近、选最低血、选最高攻，本质都是"按某个排序规则取第一个"，可以用比较器参数化。
- **数值计算公式不同**？也不是——固定值、ATK 比例、HP 比例，本质都是"取一个数乘以一个系数"，枚举 + ratio 字段就能覆盖。
- **单体 vs 范围**？这里开始有机制差异。单体和多目标在 `execute()` 里循环次数不同、伤害是否需要按人数均摊/溅射也不同。
- **是否附着 Buff**？这是最明显的机制差异。Buff 技能的 `execute()` 除了即时伤害外，还有 BuffInstance 构造和挂载步骤，且后续战斗 tick 中 Buff 有独立生命周期。

由此得出设计原则：**继承只区分"执行机制"（execute 的流程不同），JSON 枚举负责"参数组合"（目标选谁、数值怎么算）。** 在这个原则下，设计了三组正交的描述性词条——TargetType（敌方/友方/自身）、SelectType（最近/最低血/最高攻）、ValueType（固定值/ATK比例/HP比例/均摊/溅射），以及三种抽象的策略子类——`SingleTargetedSkill`（单体即时结算）、`MultiTargetedSkill`（多目标范围结算）、`BuffSingleTargetSkill`（附 Buff 结算）。三组枚举自由组合负责内容差异，三种子类负责流程差异，两者在 JSON 中由 `strategy` 字段桥接。

##### 四个阶段一览

| 阶段 | 思路 | 问题 | 保留了什么 |
|------|------|------|-----------|
| 一 | Skill 绑定到 Unit，不碰 Unit 继承 | 方向正确，具体做法待定 | Unit 数据驱动不受影响 |
| 二 | 一个 Skill 类，全 JSON 描述 | 字段冗余、嵌套 switch 不可维护 | "数据驱动技能"的意愿 |
| 三 | 一个技能一个子类 | 类爆炸、JSON 丧失新增技能的能力 | 继承管理行为差异的思路 |
| 四 | 三种机制子类 + 正交枚举组合 | 收敛 | 继承管流程（机制），JSON 管参数（内容）|

##### AI 在此过程中的角色

1. **两轮瓶颈诊断**。阶段二中 AI 指出了"单类承载所有可能性 → 字段冗余 + switch 膨胀"的具体危害；阶段三中 AI 指出了"一个子类一个技能 → 类爆炸 + JSON 意义被削弱"的矛盾。每次诊断都给出了"为什么这个方向走不远"的具体论证，而非简单否定。

2. **机制 vs 内容的边界推演**。阶段三到阶段四的转折中，AI 与我一起拆解每个技能的执行流程，逐步骤判断是机制差异还是参数差异。这个过程帮助定位了"单体/AOE/Buff 三种执行流程"的分界线，为策略子类的划分提供了依据。

3. **验证与辅助**。最终方案中，枚举维度和子类划分由我自己完成——这是整个设计的核心。AI 在此阶段主要是验证者：帮助检查现有的技能是否都能被这套枚举组合覆盖、是否还有遗漏的机制类型，以及辅助完成了排序辅助函数和 JSON 解析样板代码的编写。

#### 9.1.2 核心代码解析 —— 模块一：技能抽象基类设计

##### 设计思路

确定"技能用继承"后，下一个问题是如何设计 `Skill` 抽象基类的接口。与 AI 讨论后，所有技能的执行流程被拆解为三个正交的决策维度，每个维度用一个枚举表达：

| 维度 | 决策问题 | 枚举值 |
|------|----------|--------|
| 目标阵营 | 技能打谁？ | `Enemy` / `Ally` / `Self` |
| 选择策略 | 候选者中优先选谁？ | `Nearest` / `LowestHp` / `HighestAtk` |
| 数值计算 | 伤害/治疗怎么算？ | `fixed` / `AtkRatio` / `HpRatio` / `split` / `splash` |

所有技能的差异都可以归约到"选谁"+"怎么算"+"怎么执行"三个问题的不同回答。三组枚举的排列组合覆盖了回血、单体伤害、范围伤害、百分比伤害、溅射伤害等所有需求，无需为每种组合写一个新的 C++ 类。在此基础上增加了第四维——执行策略（`strategy` 字段），因为"即时结算"（SingleTargeted）、"范围结算"（MultiTargeted）、"附 Buff 结算"（BuffSingleTargeted）三者在 `execute()` 的流程上确实无法互相替代。三组枚举 × 三种策略即可覆盖所有当前和可预见的技能需求。

##### 关键代码流程

`Skill` 基类的核心是模板方法 `execute()`（[skill.cpp](src/entity/skill.cpp)）：

1. 调用纯虚函数 `selectTargets(units)` —— 子类各自实现目标筛选
2. 遍历结果，对每个目标调用纯虚函数 `calculateValue(caster, target, isPrimary)` —— 子类各自实现数值计算
3. 正数 = 治疗，负数 = 伤害，分别调用 Unit 的 heal/takeDamage
4. （Buff 子类额外步骤）遍历 Buff 配置，构造 `BuffInstance` 挂载到目标

基类统一了 1→2→3 的调用流程，子类只需回答"选谁"和"算多少"。加新技能策略时开发者只关注差异点，公共流程由基类保证。

#### 9.1.3 核心代码解析 —— 模块二：Buff 系统的三维抽象

##### 设计思路


| 维度 | 枚举 | 含义 |
|------|------|------|
| 类别 | `BuffCategory` | `Control`（跳过行动）/ `Dot`（周期伤害）/ `StatMod`（属性修正） |
| 影响属性 | `BuffStat` | `ATK` / `MaxMana` / `None`（仅 StatMod 有意义） |
| 叠加规则 | `BuffStackRule` | `Refresh` / `UniquePerSource` / `Independent` |

三维组合即可区分所有 Buff 类型。例如 `stun` = Control + Refresh，`dot` = Dot + UniquePerSource，`atk_affect` = StatMod(ATK) + Refresh。`Unit::addBuff()` 和 `BattleSystem::processBuffsPreActions()` 中的 Buff 处理逻辑完全由 `BuffDef` 的三个字段驱动（`switch (category)` / `switch (stackRule)`），代码是通用的——新增一种 Buff 类型只需在 `bufftypes.json` 中添加一个条目，C++ 端零修改。

##### 关键代码流程

Buff 从配置到生效的完整链路（参见 [## 8.2 以 Buff 类技能的挂载方式为例](#82-以-buff-类技能的挂载方式为例)）：

**第一层** — `bufftypes.json` 定义 Buff 的元类型（类别 + 叠加规则 + 影响属性），`BuffRegistry` 启动时加载。

**第二层** — `skills.json` 中，`BuffSingleTargeted` 策略的技能通过 `buffs` 数组指定要附着的 Buff（key + duration + 计算方式）。枚举组合（TargetType × SelectType × ValueType）决定技能的作用对象和数值来源。

**第三层** — `Unit::addBuff()` 根据 `BuffDef::stackRule` 决定新实例是刷新旧实例、按来源去重还是独立共存。每个战斗 tick 中：`processBuffsPreAction()` 遍历 Dot 累加周期伤害，`isDisabled()` 检查 Control 决定是否跳过行动，`statModSum()` 实时累加 StatMod 的属性修正。

#### 9.1.4 参数管理：从散装变量到 Param Struct

每个 Skill 子类的参数最初是逐个从 JSON 中读取到散装变量，再逐个传给构造函数的。AI 建议将这些参数封装为独立的 Params 结构体（[SingleTargetedParams](src/entity/skills/SingleTargetedSkill.h#L6-L17)、[MultiTargetedParams](src/entity/skills/MultiTargetedSkill.h#L11-L25)、[BuffSingleTargetParams](src/entity/skills/BuffSingleTargetSkill.h#L19-L28)），每个 struct 自带 `fromJson()` 静态工厂方法。

以 `SingleTargetedParams` 为例：

```cpp
struct SingleTargetedParams {
    QString name;
    TargetType targetType = TargetType::Enemy;
    SelectType selectType = SelectType::Nearest;
    ValueType valueType  = ValueType::AtkRatio;
    int castRange = 1;
    int value = 0;
    double ratio = 1.0;

    static SingleTargetedParams fromJson(const QJsonObject& json);
};
```

Skill 子类的构造函数直接接收类型安全的 `Params` 对象，不再接触 JSON。这一调整带来了几个实际好处：

**1. JSON 解析与业务逻辑分离。** 原先 JSON 字段的读取、字符串到枚举的转换、缺失字段的默认值处理全部混在 Skill 构造函数或 `execute()` 里，阅读业务逻辑时被 JSON 解析代码打断。`fromJson()` 把这些脏活集中到一处，Skill 类本身只接收已校验、已类型化的参数。三个子类的 `fromJson()` 共用了同一套匿名命名空间中的 `stringToTargetType` / `stringToSelectType` / `stringToValueType` 转换函数，避免了重复代码。

**2. 默认值在声明处直接可见。** 字段的 C++ 默认值直接写在 struct 的成员声明中（如 `int castRange = 1`、`double ratio = 1.0`）。如果 JSON 中缺少某个字段，`fromJson()` 中 `json["castRange"].toInt(1)` 的二次兜底配合 struct 默认值形成双层保护。读代码时看 struct 定义就知道每个参数的默认行为，不需要翻 `fromJson()` 的实现或 JSON 文件。

**3. 不同子类的参数差异由 struct 自然承载。** 三种策略子类的参数既有重叠（name、targetType、selectType、castRange），又有各自的独有字段——`SingleTargetedParams` 有 `value` 和 `ratio`，`MultiTargetedParams` 多了 `areaRange`、`areaCenter`、`splashRatio`，`BuffSingleTargetParams` 则包含一个 `QVector<BuffSkillParam>` 嵌套数组。如果用过继承来建模这种"部分相同、部分不同"的结构，会陷入字段该放在基类还是子类的争论；直接为每种策略各写一个朴素的 struct，字段只声明自己需要的，反而更清晰。

**4. 参数可独立测试和复用。** Skill 的单元测试可以直接在 C++ 中构造一个 `SingleTargetedParams{...}` 传给构造函数，不依赖 JSON 文件或 QJsonObject。调试时也可以观察 `fromJson()` 的输出 struct 来确认 JSON 解析是否正确，排查路径更短。

**5. 新增字段时光有默认值即可兼容旧 JSON。** `BuffSkillParam` 后来新增了 `damageInterval` 字段，struct 中给了 `int damageInterval = 6` 的默认值，`fromJson()` 中用了 `toInt(6)` 读取。已有的所有 `skills.json` 条目无需补写这个字段，旧配置自动兼容新逻辑。

AI 在此处的角色是：在我用散装变量写完了第一版 `SingleTargetedSkill` 后，AI 审阅代码并指出了参数管理的扩展性隐患——当第二个、第三个策略子类出现时，散装变量会让构造函数签名越来越长、JSON 解析逻辑四处重复。随后 AI 给出了 Params struct + `fromJson()` 的草案，三个 struct 在共同模式（字段 + 默认值 + 静态工厂）上保持一致，但在字段内容上各自独立。

##### 使用 AI 的整体感受

在整个开发过程中，AI 的核心价值不在于"自动写出代码"，而在于三个层面：

1. **架构决策的讨论对手**。当有多种设计方案可选时，AI 能逐一分析利弊，给出有说服力的推荐（如阶段二→三→四的推演过程）。这种对话式推演比独自思考更容易发现盲区。

2. **大规模代码理解的加速**。AI 能在短时间内通读整个代码库并提取关键信息（类关系、调用链、数据流向），输出的分析报告可以作为设计讨论的基础，节省了大量手动翻阅和绘图的时间。

3. **样板代码的生成**。接口设计确定后，AI 可以快速生成符合项目风格的样板代码（枚举到字符串的转换函数、JSON 解析的 fromJson 工厂方法），开发者在基础上调整逻辑细节即可。

### 9.2 敌方阵容启发式生成算法

`EnemySpawner` 负责在每关战斗前自动生成敌方阵容。生成流程分为两个阶段：第一阶段根据关卡配置随机抽取单位并掷骰星级（这部分逻辑直白，由我独立完成）；第二阶段将已确定的单位列表按合理的阵型排入敌方半场棋盘，这正是"启发式"二字的核心所在。

阵型生成的总体思路是与 AI 讨论得出的——将单位按角色归类，匹配预定义阵型模板，以代价矩阵评估排布质量并搜索最优分配。以下详细解释实现原理。

**阵型模板库。** [enemyspawner.cpp:41-135](src/core/enemyspawner.cpp#L41) 中 `buildTemplateLibrary()` 预定义了 14 个阵型模板，覆盖 1 到 8 人的规模。每个模板由若干个 `FormationSlot` 组成，包含三个字段：`rowOffset`（相对前排的行偏移，0=最前，负值=向后）、`colOffset`（相对中轴的列偏移）、`preferredRole`（该槽位期望的角色类型，Frontline/Midline/Backline）。例如 3 人规模的 `wedge-3` 模板：前排左右各一个 Frontline 槽位、后排正中一个 Backline 槽位——构成"双卫夹一后排"的楔形阵。

**代价矩阵与全排列搜索。** 单位根据 `UnitData` 中 `type` 字段被归类为三种角色：坦克/战士归 Frontline，刺客/辅助归 Midline，射手/法师归 Backline。对于每个槽位数匹配的模板，需要找到单位到槽位的最优分配。这里定义了一个 3x3 的代价矩阵 `mismatchCost[unitRole][slotRole]`：

```
//            slot: F    M    B
//   unit F         0    1    3
//   unit M         2    0    1
//   unit B        10    2    0
```

矩阵的设计意图很直白：角色与槽位匹配时代价为零；轻微错位（如 Midline 站 Frontline 槽）代价为 1-2，可以容忍；Backline 被分配到 Frontline 槽时代价为 10，严重惩罚——这意味着"宁可把坦克放错位置，也不能让脆皮暴露在前排"。需要说明的是，这些具体数值并未经过精细调参，只是实现了一个大致合理的方向性偏好。

搜索最优分配时，对每个候选模板遍历所有 n! 种排列（`std::next_permutation`），计算每种排列的总代价，取全局最低。敌方单场最多 8 人，8! = 40320，全排列搜索在毫秒内完成，因此没有引入匈牙利算法等更复杂的解法。

**HP 降级兜底。** 有时最优搜索仍可能得到代价 >= 10 的结果——例如本场生成的单位全部是 Backline 脆皮，无论怎么排都有人被推到前排。此时放弃角色匹配，转而按"HP 最高者站最前排"的纯血量策略重新分配：将单位按 HP 降序排列，槽位按"前排优先、同排居中优先"排列，一一对应。这个兜底确保了在任何极端单位组合下，阵型至少有一个物理上合理的站位。

**位置扰动。** 分配方案确定后，每个单位的基准坐标由 `CENTER_COL + slot.colOffset`（列）和 `ENEMY_FRONT_ROW + slot.rowOffset`（行）确定，其中 `ENEMY_FRONT_ROW = 3` 是敌方最前排（紧邻玩家半区），`CENTER_COL = 3` 是棋盘中央偏左的参考列。如果所有单位都严格按基准坐标放置，每场对战的敌方阵型将完全相同，缺乏变化。

`jitteredPosition()` 对每个槽位引入逐轮递减的随机扰动，共 5 轮尝试（[enemyspawner.cpp:240-272](src/core/enemyspawner.cpp#L240)）：

| 轮次 | 列扰动 (col) | 行扰动 (row) | 说明 |
|------|-------------|-------------|------|
| 0-1 | `{-1, 0, +1}` | `{0, +1}` | 最大自由度，行方向只向后不向前（row 增大 = 远离玩家半区），防止敌方单位意外侵入玩家半场 |
| 2-3 | `{-1, 0, +1}` | `0` | 仅允许列偏移，行锁定基准值 |
| 4 | `0` | `0` | 回退到基准坐标，保证至少有一个合法落点 |

每轮生成的候选坐标需满足三个条件：在棋盘范围内（`isBoardPosition`）、不在玩家半区（`!isPlayerHalf`）、不与其他已放置单位重叠（`!occupied.contains`）。一旦找到合法坐标立即返回并占位，后续槽位不会与之冲突。

放置顺序是关键：`applyTemplateFormation()` 将所有槽位按 `rowOffset` 从大到小（前排到后排）排序后逐个调用 `jitteredPosition()`。这样一来，前排单位优先占格，后排单位的扰动不会挤占前排已确认的位置。最终效果是敌方每场对战的站位在大致遵循阵型模板的前提下，每个单位的具体坐标有 1 格以内的随机偏移，阵容看起来相似但不会完全雷同。

### 9.3 存档/读档 GUI 的实现与信号流

存档/读档对话框的 GUI 和交互逻辑由 AI 辅助生成，本节从 GUI 绘制和信号流两个角度解释其运作原理。

**GUI 绘制。** `SaveLoadDialog` 继承 `QDialog`（920x520，模态），根据 `Mode::Save` 或 `Mode::Load` 展示略有不同的界面。构造在 `setupUI()` 中完成（[saveloaddialog.cpp:153-276](src/gui/saveloaddialog.cpp#L153)）：

| 组件 | 类型 | 说明 |
|------|------|------|
| 标题 | `QLabel` | "保存游戏" / "读取存档"，18px 加粗居中 |
| 状态摘要 | `QLabel` | 仅保存模式，灰色背景条提示"当前游戏状态将保存到选中的槽位" |
| 4x2 卡片网格 | `QGridLayout` | 8 张 `SaveSlotCard`，每张 200x120 的 `QFrame`，间距 16px |
| 备注输入 | `QLineEdit` | 仅保存模式，maxLength=30，placeholder 提示可选输入备注 |
| 确认/取消 | `QPushButton` | 确认按钮初始禁用，选中合法槽位后才启用 |

每张 `SaveSlotCard` 内部由 5 个 `QLabel` 构成：槽位编号 `[1]`~`[8]`、状态文本（空槽位显示"— 空 —"，有数据则显示时间戳）、轮次与等级摘要、HP/金币（带 HTML 富文本颜色）、用户备注预览（橙色）。卡片的视觉状态通过 `applyStyleSheet()` 在三种模式间切换：

| 状态 | 边框色 | 背景色 | 触发条件 |
|------|--------|--------|----------|
| 普通 | `#444455` | `#252530` | 默认 |
| 悬浮 | `#666688` | `#2a2a35` | 鼠标进入 `enterEvent` |
| 选中 | `#5599dd` | `#2a3a4a` | 用户点击该卡片 |
| 空槽（叠加） | — | `#222228` | `SaveMeta::isEmpty == true` 时覆盖背景色 |

**信号流与数据传递。** 整个交互链涉及 `SaveSlotCard` → `SaveLoadDialog` → `GameWindow` 三个类，跨越 `saveloaddialog.cpp` 和 `gamewindow.cpp` 两个文件。核心原则：`SaveLoadDialog` 是一个"纯选择器"——它只管理 UI 和用户选择状态，不执行任何文件 I/O 或游戏状态修改。实际的存/读操作全部在 `GameWindow` 中完成。

流程分四步（参见 [gamewindow.cpp:54-84](src/gui/gamewindow.cpp#L54)）：

**第一步 — 卡片点击到对话框状态更新。** `SaveSlotCard::mousePressEvent()` 发射 `clicked(int slot)` 信号，连接到 `SaveLoadDialog::onSlotClicked(int slot)`（[saveloaddialog.cpp:233](src/gui/saveloaddialog.cpp#L233)）。该槽函数依次：取消所有卡片的选中态 → 将当前卡片设为选中并记录 `m_selectedSlot` → 启用确认按钮。读档模式下，点击空槽位直接 `return` 且不启用确认按钮。保存模式下，点击已有存档槽位时自动将该存档的备注文本回填到 `QLineEdit` 中，方便用户修改或沿用。

**第二步 — 确认/取消到 QDialog 返回值。** 确认按钮连接到 `onConfirmClicked()`（[saveloaddialog.cpp:319-358](src/gui/saveloaddialog.cpp#L319)）：保存模式下若目标槽位非空，弹出 `QMessageBox::warning` 展示已有存档的详情（时间/轮次/等级/HP/金币/备注），用户选 Yes 才继续，选 No 则 `return` 不关闭对话框。通过覆盖检查后，调用 `QDialog::accept()`——这是 Qt 内置方法，将对话框的执行结果设为 `QDialog::Accepted` 并关闭窗口。取消按钮连接到 `onCancelClicked()`（[saveloaddialog.cpp:360-364](src/gui/saveloaddialog.cpp#L360)）：将 `m_selectedSlot = -1` 后调用 `QDialog::reject()`，将结果设为 `Rejected` 并关闭。

**第三步 — 调用方读取对话框结果。** `GameWindow` 中两处调用的模式一致（[gamewindow.cpp:54-84](src/gui/gamewindow.cpp#L54)）：创建 `SaveLoadDialog` 对象 → 调用 `dlg.exec()`（模态阻塞，等待用户操作完成）→ 检查返回值是否为 `QDialog::Accepted` → 通过 `dlg.selectedSlot()` 获取槽位号、`dlg.label()` 获取备注文本。`selectedSlot()` 和 `label()` 是对话框仅有的两个公有读取接口，将用户选择以最简形式暴露给调用方。

**第四步 — 执行实际数据操作。** 保存路径：`GameWindow` 调用 `m_game->collectSaveData()` 从当前游戏状态收集 `SaveData`（Player/Enemy/Board/Store/EquipBar 的完整快照），然后调 `SaveManager::saveToFile(slot, data, label)` 序列化为 JSON 写入 `build/Debug/saves/` 目录。读取路径：`SaveManager::loadFromFile(slot)` 反序列化 JSON 返回 `std::optional<SaveData>`，然后调 `m_game->applySaveData(data)` 完整恢复游戏状态。

**数据在各文件间的流动方向：**

```
SaveSlotCard                  SaveLoadDialog                GameWindow                   SaveManager / Game
    |                             |                            |                              |
    |-- clicked(slot) ----------->|                            |                              |
    |                             |-- onSlotClicked()          |                              |
    |                             |   (更新选中态、启用按钮)     |                              |
    |                             |                            |                              |
    |                             |<-- 用户点击 确认/取消 -------|                              |
    |                             |-- accept() / reject()     |                              |
    |                             |                            |                              |
    |                             |  dlg.exec() 返回 Accepted  |                              |
    |                             |                            |                              |
    |                             |<-- selectedSlot() ---------|                              |
    |                             |<-- label() ----------------|                              |
    |                             |                            |-- collectSaveData() -------->| Game
    |                             |                            |<-- SaveData -----------------| 
    |                             |                            |-- saveToFile/loadFromFile -->| SaveManager
    |                             |                            |-- applySaveData() ---------->| Game
```

`SaveLoadDialog` 本身不触碰 `Game`、不触碰 `SaveManager`、不读写文件——它唯一的职责是让用户选择一个槽位并返回选择结果。这种"对话框只负责选择、调用方负责执行"的模式，保持了 GUI 层和逻辑层的清晰边界。

### 9.4 代码风格：匿名 Namespace 的两种应用场景

在与 AI 协作过程中，有两处代码风格改进值得记录。它们的共同点是利用 C++ 匿名 namespace（或具名常量 namespace）替换难以维护的原始写法，但在动机上各有侧重。

**场景一：长 Lambda 提取为命名函数（battlesystem.cpp）。** [battlesystem.cpp:7-48](src/core/battlesystem.cpp#L7) 顶部有一个匿名 namespace，包含三个比较函数：

```cpp
namespace {
    bool moveActionLess(const PlannedAction& a, const PlannedAction& b) { ... }
    bool fallbackLess(const QPoint& a, const QPoint& b, const QPoint& target) { ... }
    bool targetLess(Unit* self, Unit* a, Unit* b) { ... }
}
```

这些函数最初是直接写在 `std::sort` 和 `std::min_element` 调用中的 inline lambda。以 `targetLess` 为例，它包含 9 行比较逻辑（距离近 → 血量低 → Y 小 → X 小的四级级联判断）。如果内嵌在 `std::min_element` 的第三个参数位置，调用代码会变成：

```cpp
auto it = std::min_element(enemies.begin(), enemies.end(),
    [self](Unit* a, Unit* b) {
        int distA = hexDistance(self->pos(), a->pos());
        int distB = hexDistance(self->pos(), b->pos());
        if (distA != distB) return distA < distB;
        if (a->hp() != b->hp()) return a->hp() < b->hp();
        // ... 继续 6 行
    });
```

阅读时视线必须跨越整段 lambda 才能看到外层在做什么。AI 建议将这类逻辑提取到匿名 namespace 中的具名函数，调用点压缩为一行 `return targetLess(self, a, b);`，比较逻辑有独立的函数名和签名，可读性明显提升。匿名 namespace 保证这些函数仅在当前翻译单元可见，不会污染全局符号表。

**场景二：JSON Key 字符串常量化（savemanager.cpp）。** [savemanager.cpp:14-47](src/core/savemanager.cpp#L14) 定义了 `namespace JsonKey`，包含约 20 个 `constexpr auto` 字符串常量：

```cpp
namespace JsonKey {
    constexpr auto kVersion      = "version";
    constexpr auto kTimestamp    = "timestamp";
    constexpr auto kLabel        = "label";
    constexpr auto kBattleIndex  = "battleIndex";
    // ... 共约 20 个 key 常量
}
```

存档模块涉及大量 JSON 序列化/反序列化，需要反复使用同一组 key 字符串（`"timestamp"`、`"battleIndex"`、`"hp"` 等）。如果使用裸字符串字面量，拼写错误（如 `"timeStamp"` 写成驼峰、`"battle_index"` 写成下划线风格）不会被编译器检测，只会在运行时表现为字段静默丢失或读档失败。IDE 对字符串字面量也不提供自动补全。

将 key 提取为 `constexpr auto` 常量后，IDE 可以自动补全 `JsonKey::` 下的所有 key 名；如果误写为 `JsonKey::kTimeStamp`（不存在的标识符），编译期即报错，无需等到运行时调试。

### 9.5 代码版本管理

项目代码托管于 GitHub：[yfxu221/cpp_PA_autobattler](https://github.com/yfxu221/cpp_PA_autobattler.git)
