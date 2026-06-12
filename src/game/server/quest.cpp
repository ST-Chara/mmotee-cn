/* Quest & DailyQuest System - Implementation */
#include <engine/server.h>
#include <engine/shared/config.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>
#include "quest.h"
#include <ctime>

// ============================================================
// 主线任务 静态数据表
// ============================================================

const SMQuestStepDef CQuestManager::ms_aMainQuestSteps[] =
{
	{
		/* QuestID     */ EMainQuests::QUEST1_PIGGY1,
		/* TitleKey    */ "第一阶仪式 I",
		/* DescLines   */ {
			"我需要一些没有被污染的猪肉..",
			"那些猪大多被污染了，你或许需要多杀一些。",
		},
		/* Requirements */ {
			{ PIGPORNO, EMainQuestNeed::QUEST1 },  // 20
			{ 0, 0 },
		},
		/* ExpReward   */ 4000,
		/* MoneyReward */ 200000,
		/* UpPointReward */ 0,
		/* MailItems   */ { { 0, 2 }, { 0, 2 } },
	},
	{
		/* QuestID     */ EMainQuests::QUEST2_PIGGY2,
		/* TitleKey    */ "第一阶仪式 II",
		/* DescLines   */ {
			"我估错了所需的数量",
			"我还需要更多的干净猪肉",
		},
		/* Requirements */ {
			{ PIGPORNO, EMainQuestNeed::QUEST2 },  // 40
			{ 0, 0 },
		},
		/* ExpReward   */ 4000,
		/* MoneyReward */ 250000,
		/* UpPointReward */ 0,
		/* MailItems   */ { { 0, 2 }, { 0, 2 } },
	},
	{
		/* QuestID     */ EMainQuests::QUEST3_KWAH1,
		/* TitleKey    */ "第一阶仪式 III",
		/* DescLines   */ {
			"猪肉已经不足以满足祂了",
			"我需要Kwah的头，尚未被污染的",
		},
		/* Requirements */ {
			{ KWAHGANDON, EMainQuestNeed::QUEST3 },  // 60
			{ 0, 0 },
		},
		/* ExpReward   */ 8000,
		/* MoneyReward */ 500000,
		/* UpPointReward */ 0,
		/* MailItems   */ { { 0, 2 }, { 0, 2 } },
	},
	{
		/* QuestID     */ EMainQuests::QUEST4_KWAH2,
		/* TitleKey    */ "第一阶仪式 IV",
		/* DescLines   */ {
			"Kwah的头效果很好",
			"祂还需要更多",
		},
		/* Requirements */ {
			{ KWAHGANDON, EMainQuestNeed::QUEST4 },  // 80
			{ 0, 0 },
		},
		/* ExpReward   */ 8000,
		/* MoneyReward */ 550000,
		/* UpPointReward */ 0,
		/* MailItems   */ { { 0, 2 }, { 0, 2 } },
	},
	{
		/* QuestID     */ EMainQuests::QUEST5_PIGGYNKWAHSTEP1,
		/* TitleKey    */ "登阶 [第一步]",
		/* DescLines   */ {
			"成功了！哈哈哈哈哈！",
			"现在..我们可以...",
			" ",
			"更进一步",
			" ",
		},
		/* Requirements */ {
			{ KWAHGANDON, EMainQuestNeed::QUEST5 },  // 100
			{ PIGPORNO, EMainQuestNeed::QUEST3  },  // 60 (原有bug: 检查用QUEST3，扣除用QUEST5)
		},
		/* ExpReward   */ 0,
		/* MoneyReward */ 1000000,
		/* UpPointReward */ 0,
		/* MailItems   */ { { EARRINGSKWAH, 2 }, { 0, 2 } },
	},
	{
		/* QuestID     */ EMainQuests::QUEST6_KWAHSTEP1,
		/* TitleKey    */ "登阶 [第二步]",
		/* DescLines   */ {
			"我看见祂了!",
			"我看见祂了!!!",
			"啊啊啊啊啊啊啊啊啊",
			"更多！更多！！",
		},
		/* Requirements */ {
			{ KWAHGANDON, EMainQuestNeed::QUEST6 },  // 120
			{ FOOTKWAH, EMainQuestNeed::QUEST6 },    // 120
		},
		/* ExpReward   */ 0,
		/* MoneyReward */ 1050000,
		/* UpPointReward */ 0,
		/* MailItems   */ { { FORMULAWEAPON, 2 }, { TITLEQUESTS, 3 } },
	},
	{
		/* QuestID     */ EMainQuests::QUEST7_BADGUARD,
		/* TitleKey    */ "第二阶仪式 I",
		/* DescLines   */ {
			"VVDieevruiumtm 5 5 4",
		},
		/* Requirements */ {
			{ GUARDHEAD, EMainQuestNeed::QUEST7 },  // 10
			{ 0, 0 },
		},
		/* ExpReward   */ 0,
		/* MoneyReward */ 5550000,
		/* UpPointReward */ 0,
		/* MailItems   */ { { TITLEGUARD, 2 }, { 0, 2 } },
	},
	{
		/* QuestID     */ EMainQuests::QUEST8_ZOMBIE,
		/* TitleKey    */ "第二阶仪式 II",
		/* DescLines   */ {
			"神说: 要有ω☪′▶",
			"神说: 要有ω☪′▶",
			"神说: 要有ω☪′▶",
		},
		/* Requirements */ {
			{ ZOMBIEBRAIN, EMainQuestNeed::QUEST8 },  // 50
			{ 0, 0 },
		},
		/* ExpReward   */ 0,
		/* MoneyReward */ 0,
		/* UpPointReward */ 500,
		/* MailItems   */ { { 0, 2 }, { 0, 2 } },
	},
	{
		/* QuestID     */ EMainQuests::QUEST8_SKELET,
		/* TitleKey    */ "第二阶仪式 III",
		/* DescLines   */ {
			"神说: 于是，便有了Lmg",
			"神说: 于是，便有了aiå",
			"神说: 于是，便有了dg",
		},
		/* Requirements */ {
			{ SKELETSKULL, EMainQuestNeed::QUEST9 },  // 50
			{ 0, 0 },
		},
		/* ExpReward   */ 0,
		/* MoneyReward */ 0,
		/* UpPointReward */ 3000,
		/* MailItems   */ { { 0, 2 }, { 0, 2 } },
	},
};
const int CQuestManager::ms_NumMainQuestSteps = sizeof(ms_aMainQuestSteps) / sizeof(ms_aMainQuestSteps[0]);

