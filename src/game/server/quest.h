// 任务系统 - 主线任务 & 每日任务
#pragma once
#include <stdint.h>

class CPlayer;
class CGameContext;

// ═══════════════════════════════════════════════════
// 每日任务 - 计算参数常量
// ═══════════════════════════════════════════════════

// 随机偏移除数（使不同任务类型取不同的随机变种）
enum DailyCalcDivisor
{
	kDailyKillDivisor       = 17,  // 击杀任务用 17 取偏移
	kDailyChallengeDivisor  = 18,  // 挑战-击杀子任务用 18 取偏移
};

// 隐藏支线（祭品）出现概率
const float kHiddenOfferingProb  = 0.005f; // 收集任务：0.5% 概率显示"食品...?"
const float kHiddenChallengeProb = 0.01f;  // 挑战任务：1% 概率显示"祭品"

// 收集任务 - 需求数量参数
enum DailyNeedParam
{
	kCollectFarmMin         = 8,   // 农业：基数 +8
	kCollectFarmMul         = 10,  // 农业：倍数
	kCollectFarmScale       = 1000000,
	kCollectFoodMin         = 30,  // 食品：基数 +30
	kCollectFoodMul         = 50,
	kCollectMiningMin       = 8,
	kCollectMiningMul       = 23,
	kCollectMiningScale     = 100000,
};

// 收集任务 - 佣兵（祭品）分支参数
enum DailyOfferingParam
{
	kOfferingSmallMax       = 300,   // 小奖上限
	kOfferingLargeMin       = 300,   // 大奖下限
	kOfferingLargeMax       = 2000,  // 大奖范围
	kOfferingBranch1        = 4,
	kOfferingBranch2        = 5,
	kOfferingBranchCount    = 6,
};

// 击杀任务 - 需求/奖励参数
enum DailyKillParam
{
	kKillNeedMin            = 500,
	kKillNeedMul            = 500,
	kKillUpgrMin            = 150, // 奖励下限
	kKillUpgrMul            = 100,
};

// 挑战任务 - 需求/奖励参数
enum DailyChallengeParam
{
	kChallengeFarmMin       = 8,
	kChallengeFarmMul       = 10,
	kChallengeFarmScale     = 10000000,
	kChallengeOfferingMin   = 2000,
	kChallengeOfferingMul   = 1000,
	kChallengeMiningMin     = 8,
	kChallengeMiningMul     = 23,
	kChallengeMiningScale   = 10000000,
	kChallengeKillNeed      = 7000,  // CHALLENGE4 固定击杀数
	kChallengeUpgrDefault   = 750,   // 挑战任务固定奖励
	kChallengeUpgrMin       = 200,   // CHALLENGE4 浮动奖励下限
	kChallengeUpgrMax       = 950,   // CHALLENGE4 浮动奖励上限 (200 + 750)

	kChallengeBranchHigh    = 4,
	kChallengeBranchHigh2   = 5,
	kChallengeBranchCount   = 6,
};

// 收集-祭品 奖励参数
enum DailyOfferingUpgrParam
{
	kOfferingUpgrBase       = 500,
	kOfferingUpgrMul        = 500,
	kOfferingUpgrHighMin    = 500,  // 大奖下限
	kOfferingUpgrHighMul    = 1000,
};

// ═══════════════════════════════════════════════════
// 主线任务 - 数据结构
// ═══════════════════════════════════════════════════

// 单个任务步骤的需求物品
struct SMQuestRequirement
{
	int m_ItemID;    // 0 = 无需求
	int m_NeedCount; // 需求数量
};

// 邮件奖励物品（附带邮件类型）
struct SMQuestMailItem
{
	int m_ItemID;   // 0 = 无奖励
	int m_MailType; // 2=物品, 3=称号
};

// 单个主线任务步骤的完整定义
struct SMQuestStepDef
{
	int                   m_QuestID;                   // EMainQuests 枚举值
	const char           *m_LocaleTitle;               // 任务标题
	const char           *m_LocaleDescLines[5];        // 描述文本 (空串 = 无)
	SMQuestRequirement    m_Requirements[2];           // 需求物品 (最多两种)
	int                   m_ExpReward;                 // 经验奖励
	int                   m_MoneyReward;               // 银币奖励
	int                   m_UpPointReward;             // 升级点奖励
	SMQuestMailItem       m_MailItems[2];              // 邮件发放的物品
};

// ═══════════════════════════════════════════════════
// 每日任务 - 数据结构
// ═══════════════════════════════════════════════════

// 任务大类
enum EDailyQuestType
{
	kDailyCollect   = 0,
	kDailyKill,
	kDailyChallenge,
	kNumDailyTypes,
};

// 收集任务 - 子类型
enum EDailyCollectSub
{
	kDailyCollectFarm      = 0,
	kDailyCollectFood,
	kDailyCollectOffering,  // 隐藏支线：祭品 (0.5%)
	kDailyCollectMining,
	kNumDailyCollectSub,
};

