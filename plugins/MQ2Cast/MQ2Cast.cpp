//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// Project: MQ2Cast.cpp   | Set DEBUGGING to true for DEBUGGING msg
// Author: s0rCieR        |
//         A_Enchanter_00 |
//         htw            |
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// Last edited by: devestator 10/30/2010 Updated for HoT & MQ2BagWindow
// Last edited by: Maskoi  4/15/2011 ->item changes
// 10.0 edited by: EqMule  12/14/2013 Removed MQ2BagWindow dependecy - Sponsored by RedGuides.com
// Since a recent patch, I think that there are very few items that require a swap/must equip to be cast.
// Therefore, this version does NOT swap stuff in and out of bags and/or use bandolier anymore.
// If you need that in your macro, use mq2exchange or /itemnotify to move/swap things. (or a version prior to 9.11)
// I also, made this plugin debugable again.
// Please future editors, keep the {} in the if's and while's
// Cause debugging without them is a total pain. (cant set bps if the return is on same line for example)
// 10.02 edited by: three-p-o 1/12/2014 switched over item casting to use EQ's own /cast command.
//    Fixed issue in CastHandle with {} not matching up properly. Resolves issue with cast returning before casting is completed.
// Also added in my changes to return CAST_UNKNOWN if you are trying to cast or memorize a spell that is not in your book.
// 10.03 edited by: three-p-o 3/23/2014  Updated for MQ2-20140322
// 10.04 edited by: trev 3/28/2014  Fixed: spells not getting memmed
// 10.05 edited by: eqmule 2/17/2016  new spell system
// 11.0 - Eqmule 07-22-2016 - Added string safety.
// 11.1 - Eqmule 08-22-2017 - Dont check Twisting if Mq2Twist is NOT loaded and some other tweaks to improve performance.
// 11.2 - SwiftyMUSE 11-20-2017 - Increased delay_cast to handle casting not happening for spells loading delays
//    Added blocked spell handling.
//    Fixed compile typecast warning.
// 11.3 - SwiftyMUSE 11-17-2019 - Fixed: Casting interrupted identification on Live
// 11.4 - EqMule 06-01-2020 - Fixed: Casting fizzled and interrupted identifications on Live
// 11.5 - AmericanNero 12/27/2021 - Fixed typo "powerfull" and Blech include
// 11.6 - brainiac 1/22/2024 - I was coerced to put something here. Cleaned things up.
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

#include <mq/Plugin.h>

#include <Blech/Blech.h>

PreSetup("MQ2Cast");
PLUGIN_VERSION(11.6);

bool DEBUGGING = false;

constexpr int DELAY_CAST = 16000;
constexpr int DELAY_STOP = 4000;
constexpr int DELAY_PULSE = 125;

enum eCastStatus {
	CAST_SUCCESS               = 0,
	CAST_INTERRUPTED           = 1,
	CAST_RESIST                = 2,
	CAST_COLLAPSE              = 3,
	CAST_RECOVER               = 4,
	CAST_FIZZLE                = 5,
	CAST_STANDING              = 6,
	CAST_STUNNED               = 7,
	CAST_INVISIBLE             = 8,
	CAST_NOTREADY              = 9,
	CAST_OUTOFMANA             = 10,
	CAST_OUTOFRANGE            = 11,
	CAST_NOTARGET              = 12,
	CAST_CANNOTSEE             = 13,
	CAST_COMPONENTS            = 14,
	CAST_OUTDOORS              = 15,
	CAST_TAKEHOLD              = 16,
	CAST_IMMUNE                = 17,
	CAST_DISTRACTED            = 18,
	CAST_ABORTED               = 19,
	CAST_UNKNOWN               = 20
};

constexpr int FLAG_COMPLETE    =  0;
constexpr int FLAG_REQUEST     = -1;
constexpr int FLAG_PROGRESS1   = -2;
constexpr int FLAG_PROGRESS2   = -3;
constexpr int FLAG_PROGRESS3   = -4;
constexpr int FLAG_PROGRESS4   = -5;

constexpr int DONE_COMPLETE    = -3;
constexpr int DONE_ABORTED     = -2;
constexpr int DONE_PROGRESS    = -1;
constexpr int DONE_SUCCESS     =  0;

enum eCastingType {
	TYPE_NONE                  = -1,
	TYPE_SPELL                 = 1,
	TYPE_ALT                   = 2,
	TYPE_ITEM                  = 3,
};

constexpr int RECAST_DEAD      = 2;
constexpr int RECAST_LAND      = 1;
constexpr int RECAST_ZERO      = 0;

constexpr int NOID             = -1;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
int          DELAY_MEMO = 13000;

bool         BardBeta = true;
bool         Immobile = false;          // Immobile?
bool         Invisible = false;         // Invisibility Check?
bool         Twisting = false;          // Twisting?
bool         Casting = false;           // Casting Window was opened?
int          Resultat = CAST_SUCCESS;   // Resultat
uint64_t     ImmobileT = 0;             // Estimate when it be immobilized!

int          CastingC = NOID;           // Casting Current ID
int          CastingE = CAST_SUCCESS;   // Casting Current Result
int          CastingL = NOID;           // Casting LastOne ID
int          CastingX = CAST_SUCCESS;   // Casting LastOne Result
uint64_t     CastingT = 0;              // Casting Timeout
int          CastingO = NOID;           // Casting OnTarget
uint64_t     CastingP = 0;              // Casting Pulse

int          TargI = 0;                 // Target ID
int          TargC = 0;                 // Target Current

int          StopF = FLAG_COMPLETE;     // Stop Event Flag Progress?
int          StopE = DONE_SUCCESS;      // Stop Event Exit Value
uint64_t     StopM = 0;                 // Stop Event Mark

int          MoveA = FLAG_COMPLETE;     // Move Event AdvPath?
int          MoveS = FLAG_COMPLETE;     // Move Event Stick?
int          MoveN = FLAG_COMPLETE;     // Move Event Nav?
int          MoveF = FLAG_COMPLETE;     // Move Event MQ2AdvPath Following?
int          MoveP = FLAG_COMPLETE;     // Move Event MQ2AdvPath Pathing?

int          MemoF = FLAG_COMPLETE;     // Memo Event Flag
int          MemoE = DONE_SUCCESS;      // Memo Event Exit
uint64_t     MemoM = 0;                 // Memo Event Mark

int          ItemF = FLAG_COMPLETE;     // Item Flag

int          DuckF = FLAG_COMPLETE;     // Duck Flag
uint64_t     DuckM = 0;                 // Duck Time Stamp

int          CastF = FLAG_COMPLETE;     // Cast Flag
int          CastE = CAST_SUCCESS;      // Cast Exit Return value
int          CastG = NOID;              // Cast Gem ID
void*        CastI = nullptr;           // Cast ID   [spell/alt/item]
eCastingType CastK = TYPE_NONE;         // Cast Kind [spell/alt/item]
int          CastT = 0;                 // Cast Time [spell/alt/item]
uint64_t     CastM = 0;                 // Cast TimeMark Start Casting
int          CastR = 0;                 // Cast Retry Counter
int          CastW = 0;                 // Cast Retry Type
char         CastC[MAX_STRING];         // Cast SpellType
char         CastN[MAX_STRING];         // Cast SpellName
EQ_Spell*    CastS = nullptr;           // Cast Spell Pointer

bool         Parsed = false;            // BTree List Found Flags
Blech        LIST013('#');              // BTree List for OnChat Message on Color  13 - CONCOLOR_RED
Blech        LIST264('#');              // BTree List for OnChat Message on Color 264 - Spells
Blech        LIST289('#');              // BTree List for OnChat Message on Color 289 - Spell failures
Blech        UNKNOWN('#');              // BTree List for OnChat Message on UNKNOWN Yet Color
Blech        SUCCESS('#');              // BTree List for OnChat Message on SUCCESS Detection

int          PulseCount = 0;

EQ_Spell*    fFIND = nullptr;           // SpellFind - Casting Spell Effect
void*        fINFO = nullptr;           // SpellFind - Casting Type Structure
eCastingType fTYPE = TYPE_NONE;         // SpellFind - Casting Type
int          fTIME = 0;                 // SpellFind - Casting Time
const char*  fNAME = nullptr;           // SpellFind - Casting Name

SpellLoadout SpellToMemorize;           // Favorite Spells Array
int          SpellTotal = 0;            // Favorite Spells Total

const char*  ListGems[] = { "1","2","3","4","5","6","7","8","9","A","B","C","D","E","F" };

void WinClick(CXWnd* Wnd, const char* ScreenID, const char* ClickNotification, uint32_t KeyState);
void ClickBack();

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

#define aCastEvent(List,Value,Filter) List.AddEvent(Filter, CastEvent, (void*)Value);

void __stdcall CastEvent(unsigned int ID, void* pData, PBLECHVALUE pValues)
{
	Parsed = true;

	int value = static_cast<int>(reinterpret_cast<intptr_t>(pData));

	if (CastingE < value)
	{
		CastingE = value;
	}

	if (DEBUGGING)
	{
		WriteChatf("[%I64u] MQ2Cast:[OnChat]: Result=[%d] Called=[%d].", GetTickCount64(), CastingE, value);
	}
}