// ============================================================
// 每日任务 静态物品池
// ============================================================

const int CQuestManager::ms_aCollectFarmItems[] = { POTATO, TOMATE, CARROT, CABBAGE };
const int CQuestManager::ms_aCollectFoodItems[] = { PIGPORNO, KWAHGANDON, HEADBOOMER, FOOTKWAH };
const int CQuestManager::ms_aCollectOfferingItems[] = { DIRTYPIG, DIRTYKWAHHEAD, DIRTYBOOMERBODY, DIRTYKWAHFEET, DIRTYGUARDHEAD, GUARDHEAD };
const int CQuestManager::ms_aCollectMiningItems[] = { COOPERORE, IRONORE, GOLDORE, DIAMONDORE, DRAGONORE, IRON, STANNUM };
const int CQuestManager::ms_aKillItems[] = { BOT_L1MONSTER, BOT_L2MONSTER, BOT_L3MONSTER, BOT_BOSSSLIME, BOT_BOSSPIGKING, BOT_BOSSVAMPIRE, BOT_BOSSGUARD };
const int CQuestManager::ms_aChallengeFarmItems[] = { POTATO, TOMATE, CARROT, CABBAGE };
const int CQuestManager::ms_aChallengeOfferingItems[] = { PIGPORNO, KWAHGANDON, HEADBOOMER, FOOTKWAH, DIRTYPIG, DIRTYKWAHHEAD, DIRTYBOOMERBODY, DIRTYKWAHFEET, DIRTYGUARDHEAD, GUARDHEAD };
const int CQuestManager::ms_aChallengeMiningItems[] = { COOPERORE, IRONORE, GOLDORE, DIAMONDORE, DRAGONORE, IRON, STANNUM };
const int CQuestManager::ms_aChallengeKillItems[] = { BOT_L1MONSTER, BOT_L2MONSTER, BOT_L3MONSTER, BOT_BOSSSLIME, BOT_BOSSPIGKING, BOT_BOSSVAMPIRE, BOT_BOSSGUARD, BOT_BOSSZOMBIE, BOT_BOSSSKELET };

// ============================================================
// 每日任务 子类型定义表
// ============================================================

const SDailySubTypeDef CQuestManager::ms_aDailySubTypes[] =
{
	// 收集任务 (DAILY_COLLECT)
	{ DAILY_COLLECT, DAILYSUB_COLLECT_FARM,     { ms_aCollectFarmItems,     4 }, "农业经济" },
	{ DAILY_COLLECT, DAILYSUB_COLLECT_FOOD,     { ms_aCollectFoodItems,     4 }, "食品"     },
	{ DAILY_COLLECT, DAILYSUB_COLLECT_OFFERING, { ms_aCollectOfferingItems, 6 }, "祭品"     },
	{ DAILY_COLLECT, DAILYSUB_COLLECT_MINING,   { ms_aCollectMiningItems,   7 }, "矿业经济" },

	// 击杀任务 (DAILY_KILL) - 只有一个子类型
	{ DAILY_KILL,    0, { ms_aKillItems, 7 }, "击杀任务" },

	// 挑战任务 (DAILY_CHALLENGE)
	{ DAILY_CHALLENGE, DAILYSUB_CHALLENGE_FARM,     { ms_aChallengeFarmItems,      4 }, "农业经济" },
	{ DAILY_CHALLENGE, DAILYSUB_CHALLENGE_OFFERING, { ms_aChallengeOfferingItems, 10 }, "祭品"     },
	{ DAILY_CHALLENGE, DAILYSUB_CHALLENGE_MINING,   { ms_aChallengeMiningItems,    7 }, "矿业经济" },
	{ DAILY_CHALLENGE, DAILYSUB_CHALLENGE_KILL,     { ms_aChallengeKillItems,      9 }, "击杀挑战" },
};
const int CQuestManager::ms_NumDailySubTypes = sizeof(ms_aDailySubTypes) / sizeof(ms_aDailySubTypes[0]);

