# Synera: Synergy Auto-Arena

基于 Qt6 与 C++17 的单机 PVE 自走棋游戏，南京大学高级程序设计课程 PA 项目。

---

## 1. 基本信息

| 项目 | 内容 |
|------|------|
| 姓名 | 徐** |
| 学号 | 25188**** |
| 项目名称 | Synera: Synergy Auto-Arena |
| 开发环境 | Qt 6.8.3 + MSVC 2022 + C++17 + CMake 3.16+ |

### 各阶段完成度

| 阶段 | 内容 | 状态 |
|------|------|------|
| 阶段一 | 基础框架、单位与交互 | 完成 |
| 阶段二 | 自动战斗核心逻辑 | 完成 |
| 阶段三 | 经济系统与进阶玩法 | 完成 |
| 阶段四 | 扩展功能 | 部分完成（利息机制 + 连胜/连败奖励） |

---

## 2. 文件树结构

```
Synera_starter/
├── README.md                         # 本文档
├── CMakeLists.txt                    # CMake 构建配置 (Qt6, C++17)
├── assets/                           # 美术资源（精灵图、序列帧）
│   ├── */normal/                     #   单位精灵图
│   ├── craftpix-*/                   #   第三方精灵资源
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
    │   ├── pathfinder.h / .cpp       #     六边形网格寻路（BFS 算法）
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
    │       └── BuffSingleTargetSkill #       带 Buff 附着的单体技能（眩晕、Dot，增益减益）
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
| `BoardANDBench` | `core/board.h` | 棋盘 + 备战席数据结构。管理 8×8 棋盘和 1×10 备战区的单位占位，维护 `QVector<Unit*>` 存储数组和 `QHash<Unit*, QPoint>` 反向索引。提供增删、移动、查询、合法性校验等接口 |
| `Unit` | `entity/unit.h` | 单位实体类。包含基础属性（HP/ATK/Range/MaxMana/Mana/Speed）、星级（影响属性倍率：1 星=1.0, 2 星=1.8, 3 星=3.24）、特质标签(traits)、归属(owner)、Buff 列表、装备列表、技能冷却等。所有单位（我方/敌方）统一使用此类，仅通过 `owner` 区分 |
| `UnitData` | `entity/unitdata.h` | 单位模板注册表（单例）。加载 `data/units.json`，提供 `createUnit(key, owner, star)` 工厂方法按模板生成单位实例 |

### 3.2 战斗系统

| 类名 | 文件 | 主要功能 |
|------|------|----------|
| `BattleSystem` | `core/battlesystem.h` | 自动战斗引擎。基于 QTimer（50ms/帧）循环驱动，每 tick 依次处理：冷却更新、目标搜索、移动/攻击/技能决策、伤害结算、死亡清理、胜负判定。战斗结束后发射 `battleFinished(BattleResult)` 信号 |
| `Pathfinder` | `core/pathfinder.h` | 六边形网格 BFS 寻路（静态工具类）。使用立方体坐标系统（x+y+z=0）进行六边形距离计算和邻居遍历，返回最短路径供单位移动 |
| `Skill` | `entity/skill.h` | 技能抽象基类。定义纯虚接口：`targetType()`（敌方/友方/自身）、`selectType()`（最近/最低血量/最高攻击）、`castRange()`、`selectTargets()`、`calculateValue()`、`execute()` |
| `SkillRegistry` | `entity/skillregistry.h` | 技能注册表（单例）。加载 `data/skills.json`，根据 `strategy` 字段工厂创建 `SingleTargetedSkill` / `MultiTargetedSkill` / `BuffSingleTargetSkill` 实例 |
| `SingleTargetedSkill` | `entity/skills/` | 单体目标技能。选择一个目标单位，计算伤害/治疗值后执行 |
| `MultiTargetedSkill` | `entity/skills/` | 多目标/AoE 技能。以施法者或目标为范围中心，对范围内所有合法目标造成伤害（支持溅射比例 `splashRatio`） |
| `BuffSingleTargetSkill` | `entity/skills/` | 附带 Buff 的单体技能。在造成伤害/治疗后，向目标附加 JSON 配置中定义的 Buff（眩晕/Dot/属性修正等） |
| `EnemySpawner` | `core/enemyspawner.h` | 敌方波次生成器（单例）。读取 `data/stages.json`，按回合索引生成包含单位 key、星级、位置、装备的 `EnemySpawnPlan` 列表 |

### 3.3 经济与商店

| 类名 | 文件 | 主要功能 |
|------|------|----------|
| `Player` | `entity/player.h` | 玩家状态。维护金币、血量、等级（最高 7 级）、经验值与升级经验曲线 |
| `Store` | `core/store.h` | 商店系统。管理 5 个招募槽位，支持随机刷新（消耗 2 金币）和购买（消耗对应金币，落位至备战区） |

### 3.4 羁绊系统

| 类名 | 文件 | 主要功能 |
|------|------|----------|
| `SynergyRegistry` | `entity/synergyregistry.h` | 羁绊规则注册表（单例）。加载 `data/synergies.json`，按 trait 名称维护阈值列表。`getBonus(trait, count)` 根据场上同 trait 单位数量取最高满足阈值的 `SynergyBonus`，多 trait 单位可叠加多个加成 |
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
| `GameWindow` | `gui/gamewindow.h` | 主窗口 (QMainWindow, 1400×850)。左侧 QGraphicsView 承载游戏场景，右侧面板展示玩家/敌方信息、轮次、按钮（战斗/刷新/升人口/存档/读档/重开） |
| `GridItem` | `gui/griditem.h` | 六边形格子图元 (QGraphicsObject)。渲染单个六边形地块，支持 hover 高亮和 drop 高亮两种视觉反馈 |
| `UnitItem` | `gui/unititem.h` | 单位图元 (QGraphicsObject)。懒加载 PNG 精灵图，显示 HP 条、蓝条、星级标记、装备图标；处理鼠标拖拽事件并发射信号 |
| `StoreSlotItem` | `gui/storeslotitem.h` | 商店槽位图元。六边形样式，显示单位精灵、名称、价格，支持点击购买 |
| `StoreRefreshButton` | `gui/storerefreshbutton.h` | 刷新按钮图元。圆角矩形，显示"Refresh (2G)"，金币不足时变暗 |
| `SellZoneItem` | `gui/sellzoneitem.h` | 出售区域图元。红色矩形区域，拖拽单位至此时高亮反馈 |
| `SettlementDialog` | `gui/settlementdialog.h` | 战斗结算弹窗 (QDialog)。展示胜负结果、HP 变化、金币变化 |
| `LootDialog` | `gui/lootdialog.h` | 战利品选择弹窗 (QDialog)。装备栏满时弹出，展示新获得装备与现有装备，玩家选择保留哪些 |
| `SaveLoadDialog` | `gui/saveloaddialog.h` | 存档/读档界面 (QDialog)。4×2 网格展示 8 个存档槽，显示时间戳/轮次/等级/HP/金币等元数据 |

---

## 4. 算法描述

### 4.1 六边形网格寻路 (BFS)

由于棋盘采用六边形网格（错列排布），不能直接使用四方向或八方向的曼哈顿网格寻路。

**坐标系统。** 使用两套坐标：偏移坐标 (offset coordinates, `row, col`) 用于存储和渲染，立方体坐标 (cube coordinates, `x, y, z` 满足 `x+y+z=0`) 用于距离计算和邻居遍历。`Pathfinder` 提供 `offsetToCube()` 和 `cubeToOffset()` 进行双向转换。转换公式（even-r 布局）：

```
x = col - (row + (row & 1)) / 2
z = row
y = -x - z
```

**六边形距离。** `hexDistance(a, b) = (|a.x - b.x| + |a.y - b.y| + |a.z - b.z|) / 2`，等价于 `max(|dx|, |dy|, |dz|)`。

**邻居遍历。** 在立方体坐标下使用 6 个固定方向向量，避免偏移坐标下偶数行/奇数行需要不同偏移量的繁琐处理：

```
directions[6] = {
    {+1, -1,  0}, {+1,  0, -1}, { 0, +1, -1},
    {-1, +1,  0}, {-1,  0, +1}, { 0, -1, +1}
}
```

**BFS 流程**（[pathfinder.cpp](src/core/pathfinder.cpp)）：

1. 若 `start == target`，无需移动，返回空路径
2. 若 `hexDistance(start, target) <= attackRange`，单位已在攻击范围内，返回仅含 `start` 的路径（无需移动即可攻击）
3. 初始化 `QQueue` 队列、`visited` 访问标记、`parent` 回溯数组
4. BFS 主循环：取队首节点，若该节点与 `target` 的六边形距离 `<= attackRange`，终止搜索；否则遍历 6 个邻居，跳过越界、已访问、被占用的格子，将合法邻居入队并记录父节点
5. 若 BFS 结束仍未找到可达节点，返回空路径（无路可走）
6. 通过 `parent` 数组回溯路径，`std::reverse` 后返回

为何 BFS 而非 A\*：六边形网格中每条边的代价相等（均为 1 步），BFS 保证首次到达目标即为最短路径，且无需设计启发函数和优先队列。棋盘规模仅为 8×8（64 格），性能满足实时战斗需求。

### 4.2 目标锁定

项目中有两层目标选择逻辑：

**普攻/移动索敌**（`targetLess` 比较器，[battlesystem.cpp](src/core/battlesystem.cpp)）。从敌对阵营存活单位中按四级级联排序选出最优目标：

1. 六边形距离最近
2. 距离相同时，HP 最低（集火残血单位）
3. HP 相同时，Y 坐标最小
4. Y 相同时，X 坐标最小

后两级平局规则保证选择结果的确定性，避免同一距离/血量下不同 tick 随机切换目标。

**技能目标选择**（`SelectType` 枚举，[skill.h](src/entity/skill.h)）。技能通过 `SelectType`（`Nearest` / `LowestHp` / `HighestAtk`）在候选者中按指定策略挑选目标，与 `TargetType`（敌方/友方/自身）独立组合，3×3 = 9 种组合可产生"处决敌方最高攻击者""治疗最近友方"等不同战术语义。

### 4.3 羁绊加成计算

[game.cpp](src/core/game.cpp) 中 `recalculateSynergies()` 负责每次布阵变动后重算羁绊加成：

1. **统计**：遍历玩家场上所有存活单位，按 `traits` 标签分组计数
2. **查表**：对每个 trait，调用 `SynergyRegistry::getBonus(trait, count)`，在预加载的阈值列表中匹配最高满足档位
3. **叠加**：遍历场上所有单位，将其所有 trait 对应的 `SynergyBonus` 通过 `operator+=` 累加到单位属性上。同一单位拥有多个 trait 时，加成叠加
4. **特殊机制**：除属性光环外，羁绊可包含机制改变类加成（如 Dot 触发间隔缩减 `dotIntervalReduction`），在战斗 tick 中被 `BattleSystem` 读取使用

---

## 5. 辅助函数

以下列出项目中关键的全局/静态工具函数。

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
| `findEmptyBenchSlot()` | `core/game.cpp` | 在备战区 1×10 中寻找第一个空槽位，购买单位时使用 |
| `countFieldUnits(owner)` | `core/game.cpp` | 统计场上某方（我方/敌方）存活单位数量 |
| `randomStarByLevel(level)` | `core/game.cpp` | 根据玩家等级按概率生成 1-3 星单位，商店刷新时使用 |
| `tryMergeStar(unit)` | `core/game.cpp` | 检查备战区+场上是否有 3 个同名同星单位，触发自动升星合并（3 合 1） |
| `dotIntervalReduction(owner)` | `core/game.cpp` | 累加某方羁绊提供的 Dot 触发间隔缩减总量 |
| `isOverSellZone(scenePos)` | `core/game.cpp` | 判断场景坐标是否落在出售区域内 |
| `recalculateSynergies()` | `core/game.cpp` | 遍历场上单位按 trait 分组统计，查 SynergyRegistry 取最高满足阈值，叠加加成 |

---

## 6. AI 使用说明

### 6.1 项目规划：技能系统从"完全数据驱动"到"机制性继承"的演变

技能系统的设计不是一步到位的，而是经历了四个阶段的推演，每个阶段都在上一轮的基础上发现问题、调整方向，最终收敛到当前方案。整个过程中，AI（Deepseek 接入 Claude Code）充当了讨论对手和瓶颈诊断者的角色。

**阶段一：Skill 绑定而非 Unit 继承。** 项目从一开始就确定 Unit 采用数据驱动：`units.json` 定义所有单位的属性模板，`UnitData` 工厂按模板创建实例，所有单位共享同一个 `Unit` 类。这套设计运行良好，所以在考虑技能系统时，首先排除的就是"为每个英雄写一个 Unit 子类"这个方向。保持 Unit 不变、技能作为外部组件绑定上去，这个方向从一开始就明确了。

**阶段二：试图用"一个 Skill 类 + 全 JSON 描述"走到底。** 受 Unit 数据驱动思路的影响，一开始想把技能也做成完全数据驱动的：只写一个 `Skill` 类，所有行为差异全在 JSON 中描述。与 AI 讨论后，AI 指出了两个问题：(1) 同一个 `Skill` 对象里，`scope` 是 `single` 时 `blastRadius` 和 `splashRatio` 毫无意义，大量字段在多数技能上是冗余的；(2) `execute()` 里需要嵌套 switch 判断伤害类型、范围、数值公式、是否有 Buff——核心逻辑会变成一张难以阅读的真值表。此时意识到：完全数据驱动对 Unit 行得通是因为所有 Unit 共享完全相同的属性结构和行为流程，但技能之间的行为流程存在质的差异（即时结算 vs 范围溅射 vs 附 Buff 持续结算）。

**阶段三：走到反面——"一个技能一个子类"。** 既然一个类消化不了所有差异，自然想到为每个具体技能写一个 Skill 子类。但 AI 指出了对称的问题：子类数量随技能数线性增长，而且很多技能之间只有数值参数不同——为了一个数字就要新建类并重新编译，JSON 退化为参数微调表，丧失了数据驱动新增技能的能力。

**阶段四：收敛——"机制性继承"与正交枚举的组合。** 在和 AI 的多轮讨论中，逐一拆解了每个现有技能的执行流程，逐步骤判断"这是机制不同（必须走不同代码路径）还是参数不同（可以归约到数值选择）"。由此得出设计原则：**继承只区分"执行机制"（execute 的流程不同），JSON 枚举负责"参数组合"（目标选谁、数值怎么算）。** 在这个原则下，我设计了三组正交的描述性词条（TargetType、SelectType、ValueType）和三种抽象的策略子类（`SingleTargetedSkill`、`MultiTargetedSkill`、`BuffSingleTargetSkill`）。三组枚举自由组合负责内容差异，三种子类负责流程差异，两者在 JSON 中由 `strategy` 字段桥接。

| 阶段 | 思路 | 问题 | 保留了什么 |
|------|------|------|-----------|
| 一 | Skill 绑定到 Unit，不碰 Unit 继承 | 方向正确，具体做法待定 | Unit 数据驱动不受影响 |
| 二 | 一个 Skill 类，全 JSON 描述 | 字段冗余、嵌套 switch 不可维护 | "数据驱动技能"的意愿 |
| 三 | 一个技能一个子类 | 类爆炸、JSON 丧失新增技能的能力 | 继承管理行为差异的思路 |
| 四 | 三种机制子类 + 正交枚举组合 | 收敛 | 继承管流程（机制），JSON 管参数（内容）|

AI 在此过程中的角色：(1) 两轮瓶颈诊断——指出"单类承载所有可能性"和"一个子类一个技能"各自的问题；(2) 机制 vs 内容的边界推演——帮助拆解每个技能的执行流程，定位"单体/AOE/Buff 三种执行流程"的分界线；(3) 验证与辅助——检查现有技能是否都能被枚举组合覆盖，辅助完成样板代码编写。

### 6.2 核心代码解析 —— 模块一：技能抽象基类设计

**设计思路。** 确定"技能用继承"后，所有技能的执行流程被拆解为三个正交的决策维度，每个维度用一个枚举表达：目标阵营 `TargetType`（Enemy/Ally/Self）、选择策略 `SelectType`（Nearest/LowestHp/HighestAtk）、数值计算 `ValueType`（fixed/AtkRatio/HpRatio/split/splash）。三组枚举的排列组合覆盖了回血、单体伤害、范围伤害、百分比伤害、溅射伤害等所有需求。在此基础上增加了第四维——执行策略（`strategy` 字段），因为"即时结算""范围结算""附 Buff 结算"三者在 `execute()` 的流程上确实无法互相替代。

**关键代码流程。** `Skill` 基类的核心是模板方法 `execute()`（[skill.cpp](src/entity/skill.cpp)）：

1. 调用纯虚函数 `selectTargets(units)` —— 子类各自实现目标筛选
2. 遍历结果，对每个目标调用纯虚函数 `calculateValue(caster, target)` —— 子类各自实现数值计算
3. 正数 = 治疗，负数 = 伤害，分别调用 Unit 的 heal/takeDamage
4. （Buff 子类额外步骤）遍历 Buff 配置，构造 `BuffInstance` 挂载到目标

基类统一了 1→2→3 的调用流程，子类只需回答"选谁"和"算多少"。加新技能策略时开发者只关注差异点，公共流程由基类保证。

### 6.3 核心代码解析 —— 模块二：Buff 系统的三维抽象

**设计思路。** Buff 系统通过三维分类定义所有 Buff 类型，三维组合即可区分所有 Buff 行为：

| 维度 | 枚举 | 含义 |
|------|------|------|
| 类别 | `BuffCategory` | `Control`（跳过行动）/ `Dot`（周期伤害）/ `StatMod`（属性修正） |
| 影响属性 | `BuffStat` | `ATK` / `MaxMana` / `None`（仅 StatMod 有意义） |
| 叠加规则 | `BuffStackRule` | `Refresh`（刷新取 max）/ `UniquePerSource`（同源刷新，异源独立）/ `Independent`（各自独立） |

例如 `stun` = Control + Refresh，`dot` = Dot + UniquePerSource，`atk_affect` = StatMod(ATK) + Refresh。`Unit::addBuff()` 和 `BattleSystem::processBuffsPreActions()` 中的 Buff 处理逻辑完全由 `BuffDef` 的三个字段驱动（`switch (category)` / `switch (stackRule)`），代码是通用的——新增一种 Buff 类型只需在 `bufftypes.json` 中添加一个条目，C++ 端零修改。

**关键代码流程。** Buff 从配置到生效的完整链路分为三层：

- **第一层**：`bufftypes.json` 定义 Buff 的元类型（类别 + 叠加规则 + 影响属性），`BuffRegistry` 启动时加载。
- **第二层**：`skills.json` 中，`BuffSingleTargeted` 策略的技能通过 `buffs` 数组指定要附着的 Buff（key + duration + 计算方式）。枚举组合（TargetType × SelectType × ValueType）决定技能的作用对象和数值来源。
- **第三层**：`Unit::addBuff()` 根据 `BuffDef::stackRule` 决定新实例是刷新旧实例、按来源去重还是独立共存。每个战斗 tick 中：`processBuffsPreAction()` 遍历 Dot 累加周期伤害，`isDisabled()` 检查 Control 决定是否跳过行动，`statModSum()` 实时累加 StatMod 的属性修正。

### 6.4 参数管理：从散装变量到 Param Struct

每个 Skill 子类的参数最初是逐个从 JSON 中读取到散装变量，再逐个传给构造函数的。AI 建议将这些参数封装为独立的 Params 结构体（`SingleTargetedParams`、`MultiTargetedParams`、`BuffSingleTargetParams`），每个 struct 自带 `fromJson()` 静态工厂方法。

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

这一调整带来了几个实际好处：

1. **JSON 解析与业务逻辑分离。** `fromJson()` 把字符串到枚举的转换、缺失字段的默认值处理集中到一处，Skill 类本身只接收已校验、已类型化的参数。
2. **默认值在声明处直接可见。** 字段的 C++ 默认值直接写在 struct 的成员声明中（如 `int castRange = 1`），读代码时看 struct 定义就知道每个参数的默认行为。
3. **不同子类的参数差异由 struct 自然承载。** 三种策略子类的参数既有重叠又有各自的独有字段——直接为每种策略各写一个朴素的 struct，字段只声明自己需要的，反而比继承层次更清晰。
4. **参数可独立测试和复用。** 单元测试可以直接在 C++ 中构造一个 `SingleTargetedParams{...}` 传给构造函数，不依赖 JSON 文件或 QJsonObject。
5. **新增字段时光有默认值即可兼容旧 JSON。** `BuffSkillParam` 后来新增了 `damageInterval` 字段，struct 中给了默认值，已有的所有 `skills.json` 条目无需补写这个字段，旧配置自动兼容新逻辑。

### 6.5 其他 AI 辅助模块

**敌方阵容生成**（[enemyspawner.cpp](src/core/enemyspawner.cpp)）。AI 辅助设计了启发式阵型排布算法。预定义 14 个阵型模板（覆盖 1~8 人），将单位按角色归类（Frontline/Midline/Backline），以代价矩阵评估单位到槽位的匹配质量，通过全排列搜索最优分配。极端情况下回退到 HP 降序排列兜底。最后通过逐轮递减的随机扰动使每场敌方站位有细微变化，避免完全雷同。

**存档/读档 GUI**（[saveloaddialog.cpp](src/gui/saveloaddialog.cpp)）。AI 辅助生成了 `SaveLoadDialog` 的完整 GUI 和交互逻辑。对话框采用 4×2 卡片网格展示 8 个存档槽，每张卡片通过 `enterEvent`/鼠标点击切换悬浮/选中态。信号流遵循"对话框只负责选择、调用方负责执行"的原则：卡片点击更新选中态 → 确认按钮触发 `accept()` → `GameWindow` 读取槽位号后调用 `SaveManager` 执行实际 I/O。`SaveLoadDialog` 本身不触碰 `Game` 或 `SaveManager`，保持了 GUI 层和逻辑层的清晰边界。

**代码风格改进。** AI 建议了两处改进：(1) 将 `battlesystem.cpp` 中的长 lambda 比较函数提取为匿名 namespace 中的具名函数（`moveActionLess`、`targetLess`），提升 `std::sort` 调用点的可读性；(2) 将 `savemanager.cpp` 中反复使用的 JSON key 字符串提取为 `constexpr auto` 常量（`JsonKey` namespace），利用编译期检查避免拼写错误，IDE 也可自动补全。

### 6.6 使用 AI 的整体感受

在整个开发过程中，AI 的核心价值不仅在于"自动写出代码"，也在于这三个层面：

1. **架构决策的讨论。** 当有多种设计方案可选时，AI 能逐一分析利弊，给出有说服力的推荐（如阶段二→三→四的推演过程）。这种对话式推演比独自思考更容易发现盲区。
2. **大规模代码理解的加速。** AI 能在短时间内通读整个代码库并提取关键信息（类关系、调用链、数据流向），输出的分析报告可以作为设计讨论的基础，节省了大量手动翻阅和绘图的时间。
3. **样板代码的生成。** 接口设计确定后，AI 可以快速生成符合项目风格的样板代码（枚举到字符串的转换函数、JSON 解析的 fromJson 工厂方法），开发者在基础上调整逻辑细节即可。

---

## 7. 数据驱动设计概述

本项目所有单位模板、技能参数、羁绊规则、关卡配置、装备属性和 Buff 类型均存储在 `data/` 目录的 JSON 文件中，C++ 代码仅负责读取和运行时执行，不硬编码任何具体数值。新增单位、装备、技能或 Buff 类型只需在 JSON 中添加条目，无需修改 C++ 代码。这一设计使得修改游戏对非程序员同样可行，也可作为未来服务器端校验或 RL 实验框架的数据基础。

**技能系统**采用"策略模式 + 工厂模式"架构。抽象基类 `Skill` 定义统一接口（目标选择 → 数值计算 → 执行），`SingleTargetedSkill`、`MultiTargetedSkill`、`BuffSingleTargetSkill` 三个子类各自实现不同的执行流程。`SkillRegistry` 根据 JSON 中的 `strategy` 字段工厂创建对应实例。技能的行为差异由三组正交枚举（TargetType × SelectType × ValueType）参数化——三组枚举自由组合即可产生大量技能变体，仅当需要全新执行模式时才新增子类。

**Buff 系统**通过三维分类（Category × StackRule × Stat）定义所有 Buff 类型。`Control` 阻止行动、`Dot` 周期伤害/治疗、`StatMod` 属性修正；叠加规则（Refresh / UniquePerSource / Independent）决定重复施加时的行为。`BattleSystem` 的 tick 逻辑完全由 `BuffDef` 的三个字段驱动，代码通用。新增 Buff 类型只需在 `bufftypes.json` 中添加一个条目。

**设计原则。** 项目中继承的使用遵循一个简单判断：当实体的执行流程存在本质差异时，使用继承（如 Skill 子类）；当差异可以归约到数据和可组合的模块时，优先使用数据驱动和组合（如 Unit + Skill + Buff）。这种分工将行为变化的"责任"集中到 Skill 和 Buff 体系中，避免了两处同时管理行为逻辑的复杂性。

---

## 8. 战斗系统：决策-执行分离

战斗以 tick 为单位推进（QTimer, 50ms/tick）。核心设计选择是将"决策"与"执行"解耦——所有单位在同一时刻、基于同一份游戏状态快照做出决策；决策全部收集完毕后，再分阶段执行；伤害和死亡在 tick 末尾批量结算。

| 原则 | 说明 |
|------|------|
| 决策基于快照 | tick 开始时遍历所有单位做决策，`decideAction()` 只读状态不写状态 |
| 执行分阶段 | 移动、技能、普攻各自独立结算，互不穿插 |
| 伤害批量结算 | 所有伤害/治疗事件累积到 `m_pendingDamageEvents`，tick 末尾统一应用 |
| 死亡批量结算 | 所有 HP ≤ 0 的单位在 `resolveDeaths()` 中统一标记为 Dead |

这带来了几个关键优势：**消除"未来视"**（所有单位的移动目标在决策阶段已基于同一快照确定，后续移动互不影响寻路决策）；**控制技能公平**（Buff 在执行阶段才挂载，决策阶段早已完成——本 tick 施加的眩晕从下一 tick 才开始生效，双方控制技能真正做到同时释放）；**Speed 职责清晰**（Speed 只影响移动冲突优先级，不做"先手权"）。

**逐帧流程简表**（[battlesystem.cpp](src/core/battlesystem.cpp)）：

| 步骤 | 阶段 | 说明 |
|------|------|------|
| 1 | 预处理 | Dot 周期累伤，Buff 剩余 tick 递减 |
| 2 | 决策 | 所有存活单位基于快照决定行动（技能 > 普攻 > 移动 > Idle） |
| 3a | 执行·移动 | 按 speed 降序结算移动（含碰撞回退） |
| 3b | 执行·技能 | 施放技能，产生伤害事件和 Buff 挂载 |
| 3c | 执行·普攻 | 造成伤害，攻击方获得法力值 |
| 4 | 批量结算 | 统一应用所有伤害，清空事件队列 |
| 5 | 清理 | 移除过期 Buff，冷却递减 |
| 6 | 死亡/胜负 | 标记死亡单位，检查胜负条件 |

三步"批量"操作是公平性的关键保障：决策全部完成再开始执行（所有单位看到的世界完全一致）、伤害全部入队再批量结算（一个 tick 内没有任何单位"先受伤"）、死亡在所有伤害结算后才判定（即使 Dot 累积了致命伤害，受害单位仍然完成了本 tick 的决策和执行）。

---

## 9. 代码版本管理

项目代码托管于 GitHub：[yfxu221/cpp_PA_autobattler](https://github.com/yfxu221/cpp_PA_autobattler.git)
