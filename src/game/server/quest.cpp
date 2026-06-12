// 任务系统 - 主线任务 & 每日任务 实现
#include <engine/server.h>
#include <engine/shared/config.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include "quest.h"
#include <ctime>

// =====================================================================
// 主线任务 - 静态数据表
// =====================================================================

const SMQuestStepDef CQuestManager::ms_aMainQuestSteps[] =
{
	// QUEST1 ─ 收集干净的猪肉 (20)
	{
		EMainQuests::QUEST1_PIGGY1,
		"第一阶仪式 I",
		{ "我需要一些没有被污染的猪肉..",
		  "那些猪大多被污染了，你或许需要多杀一些。" },
		{ { PIGPORNO, EMainQuestNeed::QUEST1 }, { 0, 0 } },
		/* Exp */ 4000, /* Money */ 200000, /* UpPg */ 0,
		{ { 0, 2 }, { 0, 2 } },
	},
	// QUEST2 ─ 更多的干净猪肉 (40)
	{
		EMainQuests::QUEST2_PIGGY2,
		"第一阶仪式 II",
		{ "我估错了所需的数量",
		  "我还需要更多的干净猪肉" },
		{ { PIGPORNO, EMainQuestNeed::QUEST2 }, { 0, 0 } },
		/* Exp */ 4000, /* Money */ 250000, /* UpPg */ 0,
		{ { 0, 2 }, { 0, 2 } },
	},
	// QUEST3 ─ Kwah 的头 (60)
	{
		EMainQuests::QUEST3_KWAH1,
		"第一阶仪式 III",
		{ "猪肉已经不足以满足祂了",
		  "我需要Kwah的头，尚未被污染的" },
		{ { KWAHGANDON, EMainQuestNeed::QUEST3 }, { 0, 0 } },
		/* Exp */ 8000, /* Money */ 500000, /* UpPg */ 0,
		{ { 0, 2 }, { 0, 2 } },
	},
	// QUEST4 ─ 更多 Kwah 的头 (80)
	{
		EMainQuests::QUEST4_KWAH2,
		"第一阶仪式 IV",
		{ "Kwah的头效果很好",
		  "祂还需要更多" },
		{ { KWAHGANDON, EMainQuestNeed::QUEST4 }, { 0, 0 } },
		/* Exp */ 8000, /* Money */ 550000, /* UpPg */ 0,
		{ { 0, 2 }, { 0, 2 } },
	},
	// QUEST5 ─ 猪 × Kwah 混合仪式 (100 + 60)
	{
		EMainQuests::QUEST5_PIGGYNKWAHSTEP1,
		"登阶 [第一步]",
		{ "成功了！哈哈哈哈哈！",
		  "现在..我们可以...",
		  " ", "更进一步", " " },
		{ { KWAHGANDON, EMainQuestNeed::QUEST5 },
		  { PIGPORNO,   EMainQuestNeed::QUEST3 } },
		/* Exp */ 0, /* Money */ 1000000, /* UpPg */ 0,
		{ { EARRINGSKWAH, 2 }, { 0, 2 } },
	},
	// QUEST6 ─ Kwah 头 + 脚 (120 + 120)
	{
		EMainQuests::QUEST6_KWAHSTEP1,
		"登阶 [第二步]",
		{ "我看见祂了!",
		  "我看见祂了!!!",
		  "啊啊啊啊啊啊啊啊啊",
		  "更多！更多！！" },
		{ { KWAHGANDON, EMainQuestNeed::QUEST6 },
		  { FOOTKWAH,   EMainQuestNeed::QUEST6 } },
		/* Exp */ 0, /* Money */ 1050000, /* UpPg */ 0,
		{ { FORMULAWEAPON, 2 }, { TITLEQUESTS, 3 } },
	},
	// QUEST7 ─ 守卫头 (10)
	{
		EMainQuests::QUEST7_BADGUARD,
		"第二阶仪式 I",
		{ "VVDieevruiumtm 5 5 4" },
		{ { GUARDHEAD, EMainQuestNeed::QUEST7 }, { 0, 0 } },
		/* Exp */ 0, /* Money */ 5550000, /* UpPg */ 0,
		{ { TITLEGUARD, 2 }, { 0, 2 } },
	},
	// QUEST8 ─ 僵尸脑 (50)
	{
		EMainQuests::QUEST8_ZOMBIE,
		"第二阶仪式 II",
		{ "神说: 要有ω☪′▶",
		  "神说: 要有ω☪′▶",
		  "神说: 要有ω☪′▶" },
		{ { ZOMBIEBRAIN, EMainQuestNeed::QUEST8 }, { 0, 0 } },
		/* Exp */ 0, /* Money */ 0, /* UpPg */ 500,
		{ { 0, 2 }, { 0, 2 } },
	},
	// QUEST9 ─ 骷髅头骨 (50)
	{
		EMainQuests::QUEST8_SKELET,
		"第二阶仪式 III",
		{ "神说: 于是，便有了Lmg",
		  "神说: 于是，便有了aiå",
		  "神说: 于是，便有了dg" },
		{ { SKELETSKULL, EMainQuestNeed::QUEST9 }, { 0, 0 } },
		/* Exp */ 0, /* Money */ 0, /* UpPg */ 3000,
		{ { 0, 2 }, { 0, 2 } },
	},
};
const int CQuestManager::ms_NumMainQuestSteps =
	sizeof(ms_aMainQuestSteps) / sizeof(ms_aMainQuestSteps[0]);