// ============================================================
// CQuestManager 实现
// ============================================================

void CQuestManager::Init() {}  // 初始化（目前不需要特殊操作）

// ---- 主线任务: 获取当前步骤定义 ----
int CQuestManager::GetMainQuestProgress(int QuestID) const
{
	return QuestID - EMainQuests::QUEST1_PIGGY1;
}

const SMQuestStepDef *CQuestManager::GetCurrentMainQuestStep(int ClientID, CGameContext *pGameServer) const
{
	int QuestID = pGameServer->m_apPlayers[ClientID]->AccData()->m_Quest;
	for (int i = 0; i < ms_NumMainQuestSteps; i++)
	{
		if (ms_aMainQuestSteps[i].m_QuestID == QuestID)
			return &ms_aMainQuestSteps[i];
	}
	return 0;
}

// ---- 主线任务: 检查是否可完成 ----
bool CQuestManager::CanCompleteMainQuest(int ClientID, CGameContext *pGameServer) const
{
	const SMQuestStepDef *pStep = GetCurrentMainQuestStep(ClientID, pGameServer);
	if (!pStep)
		return false;

	for (int i = 0; i < 2; i++)
	{
		if (pStep->m_Requirements[i].m_ItemID != 0)
		{
			if (static_cast<unsigned long long>(pGameServer->Server()->GetItemCount(ClientID, pStep->m_Requirements[i].m_ItemID))
				< static_cast<unsigned long long>(pStep->m_Requirements[i].m_NeedCount))
				return false;
		}
	}
	return true;
}

// ---- 主线任务: 完成并推进 ----
void CQuestManager::CompleteMainQuest(int ClientID, CGameContext *pGameServer)
{
	const SMQuestStepDef *pStep = GetCurrentMainQuestStep(ClientID, pGameServer);
	if (!pStep)
		return;

	// 扣除需求物品
	for (int i = 0; i < 2; i++)
	{
		if (pStep->m_Requirements[i].m_ItemID != 0)
		{
			pGameServer->Server()->RemItem(ClientID,
				pStep->m_Requirements[i].m_ItemID,
				pStep->m_Requirements[i].m_NeedCount, -1);
		}
	}

	// 发放经验奖励
	if (pStep->m_ExpReward > 0)
		pGameServer->m_apPlayers[ClientID]->ExpAdd(pStep->m_ExpReward);

	// 发放金钱奖励
	if (pStep->m_MoneyReward > 0)
		pGameServer->m_apPlayers[ClientID]->MoneyAdd(pStep->m_MoneyReward);

	// 发放升级点奖励
	if (pStep->m_UpPointReward > 0)
		pGameServer->m_apPlayers[ClientID]->GiveUpPoint(pStep->m_UpPointReward);

	// 发放邮件物品
	for (int i = 0; i < 2; i++)
	{
		if (pStep->m_MailItems[i].m_ItemID != 0)
			pGameServer->SendMail(ClientID, pStep->m_MailItems[i].m_MailType, pStep->m_MailItems[i].m_ItemID, 1);
	}

	// 推进任务进度
	pGameServer->m_apPlayers[ClientID]->AccData()->m_Quest++;
	pGameServer->UpdateStats(ClientID);
}

// ---- 主线任务: 执行 passquest 命令 ----
void CQuestManager::HandlePassQuest(int ClientID, CGameContext *pGameServer)
{
	const SMQuestStepDef *pStep = GetCurrentMainQuestStep(ClientID, pGameServer);
	if (!pStep)
	{
		// 任务已完成或不可用
		if (pGameServer->m_apPlayers[ClientID]->AccData()->m_Quest > EMainQuests::QUEST8_SKELET)
			pGameServer->SendChatTarget_Localization(ClientID, CHATCATEGORY_DEFAULT, "全部主线任务已完成!", NULL);
		return;
	}

	if (!CanCompleteMainQuest(ClientID, pGameServer))
	{
		pGameServer->SendChatTarget_Localization(ClientID, CHATCATEGORY_DEFAULT, "任务还未完成!", NULL);
		return;
	}

	CompleteMainQuest(ClientID, pGameServer);
}