CAltAbilityData* AltAbility(const char* ID)
{
	if (!ID[0]) return nullptr;

	int level = pLocalPlayer ? pLocalPlayer->Level : -1;

	for (int nAbility = 0; nAbility < AA_CHAR_MAX_REAL; nAbility++)
	{
		if (GetPcProfile()->AAList[nAbility].AAIndex)
		{
			if (IsNumber(ID))
			{
				int abilityID = GetIntFromString(ID, 0);

				if (CAltAbilityData* pAbility = GetAAByIdWrapper(GetPcProfile()->AAList[nAbility].AAIndex))
				{
					if (pAbility->ID == abilityID)
					{
						return pAbility;
					}
				}
			}
			else
			{
				if (CAltAbilityData* pAbility = GetAAByIdWrapper(GetPcProfile()->AAList[nAbility].AAIndex, level))
				{
					if (const char* pName = pCDBStr->GetString(pAbility->nName, eAltAbilityName, nullptr))
					{
						if (!_stricmp(ID, pName))
						{
							return pAbility;
						}
					}
				}
			}
		}
	}
	return nullptr;
}

bool IsBardClass()
{
	return pLocalPC && pLocalPC->GetClass() == Bard;
}

void Cast(const char* zFormat, ...)
{
	char zOutput[MAX_STRING] = { 0 };
	va_list vaList;
	va_start(vaList, zFormat);

	vsprintf_s(zOutput, zFormat, vaList);
	if (!zOutput[0])
	{
		return;
	}

	Cast(pLocalPlayer, zOutput);
}

int CastingLeft()
{
	int CL = 0;

	if (pCastingWnd && pCastingWnd->IsVisible())
	{
		CL = pLocalPlayer->CastingData.SpellETA - pLocalPlayer->TimeStamp;
		if (CL < 1)
		{
			CL = 1;
		}
	}
	return CL;
}

bool Evaluate(const char* expression)
{
	char szTempExpression[MAX_STRING] = { 0 };
	strcpy_s(szTempExpression, expression);

	ParseMacroData(szTempExpression, sizeof(szTempExpression));
	return atoi(szTempExpression) != 0;
}

void Execute(const char* zFormat, ...)
{
	char zOutput[MAX_STRING] = { 0 };

	va_list vaList;
	va_start(vaList, zFormat);
	vsprintf_s(zOutput, zFormat, vaList);

	if (!zOutput[0])
	{
		return;
	}

	DoCommand(pLocalPlayer, zOutput);
}

bool Flags()
{
	if (!IsBardClass() && pCastingWnd && pCastingWnd->IsVisible())
	{
		if (DEBUGGING)
		{
			WriteChatf("MQ2Cast: pCastingWnd=TRUE");
		}
		return true;
	}

	if (CastF != FLAG_COMPLETE) { if (DEBUGGING) WriteChatf("MQ2Cast: CastF!=FLAG_COMPLETE"); return true; }
	if (DuckF != FLAG_COMPLETE) { if (DEBUGGING) WriteChatf("MQ2Cast: DuckF!=FLAG_COMPLETE"); return true; }
	if (ItemF != FLAG_COMPLETE) { if (DEBUGGING) WriteChatf("MQ2Cast: ItemF!=FLAG_COMPLETE"); return true; }
	if (MemoF != FLAG_COMPLETE) { if (DEBUGGING) WriteChatf("MQ2Cast: MemoF!=FLAG_COMPLETE"); return true; }
	if (StopF != FLAG_COMPLETE) { if (DEBUGGING) WriteChatf("MQ2Cast: StopF!=FLAG_COMPLETE"); return true; }
	if (MoveS != FLAG_COMPLETE) { if (DEBUGGING) WriteChatf("MQ2Cast: MoveS!=FLAG_COMPLETE"); return true; }
	if (MoveF != FLAG_COMPLETE) { if (DEBUGGING) WriteChatf("MQ2Cast: MoveF!=FLAG_COMPLETE"); return true; }
	if (MoveP != FLAG_COMPLETE) { if (DEBUGGING) WriteChatf("MQ2Cast: MoveP!=FLAG_COMPLETE"); return true; }
	if (MoveA != FLAG_COMPLETE) { if (DEBUGGING) WriteChatf("MQ2Cast: MoveA!=FLAG_COMPLETE"); return true; }
	if (MoveN != FLAG_COMPLETE) { if (DEBUGGING) WriteChatf("MQ2Cast: MoveN!=FLAG_COMPLETE"); return true; }

	return false;
}

int GetGemID(int SpellID)
{
	for (int GEM = 0; GEM < NUM_SPELL_GEMS; GEM++)
	{
		if (GetMemorizedSpell(GEM) == SpellID)
		{
			return GEM;
		}
	}

	return NOID;
}

bool IsGemReady(int ID)
{
	if (GetSpellByID(GetMemorizedSpell(ID)))
	{
		if (pDisplay->TimeStamp > pLocalPlayer->SpellGemETA[ID] && pDisplay->TimeStamp > pLocalPlayer->GetSpellCooldownETA())
		{
			return true;
		}
	}
	return false;
}

bool GiftOfMana()
{
	for (int nBuff = 0; nBuff < NUM_SHORT_BUFFS; nBuff++)
	{
		if (EQ_Spell* pSpell = GetSpellByID(GetPcProfile()->GetTempEffect(nBuff).SpellID))
		{
			if (!_stricmp("Gift of Mana", pSpell->Name))
			{
				return true;
			}
		}
	}

	return false;
}

void QueueSpellForMemorization(int GemID, EQ_Spell* Spell)
{
	if (!Spell || Spell->ClassLevel[pLocalPC->GetClass()] > pLocalPC->GetLevel())
	{
		return;
	}

	for (int sp = 0; sp < NUM_SPELL_GEMS; sp++)
	{
		if (SpellToMemorize.SpellId[sp] == Spell->ID)
		{
			SpellToMemorize.SpellId[sp] = -1;
		}
	}

	SpellToMemorize.SpellId[GemID < NUM_SPELL_GEMS ? GemID : 4] = Spell->ID;
}

float Speed()
{
	float MySpeed = 0.0f;

	if (pLocalPlayer)
	{
		if (pLocalPlayer->Mount)
		{
			MySpeed = pLocalPlayer->Mount->SpeedRun;
		}
		else
		{
			MySpeed = pLocalPlayer->SpeedRun;
		}
	}

	return MySpeed;
}

void PrepareSuccessEvent(EQ_Spell* CastSpell)
{
	SUCCESS.Reset();

	if (CastSpell)
	{
		char Temps[MAX_STRING];
		bool Added = false;

		if (const char* str = GetSpellString(CastSpell->ID, SpellStringCastOnYou))
		{
			sprintf_s(Temps, "%s#*#", str);
			aCastEvent(SUCCESS, CAST_SUCCESS, Temps);
			Added = true;
		}
		if (const char* str = GetSpellString(CastSpell->ID, SpellStringCastOnAnother))
		{
			sprintf_s(Temps, "#*#%s#*#", str);
			aCastEvent(SUCCESS, CAST_SUCCESS, Temps);
			Added = true;
		}

		if (!Added)
		{
			aCastEvent(SUCCESS, CAST_SUCCESS, "You begin casting#*#");
		}
	}
}

bool IsImmobileCheck()
{
	uint64_t MyTimer = GetTickCount64();

	if (Speed() != 0.0f)
	{
		ImmobileT = MyTimer + 500;
	}

	return (!gbMoving && (!ImmobileT || MyTimer > ImmobileT));
}

bool IsPaused()
{
	if (IsBardClass())
	{
		return false;
	}

	if (pLootWnd && pLootWnd->IsVisible())
	{
		return true;
	}

	if (pBankWnd && pBankWnd->IsVisible())
	{
		return true;
	}

	if (pMerchantWnd && pMerchantWnd->IsVisible())
	{
		return true;
	}

	if (pTradeWnd && pTradeWnd->IsVisible())
	{
		return true;
	}

	if (pGiveWnd && pGiveWnd->IsVisible())
	{
		return true;
	}

	if (pTributeMasterWnd && pTributeMasterWnd->IsVisible())
	{
		return true;
	}

	if (pGuildBankWnd && pGuildBankWnd->IsVisible())
	{
		return true;
	}

	return false;
}