// =====================================================================
// 每日任务 - 物品池
// =====================================================================

const int CQuestManager::ms_aCollectFarmItems[]     = { POTATO, TOMATE, CARROT, CABBAGE };
const int CQuestManager::ms_aCollectFoodItems[]     = { PIGPORNO, KWAHGANDON, HEADBOOMER, FOOTKWAH };
const int CQuestManager::ms_aCollectOfferingItems[] = { DIRTYPIG, DIRTYKWAHHEAD, DIRTYBOOMERBODY, DIRTYKWAHFEET, DIRTYGUARDHEAD, GUARDHEAD };
const int CQuestManager::ms_aCollectMiningItems[]   = { COOPERORE, IRONORE, GOLDORE, DIAMONDORE, DRAGONORE, IRON, STANNUM };
const int CQuestManager::ms_aKillItems[]            = { BOT_L1MONSTER, BOT_L2MONSTER, BOT_L3MONSTER, BOT_BOSSSLIME, BOT_BOSSPIGKING, BOT_BOSSVAMPIRE, BOT_BOSSGUARD };
const int CQuestManager::ms_aChallengeFarmItems[]   = { POTATO, TOMATE, CARROT, CABBAGE };
const int CQuestManager::ms_aChallengeOfferingItems[] = { PIGPORNO, KWAHGANDON, HEADBOOMER, FOOTKWAH, DIRTYPIG, DIRTYKWAHHEAD, DIRTYBOOMERBODY, DIRTYKWAHFEET, DIRTYGUARDHEAD, GUARDHEAD };
const int CQuestManager::ms_aChallengeMiningItems[] = { COOPERORE, IRONORE, GOLDORE, DIAMONDORE, DRAGONORE, IRON, STANNUM };
const int CQuestManager::ms_aChallengeKillItems[]   = { BOT_L1MONSTER, BOT_L2MONSTER, BOT_L3MONSTER, BOT_BOSSSLIME, BOT_BOSSPIGKING, BOT_BOSSVAMPIRE, BOT_BOSSGUARD, BOT_BOSSZOMBIE, BOT_BOSSSKELET };

// =====================================================================
// 每日任务 - 子类型配置表
// =====================================================================

const SDailySubTypeDef CQuestManager::ms_aDailySubTypes[] =
{
	// ── 收集任务 ──
	{ kDailyCollect, kDailyCollectFarm,      { ms_aCollectFarmItems,      4 }, "农业经济" },
	{ kDailyCollect, kDailyCollectFood,      { ms_aCollectFoodItems,      4 }, "食品"     },
	{ kDailyCollect, kDailyCollectOffering,  { ms_aCollectOfferingItems,  6 }, "祭品"     },
	{ kDailyCollect, kDailyCollectMining,    { ms_aCollectMiningItems,    7 }, "矿业经济" },
	// ── 击杀任务（单分支）──
	{ kDailyKill,    kDailyKillDefault,      { ms_aKillItems, 7 },             "击杀任务" },
	// ── 挑战任务 ──
	{ kDailyChallenge, kDailyChallengeFarm,     { ms_aChallengeFarmItems,      4 }, "农业经济" },
	{ kDailyChallenge, kDailyChallengeOffering, { ms_aChallengeOfferingItems, 10 }, "祭品"     },
	{ kDailyChallenge, kDailyChallengeMining,   { ms_aChallengeMiningItems,    7 }, "矿业经济" },
	{ kDailyChallenge, kDailyChallengeKill,     { ms_aChallengeKillItems,      9 }, "击杀挑战" },
};
const int CQuestManager::ms_NumDailySubTypes =
	sizeof(ms_aDailySubTypes) / sizeof(ms_aDailySubTypes[0]);