// ---- 主线任务: 显示菜单 ----
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

	// 显示任务信息
	pGameServer->AddVote_Localization(ClientID, "null", pStep->m_LocaleTitle);

	// 显示描述行
	for (int i = 0; i < 5; i++)
	{
		const char *pDesc = pStep->m_LocaleDescLines[i];
		if (pDesc && pDesc[0] != '\0')
			pGameServer->AddVote_Localization(ClientID, "null", pDesc);
	}

	// 显示第一个需求物品
	if (pStep->m_Requirements[0].m_ItemID != 0)
	{
		int Counts = pGameServer->Server()->GetItemCount(ClientID, pStep->m_Requirements[0].m_ItemID);
		int Need = pStep->m_Requirements[0].m_NeedCount;

		if (pStep->m_Requirements[1].m_ItemID != 0)
		{
			// 有两个需求物品
			int Counts2 = pGameServer->Server()->GetItemCount(ClientID, pStep->m_Requirements[1].m_ItemID);
			int Need2 = pStep->m_Requirements[1].m_NeedCount;
			pGameServer->AddVote_Localization(ClientID, "null",
				"任务需求: {str:item1}, {str:item2} [{int:get}/{int:need} & {int:get2}/{int:need2}]",
				"item1", pGameServer->Server()->GetItemName(ClientID, pStep->m_Requirements[0].m_ItemID),
				"item2", pGameServer->Server()->GetItemName(ClientID, pStep->m_Requirements[1].m_ItemID),
				"get", &Counts, "need", &Need,
				"get2", &Counts2, "need2", &Need2);
		}
		else
		{
			// 只有一个需求物品
			pGameServer->AddVote_Localization(ClientID, "null",
				"[已获得: {int:get}/共需要: {int:need}]",
				"get", &Counts, "need", &Need);
		}
	}

	// 显示奖励信息
	char aRewardBuf[256] = {0};
	if (pStep->m_ExpReward > 0 && pStep->m_MoneyReward > 0)
		str_format(aRewardBuf, sizeof(aRewardBuf), "%d经验/%d白银", pStep->m_ExpReward, pStep->m_MoneyReward);
	else if (pStep->m_ExpReward > 0)
		str_format(aRewardBuf, sizeof(aRewardBuf), "%d经验", pStep->m_ExpReward);
	else if (pStep->m_MoneyReward > 0)
		str_format(aRewardBuf, sizeof(aRewardBuf), "%d白银", pStep->m_MoneyReward);
	else if (pStep->m_UpPointReward > 0)
		str_format(aRewardBuf, sizeof(aRewardBuf), "%d升级点", pStep->m_UpPointReward);

	if (aRewardBuf[0] != '\0')
		pGameServer->AddVote_Localization(ClientID, "null", "任务奖励: {str:got}", "got", aRewardBuf);

	// 显示特殊奖励（邮件物品等）
	if (pStep->m_MailItems[0].m_ItemID != 0 || pStep->m_MailItems[1].m_ItemID != 0)
	{
		char aMailBuf[128] = {0};
		if (pStep->m_MailItems[0].m_ItemID != 0 && pStep->m_MailItems[1].m_ItemID != 0)
			str_format(aMailBuf, sizeof(aMailBuf), "%s, %s",
				pGameServer->Server()->GetItemName(ClientID, pStep->m_MailItems[0].m_ItemID),
				pGameServer->Server()->GetItemName(ClientID, pStep->m_MailItems[1].m_ItemID));
		else if (pStep->m_MailItems[0].m_ItemID != 0)
			str_format(aMailBuf, sizeof(aMailBuf), "%s",
				pGameServer->Server()->GetItemName(ClientID, pStep->m_MailItems[0].m_ItemID));
		else
			str_format(aMailBuf, sizeof(aMailBuf), "%s",
				pGameServer->Server()->GetItemName(ClientID, pStep->m_MailItems[1].m_ItemID));
		pGameServer->AddVote_Localization(ClientID, "null", "+ {str:items}", "items", aMailBuf);
	}

	pGameServer->AddVote_Localization(ClientID, "passquest", "- 提交通过任务");
}

// ============================================================
// 每日任务实现
// ============================================================

int CQuestManager::GetDailyID() const
{
	time_t nowtime;
	time(&nowtime);
	tm *pTime = localtime(&nowtime);
	return pTime->tm_year + pTime->tm_mday + pTime->tm_mon + pTime->tm_yday + pTime->tm_wday + pTime->tm_mday;
}

void CQuestManager::DailyTick(CGameContext *pGameServer)
{
	time_t nowtime;
	time(&nowtime);
	tm *pTime = localtime(&nowtime);

	if (pTime->tm_hour != m_LastHour)
	{
		if (m_LastHour > pTime->tm_hour)
		{
			RefreshDaily(true, pGameServer);
			pGameServer->SendChatTarget_Localization(-1, CHATCATEGORY_DEFAULT, "每日任务已更新！");
		}
		m_LastHour = pTime->tm_hour;
	}
}

