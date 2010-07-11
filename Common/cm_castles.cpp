/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

#include "stdafx.h"

#define DO_INCLUDE_CREAT_DESC
#include "cm_creatures.cpp"

const uint32 CTL_HEROES[CTLT_COUNT] = {
	HTM_KNIGHT,
	HTM_BARBARIAN,
	HTM_WIZARD,
	HTM_WARLOCK,
	HTM_SORCERESS,
	HTM_NECROMANCER
};

const uint8 HERO_ORIENT[CTLO_COUNT] = {
	1, 7
};

const sint32 CTL_INCOME = 500;

// Pricelist for castle constructions
const CTLCNST_DESC_STRUCT CTLCNSTS_DESC[CTLCNST_COUNT] = {
	{
		//
		// Peasants' hut
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	200,	0,		0,		0,		0,		0,		0		},
		{ CTLCNST_TAVERN, CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_CITADEL },
		CREAT_PEASANT, 0
	},{
		//
		// Archers' tower
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	0,		5,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_PEASNHUT,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_CITADEL},
		CREAT_ARCHER, 0
	},{
		//
		// Guardhouse
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1500,	5,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_ARCHTOWER,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_CITADEL},
		CREAT_PIKEMAN, 0
	},{
		//
		// Barracks
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2000,	10,		10,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_GHOUSE,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_CITADEL},
		CREAT_SWORDSMAN, 0
	},{
		//
		// Jousting arena
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	3000,	0,		20,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_BARRACKS,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_CITADEL},
		CREAT_CAVALRY, 0
	},{
		//
		// Cathedral
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	8000,	0,		20,		0,		0,		20,		0		},	
		{ CTLCNST_DWELL_BARRACKS,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_CITADEL},
		CREAT_PALADIN, 0
	},{
		//
		// Goblin barracks
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	300,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_TAVERN,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_STRONGHOLD},
		CREAT_GOBLIN, 0
	},{
		//
		// Orc tower
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	800,	0,		5,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_GBARRACKS,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_STRONGHOLD},
		CREAT_ORC, 0
	},{
		//
		// Wolf pen
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_ORCTOWER,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_STRONGHOLD},
		CREAT_WOLF, 0
	},{
		//
		// Ogre fort
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2000,	10,		10,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_ORCTOWER,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_STRONGHOLD},
		CREAT_OGRE, 0
	},{
		//
		// Troll bridge
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	4000,	20,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_OGREFORT,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_STRONGHOLD},
		CREAT_TROLL, 0
	},{
		//
		// Cyclops cave
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	8000,	20,		0,		0,		0,		20,		0		},	
		{ CTLCNST_DWELL_WOLFDEN,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_STRONGHOLD},
		CREAT_CYCLOP, 0
	},{
		//
		// Halflings' hit
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	400,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_TAVERN,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_TOWER},
		CREAT_HALFLING, 0
	},{
		//
		// Boars' pen
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	800,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_HLFHUT,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_TOWER},
		CREAT_BOAR, 0
	},{
		//
		// Golem factory
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1500,	5,		5,		5,		0,		0,		0		},	
		{ CTLCNST_DWELL_HLFHUT,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_TOWER},
		CREAT_GOLEM, 0
	},{
		//
		// Cliff nest
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	3000,	0,		5,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_BOARPEN,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_TOWER},
		CREAT_ROC, 0
	},{
		//
		// Mage tower
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	3500,	5,		5,		5,		5,		5,		5		},	
		{ CTLCNST_DWELL_GFACTORY,CTLCNST_MAGEGUILD_L1 },
		{ CTLSM_SML, CTLTM_TOWER},
		CREAT_MAGE, 0
	},{
		//
		// Cloud castle
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	25000,	5,		5,		0,		30,		0,		0		},	
		{ CTLCNST_DWELL_CLIFFNEST,CTLCNST_DWELL_MAGETOWER },
		{ CTLSM_ML, CTLTM_TOWER},
		CREAT_TITAN, 0
	},{
		//
		// Centaurs' cave
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	500,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_TAVERN,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_DUNGEON},
		CREAT_CENTAUR, 0
	},{
		//
		// Crypt
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	10,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_CENTCAVE,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_DUNGEON},
		CREAT_GARGOYLE, 0
	},{
		//
		// Griffin tower
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2000,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_CRYPT,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_DUNGEON},
		CREAT_GRIFFIN, 0
	},{
		//
		// Labyrinth
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	3000,	0,		0,		0,		10,		0,		0		},	
		{ CTLCNST_DWELL_GRIFTOWER,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_DUNGEON},
		CREAT_MINOTAUR, 0
	},{
		//
		// Hydra pond
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	4000,	0,		0,		0,		0,		0,		10		},	
		{ CTLCNST_DWELL_LABYRINTH,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_DUNGEON},
		CREAT_HYDRA, 0
	},{
		//
		// Dragon cave
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	30000,	30,		0,		0,		0,		0,		30		},	
		{ CTLCNST_DWELL_HYDRAPOND,CTLCNST_MAGEGUILD_L2 },
		{ CTLSM_ML, CTLTM_DUNGEON},
		CREAT_BLACK_DRAGON, 0
	},{
		//
		// Treehouse
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	500,	0,		5,		0,		0,		0,		0		},	
		{ CTLCNST_TAVERN,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_FORTRESS},
		CREAT_SPRITE, 0
	},{
		//
		// Dwarf cottage
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1500,	0,		5,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_TREEHOUSE,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_FORTRESS},
		CREAT_DWARF, 0
	},{
		//
		// Homestead
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2000,	0,		10,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_DWCOTTAGE,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_FORTRESS},
		CREAT_ELF, 0
	},{
		//
		// Stonehenge
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2500,	10,		0,		5,		0,		0,		0		},	
		{ CTLCNST_DWELL_HOMESTEAD,CTLCNST_MAGEGUILD_L1 },
		{ CTLSM_SML, CTLTM_FORTRESS},
		CREAT_DRUID, 0
	},{
		//
		// Unicorn glade
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	3000,	0,		10,		0,		10,		0,		0		},	
		{ CTLCNST_DWELL_STONEHENGE,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_FORTRESS},
		CREAT_UNICORN, 0
	},{
		//
		// Red tower
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	10000,	30,		0,		20,		0,		0,		0		},	
		{ CTLCNST_DWELL_UNGLADE,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_FORTRESS},
		CREAT_PHOENIX, 0
	},{
		//
		// Excavation
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	400,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_TAVERN,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_NECROPOLIS},
		CREAT_SKELETON, 0
	},{
		//
		// Graveyard
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1500,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_EXCAVATION,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_NECROPOLIS},
		CREAT_ZOMBIE, 0
	},{
		//
		// Pyramid
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2000,	10,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_GRAVEYARD,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_NECROPOLIS},
		CREAT_MUMMY, 0
	},{
		//
		// Mansion
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	3000,	0,		10,		0,		5,		0,		0		},	
		{ CTLCNST_DWELL_PYRAMID,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_NECROPOLIS},
		CREAT_VAMPIRE, 0
	},{
		//
		// Mausoleum
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	4000,	0,		10,		0,		0,		0,		5		},	
		{ CTLCNST_DWELL_MANSION,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_NECROPOLIS},
		CREAT_LICH, 0
	},{
		//
		// Laboratory
		//
		CCT_DWELLING,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	10000,	10,		10,		20,		0,		0,		0		},	
		{ CTLCNST_DWELL_MAUSOLEUM,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_NECROPOLIS},
		CREAT_BONE_DRAGON, 0
	},{
		//
		// Mage guild 1
		//
		CCT_MAGEGUILD,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2000,	5,		5,		0,		0,		0,		0		},	
		{ CTLCNST_INVALID,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_ALL},
		1, 0
	},{
		//
		// Mage guild 2
		//
		CCT_MAGEGUILD,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	5,		5,		4,		4,		4,		4		},	
		{ CTLCNST_MAGEGUILD_L1,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_ALL},
		2, 0
	},{
		//
		// Mage guild 3
		//
		CCT_MAGEGUILD,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	5,		5,		6,		6,		6,		6		},	
		{ CTLCNST_MAGEGUILD_L2, CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_ALL},
		3, 0
	},{
		//
		// Mage guild 4
		//
		CCT_MAGEGUILD,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	5,		5,		8,		8,		8,		8		},	
		{ CTLCNST_MAGEGUILD_L3, CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_MAGIC | CTLTM_MISC},
		4, 0
	},{
		//
		// Mage guild 5
		//
		CCT_MAGEGUILD,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	5,		5,		10,		10,		10,		10		},	
		{ CTLCNST_MAGEGUILD_L4, CTLCNST_INVALID },
		{ CTLSM_L, CTLTM_MAGIC | CTLTM_MISC},
		5, 0
	},{
		//
		// Magic node
		//
		CCT_MAGICNODE,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	3000,	10,		0,		2,		2,		2,		2		},	
		{ CTLCNST_MAGEGUILD_L2,CTLCNST_INVALID },
		{ CTLSM_L, CTLTM_MAGIC},
		0, 0
	},{
		//
		// Tavern
		//
		CCT_TAVERN,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	0,		5,		0,		0,		0,		0		},	
		{ CTLCNST_INVALID,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_ALL},
		0, 0
	},{
		//
		// Marketplace
		//
		CCT_GENERIC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	500,	0,		5,		0,		0,		0,		0		},	
		{ CTLCNST_TAVERN,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_ALL},
		0, 0
	},{
		//
		// Town hall
		//
		CCT_MINE,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2500,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_TAVERN,CTLCNST_INVALID },
		{ CTLSM_M, CTLTM_ALL},
		MINERAL_GOLD, 500
	},{
		//
		// City hall
		//
		CCT_MINE,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	5000,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_MAGEGUILD_L1,CTLCNST_MARKETPLACE },
		{ CTLSM_L, CTLTM_ALL},
		MINERAL_GOLD, 1500
	},{
		//
		// Ore mine
		//
		CCT_MINE,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	3000,	5,		0,		0,		0,		0,		0		},	
		{ CTLCNST_MARKETPLACE,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_MIGHT | CTLTM_MISC},
		MINERAL_ORE, 1
	},{
		//
		// Sawmill
		//
		CCT_MINE,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	3000,	0,		5,		0,		0,		0,		0		},	
		{ CTLCNST_MARKETPLACE,CTLCNST_INVALID },
		{ CTLSM_SML, CTLTM_MIGHT | CTLTM_MISC},
		MINERAL_WOOD, 1
	},{
		//
		// Alchemists' laboratory
		//
		CCT_MINE,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	5000,	5,		5,		0,		0,		0,		0		},	
		{ CTLCNST_MARKETPLACE,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_MISC},
		MINERAL_MERCURY, 1
	},{
		//
		// Gems mine
		//
		CCT_MINE,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	5000,	5,		5,		0,		0,		0,		0		},	
		{ CTLCNST_MARKETPLACE,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_TOWER},
		MINERAL_GEMS, 1
	},{
		//
		// Crystal mine
		//
		CCT_MINE,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	5000,	5,		5,		0,		0,		0,		0		},	
		{ CTLCNST_MARKETPLACE,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_MIGHT},
		MINERAL_CRYSTAL, 1
	},{
		//
		// Sulfur mine
		//
		CCT_MINE,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	5000,	5,		5,		0,		0,		0,		0		},	
		{ CTLCNST_MARKETPLACE,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_DUNGEON},
		MINERAL_SULFUR, 1
	},{
		//
		// Moat
		//
		CCT_GENERIC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	5,		5,		0,		0,		0,		0		},	
		{ CTLCNST_INVALID,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_ALL },
		0, 0
	},{
		//
		// Mine turret
		//
		CCT_GENERIC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2500,	5,		0,		0,		0,		0,		0		},	
		{ CTLCNST_INVALID,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_ALL },
		0, 0
	},{
		//
		// Left turret
		//
		CCT_GENERIC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1500,	5,		0,		0,		0,		0,		0		},	
		{ CTLCNST_MTURRET,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_MIGHT },
		0, 0
	},{
		//
		// Right turret
		//
		CCT_GENERIC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1500,	5,		0,		0,		0,		0,		0		},	
		{ CTLCNST_MTURRET,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_MIGHT },
		0, 0
	},{
		//
		// Shooting range
		//
		CCT_DWELL_ENC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	0,		5,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_ARCHTOWER,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_CITADEL },
		CTLCNST_DWELL_ARCHTOWER, CREAT_DESC[CREAT_ARCHER].growth
	},{
		//
		// Mess hall
		//
		CCT_DWELL_ENC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	5,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_ORCTOWER,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_STRONGHOLD },
		CTLCNST_DWELL_ORCTOWER, CREAT_DESC[CREAT_ORC].growth
	},{
		//
		// Oak wood
		//
		CCT_DWELL_ENC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1500,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_BOARPEN,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_TOWER },
		CTLCNST_DWELL_BOARPEN, CREAT_DESC[CREAT_BOAR].growth
	},{
		//
		// Waterfall
		//
		CCT_DWELL_ENC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	5,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_CENTCAVE,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_DUNGEON },
		CTLCNST_DWELL_CENTCAVE, CREAT_DESC[CREAT_CENTAUR].growth
	},{
		//
		// Miners guild
		//
		CCT_DWELL_ENC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	500,	5,		5,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_DWCOTTAGE,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_FORTRESS },
		CTLCNST_DWELL_DWCOTTAGE, CREAT_DESC[CREAT_DWARF].growth
	},{
		//
		// Unearthed graves
		//
		CCT_DWELL_ENC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1500,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_EXCAVATION,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_NECROPOLIS },
		CTLCNST_DWELL_EXCAVATION, CREAT_DESC[CREAT_SKELETON].growth
	},{
		//
		// Observation post
		//
		CCT_OBSERVPOST,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2500,	5,		5,		0,		0,		0,		0		},	
		{ CTLCNST_TAVERN,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_CITADEL },
		0, 0
	},{
		//
		// Fortification
		//
		CCT_GENERIC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	10,		0,		0,		0,		0,		0		},	
		{ CTLCNST_INVALID,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_CITADEL },
		0, 0
	},{
		//
		// Hall of Valhalla
		//
		CCT_PERM_FSK_MOD,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1000,	5,		5,		0,		0,		0,		0		},	
		{ CTLCNST_TAVERN,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_STRONGHOLD },
		FSK_ATTACK, +1
	},{
		//
		// Adobe
		//
		CCT_DWELL_ENC,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2500,	10,		0,		0,		0,		0,		0		},	
		{ CTLCNST_DWELL_OGREFORT,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_STRONGHOLD },
		CTLCNST_DWELL_OGREFORT, CREAT_DESC[CREAT_OGRE].growth
	},{
		//
		// Wall of knowledge
		//
		CCT_PERM_FSK_MOD,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2000,	5,		0,		0,		0,		0,		0		},	
		{ CTLCNST_MAGEGUILD_L1,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_TOWER },
		FSK_KNOWLEDGE, +1
	},{
		//
		// Library
		//
		CCT_LIBRARY,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	5000,	5,		5,		5,		5,		5,		5		},	
		{ CTLCNST_MAGEGUILD_L2,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_TOWER },
		0, 0
	},{
		//
		// Altar
		//
		CCT_MINE,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	5000,	5,		0,		0,		0,		0,		0		},	
		{ CTLCNST_TAVERN,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_DUNGEON },
		MINERAL_GOLD, 1000
	},{
		//
		// Mana vortex
		//
		CCT_MANAVORTEX,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2500,	0,		0,		1,		1,		1,		1		},	
		{ CTLCNST_MAGEGUILD_L1,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_DUNGEON },
		0, 0
	},{
		//
		// Treasury
		//
		CCT_TREASURY,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	5000,	5,		5,		0,		0,		0,		0		},	
		{ CTLCNST_MINERSGUILD,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_FORTRESS },
		0, 0
	},{
		//
		// Mystic pound
		//
		CCT_MYSTICPOUND,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1500,	1,		1,		1,		1,		1,		1		},	
		{ CTLCNST_INVALID,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_FORTRESS },
		0, 0
	},{
		//
		// Necromancy amplifier
		//
		CCT_NECRAMPLIFIER,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	2500,	0,		0,		0,		0,		0,		0		},	
		{ CTLCNST_INVALID,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_NECROPOLIS },
		0, 0
	},{
		//
		// Cover of darkness
		//
		CCT_COVEROFDARKNESS,
		//	Gold,	Ore,	Wood,	Merc,	Gems,	Cryst,	Sulfur
		{	1500,	5,		0,		0,		0,		0,		0		},
		{ CTLCNST_INVALID,CTLCNST_INVALID },
		{ CTLSM_ML, CTLTM_NECROPOLIS },
		0, 0
	}
};