// =====================================================================
// 内部辅助 ─ 查找子类型配置 & 追踪物品映射
// =====================================================================

const SDailySubTypeDef *CQuestManager::FindDailySubType(int Type, int SubType) const
{
	for (int i = 0; i < ms_NumDailySubTypes; i++)
		if (ms_aDailySubTypes[i].m_Type == Type
		 && ms_aDailySubTypes[i].m_SubType == SubType)
			return &ms_aDailySubTypes[i];
	return 0;
}

int CQuestManager::TrackingItemForType(int Type)
{
	switch (Type)
	{
	case kDailyCollect:   return COLLECTQUEST;
	case kDailyKill:      return KILLQUEST;
	case kDailyChallenge: return CHALLENGEQUEST;
	}
	return 0;
}

// =====================================================================
// 主线任务 ─ 获取当前步骤 / 检查完成条件 / 完成 & 推进
// =====================================================================

const SMQuestStepDef *CQuestManager::GetCurrentMainQuestStep(
	int ClientID, CGameContext *pGameServer) const
{
	int QuestID = pGameServer->m_apPlayers[ClientID]->AccData()->m_Quest;
	for (int i = 0; i < ms_NumMainQuestSteps; i++)
		if (ms_aMainQuestSteps[i].m_QuestID == QuestID)
			return &ms_aMainQuestSteps[i];
	return 0;
}

bool CQuestManager::CanCompleteMainQuest(int ClientID, CGameContext *pGameServer) const
{
	const SMQuestStepDef *pStep = GetCurrentMainQuestStep(ClientID, pGameServer);
	if (!pStep) return false;

	for (int i = 0; i < 2; i++)
	{
		int ItemID = pStep->m_Requirements[i].m_ItemID;
		if (ItemID == 0) continue;
		if (pGameServer->Server()->GetItemCount(ClientID, ItemID)
		 < (unsigned long long)pStep->m_Requirements[i].m_NeedCount)
			return false;
	}
	return true;
}

void CQuestManager::CompleteMainQuest(int ClientID, CGameContext *pGameServer)
{
	const SMQuestStepDef *pStep = GetCurrentMainQuestStep(ClientID, pGameServer);
	if (!pStep) return;

	// 1) 扣除需求物品
	for (int i = 0; i < 2; i++)
	{
		int ItemID = pStep->m_Requirements[i].m_ItemID;
		if (ItemID) pGameServer->Server()->RemItem(ClientID, ItemID,
			pStep->m_Requirements[i].m_NeedCount, -1);
	}

	// 2) 发放奖励
	if (pStep->m_ExpReward   > 0) pGameServer->m_apPlayers[ClientID]->ExpAdd(pStep->m_ExpReward);
	if (pStep->m_MoneyReward > 0) pGameServer->m_apPlayers[ClientID]->MoneyAdd(pStep->m_MoneyReward);
	if (pStep->m_UpPointReward > 0) pGameServer->m_apPlayers[ClientID]->GiveUpPoint(pStep->m_UpPointReward);

	for (int i = 0; i < 2; i++)
		if (pStep->m_MailItems[i].m_ItemID)
			pGameServer->SendMail(ClientID, pStep->m_MailItems[i].m_MailType,
				pStep->m_MailItems[i].m_ItemID, 1);

	// 3) 推进进度
	pGameServer->m_apPlayers[ClientID]->AccData()->m_Quest++;
	pGameServer->UpdateStats(ClientID);
}

// =====================================================================
// 主线任务 ─ /passquest
// =====================================================================

void CQuestManager::HandlePassQuest(int ClientID, CGameContext *pGameServer)
{
	const SMQuestStepDef *pStep = GetCurrentMainQuestStep(ClientID, pGameServer);
	if (!pStep)
	{
		if (pGameServer->m_apPlayers[ClientID]->AccData()->m_Quest > EMainQuests::QUEST8_SKELET)
			pGameServer->SendChatTarget_Localization(ClientID, CHATCATEGORY_DEFAULT,
				"全部主线任务已完成!", NULL);
		return;
	}
	if (!CanCompleteMainQuest(ClientID, pGameServer))
	{
		pGameServer->SendChatTarget_Localization(ClientID, CHATCATEGORY_DEFAULT,
			"任务还未完成!", NULL);
		return;
	}
	CompleteMainQuest(ClientID, pGameServer);
}

// =====================================================================
// 主线菜单 ─ 奖励文本构建 & 菜单显示
// =====================================================================