void Reset()
{
	TargI = 0;                 // Target ID
	TargC = 0;                 // Target Check ID
	StopF = FLAG_COMPLETE;     // Stop Event Flag Progress?
	StopE = DONE_SUCCESS;      // Stop Event Exit Value
	MoveA = FLAG_COMPLETE;     // Stop Event AdvPath?
	MoveN = FLAG_COMPLETE;     // Stop Event Nav?
	MoveS = FLAG_COMPLETE;     // Stop Event Stick?
	MoveF = FLAG_COMPLETE;     // Stop Event MQ2AdvPath Following?
	MoveP = FLAG_COMPLETE;     // Stop Event MQ2AdvPath Pathing?
	MemoF = FLAG_COMPLETE;     // Memo Event Flag
	MemoE = DONE_SUCCESS;      // Memo Event Exit
	ItemF = FLAG_COMPLETE;     // Item Flag
	DuckF = FLAG_COMPLETE;     // Duck Flag
	CastF = FLAG_COMPLETE;     // Cast Flag
	CastE = CAST_SUCCESS;      // Cast Exit Return value
	CastG = NOID;              // Cast Gem ID
	CastI = nullptr;           // Cast ID   [spell/alt/item/disc]
	CastK = TYPE_NONE;         // Cast Kind [spell/alt/item/disc] [-1=unknown]
	CastT = 0;                 // Cast Time [spell/alt/item/disc]
	CastC[0] = 0;              // Cast SpellType
	CastN[0] = 0;              // Cast SpellName
	CastR = 1;                 // Cast Retry Counter
	CastW = 0;                 // Cast Retry Type
	Invisible = false;         // Invisibility Check?
	ZeroMemory(&SpellToMemorize, sizeof(SpellLoadout));
	strcpy_s(SpellToMemorize.Name, "Mem a Spell");
	SpellToMemorize.inuse = true;

	for (int sp = 0; sp < NUM_SPELL_GEMS; sp++)
	{
		SpellToMemorize.SpellId[sp] = -1;
	}
	SpellTotal = 0;
}

int GetInvSlotFromString(const char* ID)
{
	if (IsNumber(ID))
	{
		int number = GetIntFromString(ID, -1);

		return (number >= 0 && number < NUM_INV_SLOTS) ? number : NOID;
	}
	for (int idx = 0; szItemSlot[idx]; idx++)
	{
		if (ci_equals(ID, szItemSlot[idx]))
		{
			return idx;
		}
	}

	return NOID;
}

EQ_Spell* GetSpellFromSpellBook(const char* SpellNameOrID)
{
	if (SpellNameOrID[0])
	{
		if (IsNumber(SpellNameOrID))
		{
			int spellId = atoi(SpellNameOrID);

			for (int nSpell = 0; nSpell < NUM_BOOK_SLOTS; nSpell++)
			{
				if (pLocalPC->GetSpellBook(nSpell) == spellId)
				{
					return GetSpellByID(spellId);
				}
			}
		}
		else
		{
			for (int nSpell = 0; nSpell < NUM_BOOK_SLOTS; nSpell++)
			{
				if (EQ_Spell* pSpell = GetSpellByID(pLocalPC->GetSpellBook(nSpell)))
				{
					if (ci_equals(SpellNameOrID, pSpell->Name))
					{
						return pSpell;
					}
				}
			}
		}
	}

	return nullptr;
}

ItemClient* FindItemByNameOrID(const char* ItemNameOrID)
{
	if (IsNumber(ItemNameOrID))
	{
		return FindItemByID(atoi(ItemNameOrID));
	}
	
	return FindItemByName(ItemNameOrID, true);
}

bool LocateSpellToCast(const char* szSpellorAltorItemName, const char* szTYPE)
{
	if (szSpellorAltorItemName[0])
	{
		// is it an alt ability?
		if (!szTYPE[0] || !_strnicmp(szTYPE, "alt", 3))
		{
			if (CAltAbilityData* Search = AltAbility(szSpellorAltorItemName))
			{
				if (EQ_Spell* spell = GetSpellByID(Search->SpellID))
				{
					fFIND = spell;
					fINFO = Search;
					fTIME = fFIND->CastTime;
					fNAME = fFIND->Name;
					fTYPE = TYPE_ALT;
					return true;
				}
			}
		}

		// nope wasn't an altability, so is it a spell?
		if (!szTYPE[0] || !_strnicmp(szTYPE, "gem", 3) || IsNumber(szTYPE))
		{
			if (EQ_Spell* Search = GetSpellFromSpellBook(szSpellorAltorItemName))
			{
				ItemPtr pFocusItem;

				fFIND = Search;
				fINFO = Search;
				fTIME = GetCastingTimeModifier(fFIND) + GetFocusCastingTimeModifier(fFIND, pFocusItem, false) + fFIND->CastTime;
				fNAME = fFIND->Name;
				fTYPE = TYPE_SPELL;
				return true;
			}
		}

		// not a spell, is it a clicky then?
		if (ItemClient* pItem = FindItemByNameOrID(szSpellorAltorItemName))
		{
			ItemDefinition* itemDef = pItem->GetItemDefinition();

			if (GetSpellByID(itemDef->Clicky.SpellID))
			{
				fFIND = GetSpellByID(itemDef->Clicky.SpellID);
				fINFO = pItem;
				fTIME = GetItemFromContents(pItem)->Clicky.CastTime;
				fNAME = itemDef->Name;
				fTYPE = TYPE_ITEM;
				return true;
			}
		}
	}

	fFIND = nullptr;
	fINFO = nullptr;
	fTYPE = TYPE_NONE;
	return false;
}

int GetSpellTimer(EQ_Spell* pSpell)
{
	if (IsGemReady(GetGemID(pSpell->ID)))
	{
		return 0;
	}

	if (IsBardClass())
	{
		return 2;
	}

	if (pLocalPC->GetLevel() < 4)
	{
		return pSpell->RecoveryTime * 2;
	}

	return pSpell->RecoveryTime;
}

int GetAltAbilityTimer(CAltAbilityData* pAltAbility)
{
	if (pAltAdvManager->GetCalculatedTimer(pLocalPC, pAltAbility) > 0)
	{
		int ready = 0;
		pAltAdvManager->IsAbilityReady(pLocalPC, pAltAbility, &ready);

		return (ready < 1) ? 0 : ready * 1000;
	}

	return 999999;
}

int GetTimerFromSpellFindResults(eCastingType Type, void* Data)
{
	switch (Type)
	{
	case TYPE_SPELL:
		return GetSpellTimer(static_cast<EQ_Spell*>(Data));

	case TYPE_ALT:
		return GetAltAbilityTimer(static_cast<CAltAbilityData*>(Data));

	case TYPE_ITEM:
		return GetItemTimer(static_cast<ItemClient*>(Data)) * 1000;

	default: break;
	}

	return 999999;
}

bool SpellReady(const char* szSpellName)
{
	if (szSpellName[0] == 0)
	{
		return true;
	}

	if (IsNumber(szSpellName))
	{
		int number = atoi(szSpellName) - 1;
		if (number < NUM_SPELL_GEMS)
		{
			return IsGemReady(number);
		}
	}

	if (szSpellName[0] == 'M' && strlen(szSpellName) == 1)
	{
		if (FindMQ2DataType("Twist"))
			Twisting = Evaluate("${If[${Twist.Twisting},1,0]}") != 0;

		return !Twisting;
	}

	char zName[MAX_STRING];
	GetArg(zName, szSpellName, 1, false, false, false, '|');

	char zType[MAX_STRING];
	GetArg(zType, szSpellName, 2, false, false, false, '|');

	if (LocateSpellToCast(zName, zType))
	{
		if (!GetTimerFromSpellFindResults(fTYPE, fINFO))
		{
			return true;
		}
	}

	return false;
}

void Stick(const char* argument)
{
	using StickCALL = void(*)(PlayerClient*, const char*);

	MQPlugin* plugin = GetPlugin("MQ2MoveUtils");
	if (plugin && plugin->fpVersion > 0.9999 && plugin->RemoveSpawn)
	{
		if (StickCALL Request = reinterpret_cast<StickCALL>(GetProcAddress(plugin->hModule, "StickCommand")))
		{
			Request(pLocalPlayer, argument);
		}
	}
}

