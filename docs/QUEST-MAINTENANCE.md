# 任务系统维护指南

本文档介绍如何在 Mmotee 服务器中添加新任务、修改现有任务配置，以及理解任务系统的数据流。

## 目录

- [代码架构](#代码架构)
- [关键文件索引](#关键文件索引)
- [数据流概览](#数据流概览)
- [添加新的主线任务](#添加新的主线任务)
- [修改每日任务配置](#修改每日任务配置)
- [添加新的每日任务子类型](#添加新的每日任务子类型)
- [任务枚举速查表](#任务枚举速查表)
- [物品池 ID 速查](#物品池-id-速查)
- [编译与部署](#编译与部署)
- [常见问题](#常见问题)

---

## 代码架构

```
                    ┌─────────────────────────┐
                    │     gamecontext.cpp      │
                    │  命令入口 ("passquest",  │
                    │  "passdayquest")         │
                    └───────────┬─────────────┘
                                │ 委托
                    ┌───────────▼─────────────┐
                    │      CQuestManager       │
                    │  quest.h / quest.cpp     │
                    │                         │
                    │  ┌─────────────────┐    │
                    │  │ 主线任务处理器   │    │
                    │  │ HandlePassQuest  │    │
                    │  │ ShowMainQuestMenu│    │
                    │  └─────────────────┘    │
                    │  ┌─────────────────┐    │
                    │  │ 每日任务处理器   │    │
                    │  │ DailyTick        │    │
                    │  │ RefreshDaily     │    │
                    │  │ HandlePassDay    │    │
                    │  └─────────────────┘    │
                    └───────────┬─────────────┘
                                │ 读取
                    ┌───────────▼─────────────┐
                    │     静态配置数据表        │
                    │                         │
                    │ ms_aMainQuestSteps[]    │ ← 主线任务步骤
                    │ ms_aDailySubTypes[]     │ ← 每日任务配置
                    │ ms_aCollect*Items[]     │ ← 物品池
                    │ ms_aKillItems[]         │
                    │ ms_aChallenge*Items[]   │
                    └─────────────────────────┘
```

### 数据驱动设计

任务系统采用**纯数据驱动**设计。所有任务参数（物品需求、奖励、描述文本）都存储在 `quest.cpp` 顶部的静态数组中。`CQuestManager` 类只包含**通用处理逻辑**，不包含任何硬编码的任务数据。

这意味着：

- **加任务不需要改逻辑代码** — 只在数据表中新增一行即可
- **改奖励不需要改菜单代码** — 改数据表中的数字即可
- **菜单文本在数据表中** — 不需要在分散的各处找字符串

---

## 关键文件索引

| 文件 | 作用 |
|------|------|
| `src/game/server/quest.h` | 任务数据结构、枚举、`CQuestManager` 类声明 |
| `src/game/server/quest.cpp` | 任务静态数据表、所有任务逻辑实现 |
| `src/game/server/gamecontext.h` | `EMainQuests` 枚举、`EMainQuestNeed` 枚举、`EDailyQuests` 旧枚举（兼容）|
| `src/game/server/gamecontext.cpp` | 命令入口（只做转发，不改任务逻辑） |
| `src/engine/server.h` | `Items` 枚举（物品 ID 全局定义）、`Menus` 枚举 |
| `template.sql` | 数据库表结构（`tw_Users.Quest` 字段） |

---

## 数据流概览

### 主线任务生命周期

```
玩家登录 ──→ m_Quest 字段（当前任务进度）
                     │
                     ▼
              /passquest 命令
                     │
                     ▼
         CQuestManager::HandlePassQuest()
              │
              ├─ GetCurrentMainQuestStep() ← 查 ms_aMainQuestSteps 表
              ├─ CanCompleteMainQuest()    ← 检查物品数量是否达标
              └─ CompleteMainQuest()
                   ├─ 扣除需求物品
                   ├─ 发放经验/金钱/升级点
                   ├─ 发送邮件奖励
                   └─ m_Quest++ ──→ 推进到下一步
```

### 每日任务生命周期

```
服务器 Tick ──→ DailyTick()
                   │ 每小时检查一次
                   ├─ 跨天时 ──→ RefreshDaily()
                   │                ├─ 重新生成随机种子
                   │                ├─ 重置所有玩家进度
                   │                └─ 广播 "每日任务已更新"
                   │
玩家选择类型 ──→ HandleSelectQuestType()  ──→ m_SelectQuest
玩家选择子类 ──→ HandleSelectSubQuest()   ──→ m_SelectSubQuest
                     │
                     ▼
              每日任务菜单显示
              ShowDailyQuestMenu()
                   │ 根据 m_SelectQuest / m_SelectSubQuest
                   │ 调用 GetDailyQuestItem/Need/Upgr 获取今日随机值
                     │
                     ▼
              /passdayquest 命令
                     │
                     ▼
         HandlePassDayQuest()
              ├─ CheckDailyPassConditions()
              └─ GrantDailyReward()
                   ├─ 扣除物品
                   ├─ 发放升级点
                   └─ 写入完成标记（防止重复领取）
```

### 随机种子机制

每日任务的所有随机数据（目标物品、需求数量、升级点奖励）都由当天的**随机种子**决定。

```
时间源:
  GetDailyID()  = year + mday + mon + yday + wday + mday
                    │
                    ▼ srand(DailyID)
                 rand() ──→ m_RandomNumber
                    │
    ┌───────────────┼───────────────┐
    ▼               ▼               ▼
GetDailyQuestItem  GetDailyQuestNeed  GetDailyQuestUpgr
（物品池选一）     （计算需求量）     （计算奖励点）
```

**关键特性**：同一天所有玩家看到的是同一个随机序列，因为种子基于日期计算。

---

## 添加新的主线任务

### 步骤 1：在 `gamecontext.h` 中添加枚举值

在 `EMainQuests` 枚举末尾（`QUEST8_SKELET` 之后、`END_MAIN_QUEST` 之前）添加新任务 ID：

```cpp
// src/game/server/gamecontext.h
enum EMainQuests
{
    // ... 已有任务 ...
    QUEST8_SKELET,
    QUEST9_NEWQUEST,       // ← 新增
    END_MAIN_QUEST,

    NUM_MAIN_QUEST = END_MAIN_QUEST - START_MAIN_QUEST - 1,
};
```

如果新任务需要新的需求数量，在 `EMainQuestNeed` 中也添加：

```cpp
enum EMainQuestNeed
{
    // ... 已有需求 ...
    QUEST9 = 50,
    QUEST10 = 100,          // ← 新增
};
```

### 步骤 2：在 `quest.cpp` 数据表中添加任务步骤

在 `ms_aMainQuestSteps` 数组末尾（`};` 之前）追加新条目：

```cpp
const SMQuestStepDef CQuestManager::ms_aMainQuestSteps[] =
{
    // ... 已有任务 ...

    // QUEST9 ─ 新任务描述 (需求数量)
    {
        EMainQuests::QUEST9_NEWQUEST,             // m_QuestID
        "第三阶仪式 I",                             // m_LocaleTitle
        {
            "这是新任务的描述第一行",
            "这是新任务的描述第二行",
        },
        {
            { ZOMBIEBRAIN, EMainQuestNeed::QUEST10 },  // 需求1: 物品ID, 数量
            { SKELETSKULL, EMainQuestNeed::QUEST10 },  // 需求2: 物品ID, 数量(不需要则 {0,0})
        },
        /* Exp */       16000,    // 经验奖励（0 = 无）
        /* Money */   2000000,    // 银币奖励（0 = 无）
        /* UpPoint */       0,    // 升级点奖励（0 = 无）
        /* MailItems */
        {
            { NEW_REWARD_ITEM, 2 },   // 邮件物品1: {物品ID, 邮件类型}
            { TITLEQUESTS,     3 },   // 邮件物品2: {0,2}=空, MailType: 2=物品, 3=称号
        },
    },
};
```

**字段说明**：

| 字段 | 类型 | 说明 |
|------|------|------|
| `m_QuestID` | `int` | `EMainQuests` 枚举值，表示此步骤属于哪个任务 |
| `m_LocaleTitle` | `const char*` | 任务标题（会显示在菜单中） |
| `m_LocaleDescLines[5]` | `const char*` | 最多 5 行描述文本，未使用的行填空字符串 `""` 或 `" "` |
| `m_Requirements[2]` | `SMQuestRequirement` | 最多 2 种需求物品，不用则填 `{0,0}` |
| `m_ExpReward` | `int` | 经验奖励 |
| `m_MoneyReward` | `int` | 银币奖励 |
| `m_UpPointReward` | `int` | 升级点奖励 |
| `m_MailItems[2]` | `SMQuestMailItem` | 邮件发放物品，`m_MailType`: `2`=物品, `3`=称号 |

### 步骤 3：编译验证

```bash
cd build && make -j$(nproc)
```

新文件会通过 `CMakeLists.txt` 的 `GLOB_RECURSE` 自动发现，**无需手动修改 CMake 文件**。

### 步骤 4：数据库（如需要）

主线任务进度存储在 `tw_Users.Quest` 字段（`int(11) DEFAULT '1'`），无需修改数据库结构。

---

## 修改每日任务配置

### 调整现有子类型的参数

所有每日任务的数值参数已提取为命名常量，位于 `quest.h`：

```cpp
// 击杀任务参数
enum DailyKillParam
{
    kKillNeedMin  = 500,   // 需求基数
    kKillNeedMul  = 500,   // 需求倍数 → 实际需求 = rand()%500 + 500
    kKillUpgrMin  = 150,   // 奖励下限
    kKillUpgrMul  = 100,   // 奖励倍数 → 实际奖励 = rand()%100 + 150
};

// 挑战任务参数
enum DailyChallengeParam
{
    kChallengeKillNeed    = 7000,  // CHALLENGE4 固定击杀数
    kChallengeUpgrDefault = 750,   // 固定奖励
    // ...
};
```

修改这些常量即可调整对应的计算逻辑。无需改动 `quest.cpp` 中的逻辑代码。

### 修改物品池

在 `quest.cpp` 的物品池数组中增删物品 ID：

```cpp
// 收集-农业 物品池
const int CQuestManager::ms_aCollectFarmItems[] =
    { POTATO, TOMATE, CARROT, CABBAGE };  // 增删这里的物品即可

// 击杀任务 目标池
const int CQuestManager::ms_aKillItems[] =
    { BOT_L1MONSTER, BOT_L2MONSTER, BOT_L3MONSTER,
      BOT_BOSSSLIME, BOT_BOSSPIGKING, BOT_BOSSVAMPIRE, BOT_BOSSGUARD };
```

**注意**：修改物品池后，需要同步更新对应 `SDailySubTypeDef` 中的 `m_Count` 值：

```cpp
// 如果 ms_aKillItems 从 7 个改成 8 个，这里也要改
{ kDailyKill, 0, { ms_aKillItems, 7 }, "击杀任务" },
// 改成 →              { ms_aKillItems, 8 }, "击杀任务" },
```

---

## 添加新的每日任务子类型

### 场景：给收集任务添加第 5 个子类型 "木材采集"

#### 步骤 1：在 `quest.h` 中的子类型枚举添加新值

```cpp
enum EDailyCollectSub
{
    kDailyCollectFarm      = 0,
    kDailyCollectFood,
    kDailyCollectOffering,
    kDailyCollectMining,
    kDailyCollectWood,       // ← 新增
    kNumDailyCollectSub,
};
```

#### 步骤 2：在 `quest.cpp` 中添加物品池

```cpp
const int CQuestManager::ms_aCollectWoodItems[] =
    { WOOD, BIGWOOD, WOODCORE };   // 木材类物品
```

#### 步骤 3：在 `quest.h` 中声明静态数组

```cpp
private:
    static const int ms_aCollectWoodItems[];  // ← 新增声明
```

#### 步骤 4：在 `ms_aDailySubTypes` 表中注册

```cpp
const SDailySubTypeDef CQuestManager::ms_aDailySubTypes[] =
{
    // ── 收集任务 ──
    { kDailyCollect, kDailyCollectFarm,      { ms_aCollectFarmItems,      4 }, "农业经济" },
    { kDailyCollect, kDailyCollectFood,      { ms_aCollectFoodItems,      4 }, "食品"     },
    { kDailyCollect, kDailyCollectOffering,  { ms_aCollectOfferingItems,  6 }, "祭品"     },
    { kDailyCollect, kDailyCollectMining,    { ms_aCollectMiningItems,    7 }, "矿业经济" },
    { kDailyCollect, kDailyCollectWood,      { ms_aCollectWoodItems,      3 }, "木材采集" },  // ← 新增
    // ...
};
```

#### 步骤 5：添加需求量和奖励计算逻辑

在 `GetDailyQuestNeed()` 的 `kDailyCollect` 分支添加新 case：

```cpp
case kDailyCollectWood:
    return (r % 20 + 5) * 100;  // 需求量: 500~2400
```

在 `GetDailyQuestUpgr()` 的 `kDailyCollect` 分支添加：

```cpp
case kDailyCollectWood:
    return 60;  // 固定 60 升级点
```

#### 步骤 6：在菜单中添加显示逻辑

在 `ShowDailyCollectSection()` 的 switch 中添加：

```cpp
case kDailyCollectWood:
    pGameServer->AddVote_Localization(ClientID, "null",
        "收集 {str:iname} [{int:num}/{int:need}]",
        "iname", pGameServer->Server()->GetItemName(ClientID, Item),
        "num", &Have, "need", &Need);
    pGameServer->AddVote_Localization(ClientID, "null",
        "任务奖励：{int:num}升级点", "num", &Upgr);
    pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 提交任务");
    break;
```

并在子类型菜单中添加新按钮：

```cpp
pGameServer->AddVote_Localization(ClientID, "sque4", "- 木材采集");  // ← sque4
```

---

## 任务枚举速查表

### 主线任务 ID

| `EMainQuests` | 内容 | 需求物品 | 数量 |
|---------------|------|----------|------|
| `QUEST1_PIGGY1` | 第一阶仪式 I | 干净猪肉 | 20 |
| `QUEST2_PIGGY2` | 第一阶仪式 II | 干净猪肉 | 40 |
| `QUEST3_KWAH1` | 第一阶仪式 III | Kwah 头 | 60 |
| `QUEST4_KWAH2` | 第一阶仪式 IV | Kwah 头 | 80 |
| `QUEST5_PIGGYNKWAHSTEP1` | 登阶第一步 | Kwah头+猪肉 | 100+60 |
| `QUEST6_KWAHSTEP1` | 登阶第二步 | Kwah头+Kwah脚 | 120+120 |
| `QUEST7_BADGUARD` | 第二阶仪式 I | 守卫头 | 10 |
| `QUEST8_ZOMBIE` | 第二阶仪式 II | 僵尸脑 | 50 |
| `QUEST8_SKELET` | 第二阶仪式 III | 骷髅头骨 | 50 |

### 每日任务类型

| `EDailyQuestType` | 说明 |
|-------------------|------|
| `kDailyCollect = 0` | 收集任务 |
| `kDailyKill = 1` | 击杀任务 |
| `kDailyChallenge = 2` | 挑战任务 |

### 每日任务子类型 - 收集

| `EDailyCollectSub` | 显示名 | 物品池 | 隐藏概率 |
|--------------------|--------|--------|----------|
| `kDailyCollectFarm` | 农业经济 | 作物 (4种) | — |
| `kDailyCollectFood` | 食品 | 肉类 (4种) | 0.5% → "食品...?" |
| `kDailyCollectOffering` | 祭品 | 污秽物品 (6种) | — |
| `kDailyCollectMining` | 矿业经济 | 矿石 (7种) | — |

### 每日任务子类型 - 挑战

| `EDailyChallengeSub` | 显示名 | 物品池 | 隐藏概率 |
|----------------------|--------|--------|----------|
| `kDailyChallengeFarm` | 农业经济 | 作物 (4种) | — |
| `kDailyChallengeOffering` | 祭品 | 肉类+污秽 (10种) | 1% 概率显示 |
| `kDailyChallengeMining` | 矿业经济 | 矿石 (7种) | — |
| `kDailyChallengeKill` | 击杀挑战 | Boss (9种) | — 固定 7000 击杀 |

### 需求数量计算公式

| 子类型 | 公式 | 示例 |
|--------|------|------|
| 收集-农业 | `(rand()%10 + 8) × 1,000,000` | 8M~17M |
| 收集-食品 | `rand()%50 + 30` | 30~79 |
| 收集-祭品(常规) | `rand()%2000 + 300` | 300~2299 |
| 收集-祭品(特殊)* | `rand()%300` | 0~299 |
| 收集-矿业 | `(rand()%23 + 8) × 100,000` | 800K~3M |
| 击杀 | `(rand()%500) + 500` | 500~999 |
| 挑战-农业 | `(rand()%10 + 8) × 10,000,000` | 80M~170M |
| 挑战-祭品 | `rand()%1000 + 2000` | 2000~2999 |
| 挑战-矿业 | `(rand()%23 + 8) × 10,000,000` | 80M~300M |
| 挑战-击杀 | **固定 7000** | 7000 |

\* 祭品特殊分支: 当 `rand()%6 == 4` 或 `== 5` 时触发

### 邮件类型

| `m_MailType` | 含义 |
|--------------|------|
| `2` | 物品邮件 |
| `3` | 称号邮件 |

---

## 物品池 ID 速查

### 作物
`POTATO`, `TOMATE`, `CARROT`, `CABBAGE`

### 肉类/战利品
`PIGPORNO` (干净猪肉), `KWAHGANDON` (Kwah头), `HEADBOOMER`, `FOOTKWAH` (Kwah脚)

### 污秽物品（祭品专用）
`DIRTYPIG`, `DIRTYKWAHHEAD`, `DIRTYBOOMERBODY`, `DIRTYKWAHFEET`, `DIRTYGUARDHEAD`, `GUARDHEAD`

### 矿石
`COOPERORE`, `IRONORE`, `GOLDORE`, `DIAMONDORE`, `DRAGONORE`, `IRON`, `STANNUM`

### 怪物/Boss（击杀任务目标）
`BOT_L1MONSTER`, `BOT_L2MONSTER`, `BOT_L3MONSTER`, `BOT_BOSSSLIME`, `BOT_BOSSPIGKING`, `BOT_BOSSVAMPIRE`, `BOT_BOSSGUARD`, `BOT_BOSSZOMBIE`, `BOT_BOSSSKELET`

### 任务追踪物品（内部使用，不可交易的进度标记）
`COLLECTQUEST` (159), `KILLQUEST` (164), `CHALLENGEQUEST` (165)

---

## 编译与部署

### 编译

```bash
cd build
cmake ..
make -j$(nproc)
```

生成的二进制文件位于 `build/Mmotee-Server`。

### 热重载

任务数据表是**编译期常量**，修改需要重新编译。日常运维只需调整常量值，无需重启数据库。

### 数据库

| 表 | 字段 | 用途 |
|----|------|------|
| `tw_Users` | `Quest` | 玩家当前主线任务进度（整数值） |
| `tw_Users` | `QuestDat` | 保留字段 |
| `tw_uItems` | `il_id` | 物品 ID，COLECTQUEST=159, KILLQUEST=164, CHALLENGEQUEST=165 |
| `tw_uItems` | `item_settings` | 每日任务完成标记（存储当日 DailyID） |

---

## 常见问题

### Q: 新增主线任务后，旧玩家会受影响吗？

不会。`m_Quest` 是递增的整数值。旧玩家完成到哪一步就停在哪一步，新增的任务自动排在他们当前进度之后。

### Q: 如何调整某个主线任务的奖励？

直接修改 `quest.cpp` 中 `ms_aMainQuestSteps` 对应条目的 `m_ExpReward`、`m_MoneyReward`、`m_UpPointReward` 或 `m_MailItems` 即可。

### Q: 如何让某个任务步骤有 3 种以上需求物品？

当前设计最多支持 2 种。如需扩展，需要修改 `SMQuestStepDef::m_Requirements[2]` 的数组大小，并相应调整 `CanCompleteMainQuest()` 和 `CompleteMainQuest()` 中的 `for (i < 2)` 循环。

### Q: 祭品任务的隐藏概率在哪里修改？

在 `quest.h` 中：

```cpp
const float kHiddenOfferingProb  = 0.005f; // 收集任务的隐藏食品支线概率
const float kHiddenChallengeProb = 0.01f;  // 挑战任务的隐藏祭品支线概率
```

### Q: 每日任务的随机物品为什么连续几天不变？

`GetDailyID()` 的计算公式为 `year + mday + mon + yday + wday + mday`。如果日期变化不大，值可能相近。这是设计如此——日期间的任务目标应当有一定连续性。

### Q: 新增物品池后编译报错 "undefined reference"？

检查是否在 `quest.h` 的 `CQuestManager` 类中声明了对应的静态数组成员变量（参考已有的 `ms_aCollectFarmItems` 等声明）。