void CQuestManager::BuildRewardText(const SMQuestStepDef *pStep,
	char *pBuf, int BufSize) const
{
	pBuf[0] = '\0';
	if      (pStep->m_ExpReward > 0 && pStep->m_MoneyReward > 0)
		str_format(pBuf, BufSize, "%d经验/%d白银", pStep->m_ExpReward, pStep->m_MoneyReward);
	else if (pStep->m_ExpReward > 0)
		str_format(pBuf, BufSize, "%d经验", pStep->m_ExpReward);
	else if (pStep->m_MoneyReward > 0)
		str_format(pBuf, BufSize, "%d白银", pStep->m_MoneyReward);
	else if (pStep->m_UpPointReward > 0)
		str_format(pBuf, BufSize, "%d升级点", pStep->m_UpPointReward);
}

void CQuestManager::ShowMainQuestMenu(int ClientID, CGameContext *pGameServer)
{
	const SMQuestStepDef *pStep = GetCurrentMainQuestStep(ClientID, pGameServer);

	pGameServer->AddVote_Localization(ClientID, "null", "☪ 信息 ( ′ ω ` )?:");
	pGameServer->AddVote_Localization(ClientID, "null", "主任务菜单");
	pGameServer->AddVote("", "null", ClientID);

	if (!pStep)
	{
		pGameServer->AddVote_Localization(ClientID, "null", "全部主线任务已完成!");
		return;
	}

	// 标题 + 描述
	pGameServer->AddVote_Localization(ClientID, "null", pStep->m_LocaleTitle);
	for (int i = 0; i < 5; i++)
		if (pStep->m_LocaleDescLines[i][0])
			pGameServer->AddVote_Localization(ClientID, "null", pStep->m_LocaleDescLines[i]);

	// 需求物品 (支持 1 或 2 种)
	if (pStep->m_Requirements[0].m_ItemID != 0)
	{
		IServer *pServer = pGameServer->Server();
		int Have1 = pServer->GetItemCount(ClientID, pStep->m_Requirements[0].m_ItemID);
		int Need1 = pStep->m_Requirements[0].m_NeedCount;

		if (pStep->m_Requirements[1].m_ItemID != 0)
		{
			int Have2 = pServer->GetItemCount(ClientID, pStep->m_Requirements[1].m_ItemID);
			int Need2 = pStep->m_Requirements[1].m_NeedCount;
			pGameServer->AddVote_Localization(ClientID, "null",
				"任务需求: {str:item1}, {str:item2} [{int:get}/{int:need} & {int:get2}/{int:need2}]",
				"item1", pServer->GetItemName(ClientID, pStep->m_Requirements[0].m_ItemID),
				"item2", pServer->GetItemName(ClientID, pStep->m_Requirements[1].m_ItemID),
				"get", &Have1, "need", &Need1,
				"get2", &Have2, "need2", &Need2);
		}
		else
		{
			pGameServer->AddVote_Localization(ClientID, "null",
				"[已获得: {int:get}/共需要: {int:need}]",
				"get", &Have1, "need", &Need1);
		}
	}

	// 奖励文本
	char aReward[256] = {0};
	BuildRewardText(pStep, aReward, sizeof(aReward));
	if (aReward[0])
		pGameServer->AddVote_Localization(ClientID, "null", "任务奖励: {str:got}", "got", aReward);

	// 邮件物品提示
	if (pStep->m_MailItems[0].m_ItemID || pStep->m_MailItems[1].m_ItemID)
	{
		char aMail[128] = {0};
		const char *pName0 = pStep->m_MailItems[0].m_ItemID
			? pGameServer->Server()->GetItemName(ClientID, pStep->m_MailItems[0].m_ItemID) : "";
		const char *pName1 = pStep->m_MailItems[1].m_ItemID
			? pGameServer->Server()->GetItemName(ClientID, pStep->m_MailItems[1].m_ItemID) : "";

		if (pName0[0] && pName1[0])
			str_format(aMail, sizeof(aMail), "%s, %s", pName0, pName1);
		else if (pName0[0])
			str_format(aMail, sizeof(aMail), "%s", pName0);
		else
			str_format(aMail, sizeof(aMail), "%s", pName1);

		pGameServer->AddVote_Localization(ClientID, "null", "+ {str:items}", "items", aMail);
	}

	pGameServer->AddVote_Localization(ClientID, "passquest", "- 提交通过任务");
}

// =====================================================================
// 每日任务 ─ Tick / 刷新 / 日期 ID
// =====================================================================

int CQuestManager::GetDailyID() const
{
	time_t t; time(&t);
	tm *pT = localtime(&t);
	return pT->tm_year + pT->tm_mday + pT->tm_mon + pT->tm_yday + pT->tm_wday + pT->tm_mday;
}