void FollowPath(const char* argument)
{
	using FollowCALL = void(*)(PlayerClient*, const char*);

	MQPlugin* plugin = GetPlugin("MQ2AdvPath");
	if (plugin && plugin->fpVersion > 0.999 && plugin->RemoveSpawn)
	{
		if (FollowCALL Request = reinterpret_cast<FollowCALL>(GetProcAddress(plugin->hModule, "MQFollowCommand")))
		{
			Request(pLocalPlayer, argument);
		}
	}
}
void Path(const char* argument)
{
	using FollowCALL = void(*)(PlayerClient*, const char*);

	MQPlugin* plugin = GetPlugin("MQ2AdvPath");
	if (plugin && plugin->fpVersion > 0.999 && plugin->RemoveSpawn)
	{
		if (FollowCALL Request = reinterpret_cast<FollowCALL>(GetProcAddress(plugin->hModule, "MQPlayCommand")))
		{
			Request(pLocalPlayer, argument);
		}
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

class MQ2CastType : public MQ2Type
{
public:

	enum class CastMembers
	{
		Active,
		Effect,
		Stored,
		Result,
		Return,
		Status,
		Timing,
		Taken,
		Ready,
	};

	MQ2CastType() : MQ2Type("Cast")
	{
		ScopedTypeMember(CastMembers, Active);
		ScopedTypeMember(CastMembers, Effect);
		ScopedTypeMember(CastMembers, Stored);
		ScopedTypeMember(CastMembers, Result);
		ScopedTypeMember(CastMembers, Return);
		ScopedTypeMember(CastMembers, Status);
		ScopedTypeMember(CastMembers, Timing);
		ScopedTypeMember(CastMembers, Taken);
		ScopedTypeMember(CastMembers, Ready);
	}

	bool GetMember(MQVarPtr VarPtr, const char* Member, char* Index, MQTypeVar &Dest) override
	{
		MQTypeMember* pMember = MQ2CastType::FindMember(Member);

		if (!pMember || !pLocalPlayer)
		{
			return false;
		}

		switch (static_cast<CastMembers>(pMember->ID))
		{
		case CastMembers::Active:
			Dest.Set(gbInZone);
			Dest.Type = mq::datatypes::pBoolType;
			return true;

		case CastMembers::Effect: {
			int spellID = pLocalPlayer->CastingData.SpellID;
			if (spellID == -1 && CastF != FLAG_COMPLETE)
			{
				spellID = CastS->ID;
			}

			Dest.Ptr = GetSpellByID(spellID);
			Dest.Type = mq::datatypes::pSpellType;
			return true;
		}

		case CastMembers::Stored:
			Dest.Ptr = GetSpellByID(CastingL);
			Dest.Type = mq::datatypes::pSpellType;
			return true;

		case CastMembers::Timing:
			Dest.Int = CastingLeft();
			Dest.Type = mq::datatypes::pIntType;
			return true;

		case CastMembers::Status:
			DataTypeTemp[0] = '\0';
			if (CastingC != NOID || CastF != FLAG_COMPLETE || (pCastingWnd && pCastingWnd->IsVisible()))
			{
				strcat_s(DataTypeTemp, "C");
			}

			if (StopF != FLAG_COMPLETE) strcat_s(DataTypeTemp, "S");
			if (MoveA != FLAG_COMPLETE) strcat_s(DataTypeTemp, "A");
			if (MoveN != FLAG_COMPLETE) strcat_s(DataTypeTemp, "N");
			if (MoveS != FLAG_COMPLETE) strcat_s(DataTypeTemp, "F");
			if (MoveF != FLAG_COMPLETE) strcat_s(DataTypeTemp, "P");
			if (MoveP != FLAG_COMPLETE) strcat_s(DataTypeTemp, "P");
			if (MemoF != FLAG_COMPLETE) strcat_s(DataTypeTemp, "M");
			if (DuckF != FLAG_COMPLETE) strcat_s(DataTypeTemp, "D");
			if (ItemF != FLAG_COMPLETE) strcat_s(DataTypeTemp, "E");
			if (!DataTypeTemp[0])
			{
				strcat_s(DataTypeTemp, "I");
			}

			Dest.Ptr = &DataTypeTemp[0];
			Dest.Type = mq::datatypes::pStringType;
			return true;

		case CastMembers::Result:
		case CastMembers::Return:
			switch (static_cast<CastMembers>(pMember->ID) == CastMembers::Result ? CastingX : Resultat)
			{
			case DONE_PROGRESS:
			case CAST_SUCCESS:      strcpy_s(DataTypeTemp, "CAST_SUCCESS");     break;
			case CAST_INTERRUPTED:  strcpy_s(DataTypeTemp, "CAST_INTERRUPTED"); break;
			case CAST_RESIST:       strcpy_s(DataTypeTemp, "CAST_RESIST");      break;
			case CAST_COLLAPSE:     strcpy_s(DataTypeTemp, "CAST_COLLAPSE");    break;
			case CAST_RECOVER:      strcpy_s(DataTypeTemp, "CAST_RECOVER");     break;
			case CAST_FIZZLE:       strcpy_s(DataTypeTemp, "CAST_FIZZLE");      break;
			case CAST_STANDING:     strcpy_s(DataTypeTemp, "CAST_STANDING");    break;
			case CAST_STUNNED:      strcpy_s(DataTypeTemp, "CAST_STUNNED");     break;
			case CAST_INVISIBLE:    strcpy_s(DataTypeTemp, "CAST_INVISIBLE");   break;
			case CAST_NOTREADY:     strcpy_s(DataTypeTemp, "CAST_NOTREADY");    break;
			case CAST_OUTOFMANA:    strcpy_s(DataTypeTemp, "CAST_OUTOFMANA");   break;
			case CAST_OUTOFRANGE:   strcpy_s(DataTypeTemp, "CAST_OUTOFRANGE");  break;
			case CAST_NOTARGET:     strcpy_s(DataTypeTemp, "CAST_NOTARGET");    break;
			case CAST_CANNOTSEE:    strcpy_s(DataTypeTemp, "CAST_CANNOTSEE");   break;
			case CAST_COMPONENTS:   strcpy_s(DataTypeTemp, "CAST_COMPONENTS");  break;
			case CAST_OUTDOORS:     strcpy_s(DataTypeTemp, "CAST_OUTDOORS");    break;
			case CAST_TAKEHOLD:     strcpy_s(DataTypeTemp, "CAST_TAKEHOLD");    break;
			case CAST_IMMUNE:       strcpy_s(DataTypeTemp, "CAST_IMMUNE");      break;
			case CAST_DISTRACTED:   strcpy_s(DataTypeTemp, "CAST_DISTRACTED");  break;
			case CAST_ABORTED:      strcpy_s(DataTypeTemp, "CAST_CANCELLED");   break;
			case CAST_UNKNOWN:      strcpy_s(DataTypeTemp, "CAST_UNKNOWN");     break;
			default:                strcpy_s(DataTypeTemp, "CAST_NEEDFIXTYPE"); break;
			}
			Dest.Ptr = &DataTypeTemp[0];
			Dest.Type = mq::datatypes::pStringType;
			return true;

		case CastMembers::Ready:
			Dest.Set(gbInZone
				&& !Flags()
				&& !IsPaused()
				&& (pSpellBookWnd && !pSpellBookWnd->IsVisible())
				&& !pLocalPC->Stunned
				&& SpellReady(Index)
			);
			Dest.Type = mq::datatypes::pBoolType;
			return true;

		case CastMembers::Taken:
			Dest.Set(CastingX == CAST_TAKEHOLD);
			Dest.Type = mq::datatypes::pBoolType;
			return true;
		}

		return false;
	}

	bool ToString(MQVarPtr VarPtr, char* Destination) override
	{
		strcpy_s(Destination, MAX_STRING, "TRUE");
		return true;
	}
};
MQ2CastType* pCastType = nullptr;

bool dataCast(const char* szName, MQTypeVar& Dest)
{
	Dest.DWord = 1;
	Dest.Type = pCastType;
	return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

void StopEnding()
{
	if (MoveS != FLAG_COMPLETE)
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Immobilize]: Stick UnPause Request.", GetTickCount64());
		}

		Stick("unpause");
		MoveS = FLAG_COMPLETE;
	}

	if (MoveA != FLAG_COMPLETE)
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Immobilize]: AdvPath UnPause Request.", GetTickCount64());
		}

		Execute("/varcalc PauseFlag 0");
		MoveA = FLAG_COMPLETE;
	}

	if (MoveN != FLAG_COMPLETE)
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Immobilize]: MQ2Nav UnPause Request.", GetTickCount64());
		}

		Execute("/nav pause");
		MoveN = FLAG_COMPLETE;
	}

	if (MoveF != FLAG_COMPLETE)
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Immobilize]: MQ2AdvPath UnPause Request.", GetTickCount64());
		}

		FollowPath("unpause");
		MoveF = FLAG_COMPLETE;
	}

	if (MoveP != FLAG_COMPLETE)
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Immobilize]: MQ2AdvPath UnPause Request.", GetTickCount64());
		}

		Path("unpause");
		MoveP = FLAG_COMPLETE;
	}
}