// 击杀任务 - 子类型（统一下标 0，仅一个分支）
enum EDailyKillSub
{
	kDailyKillDefault = 0,
};

// 挑战任务 - 子类型
enum EDailyChallengeSub
{
	kDailyChallengeFarm      = 0,
	kDailyChallengeOffering,  // 隐藏支线：祭品 (1%)
	kDailyChallengeMining,
	kDailyChallengeKill,
	kNumDailyChallengeSub,
};

// 物品池：用于随机选取每日目标任务物品
struct SDailyItemPool
{
	const int *m_Items;  // 物品 ID 数组
	int        m_Count;  // 数组长度
};

// 每日任务子类型配置条目
struct SDailySubTypeDef
{
	int             m_Type;       // kDailyCollect / kDailyKill / kDailyChallenge
	int             m_SubType;    // 子类型索引
	SDailyItemPool  m_ItemPool;   // 可选的物品池
	const char     *m_LocaleName; // 显示用分类名
};

// ═══════════════════════════════════════════════════
// 任务管理器
// ═══════════════════════════════════════════════════

class CQuestManager
{
public:
	CQuestManager() : m_RandomNumber(0), m_LastHour(0) {}

	// ---- 主线任务 -----------------------------------------
	void HandlePassQuest(int ClientID, CGameContext *pGameServer);
	void ShowMainQuestMenu(int ClientID, CGameContext *pGameServer);

	// ---- 每日任务 Tick / 刷新 ----------------------------
	void DailyTick(CGameContext *pGameServer);
	void RefreshDaily(bool Reset, CGameContext *pGameServer);
	int  GetDailyID() const;

	// ---- 每日任务数据查询（基于随机种子）-----------------
	int  GetDailyQuestItem(int Type, int SubType) const;
	int  GetDailyQuestNeed(int Type, int SubType) const;
	int  GetDailyQuestUpgr(int Type, int SubType) const;

	// ---- 每日任务 玩家交互 ------------------------------
	void HandlePassDayQuest(int ClientID, CGameContext *pGameServer);
	void HandleSelectQuestType(int ClientID, int QuestType, CGameContext *pGameServer);
	void HandleSelectSubQuest(int ClientID, int SubType, CGameContext *pGameServer);
	void ShowDailyQuestMenu(int ClientID, CGameContext *pGameServer);

	// ---- 快捷查询 ----------------------------------------
	long long int GetRandomNumber() const { return m_RandomNumber; }
	int  GetLastHour() const { return m_LastHour; }
	void SetLastHour(int Hour) { m_LastHour = Hour; }

private:
	// ---- 主线任务 内部辅助 ------------------------------
	const SMQuestStepDef *GetCurrentMainQuestStep(int ClientID, CGameContext *pGameServer) const;
	bool CanCompleteMainQuest(int ClientID, CGameContext *pGameServer) const;
	void CompleteMainQuest(int ClientID, CGameContext *pGameServer);
	void BuildRewardText(const SMQuestStepDef *pStep, char *pBuf, int BufSize) const;

	// ---- 每日任务 内部辅助 ------------------------------
	bool IsPlayerDailyCompleted(int ClientID, int Type, CGameContext *pGameServer) const;
	bool CheckDailyPassConditions(int ClientID, int Type, int SubType, CGameContext *pGameServer) const;
	void GrantDailyReward(int ClientID, int Type, int SubType, CGameContext *pGameServer);

	// 获取任务类型对应的追踪物品 ID (COLLECTQUEST / KILLQUEST / CHALLENGEQUEST)
	static int TrackingItemForType(int Type);

	// 查找子类型配置表中的条目
	const SDailySubTypeDef *FindDailySubType(int Type, int SubType) const;

	// ---- 每日菜单渲染辅助（拆分自 ShowDailyQuestMenu）---
	void ShowDailyHeader(int ClientID, CGameContext *pGameServer) const;
	void ShowDailyCollectSection(int ClientID, int SubType, CGameContext *pGameServer);
	void ShowDailyKillSection(int ClientID, CGameContext *pGameServer);
	void ShowDailyChallengeSection(int ClientID, int SubType, CGameContext *pGameServer);

	// ---- 状态成员 ----------------------------------------
	long long int m_RandomNumber;
	int           m_LastHour;

	// ---- 静态配置表 --------------------------------------
	static const SMQuestStepDef  ms_aMainQuestSteps[];
	static const int             ms_NumMainQuestSteps;
	static const SDailySubTypeDef ms_aDailySubTypes[];
	static const int             ms_NumDailySubTypes;

	// 每日任务物品池
	static const int ms_aCollectFarmItems[];
	static const int ms_aCollectFoodItems[];
	static const int ms_aCollectOfferingItems[];
	static const int ms_aCollectMiningItems[];
	static const int ms_aKillItems[];
	static const int ms_aChallengeFarmItems[];
	static const int ms_aChallengeOfferingItems[];
	static const int ms_aChallengeMiningItems[];
	static const int ms_aChallengeKillItems[];
};