void CQuestManager::RefreshDaily(bool Reset, CGameContext *pGameServer)
{
	srand(GetDailyID());
	m_RandomNumber = rand();

	if (Reset)
	{
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			CPlayer *pP = pGameServer->m_apPlayers[i];
			if (!pP || pP->IsBot())
				continue;

			pGameServer->Server()->RemItem(i, COLLECTQUEST, pGameServer->Server()->GetItemCount(i, COLLECTQUEST), -1);
			pGameServer->Server()->RemItem(i, KILLQUEST, pGameServer->Server()->GetItemCount(i, KILLQUEST), -1);
			pGameServer->Server()->RemItem(i, CHALLENGEQUEST, pGameServer->Server()->GetItemCount(i, CHALLENGEQUEST), -1);
			pGameServer->Server()->SetItemSettingsCount(i, COLLECTQUEST, 0);
			pGameServer->Server()->SetItemSettingsCount(i, KILLQUEST, 0);
			pGameServer->Server()->SetItemSettingsCount(i, CHALLENGEQUEST, 0);

			pGameServer->m_apPlayers[i]->m_FinishedCollectQuest = false;
			pGameServer->m_apPlayers[i]->m_FinishedKillQuest = false;
			pGameServer->m_apPlayers[i]->m_FinishedChallengeQuest = false;

			pGameServer->UpdateStats(i);
		}
		pGameServer->Server()->Execute("UPDATE tw_uItems SET item_count = 1,item_settings = 0 WHERE il_id = 159");
		pGameServer->Server()->Execute("UPDATE tw_uItems SET item_count = 0,item_settings = 0 WHERE il_id = 164");
		pGameServer->Server()->Execute("UPDATE tw_uItems SET item_count = 0,item_settings = 0 WHERE il_id = 165");
	}
}

// ---- 每日任务: 根据类型和子类型计算物品/需求/奖励 ----

int CQuestManager::GetDailyQuestItem(int QuestType, int SubType) const
{
	long long int RandomNumber = m_RandomNumber;
	const SDailySubTypeDef *pDef = 0;

	for (int i = 0; i < ms_NumDailySubTypes; i++)
	{
		if (ms_aDailySubTypes[i].m_Type == QuestType && ms_aDailySubTypes[i].m_SubType == SubType)
		{
			pDef = &ms_aDailySubTypes[i];
			break;
		}
	}
	if (!pDef || pDef->m_ItemPool.m_Count <= 0)
		return 1;

	// 为不同类型的每日任务使用不同的随机偏移
	long long int Divisor = 1;
	switch (QuestType)
	{
	case DAILY_KILL:
		Divisor = 17;
		break;
	case DAILY_CHALLENGE:
		if (SubType == DAILYSUB_CHALLENGE_KILL)
			Divisor = 18;
		break;
	}

	int Index = (RandomNumber / Divisor) % pDef->m_ItemPool.m_Count;
	return pDef->m_ItemPool.m_Items[Index];
}

int CQuestManager::GetDailyQuestNeed(int QuestType, int SubType) const
{
	long long int R = m_RandomNumber;
	int Index;

	switch (QuestType)
	{
	case DAILY_COLLECT:
		switch (SubType)
		{
		case DAILYSUB_COLLECT_FARM:     return (R % 10 + 8) * 1000000;
		case DAILYSUB_COLLECT_FOOD:     return R % 50 + 30;
		case DAILYSUB_COLLECT_OFFERING:
			Index = R % 6;
			if (Index == 4 || Index == 5)
				return R % 300;
			return R % 2000 + 300;
		case DAILYSUB_COLLECT_MINING:   return (R % 23 + 8) * 100000;
		}
		break;

	case DAILY_KILL:
		return ((R / 2) % 500) + 500;

	case DAILY_CHALLENGE:
		switch (SubType)
		{
		case DAILYSUB_CHALLENGE_FARM:     return (R % 10 + 8) * 10000000;
		case DAILYSUB_CHALLENGE_OFFERING: return R % 1000 + 2000;
		case DAILYSUB_CHALLENGE_MINING:   return (R % 23 + 8) * 10000000;
		case DAILYSUB_CHALLENGE_KILL:     return 7000;
		}
		break;
	}
	return 1145141919;  // 哨兵值
}

int CQuestManager::GetDailyQuestUpgr(int QuestType, int SubType) const
{
	long long int R = m_RandomNumber;

	switch (QuestType)
	{
	case DAILY_COLLECT:
		switch (SubType)
		{
		case DAILYSUB_COLLECT_FARM:     return 50;
		case DAILYSUB_COLLECT_FOOD:     return 70;
		case DAILYSUB_COLLECT_OFFERING:
			if (R % 6 == 4 || R % 6 == 5)
				return R % 1000 + 500;
			return R % 500;
		case DAILYSUB_COLLECT_MINING:   return 70;
		}
		break;

	case DAILY_KILL:
		return R % 100 + 150;

	case DAILY_CHALLENGE:
		switch (SubType)
		{
		case DAILYSUB_CHALLENGE_FARM:     return 750;
		case DAILYSUB_CHALLENGE_OFFERING: return 750;
		case DAILYSUB_CHALLENGE_MINING:   return 750;
		case DAILYSUB_CHALLENGE_KILL:
			if (R % 6 == 4 || R % 6 == 5)
				return 750;
			return R % 750 + 200;
		}
		break;
	}
	return 1;
}