void HandleStopMovement()
{
	if (StopF == FLAG_REQUEST)
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Immobilize]: Request.", GetTickCount64());
		}

		StopM = GetTickCount64() + DELAY_STOP;
		StopF = FLAG_PROGRESS1;
		StopE = DONE_PROGRESS;
	}

	if (Evaluate("${If[${Stick.Status.Equal[ON]},1,0]}"))
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Immobilize]: Stick Pause Request.", GetTickCount64());
		}

		Stick("pause");
		MoveS = FLAG_PROGRESS1;
	}

	if (FindMQ2DataVariable("FollowFlag"))
	{
		// looks like AdvPath.inc is loaded...
		if (Evaluate("${If[${Bool[${FollowFlag}]},1,0]}"))
		{
			if (DEBUGGING)
			{
				WriteChatf("[%I64u] MQ2Cast:[Immobilize]: AdvPath Pause Request.", GetTickCount64());
			}

			Execute("/varcalc PauseFlag 1");
			MoveA = FLAG_PROGRESS1;
		}
	}

	if (MQPlugin* pNavPlugin = GetPlugin("MQ2Nav"))
	{
		using IsNavFunc = bool(*)();

		IsNavFunc isNavPathActive = reinterpret_cast<IsNavFunc>(GetProcAddress(pNavPlugin->hModule, "IsNavPathActive"));
		if (isNavPathActive && isNavPathActive())
		{
			IsNavFunc isNavPathPaused = reinterpret_cast<IsNavFunc>(GetProcAddress(pNavPlugin->hModule, "IsNavPathPaused"));
			if (isNavPathPaused && !isNavPathPaused)
			{
				if (DEBUGGING)
				{
					WriteChatf("[%I64u] MQ2Cast:[Immobilize]: MQ2Nav Pause Request.", GetTickCount64());
				}

				Execute("/nav pause");
				MoveN = FLAG_PROGRESS1;
			}
		}
	}

	if (FindMQ2DataType("AdvPath"))
	{
		if (Evaluate("${If[${AdvPath.Following} && !${AdvPath.Paused},1,0]}"))
		{
			if (DEBUGGING)
			{
				WriteChatf("[%I64u] MQ2Cast:[Immobilize]: MQ2AdvPath Pause Request.", GetTickCount64());
			}

			FollowPath("pause");
			MoveF = FLAG_PROGRESS1;
		}

		if (Evaluate("${If[${AdvPath.Playing} && !${AdvPath.Paused},1,0]}"))
		{
			if (DEBUGGING)
			{
				WriteChatf("[%I64u] MQ2Cast:[Immobilize]: MQ2AdvPath Pause Request.", GetTickCount64());
			}

			Path("pause");
			MoveP = FLAG_PROGRESS1;
		}
	}

	Immobile = IsImmobileCheck();
	if (Immobile)
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Immobilize]: Complete.", GetTickCount64());
		}

		StopF = FLAG_COMPLETE;
		StopE = DONE_SUCCESS;
	}

	if (GetTickCount64() > StopM)
	{
		WriteChatf("[%I64u] MQ2Cast:[Immobilize]: Aborting!", GetTickCount64());
		StopF = FLAG_COMPLETE;
		StopE = DONE_ABORTED;
		return;
	}

	if (StopF == FLAG_PROGRESS1)
	{
		StopF = FLAG_PROGRESS2;

		if (Speed() != 0.0f)
		{
			ExecuteCmd(FindMappableCommand("back"), true, nullptr);
			ExecuteCmd(FindMappableCommand("back"), false, nullptr);
		}
	}
}

void HandleSpellMemorization()
{
	if (!pSpellBookWnd)
	{
		MemoE = DONE_ABORTED;
	}
	else
	{
		bool Complete = true;

		for (int sp = 0; sp<NUM_SPELL_GEMS; sp++)
		{
			if (SpellToMemorize.SpellId[sp] != -1
				&& SpellToMemorize.SpellId[sp] != pLocalPC->GetMemorizedSpell(sp))
			{
				Complete = false;
				break;
			}
		}

		if (!Complete)
		{
			if (MemoF == FLAG_REQUEST)
			{
				if (DEBUGGING)
				{
					WriteChatf("[%I64u] MQ2Cast:[Memorize]: Immobilize.", GetTickCount64());
				}

				MemoF = FLAG_PROGRESS1;
				MemoE = DONE_PROGRESS;

				if (pLocalPC->GetLevel() < 4 && DELAY_MEMO < 15000)
				{
					DELAY_MEMO = 15000;
				}

				MemoM = GetTickCount64() + DELAY_STOP + DELAY_MEMO * SpellTotal;

				if (StopF == FLAG_COMPLETE)
				{
					StopE = DONE_SUCCESS;
				}

				if (StopF == FLAG_COMPLETE)
				{
					StopF = FLAG_REQUEST;
				}

				if (StopF != FLAG_COMPLETE)
				{
					HandleStopMovement();
				}
			}

			if (MemoF == FLAG_PROGRESS1 && StopE == DONE_SUCCESS)
			{
				if (DEBUGGING)
				{
					WriteChatf("[%I64u] MQ2Cast:[Memorize]: Spell(s).", GetTickCount64());
				}

				MemoF = FLAG_PROGRESS2;
				pSpellBookWnd->MemorizeSet(SpellToMemorize.SpellId, NUM_SPELL_GEMS);
			}

			if (StopE == DONE_ABORTED || GetTickCount64()>MemoM)
			{
				MemoE = DONE_ABORTED;
			}
		}
		else
		{
			if (DEBUGGING)
			{
				WriteChatf("[%I64u] MQ2Cast:[Memorize]: Complete.", GetTickCount64());
			}

			MemoF = FLAG_COMPLETE;
			MemoE = DONE_SUCCESS;
		}
	}

	if (MemoE == DONE_ABORTED || !pSpellBookWnd)
	{
		WriteChatf("[%I64u] MQ2Cast:[Memorize]: Aborting!", GetTickCount64());
		MemoF = FLAG_COMPLETE;
	}

	if (MemoF == FLAG_COMPLETE && (pSpellBookWnd && pSpellBookWnd->IsVisible()))
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Memorize]: Closebook.", GetTickCount64());
		}

		Execute("/book");
	}
}

void HandleSpellInterrupt(int flag)
{
	if (DEBUGGING)
	{
		WriteChatf("[%I64u] MQ2Cast:[Duck]: StopCast.", GetTickCount64());
	}

	Execute("/stopcast");
	CastingE = CAST_ABORTED;
	DuckF = FLAG_COMPLETE;
	CastR = 0;
}

void HandleSpellCast()
{
	// we got the casting request cookies, request immobilize/memorize if needed.
	if (CastF == FLAG_REQUEST)
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Casting]: Request.", GetTickCount64());
		}

		CastF = FLAG_PROGRESS1;

		if (StopF == FLAG_COMPLETE)
		{
			StopF = DONE_SUCCESS;
		}

		if (StopF == FLAG_COMPLETE && CastT>100 && !IsBardClass())
		{
			StopF = FLAG_REQUEST;
		}

		if (MemoF != FLAG_COMPLETE)
		{
			HandleSpellMemorization();
		}
		else
		{
			if (StopF != FLAG_COMPLETE)
			{
				HandleStopMovement();
			}
		}
	}

	// waiting on the casting results to take actions.
	if (CastF == FLAG_PROGRESS3 && CastingE != DONE_PROGRESS)
	{
		CastF = FLAG_PROGRESS4;
		if (CastR)
		{
			CastR--;
		}

		if (CastR)
		{
			if ((CastingE == CAST_SUCCESS && CastW != RECAST_LAND)
				|| (CastingE == CAST_COLLAPSE) || (CastingE == CAST_FIZZLE)
				|| (CastingE == CAST_INTERRUPTED) || (CastingE == CAST_RECOVER) || (CastingE == CAST_RESIST))
			{
				if (DEBUGGING)
				{
					WriteChatf("[%I64u] MQ2Cast:[Casting]: AutoRecast [%d].", GetTickCount64(), CastingE);
				}

				if (CastW != RECAST_ZERO && !TargC)
				{
					TargC = pTarget ? pTarget->SpawnID : 0;
				}

				CastM = GetTickCount64() + DELAY_CAST;
				CastF = FLAG_REQUEST;
			}
		}
	}

	// casting is over, grab latest casting results and exit.
	if (CastF == FLAG_PROGRESS4)
	{
		if (CastE>CastingE)
		{
			CastingE = CastE;
		}

		CastF = FLAG_COMPLETE;
	}

	// evaluate if we are taking too long, or immobilize/memorize event failed.
	if (CastF != FLAG_COMPLETE)
	{
		if (StopE == DONE_ABORTED || MemoE == DONE_ABORTED || GetTickCount64() > CastM)
		{
			WriteChatf("[%I64u] MQ2Cast:[Casting]: Aborting! (%s)", GetTickCount64(),
				StopE == DONE_ABORTED ? "StopE" : (MemoE == DONE_ABORTED ? "MemoE" : "CastM"));
			CastF = FLAG_PROGRESS4;
			CastE = CAST_NOTREADY;
		}
	}

	// waiting for opportunity to start casting, end if conditions not favorables.
	if (CastF == FLAG_PROGRESS1)
	{
		if (pCastingWnd && pCastingWnd->IsVisible())
		{
			return; // casting going on
		}

		CastingC = CastS->ID;
		CastF = FLAG_PROGRESS4;

		if (TargC && (!pTarget || (pTarget && pTarget->SpawnID != TargC)))
		{
			if (CastW == RECAST_DEAD)
			{
				CastE = CAST_NOTARGET;
			}
			else if (CastW == RECAST_LAND)
			{
				CastE = CAST_ABORTED;
			}
		}
		else
		{
			if (Invisible && pLocalPlayer->HideMode)
			{
				CastE = CAST_INVISIBLE;
			}
			else if (pLocalPC->Stunned)
			{
				CastE = CAST_STUNNED;
			}
			else if (StopF != FLAG_COMPLETE || MemoF != FLAG_COMPLETE)
			{
				CastF = FLAG_PROGRESS1;
			}
			else
			{
				int TimeReady = GetTimerFromSpellFindResults(CastK, CastI);  // get estimate time before it's ready.
				if (TimeReady > 3000)
				{
					CastE = CAST_NOTREADY;   // if estimate higher then 3 seconds, abort.
				}
				else if (!TimeReady)
				{
					CastF = FLAG_PROGRESS2;  // estimate says it's ready, so cast it
				}
				else
				{
					CastF = FLAG_PROGRESS1;  // otherwise give it some time to be ready.
				}
			}
		}
	}

	// we got the final approbation to cast, so lets do it.
	// this is where it breaks
	if (CastF == FLAG_PROGRESS2)
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Casting]: Cast.", GetTickCount64());
		}

		PrepareSuccessEvent(CastS);

		CastF = FLAG_PROGRESS3;
		CastE = DONE_PROGRESS;
		CastingT = GetTickCount64() + CastT + 250 + (pConnection->Last) * 4;
		CastingE = DONE_PROGRESS;
		CastingC = CastS->ID;

		if (pLocalPlayer->CastingData.SpellID > 0)
		{
			CastingX = (CastingE < CAST_SUCCESS) ? CAST_SUCCESS : CastingE;
			CastingL = CastingC;

			if (CastK == TYPE_SPELL)
			{
				Execute("/multiline ; /stopsong ; /cast \"%s\"", CastN);
			}
			else if (CastK == TYPE_ITEM)
			{
				if (!BardBeta)
				{
					Execute("/multiline ; /stopsong ; /useitem \"%s\"", CastN);
				}
				else
				{
					Execute("/useitem \"%s\"", CastN);
				}
			}
			else if (CastK == TYPE_ALT)
			{
				CAltAbilityData* pData = static_cast<CAltAbilityData*>(CastI);
				if (!BardBeta)
				{
					Execute("/multiline ; /stopsong ; /alt activate %d", pData->ID);
				}
				else
				{
					Execute("/alt activate %d", pData->ID);
				}
			}
		}
		else
		{
			if (CastK == TYPE_SPELL)
			{
				Cast("\"%s\"", CastN);
			}
			else if (CastK == TYPE_ITEM)
			{
				if (DEBUGGING)
				{
					WriteChatf("/useitem \"%s\"", CastN);
				}

				Execute("/useitem \"%s\"", CastN);
			}
			else if (CastK == TYPE_ALT)
			{
				CAltAbilityData* pData = static_cast<CAltAbilityData*>(CastI);

				Execute("/alt activate %d", pData->ID);
			}
		}
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