const iSpellFilter CTL_MAGE_GUILD_FILTER[CTLT_COUNT][5] = {
	{
		// Citadel
		{SPT_MASK_ALL, SPL_MASK_FIRST, MST_MASK_GOOD},
		{SPT_MASK_ALL, SPL_MASK_SECOND, MST_MASK_GOOD},
		{SPT_MASK_ALL, SPL_MASK_THIRD, MST_MASK_GOOD}
	},{
		// Stronghold
		{SPT_MASK_ALL, SPL_MASK_FIRST, MST_MASK_EVIL},
		{SPT_MASK_ALL, SPL_MASK_SECOND, MST_MASK_EVIL},
		{SPT_MASK_ALL, SPL_MASK_THIRD, MST_MASK_EVIL}
	},{
		// Tower
		{SPT_MASK_ALL, SPL_MASK_FIRST, MST_MASK_GOOD},
		{SPT_MASK_ALL, SPL_MASK_SECOND, MST_MASK_GOOD},
		{SPT_MASK_ALL, SPL_MASK_THIRD, MST_MASK_GOOD},
		{SPT_MASK_ALL, SPL_MASK_FOURTH, MST_MASK_GOOD},
		{SPT_MASK_ALL, SPL_MASK_FIFTH, MST_MASK_GOOD}
	},{
		// Dungeon
		{SPT_MASK_ALL, SPL_MASK_FIRST, MST_MASK_EVIL},
		{SPT_MASK_ALL, SPL_MASK_SECOND, MST_MASK_EVIL},
		{SPT_MASK_ALL, SPL_MASK_THIRD, MST_MASK_EVIL},
		{SPT_MASK_ALL, SPL_MASK_FOURTH, MST_MASK_EVIL},
		{SPT_MASK_ALL, SPL_MASK_FIFTH, MST_MASK_EVIL}
	},{
		// Fortress
		{SPT_MASK_ALL, SPL_MASK_FIRST, MST_MASK_GOOD},
		{SPT_MASK_ALL, SPL_MASK_SECOND, MST_MASK_GOOD},
		{SPT_MASK_ALL, SPL_MASK_THIRD, MST_MASK_GOOD},
		{SPT_MASK_ALL, SPL_MASK_FOURTH, MST_MASK_GOOD},
		{SPT_MASK_ALL, SPL_MASK_FIFTH, MST_MASK_GOOD}
	},{
		// Necropolis
		{SPT_MASK_ALL, SPL_MASK_FIRST, MST_MASK_EVIL},
		{SPT_MASK_ALL, SPL_MASK_SECOND, MST_MASK_EVIL},
		{SPT_MASK_ALL, SPL_MASK_THIRD, MST_MASK_EVIL},
		{SPT_MASK_ALL, SPL_MASK_FOURTH, MST_MASK_EVIL},
		{SPT_MASK_ALL, SPL_MASK_FIFTH, MST_MASK_EVIL}
	}
};

const uint8 CTL_MAGE_GUILD_SPELLS[CTLT_COUNT][5] = {
	{
		// Citadel
		5, 4, 3
	},{
		// Stronghold
		5, 4, 3
	},{
		// Tower
		5, 4, 3, 2, 1
	},{
		// Dungeon
		5, 4, 3, 2, 1
	},{
		// Fortress
		5, 4, 3, 2, 1
	},{
		// Necropolis
		5, 4, 3, 2, 1
	}
};