// ---- 每日任务: 检查是否已完成 ----
bool CQuestManager::IsPlayerDailyQuestCompleted(int ClientID, int QuestType, CGameContext *pGameServer) const
{
	int DailyID = GetDailyID();
	switch (QuestType)
	{
	case DAILY_COLLECT:
		return pGameServer->Server()->GetItemSettings(ClientID, COLLECTQUEST) == DailyID;
	case DAILY_KILL:
		return pGameServer->Server()->GetItemSettings(ClientID, KILLQUEST) == DailyID;
	case DAILY_CHALLENGE:
		return pGameServer->Server()->GetItemSettings(ClientID, CHALLENGEQUEST) == DailyID;
	}
	return false;
}

	// ---- 每日任务: 检查是否可完成（用户操作 'passdayquest'） ----
	bool CQuestManager::CanCompleteDailyQuest(int ClientID, int QuestType, int SubType, CGameContext *pGameServer) const
	{
		CPlayer *pPlayer = pGameServer->m_apPlayers[ClientID];

		switch (QuestType)
		{
		case DAILY_COLLECT:
			if (pPlayer->m_FinishedCollectQuest)
				return false;
			break;
		case DAILY_KILL:
			if (pPlayer->m_FinishedKillQuest)
				return false;
			break;
		case DAILY_CHALLENGE:
			if (pPlayer->m_FinishedChallengeQuest)
				return false;
			break;
		default:
			return false;
		}

		int Item = GetDailyQuestItem(QuestType, SubType);
		int Need = GetDailyQuestNeed(QuestType, SubType);

		// CHALLENGE4 检查专用的任务计数物品(CHALLENGEQUEST)，而非目标物品列表中的 bot ID
		if (QuestType == DAILY_CHALLENGE && SubType == DAILYSUB_CHALLENGE_KILL)
		{
			return static_cast<unsigned long long>(pGameServer->Server()->GetItemCount(ClientID, CHALLENGEQUEST))
			       >= static_cast<unsigned long long>(Need);
		}

		return static_cast<unsigned long long>(pGameServer->Server()->GetItemCount(ClientID, Item))
		       >= static_cast<unsigned long long>(Need);
	}

// ---- 每日任务: 完成并领取奖励 ----
void CQuestManager::CompleteDailyQuest(int ClientID, int QuestType, int SubType, CGameContext *pGameServer)
{
	int Item = GetDailyQuestItem(QuestType, SubType);
	int Need = GetDailyQuestNeed(QuestType, SubType);
	int Upgr = GetDailyQuestUpgr(QuestType, SubType);
	int DailyID = GetDailyID();

	// 扣除物品（挑战任务子类型4不需要扣除物品，只需要达到击杀数）
	if (QuestType == DAILY_CHALLENGE && SubType == DAILYSUB_CHALLENGE_KILL)
	{
		// CHALLENGE4: 只检查 CHALLENGEQUEST 计数，不扣除
	}
	else
	{
		pGameServer->Server()->RemItem(ClientID, Item, Need, -1);
	}

	// 发放升级点奖励
	pGameServer->m_apPlayers[ClientID]->GiveUpPoint(Upgr);

	// 标记完成
	int QuestItemID;
	switch (QuestType)
	{
	case DAILY_COLLECT:
		QuestItemID = COLLECTQUEST;
		pGameServer->m_apPlayers[ClientID]->m_FinishedCollectQuest = true;
		break;
	case DAILY_KILL:
		QuestItemID = KILLQUEST;
		pGameServer->m_apPlayers[ClientID]->m_FinishedKillQuest = true;
		break;
	case DAILY_CHALLENGE:
		QuestItemID = CHALLENGEQUEST;
		pGameServer->m_apPlayers[ClientID]->m_FinishedChallengeQuest = true;
		break;
	default:
		return;
	}

	pGameServer->Server()->SetItemSettingsCount(ClientID, QuestItemID, DailyID);
}