PLUGIN_API void CastDebug(PlayerClient* pChar, const char* Cmd)
{
	char zParm[MAX_STRING];
	GetArg(zParm, Cmd, 1);

	if (zParm[0] == 0)
	{
		DEBUGGING = !DEBUGGING;
	}
	else if (!_strnicmp(zParm, "on", 2))
	{
		DEBUGGING = true;
	}
	else if (!_strnicmp(zParm, "off", 2))
	{
		DEBUGGING = false;
	}
	else
	{
		DEBUGGING = !DEBUGGING;
	}

	WriteChatf("\arMQ2Cast\ax::\amDEBUGGING is now %s\ax.", DEBUGGING ? "\aoON" : "\agOFF");
}

PLUGIN_API void CastCommand(PlayerClient* pChar, const char* Cmd)
{
	Resultat = CAST_DISTRACTED;

	if (!gbInZone || Flags() || IsPaused() || (pSpellBookWnd && pSpellBookWnd->IsVisible()))
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Casting]: Complete. [%d][%s%s%s%s]", GetTickCount64(), Resultat,
				gbInZone ? " ZONE " : "", Flags() ? " FLAGS " : "", IsPaused() ? " PAUSED " : "", (pSpellBookWnd && pSpellBookWnd->IsVisible()) ? " SHOW " : "");
		}
		return;
	}

	Reset();

	char zParm[MAX_STRING];
	int iParm = 0;

	do
	{
		GetArg(zParm, Cmd, ++iParm);

		if (zParm[0] == 0)
		{
			break;
		}

		if (!_strnicmp(zParm, "-targetid|", 10))
		{
			TargI = atoi(&zParm[10]);
		}
		else if (!_strnicmp(zParm, "-kill", 5))
		{
			CastW = RECAST_DEAD;
			CastR = 9999;
		}
		else if (!_strnicmp(zParm, "-maxtries|", 10))
		{
			CastW = RECAST_LAND;
			CastR = atoi(&zParm[10]);
		}
		else if (!_strnicmp(zParm, "-recast|", 8))
		{
			CastW = RECAST_ZERO; CastR = atoi(&zParm[8]);
		}
		else if (!_strnicmp(zParm, "-invis", 6))
		{
			Invisible = true;
		}
		else if (zParm[0] != '-' && CastN[0] == 0)
		{
			GetArg(CastN, zParm, 1, false, false, false, '|');
			GetArg(CastC, zParm, 2, false, false, false, '|');
		}
		else if (zParm[0] != '-' && CastC[0] == 0)
		{
			GetArg(CastC, zParm, 1, false, false, false, '|');
		}
	} while (true);

	Resultat = CAST_SUCCESS;

	if (pLocalPC->Stunned)
	{
		Resultat = CAST_STUNNED;
	}
	else if (Invisible && pLocalPlayer->HideMode)
	{
		Resultat = CAST_INVISIBLE;
	}
	else if (!LocateSpellToCast(CastN, CastC))
	{
		Resultat = CAST_UNKNOWN;
	}
	else if (fTYPE != TYPE_SPELL && GetTimerFromSpellFindResults(fTYPE, fINFO))
	{
		Resultat = CAST_NOTREADY;
	}
	else if (TargI)
	{
		if (PlayerClient* Target = GetSpawnByID(TargI))
		{
			pTarget = Target;
		}
		else
		{
			Resultat = CAST_NOTARGET;
		}
	}

	if (Resultat == CAST_SUCCESS && fTYPE == TYPE_SPELL)
	{
		if (IsBardClass())
		{
			if (Twisting)
			{
				Execute("/stoptwist");
			}
			if (pLocalPlayer->CastingData.SpellID)
			{
				Execute("/stopsong");
			}
		}

		CastG = GetGemID(fFIND->ID);

		if (CastG == NOID)
		{
			CastG = atoi(&CastC[(_strnicmp(CastC, "gem", 3)) ? 0 : 3]) - 1;

			QueueSpellForMemorization(CastG, fFIND);

			SpellTotal = 1;
			MemoF = FLAG_REQUEST;
			MemoE = DONE_SUCCESS;
		}
	}

	if (Resultat != CAST_SUCCESS)
	{
		if (DEBUGGING)
		{
			WriteChatf("[%I64u] MQ2Cast:[Casting]: Complete. [%d]", GetTickCount64(), Resultat);
		}

		return;
	}

	CastF = FLAG_REQUEST;
	CastI = fINFO;
	CastK = fTYPE;
	CastT = fTIME;
	CastS = fFIND;
	CastM = GetTickCount64() + DELAY_CAST;

	strcpy_s(CastN, fNAME);

	if (DEBUGGING)
	{
		WriteChatf("[%I64u] MQ2Cast:[Casting]: Name<%s> Type<%d>.", GetTickCount64(), CastN, CastK);
	}

	HandleSpellCast();
}

PLUGIN_API void DuckCommand(PlayerClient* pChar, const char* Cmd)
{
	if (gbInZone)
	{
		if (CastF != FLAG_COMPLETE)
		{
			CastR = 0;
		}

		if ((pCastingWnd && pCastingWnd->IsVisible()) && CastingLeft() > 500)
		{
			DuckF = FLAG_REQUEST;
			HandleSpellInterrupt(DuckF);
		}
	}

	Resultat = CAST_SUCCESS;
}

PLUGIN_API void MemorizeCommand(PlayerClient* pChar, const char* zLine)
{
	Resultat = CAST_DISTRACTED;

	if (!gbInZone || Flags() || IsPaused() || !pSpellBookWnd)
	{
		return;
	}

	if (pLocalPC->Stunned)
	{
		Resultat = CAST_STUNNED;
		return;
	}

	Reset();

	int iParm = 0;
	char zTemp[MAX_STRING];
	CastingX = CAST_SUCCESS;

	do
	{
		char zParm[MAX_STRING];
		GetArg(zParm, zLine, ++iParm);

		if (!zParm[0])
		{
			break;
		}

		GetArg(zTemp, zParm, 1, false, false, false, '|');

		if (EQ_Spell* Search = GetSpellFromSpellBook(zTemp))
		{
			if (DEBUGGING)
			{
				WriteChatf("[%I64u] MQ2Cast:[Memorize]: Spell Found.", GetTickCount64());
			}

			GetArg(zTemp, zParm, 2, false, false, false, '|');
			int Gem = atoi(&zTemp[(_strnicmp(zTemp, "gem", 3)) ? 0 : 3]) - 1;

			if (Gem < 0 || Gem >= NUM_SPELL_GEMS)
			{
				GetArg(zTemp, zLine, 1 + iParm);
				Gem = atoi(&zTemp[(_strnicmp(zTemp, "gem", 3)) ? 0 : 3]) - 1;
				if (Gem >= 0 && Gem < NUM_SPELL_GEMS)
				{
					iParm++;
				}
			}

			QueueSpellForMemorization(Gem, Search);
		}
		else
		{
			CastingX = CAST_UNKNOWN;

			if (DEBUGGING)
			{
				WriteChatf("[%I64u] MQ2Cast:[Memorize]: Spell Not Found. %d", GetTickCount64(), CastingX);
			}

			return;
		}
	} while (true);

	for (int sp = 0; sp < NUM_SPELL_GEMS; sp++)
	{
		if (SpellToMemorize.SpellId[sp] != -1 && SpellToMemorize.SpellId[sp] != pLocalPC->GetMemorizedSpell(sp))
		{
			SpellTotal++;
		}
	}

	if (SpellTotal)
	{
		MemoF = FLAG_REQUEST;
		MemoE = DONE_SUCCESS;

		HandleSpellMemorization();
	}
}