void CQuestManager::DailyTick(CGameContext *pGameServer)
{
	time_t t; time(&t);
	tm *pT = localtime(&t);
	if (pT->tm_hour == m_LastHour) return;

	if (m_LastHour > pT->tm_hour)
	{
		RefreshDaily(true, pGameServer);
		pGameServer->SendChatTarget_Localization(-1, CHATCATEGORY_DEFAULT, "每日任务已更新！");
	}
	m_LastHour = pT->tm_hour;
}

void CQuestManager::RefreshDaily(bool Reset, CGameContext *pGameServer)
{
	srand(GetDailyID());
	m_RandomNumber = rand();
	if (!Reset) return;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer *pP = pGameServer->m_apPlayers[i];
		if (!pP || pP->IsBot()) continue;
		pGameServer->Server()->RemItem(i, COLLECTQUEST,  pGameServer->Server()->GetItemCount(i, COLLECTQUEST), -1);
		pGameServer->Server()->RemItem(i, KILLQUEST,     pGameServer->Server()->GetItemCount(i, KILLQUEST), -1);
		pGameServer->Server()->RemItem(i, CHALLENGEQUEST,pGameServer->Server()->GetItemCount(i, CHALLENGEQUEST), -1);
		pGameServer->Server()->SetItemSettingsCount(i, COLLECTQUEST, 0);
		pGameServer->Server()->SetItemSettingsCount(i, KILLQUEST, 0);
		pGameServer->Server()->SetItemSettingsCount(i, CHALLENGEQUEST, 0);
		pP->m_FinishedCollectQuest = false;
		pP->m_FinishedKillQuest = false;
		pP->m_FinishedChallengeQuest = false;
		pGameServer->UpdateStats(i);
	}
	pGameServer->Server()->Execute("UPDATE tw_uItems SET item_count = 1,item_settings = 0 WHERE il_id = 159");
	pGameServer->Server()->Execute("UPDATE tw_uItems SET item_count = 0,item_settings = 0 WHERE il_id = 164");
	pGameServer->Server()->Execute("UPDATE tw_uItems SET item_count = 0,item_settings = 0 WHERE il_id = 165");
}

// =====================================================================
// 每日任务 ─ 随机数据计算: 目标物品 / 需求数量 / 升级点奖励
// =====================================================================

int CQuestManager::GetDailyQuestItem(int QuestType, int SubType) const
{
	const SDailySubTypeDef *pDef = FindDailySubType(QuestType, SubType);
	if (!pDef || pDef->m_ItemPool.m_Count <= 0) return 1;

	long long Divisor = 1;
	if (QuestType == kDailyKill)                        Divisor = kDailyKillDivisor;
	else if (QuestType == kDailyChallenge
	      && SubType == kDailyChallengeKill)             Divisor = kDailyChallengeDivisor;

	return pDef->m_ItemPool.m_Items[(m_RandomNumber / Divisor) % pDef->m_ItemPool.m_Count];
}

int CQuestManager::GetDailyQuestNeed(int QuestType, int SubType) const
{
	long long r = m_RandomNumber;

	switch (QuestType)
	{
	case kDailyCollect:
		switch (SubType)
		{
		case kDailyCollectFarm:    return (r % kCollectFarmMul + kCollectFarmMin) * kCollectFarmScale;
		case kDailyCollectFood:    return (r % kCollectFoodMul) + kCollectFoodMin;
		case kDailyCollectOffering:
			if ((r % kOfferingBranchCount) == kOfferingBranch1
			 || (r % kOfferingBranchCount) == kOfferingBranch2)
				return r % kOfferingSmallMax;
			return (r % kOfferingLargeMax) + kOfferingLargeMin;
		case kDailyCollectMining:  return (r % kCollectMiningMul + kCollectMiningMin) * kCollectMiningScale;
		}
		break;
	case kDailyKill:
		return ((r / 2) % kKillNeedMul) + kKillNeedMin;
	case kDailyChallenge:
		switch (SubType)
		{
		case kDailyChallengeFarm:    return (r % kChallengeFarmMul + kChallengeFarmMin) * kChallengeFarmScale;
		case kDailyChallengeOffering: return (r % kChallengeOfferingMul) + kChallengeOfferingMin;
		case kDailyChallengeMining:  return (r % kChallengeMiningMul + kChallengeMiningMin) * kChallengeMiningScale;
		case kDailyChallengeKill:    return kChallengeKillNeed;
		}
		break;
	}
	return 0;
}