// ---- 每日任务: 显示菜单 ----
void CQuestManager::ShowDailyQuestMenu(int ClientID, CGameContext *pGameServer, bool ResetVotes)
{
	if (ResetVotes)
		pGameServer->m_apPlayers[ClientID]->m_LastVotelist = QUESTMENU;

	int QuestType = pGameServer->m_apPlayers[ClientID]->m_SelectQuest;
	int SubType = pGameServer->m_apPlayers[ClientID]->m_SelectSubQuest;

	int Item = GetDailyQuestItem(QuestType, SubType);
	int Num = GetDailyQuestNeed(QuestType, SubType);
	int Upgr = GetDailyQuestUpgr(QuestType, SubType);

	pGameServer->AddVote_Localization(ClientID, "null", "☪ 信息 ( ′ ω ` )?:");
	pGameServer->AddVote_Localization(ClientID, "null", "每日任务!");
	int iRand = (int)m_RandomNumber;
	pGameServer->AddVote_Localization(ClientID, "null", "今日任务序列：{int:randomNum}", "randomNum", &iRand);
	pGameServer->AddVote("", "null", ClientID);
	pGameServer->AddVote_Localization(ClientID, "que0", "☞ 收集任务");
	pGameServer->AddVote_Localization(ClientID, "que1", "☞ 击杀任务");
	pGameServer->AddVote_Localization(ClientID, "que2", "☞ 挑战任务");

	// 收集任务详情
	if (QuestType == DAILY_COLLECT)
	{
		pGameServer->AddVote("", "null", ClientID);
		if (IsPlayerDailyQuestCompleted(ClientID, DAILY_COLLECT, pGameServer))
		{
			pGameServer->AddVote_Localization(ClientID, "que0", "- 收集任务已完成");
		}
		else
		{
			int Have = pGameServer->Server()->GetItemCount(ClientID, Item);

			pGameServer->AddVote_Localization(ClientID, "sque0", "- 农业经济");
			if (random_prob(0.005))
				pGameServer->AddVote_Localization(ClientID, "sque2", "- 食品...?");
			else
				pGameServer->AddVote_Localization(ClientID, "sque1", "- 食品");
			pGameServer->AddVote_Localization(ClientID, "sque3", "- 矿业经济");
			pGameServer->AddVote("", "null", ClientID);

			if (SubType == DAILYSUB_COLLECT_FARM)
			{
				pGameServer->AddVote_Localization(ClientID, "null", "收集 {str:iname} [{int:num}/{int:need}]",
					"iname", pGameServer->Server()->GetItemName(ClientID, Item), "num", &Have, "need", &Num);
				pGameServer->AddVote_Localization(ClientID, "null", "任务奖励：{int:num}升级点", "num", &Upgr);
				pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 提交任务");
			}
			else if (SubType == DAILYSUB_COLLECT_FOOD)
			{
				pGameServer->AddVote_Localization(ClientID, "null", "收集 {str:iname} [{int:num}/{int:need}]",
					"iname", pGameServer->Server()->GetItemName(ClientID, Item), "num", &Have, "need", &Num);
				pGameServer->AddVote_Localization(ClientID, "null", "任务奖励：{int:num}升级点", "num", &Upgr);
				pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 提交任务");
			}
			else if (SubType == DAILYSUB_COLLECT_OFFERING)
			{
				pGameServer->AddVote_Localization(ClientID, "null", "Vivit为DerumDeumVivitDerumDeumVivitDerumDeum");
				pGameServer->AddVote_Localization(ClientID, "null", "Vivit神DerumDeumVivitDerumDeumVivitDerumDeum");
				pGameServer->AddVote_Localization(ClientID, "null", "Vivit献DerumDeumVivitDerumDeumVivitDerumDeum");
				pGameServer->AddVote_Localization(ClientID, "null", "Vivit上DerumDeumVivitDerumDeumVivitDerumDeum");
				pGameServer->AddVote_Localization(ClientID, "null", "THA{str:iname} [{int:num}/{int:need}]",
					"iname", pGameServer->Server()->GetItemName(ClientID, Item), "num", &Have, "need", &Num);
				pGameServer->AddVote_Localization(ClientID, "null", "enruw奖ails：???升级点");
				pGameServer->AddVote_Localization(ClientID, "passdayquest", "- S贡品S -");
			}
			else if (SubType == DAILYSUB_COLLECT_MINING)
			{
				pGameServer->AddVote_Localization(ClientID, "null", "收集 {str:iname} [{int:num}/{int:need}]",
					"iname", pGameServer->Server()->GetItemName(ClientID, Item), "num", &Have, "need", &Num);
				pGameServer->AddVote_Localization(ClientID, "null", "任务奖励：{int:num}升级点", "num", &Upgr);
				pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 提交任务");
			}
		}
		pGameServer->AddVote("", "null", ClientID);
		pGameServer->AddVote("", "null", ClientID);
	}

	// 击杀任务详情
	if (QuestType == DAILY_KILL)
	{
		if (IsPlayerDailyQuestCompleted(ClientID, DAILY_KILL, pGameServer))
		{
			pGameServer->AddVote_Localization(ClientID, "que0", "- 击杀任务已完成");
		}
		else
		{
			int Have = pGameServer->Server()->GetItemCount(ClientID, KILLQUEST);
			pGameServer->AddVote_Localization(ClientID, "null", "击杀 {str:iname} [{int:num}/{int:need}]",
				"iname", pGameServer->GetBotName(Item), "num", &Have, "need", &Num);
			pGameServer->AddVote_Localization(ClientID, "null", "任务奖励：{int:num}升级点", "num", &Upgr);
			pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 提交任务");
			pGameServer->AddVote("", "null", ClientID);
			pGameServer->AddVote("", "null", ClientID);
		}
	}

	// 挑战任务详情
	if (QuestType == DAILY_CHALLENGE)
	{
		pGameServer->AddVote("", "null", ClientID);
		if (IsPlayerDailyQuestCompleted(ClientID, DAILY_CHALLENGE, pGameServer))
		{
			pGameServer->AddVote_Localization(ClientID, "que0", "- 挑战任务已完成");
		}
		else
		{
			int Have = pGameServer->Server()->GetItemCount(ClientID, Item);

			pGameServer->AddVote_Localization(ClientID, "sque0", "- 农业经济");
			if (random_prob(0.01f))
				pGameServer->AddVote_Localization(ClientID, "sque1", "- 祭品");
			pGameServer->AddVote_Localization(ClientID, "sque2", "- 矿业经济");
			pGameServer->AddVote_Localization(ClientID, "sque3", "- 击杀挑战");
			pGameServer->AddVote("", "null", ClientID);

			if (SubType == DAILYSUB_CHALLENGE_FARM)
			{
				pGameServer->AddVote_Localization(ClientID, "null", "收集 {str:iname} [{int:num}/{int:need}]",
					"iname", pGameServer->Server()->GetItemName(ClientID, Item), "num", &Have, "need", &Num);
				pGameServer->AddVote_Localization(ClientID, "null", "任务奖励：{int:num}升级点", "num", &Upgr);
				pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 提交任务");
			}
			else if (SubType == DAILYSUB_CHALLENGE_OFFERING)
			{
				pGameServer->AddVote_Localization(ClientID, "null", "VDD献上 {str:iname} [{int:num}/{int:need}]",
					"iname", pGameServer->Server()->GetItemName(ClientID, Item), "num", &Have, "need", &Num);
				pGameServer->AddVote_Localization(ClientID, "null", "任务奖励：{int:num}升级点", "num", &Upgr);
				pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 贡品");
			}
			else if (SubType == DAILYSUB_CHALLENGE_MINING)
			{
				pGameServer->AddVote_Localization(ClientID, "null", "收集 {str:iname} [{int:num}/{int:need}]",
					"iname", pGameServer->Server()->GetItemName(ClientID, Item), "num", &Have, "need", &Num);
				pGameServer->AddVote_Localization(ClientID, "null", "任务奖励：{int:num}升级点", "num", &Upgr);
				pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 提交任务");
			}
			else if (SubType == DAILYSUB_CHALLENGE_KILL)
			{
				Have = pGameServer->Server()->GetItemCount(ClientID, CHALLENGEQUEST);
				pGameServer->AddVote_Localization(ClientID, "null", "击杀 {str:iname} [{int:num}/{int:need}]",
					"iname", pGameServer->GetBotName(Item), "num", &Have, "need", &Num);
				pGameServer->AddVote_Localization(ClientID, "null", "任务奖励：{int:num}升级点", "num", &Upgr);
				pGameServer->AddVote_Localization(ClientID, "passdayquest", "- 提交任务");
			}
		}
		pGameServer->AddVote("", "null", ClientID);
	}

	pGameServer->AddBack(ClientID);
}