PLUGIN_API void SpellSetDelete(PlayerClient* pChar, const char* Cmd)
{
	Resultat = CAST_ABORTED;

	if (!gbInZone)
	{
		return;
	}
	else if (!Cmd[0])
	{
		MacroError("Usage: /ssd setname");
	}
	else
	{
		Resultat = CAST_SUCCESS;

		sprintf_s(INIFileName, "%s\\%s_%s.ini", gPathConfig, GetServerShortName(), pLocalPC->Name);
		WritePrivateProfileString("MQ2Cast(SpellSet)", Cmd, nullptr, INIFileName);
	}
}

PLUGIN_API void SpellSetList(PlayerClient* pChar, const char* Cmd)
{
	Resultat = CAST_SUCCESS;
	if (!gbInZone)
		return;

	char Keys[MAX_STRING*NUM_SPELL_GEMS] = { 0 };
	char* pKeys = Keys;
	int Disp = 0;

	WriteChatf("MQ2Cast:: SpellSet [\ay Listing... \ax].");

	sprintf_s(INIFileName, "%s\\%s_%s.ini", gPathConfig, GetServerShortName(), pLocalPC->Name);
	GetPrivateProfileString("MQ2Cast(SpellSet)", nullptr, "", Keys, MAX_STRING * 10, INIFileName);

	while (pKeys[0])
	{
		char Temp[MAX_STRING];
		GetPrivateProfileString("MQ2Cast(SpellSet)", pKeys, "", Temp, MAX_STRING, INIFileName);

		if (Temp[0])
		{
			if (!Disp)
				WriteChatf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");

			WriteChatf("\ay%s\ax", pKeys);
			Disp++;
		}

		pKeys += strlen(pKeys) + 1;
	}

	if (Disp)
		WriteChatf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");

	WriteChatf("MQ2Cast:: SpellSet [\ay %d Displayed\ax ].", Disp);
}

PLUGIN_API void SpellSetMemorize(PlayerClient* pChar, const char* Cmd)
{
	Resultat = CAST_UNKNOWN;

	if (!gbInZone)
	{
		return;
	}
	else if (!Cmd[0])
	{
		MacroError("Usage: /ssm setname");
	}
	else
	{
		Resultat = CAST_SUCCESS;

		char List[MAX_STRING];
		sprintf_s(INIFileName, "%s\\%s_%s.ini", gPathConfig, GetServerShortName(), pLocalPC->Name);
		GetPrivateProfileString("MQ2Cast(SpellSet)", Cmd, "", List, MAX_STRING, INIFileName);

		if (List[0])
		{
			MemorizeCommand(pLocalPlayer, List);
		}
	}
}