int CQuestManager::GetDailyQuestUpgr(int QuestType, int SubType) const
{
	long long r = m_RandomNumber;
	switch (QuestType)
	{
	case kDailyCollect:
		switch (SubType)
		{
		case kDailyCollectFarm:   return 50;
		case kDailyCollectFood:   return 70;
		case kDailyCollectMining: return 70;
		case kDailyCollectOffering:
			if ((r % kOfferingBranchCount) == kOfferingBranch1
			 || (r % kOfferingBranchCount) == kOfferingBranch2)
				return (r % kOfferingUpgrHighMul) + kOfferingUpgrHighMin;
			return (r % kOfferingUpgrMul) + kOfferingUpgrBase;
		}
		break;
	case kDailyKill:
		return (r % kKillUpgrMul) + kKillUpgrMin;
	case kDailyChallenge:
		switch (SubType)
		{
		case kDailyChallengeFarm:
		case kDailyChallengeOffering:
		case kDailyChallengeMining:  return kChallengeUpgrDefault;
		case kDailyChallengeKill:
			if ((r % kChallengeBranchCount) == kChallengeBranchHigh
			 || (r % kChallengeBranchCount) == kChallengeBranchHigh2)
				return kChallengeUpgrDefault;
			return (r % kChallengeUpgrDefault) + kChallengeUpgrMin;
		}
		break;
	}
	return 1;
}

// =====================================================================
// 每日任务 ─ 玩家状态检查 & 奖励发放
// =====================================================================

bool CQuestManager::IsPlayerDailyCompleted(int ClientID, int Type,
	CGameContext *pGameServer) const
{
	return pGameServer->Server()->GetItemSettings(ClientID,
		TrackingItemForType(Type)) == GetDailyID();
}

bool CQuestManager::CheckDailyPassConditions(int ClientID, int Type,
	int SubType, CGameContext *pGameServer) const
{
	CPlayer *pP = pGameServer->m_apPlayers[ClientID];

	switch (Type)
	{
	case kDailyCollect:   if (pP->m_FinishedCollectQuest)    return false; break;
	case kDailyKill:      if (pP->m_FinishedKillQuest)       return false; break;
	case kDailyChallenge: if (pP->m_FinishedChallengeQuest)  return false; break;
	default: return false;
	}

	int Need = GetDailyQuestNeed(Type, SubType);

	if (Type == kDailyChallenge && SubType == kDailyChallengeKill)
		return pGameServer->Server()->GetItemCount(ClientID, CHALLENGEQUEST)
		       >= (unsigned long long)Need;

	int Item = GetDailyQuestItem(Type, SubType);
	return pGameServer->Server()->GetItemCount(ClientID, Item) >= (unsigned long long)Need;
}

void CQuestManager::GrantDailyReward(int ClientID, int Type, int SubType,
	CGameContext *pGameServer)
{
	// CHALLENGE4 只计数不扣物品
	if (!(Type == kDailyChallenge && SubType == kDailyChallengeKill))
	{
		int Item = GetDailyQuestItem(Type, SubType);
		pGameServer->Server()->RemItem(ClientID, Item,
			GetDailyQuestNeed(Type, SubType), -1);
	}

	pGameServer->m_apPlayers[ClientID]->GiveUpPoint(GetDailyQuestUpgr(Type, SubType));

	int TrackID = TrackingItemForType(Type);
	pGameServer->Server()->SetItemSettingsCount(ClientID, TrackID, GetDailyID());

	switch (Type)
	{
	case kDailyCollect:   pGameServer->m_apPlayers[ClientID]->m_FinishedCollectQuest   = true; break;
	case kDailyKill:      pGameServer->m_apPlayers[ClientID]->m_FinishedKillQuest      = true; break;
	case kDailyChallenge: pGameServer->m_apPlayers[ClientID]->m_FinishedChallengeQuest = true; break;
	}
}

// =====================================================================
// 每日任务 ─ /passdayquest
// =====================================================================

void CQuestManager::HandlePassDayQuest(int ClientID, CGameContext *pGameServer)
{
	int Type = pGameServer->m_apPlayers[ClientID]->m_SelectQuest;
	int Sub  = pGameServer->m_apPlayers[ClientID]->m_SelectSubQuest;
	if (Type < 0 || Type >= kNumDailyTypes) return;

	if (!CheckDailyPassConditions(ClientID, Type, Sub, pGameServer))
	{
		pGameServer->SendChatTarget_Localization(ClientID, CHATCATEGORY_DEFAULT,
			"任务还未完成!", NULL);
		return;
	}

	GrantDailyReward(ClientID, Type, Sub, pGameServer);
	pGameServer->ResetVotes(ClientID, DAYQUEST);
}

// =====================================================================
// 每日任务 ─ 类型 / 子类型 切换
// =====================================================================