// ---- 每日任务: 切换任务类型 ----
void CQuestManager::HandleSelectQuestType(int ClientID, int QuestType, CGameContext *pGameServer)
{
	if (pGameServer->m_apPlayers[ClientID]->m_SelectQuest == QuestType)
		pGameServer->m_apPlayers[ClientID]->m_SelectQuest = -1;
	else
		pGameServer->m_apPlayers[ClientID]->m_SelectQuest = QuestType;
	pGameServer->ResetVotes(ClientID, DAYQUEST);
}

// ---- 每日任务: 切换子任务 ----
void CQuestManager::HandleSelectSubQuest(int ClientID, int SubType, CGameContext *pGameServer)
{
	if (pGameServer->m_apPlayers[ClientID]->m_SelectSubQuest == SubType)
		pGameServer->m_apPlayers[ClientID]->m_SelectSubQuest = -1;
	else
		pGameServer->m_apPlayers[ClientID]->m_SelectSubQuest = SubType;
	pGameServer->ResetVotes(ClientID, DAYQUEST);
}

// ---- 每日任务: 处理 passdayquest 命令 ----
void CQuestManager::HandlePassDayQuest(int ClientID, CGameContext *pGameServer)
{
	int Quest = pGameServer->m_apPlayers[ClientID]->m_SelectQuest;
	int Sub = pGameServer->m_apPlayers[ClientID]->m_SelectSubQuest;

	if (Quest < 0 || Quest >= NUM_DAILY_TYPES)
		return;

	CPlayer *pPlayer = pGameServer->m_apPlayers[ClientID];

	// 检查是否已完成
	switch (Quest)
	{
	case DAILY_COLLECT:
		if (pPlayer->m_FinishedCollectQuest)
		{
			pGameServer->SendChatTarget_Localization(ClientID, CHATCATEGORY_DEFAULT, "你滴任务，完成啦！(大声)", NULL);
			return;
		}
		break;
	case DAILY_KILL:
		if (pPlayer->m_FinishedKillQuest)
		{
			pGameServer->SendChatTarget_Localization(ClientID, CHATCATEGORY_DEFAULT, "你滴任务，完成啦！(大声)", NULL);
			return;
		}
		break;
	case DAILY_CHALLENGE:
		if (pPlayer->m_FinishedChallengeQuest)
		{
			pGameServer->SendChatTarget_Localization(ClientID, CHATCATEGORY_DEFAULT, "你滴任务，完成啦！(大声)", NULL);
			return;
		}
		break;
	default:
		return;
	}

	// 检查是否可以完成
	if (!CanCompleteDailyQuest(ClientID, Quest, Sub, pGameServer))
	{
		pGameServer->SendChatTarget_Localization(ClientID, CHATCATEGORY_DEFAULT, "任务还未完成!", NULL);
		return;
	}

	CompleteDailyQuest(ClientID, Quest, Sub, pGameServer);
	pGameServer->ResetVotes(ClientID, DAYQUEST);
}