PLUGIN_API void SpellSetSave(PlayerClient* pChar, const char* Cmd)
{
	if (!gbInZone)
	{
		return;
	}

	char zSet[MAX_STRING];
	GetArg(zSet, Cmd, 1);

	char zGem[MAX_STRING];
	GetArg(zGem, Cmd, 2);

	Resultat = CAST_ABORTED;

	if (!zSet[0])
	{
		MacroError("Usage: /sss setname <gemlist>");
		return;
	}

	if (!zGem[0])
	{
		sprintf_s(zGem, "123456789ABC");
	}

	char zLst[MAX_STRING] = { 0 };
	char zTmp[MAX_STRING];
	int find = 0;

	for (int g = 0; g < NUM_SPELL_GEMS; g++)
	{
		if (pLocalPC->GetMemorizedSpell(g) > 0)
		{
			if (strstr(zGem, ListGems[g]))
			{
				sprintf_s(zTmp, "%d|%d", GetPcProfile()->MemorizedSpells[g], g + 1);
				if (find)
				{
					strcat_s(zLst, " ");
				}
				strcat_s(zLst, zTmp);
				find++;
			}
		}
	}

	Resultat = CAST_UNKNOWN;

	if (find)
	{
		Resultat = CAST_SUCCESS;
		sprintf_s(INIFileName, "%s\\%s_%s.ini", gPathConfig, GetServerShortName(), pLocalPC->Name);
		WritePrivateProfileString("MQ2Cast(SpellSet)", Cmd, zLst, INIFileName);
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

PLUGIN_API void InitializePlugin()
{
	char BetaSwitch[MAX_STRING] = { 0 };
	if (GetPrivateProfileString("Settings", "Normal", "", BetaSwitch, MAX_STRING, INIFileName))
	{
		BardBeta = false;
	}

	aCastEvent(LIST289, CAST_COLLAPSE, "Your gate is too unstable, and collapses#*#");
	aCastEvent(LIST289, CAST_CANNOTSEE, "You cannot see your target#*#");
	aCastEvent(LIST289, CAST_COMPONENTS, "You are missing some required components#*#");
	aCastEvent(UNKNOWN, CAST_COMPONENTS, "Your ability to use this item has been disabled because you do not have at least a gold membership#*#");
	aCastEvent(LIST289, CAST_COMPONENTS, "You need to play a#*#instrument for this song#*#");
	aCastEvent(LIST289, CAST_DISTRACTED, "You are too distracted to cast a spell now#*#");
	aCastEvent(LIST289, CAST_DISTRACTED, "You can't cast spells while invulnerable#*#");
	aCastEvent(LIST289, CAST_DISTRACTED, "You *CANNOT* cast spells, you have been silenced#*#");
	aCastEvent(LIST289, CAST_IMMUNE, "Your target has no mana to affect#*#");
	aCastEvent(LIST289, CAST_IMMUNE, "Your target looks unaffected#*#");
	aCastEvent(LIST013, CAST_IMMUNE, "Your target is immune to changes in its attack speed#*#");
	aCastEvent(LIST013, CAST_IMMUNE, "Your target is immune to changes in its run speed#*#");
	aCastEvent(LIST013, CAST_IMMUNE, "Your target is immune to snare spells#*#");
	aCastEvent(LIST289, CAST_IMMUNE, "Your target cannot be mesmerized#*#");
	aCastEvent(UNKNOWN, CAST_IMMUNE, "Your target looks unaffected#*#");
	aCastEvent(LIST289, CAST_INTERRUPTED, "Your #*# is interrupted#*#");
	aCastEvent(LIST264, CAST_INTERRUPTED, "Your spell is interrupted#*#");
	aCastEvent(LIST264, CAST_INTERRUPTED, "Your #*# spell is interrupted#*#");
	aCastEvent(UNKNOWN, CAST_INTERRUPTED, "Your casting has been interrupted#*#");
	aCastEvent(LIST289, CAST_FIZZLE, "Your spell fizzles#*#");
	aCastEvent(LIST289, CAST_FIZZLE, "Your #*# spell fizzles#*#");
	aCastEvent(LIST289, CAST_FIZZLE, "You miss a note, bringing your song to a close#*#");
	aCastEvent(LIST289, CAST_NOTARGET, "You must first select a target for this spell#*#");
	aCastEvent(LIST289, CAST_NOTARGET, "This spell only works on#*#");
	aCastEvent(LIST289, CAST_NOTARGET, "You must first target a group member#*#");
	aCastEvent(LIST289, CAST_NOTREADY, "Spell recast time not yet met#*#");
	aCastEvent(LIST289, CAST_OUTOFMANA, "Insufficient Mana to cast this spell#*#");
	aCastEvent(LIST289, CAST_OUTOFRANGE, "Your target is out of range, get closer#*#");
	aCastEvent(LIST289, CAST_OUTDOORS, "This spell does not work here#*#");
	aCastEvent(LIST289, CAST_OUTDOORS, "You can only cast this spell in the outdoors#*#");
	aCastEvent(LIST289, CAST_OUTDOORS, "You can not summon a mount here#*#");
	aCastEvent(LIST289, CAST_OUTDOORS, "You must have both the Horse Models and your current Luclin Character Model enabled to summon a mount#*#");
	aCastEvent(LIST264, CAST_RECOVER, "You haven't recovered yet#*#");
#if defined(UFEMU) || defined(ROF2EMU)
	aCastEvent(LIST289, CAST_RESIST, "Your target resisted the#*#spell#*#");
#else
	aCastEvent(LIST289, CAST_RESIST, "#*# resisted your #*#!");
#endif
	aCastEvent(LIST289, CAST_RECOVER, "Spell recovery time not yet met#*#");
	aCastEvent(LIST289, CAST_RESIST, "Your target resisted the#*#spell#*#");
	aCastEvent(LIST289, CAST_RESIST, "#*#avoided your#*#!#*#");
	aCastEvent(LIST289, CAST_STANDING, "You must be standing to cast a spell#*#");
	aCastEvent(LIST289, CAST_STUNNED, "You can't cast spells while stunned#*#");
	aCastEvent(LIST289, CAST_SUCCESS, "You are already on a mount#*#");
	aCastEvent(LIST289, CAST_TAKEHOLD, "#*#spell did not take hold. (Blocked by#*#");
	aCastEvent(LIST289, CAST_TAKEHOLD, "#*#spell did not take hold on#*#(Blocked by#*#");
	aCastEvent(LIST289, CAST_TAKEHOLD, "Your spell did not take hold#*#");
	aCastEvent(LIST289, CAST_TAKEHOLD, "Your spell would not have taken hold#*#");
	aCastEvent(LIST289, CAST_TAKEHOLD, "Your spell is too powerful for your intended target#*#");
	aCastEvent(LIST289, CAST_TAKEHOLD, "You need to be in a more open area to summon a mount#*#");
	aCastEvent(LIST289, CAST_TAKEHOLD, "You can only summon a mount on dry land#*#");
	aCastEvent(LIST289, CAST_TAKEHOLD, "This pet may not be made invisible#*#");

	pCastType = new MQ2CastType;
	AddMQ2Data("Cast", dataCast);
	AddCommand("/castdebug", CastDebug);
	AddCommand("/casting", CastCommand);
	AddCommand("/interrupt", DuckCommand);
	AddCommand("/memorize", MemorizeCommand);
	AddCommand("/ssd", SpellSetDelete);
	AddCommand("/ssl", SpellSetList);
	AddCommand("/ssm", SpellSetMemorize);
	AddCommand("/sss", SpellSetSave);
}

PLUGIN_API void ShutdownPlugin()
{
	RemoveMQ2Data("Cast");
	delete pCastType;

	RemoveCommand("/castdebug");
	RemoveCommand("/casting");
	RemoveCommand("/interrupt");
	RemoveCommand("/memorize");
	RemoveCommand("/ssd");
	RemoveCommand("/ssl");
	RemoveCommand("/ssm");
	RemoveCommand("/sss");
}

PLUGIN_API void OnEndZone()
{
	Reset();
	CastingO = NOID;
	CastingC = NOID;
	CastingE = CAST_SUCCESS;
	CastingT = 0;
	ImmobileT = 0;
}

PLUGIN_API DWORD OnIncomingChat(const char* Line, int Color)
{
	CHAR szLine[MAX_STRING] = { 0 };
	strcpy_s(szLine, Line);

	if (gbInZone)
	{
		if (strchr(szLine, '\x12'))
		{
			// Message includes link (item tags/spell), must clean first
			StripTextLinks(szLine);
		}

		if (CastingC != NOID && !Twisting)
		{
			Parsed = false;

			if (DEBUGGING)
			{
				WriteChatf("\ar%s::[%I64u] OnIncomingChat:: ChatLine: %s Color: %d", mqplugin::PluginName, GetTickCount64(), szLine, Color);
			}

			if (Color == USERCOLOR_SPELLS)
			{
				LIST264.Feed(szLine);
				SUCCESS.Feed(szLine);
			}
			else if (Color == USERCOLOR_SPELL_FAILURE)
			{
				LIST289.Feed(szLine);
			}
			else if (Color == CONCOLOR_RED)
			{
				LIST013.Feed(szLine);
			}

			if (!Parsed)
			{
				UNKNOWN.Feed(szLine);

				if (Parsed)
				{
					WriteChatf("\arMQ2Cast::Note for Author[\ay%s\ar]=(\ag%d\ar)\ax", szLine, Color);
				}
			}
		}
	}

	return 0;
}

PLUGIN_API void OnPulse()
{
	if (gbInZone && GetTickCount64() > CastingP && pLocalPC && pLocalPlayer)
	{
		CastingP = GetTickCount64() + DELAY_PULSE;

		// evaluate immobile flag and handle immobilize request
		Immobile = IsImmobileCheck();

		if (StopF != FLAG_COMPLETE)
		{
			HandleStopMovement();
		}

		int DetectedSpellCast = pLocalPlayer->CastingData.SpellID;

		// casting window currently openened?
		if (pCastingWnd && pCastingWnd->IsVisible())
		{
			Casting = true;

			if (CastingO == NOID)
			{
				CastingO = pTarget ? pTarget->SpawnID : 0;
			}

			// was this an unexpected cast?
			if (DetectedSpellCast != CastingC && DetectedSpellCast != NOID)
			{
				CastingE = DONE_PROGRESS;
				CastingC = DetectedSpellCast;
				CastingT = pLocalPlayer->CastingData.SpellETA - pLocalPlayer->TimeStamp + GetTickCount64() + 450 + (pConnection->Last) * 4;

				PrepareSuccessEvent(GetSpellByID(DetectedSpellCast));
			}

			// are we attempting to interrupt this?
			if (DuckF != FLAG_COMPLETE)
			{
				HandleSpellInterrupt(DuckF);
			}

			return;
		}

		// wait for incoming chat, timers, and windows to be closed.
		DuckF = FLAG_COMPLETE;

		if (FindMQ2DataType("Twist"))
		{
			Twisting = Evaluate("${If[${Twist.Twisting},1,0]}");
		}

		if (Casting)
		{
			if (CastingC == DetectedSpellCast)
			{
				if (PSPELL Spell = GetSpellByID(CastingC))
				{
					switch (Spell->TargetType)
					{
					case TargetType_TargetDragons: // Uber Dragons
					case TargetType_TargetGiants: // Uber Giants
					case TargetType_TargetPlant: // Plant
					case TargetType_TargetCorpse: // Corpse
					case TargetType_Pet: // Pet
					case TargetType_TargetSummoned: // Summoned
					case TargetType_TargetUndead: // Undead
					case TargetType_TargetAnimal: // Animal
					case TargetType_Single: // Single
						if (!pTarget)
						{
							CastingE = CAST_NOTARGET;
						}
						break;

					default: break;
					}
				}
			}

			// re-evaluate casting timer after cast window close
			CastingT = GetTickCount64() + 450 + (pConnection->Last) * 2;
			Casting = false;
		}

		if (CastingE == DONE_PROGRESS)
		{
			if (GetTickCount64() > CastingT)
			{
				CastingE = CAST_SUCCESS;
			}
			else if (!Twisting)
			{
				return;
			}
		}

		if (IsPaused())
		{
			if (pLocalPlayer->CastingData.SpellID > 0)
			{
				Execute("/stopsong");
			}

			return;
		}

		// give time to proceed other casting events
		if (MemoF != FLAG_COMPLETE)
			HandleSpellMemorization();
		if (MemoF != FLAG_COMPLETE)
			return;
		if (CastF != FLAG_COMPLETE)
			HandleSpellCast();

		// make sure we get final casting results
		if ((CastF == FLAG_COMPLETE && CastingC != NOID && DetectedSpellCast == NOID)
			|| (IsBardClass() && CastingC != NOID && (DetectedSpellCast != NOID)))
		{
			CastingX = (CastingE < CAST_SUCCESS) ? CAST_SUCCESS : CastingE;
			CastingL = CastingC;
			CastingE = DONE_COMPLETE;

			if (!Twisting)
			{
				if (DEBUGGING)
				{
					WriteChatf("[%I64u] MQ2Cast:: Casting Complete ID[%d] Result=[%d]", GetTickCount64(), CastingL, CastingX);
				}
			}

			CastingC = NOID;
			CastingO = NOID;
		}

		// make sure we finish other casting events
		if (CastF == FLAG_COMPLETE)
		{
			StopEnding();

			if (PulseCount)
			{
				auto pCursor = GetPcProfile()->GetInventorySlot(InvSlot_Cursor);

				if (PulseCount > 5 && !pCursor)
				{
					PulseCount = 0;
					return;
				}

				if (pCursor && PulseCount)
				{
					ClickBack();
				}

				if (PulseCount && PulseCount < 7)
				{
					PulseCount++;
				}
			}
		}
	}
}

void WinClick(CXWnd* Wnd, const char* ScreenID, const char* ClickNotification, uint32_t KeyState)
{
	if (Wnd)
	{
		if (CXWnd* Child = Wnd->GetChildItem(ScreenID))
		{
			bool KeyboardFlags[4];
			*reinterpret_cast<uint32_t*>(&KeyboardFlags) = *reinterpret_cast<uint32_t*>(&pWndMgr->KeyboardFlags);
			*reinterpret_cast<uint32_t*>(&pWndMgr->KeyboardFlags) = KeyState;
			SendWndClick2(Child, ClickNotification);
			*reinterpret_cast<uint32_t*>(&pWndMgr->KeyboardFlags) = *reinterpret_cast<uint32_t*>(&KeyboardFlags);
		}
	}
}

void ClickBack()
{
	auto pCursor = GetPcProfile()->GetInventorySlot(InvSlot_Cursor);

	if (!pCursor || (pCastingWnd && pCastingWnd->IsVisible())
		|| (pSpellBookWnd && pSpellBookWnd->IsVisible()))
	{
		return;
	}

	if (pCursor && PulseCount)
	{
		if (pCursor->IsContainer())
		{
			WriteChatf("Pack Type");
			WinClick(pInventoryWnd, "InvSlot30", "leftmouseup", 0);
			PulseCount = 1;
		}
		else
		{
			WriteChatf("Not a pack");
			WinClick(pInventoryWnd, "IW_CharacterView", "leftmouseup", 0);
			PulseCount = 0;
		}
	}
}