void CQuestManager::HandleSelectQuestType(int ClientID, int QuestType,
	CGameContext *pGameServer)
{
	int *pSelect = &pGameServer->m_apPlayers[ClientID]->m_SelectQuest;
	*pSelect = (*pSelect == QuestType) ? -1 : QuestType;
	pGameServer->ResetVotes(ClientID, DAYQUEST);
}

void CQuestManager::HandleSelectSubQuest(int ClientID, int SubType,
	CGameContext *pGameServer)
{
	int *pSel = &pGameServer->m_apPlayers[ClientID]->m_SelectSubQuest;
	*pSel = (*pSel == SubType) ? -1 : SubType;
	pGameServer->ResetVotes(ClientID, DAYQUEST);
}

// =====================================================================
// 每日菜单 ─ 分区渲染辅助
// =====================================================================

void CQuestManager::ShowDailyHeader(int ClientID, CGameContext *pGameServer) const
{
	pGameServer->m_apPlayers[ClientID]->m_LastVotelist = QUESTMENU;
	pGameServer->AddVote_Localization(ClientID, "null", "☪ 信息 ( ′ ω ` )?:");
	pGameServer->AddVote_Localization(ClientID, "null", "每日任务!");
	int iRand = (int)m_RandomNumber;
	pGameServer->AddVote_Localization(ClientID, "null",
		"今日任务序列：{int:randomNum}", "randomNum", &iRand);
	pGameServer->AddVote("", "null", ClientID);
	pGameServer->AddVote_Localization(ClientID, "que0", "☞ 收集任务");
	pGameServer->AddVote_Localization(ClientID, "que1", "☞ 击杀任务");
	pGameServer->AddVote_Localization(ClientID, "que2", "☞ 挑战任务");
}

void CQuestManager::ShowDailyCollectSection(int ClientID, int SubType,
	CGameContext *pGameServer)
{
	pGameServer->AddVote("", "null", ClientID);

	if (IsPlayerDailyCompleted(ClientID, kDailyCollect, pGameServer))
	{
		pGameServer->AddVote_Localization(ClientID, "que0", "- 收集任务已完成");
		return;
	}

	int Item = GetDailyQuestItem(kDailyCollect, SubType);
	int Need = GetDailyQuestNeed(kDailyCollect, SubType);
	int Upgr = GetDailyQuestUpgr(kDailyCollect, SubType);
	int Have = pGameServer->Server()->GetItemCount(ClientID, Item);

	// 子类型选单（隐藏支线: 0.5% 概率食品 → "食品...?"）
	pGameServer->AddVote_Localization(ClientID, "sque0", "- 农业经济");
	pGameServer->AddVote_Localization(ClientID,
		random_prob(kHiddenOfferingProb) ? "sque2" : "sque1",
		random_prob(kHiddenOfferingProb) ? "- 食品...?" : "- 食品");
	pGameServer->AddVote_Localization(ClientID, "sque3", "- 矿业经济");
	pGameServer->AddVote("", "null", ClientID);

	if (SubType == kDailyCollectOffering)
	{
		pGameServer->AddVote_Localization(ClientID, "null", "Vivit为DerumDeumVivitDerumDeumVivitDerumDeum");
		pGameServer->AddVote_Localization(ClientID, "null", "Vivit神DerumDeumVivitDerumDeumVivitDerumDeum");
		pGameServer->AddVote_Localization(ClientID, "null", "Vivit献DerumDeumVivitDerumDeumVivitDerumDeum");
		pGameServer->AddVote_Localization(ClientID, "null", "Vivit上DerumDeumVivitDerumDeumVivitDerumDeum");
		pGameServer->AddVote_Localization(ClientID, "null",
			"THA{str:iname} [{int:num}/{int:need}]",
			"iname", pGameServer->Server()->GetItemName(ClientID, Item),
			"num", &Have, "need", &Need);
		pGameServer->AddVote_Localization(ClientID, "null", "enruw奖ails：???升级点");
		pGameServer->AddVote_Localization(ClientID, "passdayquest", "- S贡品S -");
	}
	else
	{
		pGameServer->AddVote_Localization(ClientID, "null",
			"收集 {str:iname} [{int:num}/{int:need}]",
			"iname", pGameServer->Server()->GetItemName(ClientID, Item),
			"num", &Have, "need", &Need);
		pGameServer->AddVote_Localization(ClientID, "null",
			"任务奖励：{int:num}升级点", "num", &Upgr);
		pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 提交任务");
	}
	pGameServer->AddVote("", "null", ClientID);
	pGameServer->AddVote("", "null", ClientID);
}

