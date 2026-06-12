// Quest & DailyQuest System - Refactored
#pragma once
#include <stdint.h>
#include "playerdata.h"

class CPlayer;
class CGameContext;
class IServer;

// ============================================================
// 主线任务(Main Quest) 数据结构
// ============================================================

// 单个主线任务步骤的需求物品
struct SMQuestRequirement
{
	int m_ItemID;      // 需求物品ID
	int m_NeedCount;   // 需求数量（来自 EMainQuestNeed 枚举）
};

// 邮件奖励物品
struct SMQuestMailItem
{
	int m_ItemID;      // 物品ID（0=无）
	int m_MailType;    // 邮件类型（2=物品, 3=称号）
};

// 单个主线任务步骤的完整定义
struct SMQuestStepDef
{
	int m_QuestID;                          // EMainQuests 枚举值
	const char *m_LocaleTitle;              // 任务标题本地化key
	const char *m_LocaleDescLines[5];       // 任务描述文本（最多5行）
	SMQuestRequirement m_Requirements[2];   // 任务需求（最多2种物品）
	int m_ExpReward;                        // 经验奖励
	int m_MoneyReward;                      // 白银奖励
	int m_UpPointReward;                    // 升级点奖励
	SMQuestMailItem m_MailItems[2];         // 通过邮件发送的物品
};

// ============================================================
// 每日任务(Daily Quest) 数据结构
// ============================================================

// 每日任务类型
enum
{
	DAILY_COLLECT = 0,
	DAILY_KILL,
	DAILY_CHALLENGE,
	NUM_DAILY_TYPES,
};

// 每日任务子类型
enum
{
	DAILYSUB_COLLECT_FARM = 0,
	DAILYSUB_COLLECT_FOOD,
	DAILYSUB_COLLECT_OFFERING,
	DAILYSUB_COLLECT_MINING,
	NUM_DAILYSUB_COLLECT,

	DAILYSUB_CHALLENGE_FARM = 0,
	DAILYSUB_CHALLENGE_OFFERING,
	DAILYSUB_CHALLENGE_MINING,
	DAILYSUB_CHALLENGE_KILL,
	NUM_DAILYSUB_CHALLENGE,
};

// 每日任务物品池条目
struct SDailyItemPool
{
	const int *m_Items;   // 物品ID数组
	int m_Count;          // 数组大小
};

// 每日任务子类型定义
struct SDailySubTypeDef
{
	int m_Type;               // DAILY_COLLECT / DAILY_KILL / DAILY_CHALLENGE
	int m_SubType;            // 子类型索引
	SDailyItemPool m_ItemPool;// 可选物品池
	const char *m_LocaleName; // 中文分类名
};

// ============================================================
// 任务管理器
// ============================================================

class CQuestManager
{
public:
	CQuestManager() : m_RandomNumber(0), m_LastHour(0) {}
	void Init();

	// ---- 主线任务 ----
	int GetMainQuestProgress(int QuestID) const;                      // 获取任务步骤索引
	const SMQuestStepDef *GetCurrentMainQuestStep(int ClientID, CGameContext *pGameServer) const;
	bool CanCompleteMainQuest(int ClientID, CGameContext *pGameServer) const;
	void CompleteMainQuest(int ClientID, CGameContext *pGameServer);  // 完成任务并推进到下一步
	void ShowMainQuestMenu(int ClientID, CGameContext *pGameServer);  // 显示主线任务菜单
	void HandlePassQuest(int ClientID, CGameContext *pGameServer);    // 处理 passquest 命令

	// ---- 每日任务 ----
	void DailyTick(CGameContext *pGameServer);                        // 每日tick检查
	void RefreshDaily(bool Reset, CGameContext *pGameServer);         // 刷新每日任务
	int GetDailyID() const;                                              // 获取当日ID

	int GetDailyQuestItem(int QuestType, int SubType) const;         // 获取任务目标物品
	int GetDailyQuestNeed(int QuestType, int SubType) const;         // 获取任务需求数量
	int GetDailyQuestUpgr(int QuestType, int SubType) const;         // 获取任务奖励

	bool CanCompleteDailyQuest(int ClientID, int QuestType, int SubType, CGameContext *pGameServer) const;
	void CompleteDailyQuest(int ClientID, int QuestType, int SubType, CGameContext *pGameServer);
	void ResetPlayerDailyQuest(int ClientID, CGameContext *pGameServer);

	void ShowDailyQuestMenu(int ClientID, CGameContext *pGameServer, bool ResetVotes = true);
	void HandleSelectQuestType(int ClientID, int QuestType, CGameContext *pGameServer);
	void HandleSelectSubQuest(int ClientID, int SubType, CGameContext *pGameServer);
	void HandlePassDayQuest(int ClientID, CGameContext *pGameServer);

	// ---- 任务进度查询 ----
	bool IsPlayerDailyQuestCompleted(int ClientID, int QuestType, CGameContext *pGameServer) const;

	// ---- 每日任务随机种子 ----
	long long int GetRandomNumber() const { return m_RandomNumber; }
	int GetLastHour() const { return m_LastHour; }
	void SetLastHour(int Hour) { m_LastHour = Hour; }

private:
	long long int m_RandomNumber;
	int m_LastHour;

	static const SMQuestStepDef ms_aMainQuestSteps[];
	static const int ms_NumMainQuestSteps;
	static const SDailySubTypeDef ms_aDailySubTypes[];
	static const int ms_NumDailySubTypes;

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