void CQuestManager::ShowDailyKillSection(int ClientID, CGameContext *pGameServer)
{
	if (IsPlayerDailyCompleted(ClientID, kDailyKill, pGameServer))
	{
		pGameServer->AddVote_Localization(ClientID, "que0", "- 击杀任务已完成");
		return;
	}

	int Item = GetDailyQuestItem(kDailyKill, kDailyKillDefault);
	int Need = GetDailyQuestNeed(kDailyKill, kDailyKillDefault);
	int Upgr = GetDailyQuestUpgr(kDailyKill, kDailyKillDefault);
	int Have = pGameServer->Server()->GetItemCount(ClientID, KILLQUEST);

	pGameServer->AddVote_Localization(ClientID, "null",
		"击杀 {str:iname} [{int:num}/{int:need}]",
		"iname", pGameServer->GetBotName(Item), "num", &Have, "need", &Need);
	pGameServer->AddVote_Localization(ClientID, "null",
		"任务奖励：{int:num}升级点", "num", &Upgr);
	pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 提交任务");
	pGameServer->AddVote("", "null", ClientID);
	pGameServer->AddVote("", "null", ClientID);
}

void CQuestManager::ShowDailyChallengeSection(int ClientID, int SubType,
	CGameContext *pGameServer)
{
	pGameServer->AddVote("", "null", ClientID);

	if (IsPlayerDailyCompleted(ClientID, kDailyChallenge, pGameServer))
	{
		pGameServer->AddVote_Localization(ClientID, "que0", "- 挑战任务已完成");
		return;
	}

	int Item = GetDailyQuestItem(kDailyChallenge, SubType);
	int Need = GetDailyQuestNeed(kDailyChallenge, SubType);
	int Upgr = GetDailyQuestUpgr(kDailyChallenge, SubType);
	int Have = (SubType == kDailyChallengeKill)
		? pGameServer->Server()->GetItemCount(ClientID, CHALLENGEQUEST)
		: pGameServer->Server()->GetItemCount(ClientID, Item);

	// 子类型选单 (隐藏支线: 1% 概率显示 "祭品")
	pGameServer->AddVote_Localization(ClientID, "sque0", "- 农业经济");
	if (random_prob(kHiddenChallengeProb))
		pGameServer->AddVote_Localization(ClientID, "sque1", "- 祭品");
	pGameServer->AddVote_Localization(ClientID, "sque2", "- 矿业经济");
	pGameServer->AddVote_Localization(ClientID, "sque3", "- 击杀挑战");
	pGameServer->AddVote("", "null", ClientID);

	switch (SubType)
	{
	case kDailyChallengeKill:
		pGameServer->AddVote_Localization(ClientID, "null",
			"击杀 {str:iname} [{int:num}/{int:need}]",
			"iname", pGameServer->GetBotName(Item), "num", &Have, "need", &Need);
		break;
	case kDailyChallengeOffering:
		pGameServer->AddVote_Localization(ClientID, "null",
			"VDD献上 {str:iname} [{int:num}/{int:need}]",
			"iname", pGameServer->Server()->GetItemName(ClientID, Item),
			"num", &Have, "need", &Need);
		break;
	default: // Farm / Mining
		pGameServer->AddVote_Localization(ClientID, "null",
			"收集 {str:iname} [{int:num}/{int:need}]",
			"iname", pGameServer->Server()->GetItemName(ClientID, Item),
			"num", &Have, "need", &Need);
		break;
	}

	if (SubType == kDailyChallengeOffering)
		pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 贡品");
	else
	{
		pGameServer->AddVote_Localization(ClientID, "null",
			"任务奖励：{int:num}升级点", "num", &Upgr);
		pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 提交任务");
	}
	pGameServer->AddVote("", "null", ClientID);
}

// =====================================================================
// 每日菜单 ─ 入口
// =====================================================================

void CQuestManager::ShowDailyQuestMenu(int ClientID, CGameContext *pGameServer)
{
	int Type = pGameServer->m_apPlayers[ClientID]->m_SelectQuest;
	int Sub  = pGameServer->m_apPlayers[ClientID]->m_SelectSubQuest;

	ShowDailyHeader(ClientID, pGameServer);

	switch (Type)
	{
	case kDailyCollect:   ShowDailyCollectSection(ClientID, Sub, pGameServer);   break;
	case kDailyKill:      ShowDailyKillSection(ClientID, pGameServer);            break;
	case kDailyChallenge: ShowDailyChallengeSection(ClientID, Sub, pGameServer);  break;
	}

	pGameServer->AddBack(ClientID);
}
