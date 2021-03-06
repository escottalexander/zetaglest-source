// ==============================================================
//      This file is part of Glest (www.glest.org)
//
//      Copyright (C) 2001-2008 Martiño Figueroa
//
//      You can redistribute this code and/or modify it under
//      the terms of the GNU General Public License as published
//      by the Free Software Foundation; either version 2 of the
//      License, or (at your option) any later version
// ==============================================================

#include "tech_tree.h"

#include <cassert>

#include "util.h"
#include "resource.h"
#include "faction_type.h"
#include "logger.h"
#include "xml_parser.h"
#include "platform_util.h"
#include "game_util.h"
#include "window.h"
#include "common_scoped_ptr.h"
#include "leak_dumper.h"

using namespace Shared::Util;
using namespace Shared::Xml;

namespace Glest {
	namespace Game {

		// =====================================================
		//      class TechTree
		// =====================================================

		TechTree::TechTree(const vector < string > pathList) {
			SkillType::resetNextAttackBoostId();

			name = "";
			treePath = "";
			this->pathList.assign(pathList.begin(), pathList.end());

			resourceTypes.clear();
			factionTypes.clear();
			armorTypes.clear();
			attackTypes.clear();
			translatedTechNames.clear();
			translatedTechFactionNames.clear();
			languageUsedForCache = "";
			isValidationModeEnabled = false;
		}

		string TechTree::getNameUntranslated() const {
			return name;
		}

		string TechTree::getName(bool translatedValue) {
			if (translatedValue == false) {
				return getNameUntranslated();
			}

			bool foundTranslation = false;
			Lang & lang = Lang::getInstance();
			if (lang.getTechNameLoaded() != name ||
				lang.getLanguage() != languageUsedForCache) {
				//printf("Line: %d Tech [%s]\n",__LINE__,name.c_str());

				foundTranslation =
					lang.loadTechTreeStrings(name,
						lang.getLanguage() !=
						languageUsedForCache);
				languageUsedForCache = lang.getLanguage();
				translatedTechFactionNames.erase(name);
				translatedTechNames.erase(name);
			}

			string result = name;
			if (foundTranslation == true) {
				result = lang.getTechTreeString("TechTreeName", name.c_str());
			} else {
				result = formatString(result);
			}


			//printf("Line: %d Tech [%s] result [%s]\n",__LINE__,name.c_str(),result.c_str());
			return result;
		}

		string TechTree::getTranslatedName(string techName, bool forceLoad,
			bool forceTechtreeActiveFile) {
			string result = techName;

			//printf("Line: %d Tech [%s] forceLoad = %d forceTechtreeActiveFile = %d\n",__LINE__,techName.c_str(),forceLoad,forceTechtreeActiveFile);

			Lang & lang = Lang::getInstance();
			if (forceTechtreeActiveFile == false &&
				translatedTechNames.find(techName) != translatedTechNames.end() &&
				lang.getLanguage() == languageUsedForCache) {
				result = translatedTechNames[techName];
			} else {
				name = "";
				string path = findPath(techName);
				if (path != "") {
					string currentPath = path;
					endPathWithSlash(currentPath);
					treePath = currentPath;
					name = lastDir(currentPath);

					lang.loadTechTreeStrings(name,
						lang.getLanguage() !=
						languageUsedForCache);
					languageUsedForCache = lang.getLanguage();

					translatedTechFactionNames.erase(techName);
					translatedTechNames.erase(techName);

					result = getName(true);

					//printf("techName [%s] name [%s] result [%s]\n",techName.c_str(),name.c_str(),result.c_str());
					translatedTechNames[name] = result;
				}

			}
			return result;
		}

		string TechTree::getTranslatedFactionName(string techName,
			string factionName) {
			//printf("Line: %d Tech [%s] name [%s] factionName [%s]\n",__LINE__,techName.c_str(),name.c_str(),factionName.c_str());

			Lang & lang = Lang::getInstance();
			if (lang.getTechNameLoaded() != techName ||
				lang.getLanguage() != languageUsedForCache) {
				//printf("Line: %d Tech [%s] name [%s] lang.getTechNameLoaded() [%s] factionName [%s]\n",__LINE__,techName.c_str(),name.c_str(),lang.getTechNameLoaded().c_str(),factionName.c_str());

				lang.loadTechTreeStrings(techName,
					lang.getLanguage() !=
					languageUsedForCache);
				languageUsedForCache = lang.getLanguage();

				translatedTechFactionNames.erase(techName);
			}

			std::map < string, std::map < string, string > >::iterator iterMap =
				translatedTechFactionNames.find(techName);
			if (iterMap != translatedTechFactionNames.end()) {
				if (iterMap->second.find(factionName) != iterMap->second.end()) {
					//printf("Line: %d Tech [%s] factionName [%s]\n",__LINE__,techName.c_str(),factionName.c_str());

					return iterMap->second.find(factionName)->second;
				}
			}

			//printf("Line: %d Tech [%s] factionName [%s]\n",__LINE__,techName.c_str(),factionName.c_str());

			getTranslatedName(techName, false, true);

			string result =
				lang.getTechTreeString("FactionName_" + factionName,
					formatString(factionName).c_str());
			//printf(">>result = %s\n",result.c_str());
			translatedTechFactionNames[techName][factionName] = result;

			//printf("Line: %d Translated faction for Tech [%s] faction [%s] result [%s]\n",__LINE__,techName.c_str(),factionName.c_str(),result.c_str());

			return result;
		}

		Checksum TechTree::loadTech(const string & techName,
			set < string > &factions,
			Checksum * checksum, std::map < string,
			vector < pair < string,
			string > > >&loadedFileList,
			bool validationMode) {
			name = "";
			isValidationModeEnabled = validationMode;
			Checksum techtreeChecksum;
			string path = findPath(techName);
			if (path != "") {
				//printf(">>> path=%s\n",path.c_str());
				load(path, factions, checksum, &techtreeChecksum, loadedFileList,
					validationMode);
			} else {
				printf(">>> techtree [%s] path not found.\n", techName.c_str());
			}
			return techtreeChecksum;
		}

		bool TechTree::exists(const string & techName,
			const vector < string > &pathTechList) {
			bool techFound = false;
			auto_ptr < TechTree > techTree(new TechTree(pathTechList));
			string path = techTree->findPath(techName);
			if (path != "") {
				techFound = true;
			}
			return techFound;
		}

		string TechTree::findPath(const string & techName) const {
			return findPath(techName, pathList);
		}
		string TechTree::findPath(const string & techName,
			const vector < string > &pathTechList) {
			for (unsigned int idx = 0; idx < pathTechList.size(); ++idx) {
				string currentPath = (pathTechList)[idx];
				endPathWithSlash(currentPath);

				string path = currentPath + techName;

				//printf(">>> test path=%s\n",path.c_str());
				if (isdir(path.c_str()) == true) {
					return path;
					//break;
				}
			}
			//return "no path found for tech: \""+techname+"\"";
			return "";
		}


		void TechTree::load(const string & dir, set < string > &factions,
			Checksum * checksum, Checksum * techtreeChecksum,
			std::map < string, vector < pair < string,
			string > > >&loadedFileList, bool validationMode) {
			if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
				enabled)
				SystemFlags::OutputDebug(SystemFlags::debugSystem,
					"In [%s::%s Line: %d]\n",
					extractFileFromDirectoryPath(__FILE__).
					c_str(), __FUNCTION__, __LINE__);

			string currentPath = dir;
			endPathWithSlash(currentPath);
			treePath = currentPath;
			name = lastDir(currentPath);

			Lang & lang = Lang::getInstance();
			lang.loadTechTreeStrings(name, true);
			languageUsedForCache = lang.getLanguage();

			char szBuf[8096] = "";
			snprintf(szBuf, 8096,
				Lang::getInstance().
				getString("LogScreenGameLoadingTechtree", "", true).c_str(),
				formatString(getName(true)).c_str());
			Logger::getInstance().add(szBuf, true);

			vector < string > filenames;
			//load resources
			string str = currentPath + "resources/*.";

			try {
				findAll(str, filenames);
				resourceTypes.resize(filenames.size());

				for (int i = 0; i < (int) filenames.size(); ++i) {
					str = currentPath + "resources/" + filenames[i];
					resourceTypes[i].load(str, checksum, &checksumValue,
						loadedFileList, treePath);
					Window::handleEvent();
					SDL_PumpEvents();
				}

				// Cleanup pixmap memory
				for (int i = 0; i < (int) filenames.size(); ++i) {
					resourceTypes[i].deletePixels();
				}
			} catch (megaglest_runtime_error & ex) {
				SystemFlags::OutputDebug(SystemFlags::debugError,
					"In [%s::%s Line: %d] Error [%s]\n",
					extractFileFromDirectoryPath(__FILE__).
					c_str(), __FUNCTION__, __LINE__,
					ex.what());
				throw megaglest_runtime_error("Error loading Resource Types in: " +
					currentPath + "\nMessage: " +
					ex.what(), !ex.wantStackTrace()
					|| isValidationModeEnabled);
			} catch (const exception & e) {
				SystemFlags::OutputDebug(SystemFlags::debugError,
					"In [%s::%s Line: %d] Error [%s]\n",
					extractFileFromDirectoryPath(__FILE__).
					c_str(), __FUNCTION__, __LINE__,
					e.what());
				throw megaglest_runtime_error("Error loading Resource Types in: " +
					currentPath + "\nMessage: " +
					e.what(), isValidationModeEnabled);
			}

			// give CPU time to update other things to avoid apperance of hanging
			sleep(0);
			Window::handleEvent();
			SDL_PumpEvents();

			//load tech tree xml info
			try {
				XmlTree xmlTree;
				string currentPath = dir;
				endPathWithSlash(currentPath);
				string path = currentPath + lastDir(dir) + ".xml";

				checksum->addFile(path);
				checksumValue.addFile(path);

				std::map < string, string > mapExtraTagReplacementValues;
				mapExtraTagReplacementValues["$COMMONDATAPATH"] =
					currentPath + "/commondata/";
				xmlTree.load(path,
					Properties::
					getTagReplacementValues
					(&mapExtraTagReplacementValues));
				loadedFileList[path].push_back(make_pair(currentPath, currentPath));

				Properties::setTechtreePath(currentPath);
				if (SystemFlags::VERBOSE_MODE_ENABLED)
					printf("==> Set techtree path to [%s]\n", currentPath.c_str());

				const XmlNode *techTreeNode = xmlTree.getRootNode();

				//attack types
				const XmlNode *attackTypesNode =
					techTreeNode->getChild("attack-types");
				attackTypes.resize(attackTypesNode->getChildCount());
				for (int i = 0; i < (int) attackTypes.size(); ++i) {
					const XmlNode *attackTypeNode =
						attackTypesNode->getChild("attack-type", i);
					attackTypes[i].setName(attackTypeNode->getAttribute("name")->
						getRestrictedValue());
					attackTypes[i].setId(i);

					Window::handleEvent();
					SDL_PumpEvents();
				}

				// give CPU time to update other things to avoid apperance of hanging
				sleep(0);
				//SDL_PumpEvents();

				//armor types
				const XmlNode *armorTypesNode =
					techTreeNode->getChild("armor-types");
				armorTypes.resize(armorTypesNode->getChildCount());
				for (int i = 0; i < (int) armorTypes.size(); ++i) {
					const XmlNode *armorTypeNode =
						armorTypesNode->getChild("armor-type", i);
					armorTypes[i].setName(armorTypeNode->getAttribute("name")->
						getRestrictedValue());
					armorTypes[i].setId(i);

					Window::handleEvent();
					SDL_PumpEvents();
				}

				//damage multipliers
				damageMultiplierTable.init((int) attackTypes.size(),
					(int) armorTypes.size());
				const XmlNode *damageMultipliersNode =
					techTreeNode->getChild("damage-multipliers");
				for (int i = 0; i < (int) damageMultipliersNode->getChildCount();
					++i) {
					const XmlNode *damageMultiplierNode =
						damageMultipliersNode->getChild("damage-multiplier", i);
					const AttackType *attackType =
						getAttackType(damageMultiplierNode->getAttribute("attack")->
							getRestrictedValue());
					const ArmorType *armorType =
						getArmorType(damageMultiplierNode->getAttribute("armor")->
							getRestrictedValue());
					double multiplier =
						damageMultiplierNode->getAttribute("value")->getFloatValue();
					damageMultiplierTable.setDamageMultiplier(attackType, armorType,
						multiplier);

					Window::handleEvent();
					SDL_PumpEvents();
				}
			} catch (megaglest_runtime_error & ex) {
				SystemFlags::OutputDebug(SystemFlags::debugError,
					"In [%s::%s Line: %d] Error [%s]\n",
					extractFileFromDirectoryPath(__FILE__).
					c_str(), __FUNCTION__, __LINE__,
					ex.what());
				throw megaglest_runtime_error("Error loading Tech Tree: " +
					currentPath + "\nMessage: " +
					ex.what(), !ex.wantStackTrace()
					|| isValidationModeEnabled);
			} catch (const exception & e) {
				SystemFlags::OutputDebug(SystemFlags::debugError,
					"In [%s::%s Line: %d] Error [%s]\n",
					extractFileFromDirectoryPath(__FILE__).
					c_str(), __FUNCTION__, __LINE__,
					e.what());
				throw megaglest_runtime_error("Error loading Tech Tree: " +
					currentPath + "\nMessage: " +
					e.what(), isValidationModeEnabled);
			}

			// give CPU time to update other things to avoid apperance of hanging
			sleep(0);
			//SDL_PumpEvents();

			//load factions
			try {
				factionTypes.resize(factions.size());

				int i = 0;
				for (set < string >::iterator it = factions.begin();
					it != factions.end(); ++it) {
					string factionName = *it;

					char szBuf[8096] = "";
					snprintf(szBuf, 8096, "%s %s [%d / %d] - %s",
						Lang::getInstance().getString("Loading").c_str(),
						Lang::getInstance().getString("Faction").c_str(),
						i + 1, (int) factions.size(),
						formatString(this->
							getTranslatedFactionName(name,
								factionName)).
						c_str());
					Logger & logger = Logger::getInstance();
					logger.setState(szBuf);
					logger.
						setProgress((int)
						((((double) i) / (double) factions.size()) *
							100.0));

					factionTypes[i++].load(factionName, this, checksum, &checksumValue,
						loadedFileList, validationMode);

					// give CPU time to update other things to avoid apperance of hanging
					sleep(0);
					Window::handleEvent();
					SDL_PumpEvents();
				}
			} catch (megaglest_runtime_error & ex) {
				SystemFlags::OutputDebug(SystemFlags::debugError,
					"In [%s::%s Line: %d] Error [%s]\n",
					extractFileFromDirectoryPath(__FILE__).
					c_str(), __FUNCTION__, __LINE__,
					ex.what());
				throw megaglest_runtime_error("Error loading Faction Types: " +
					currentPath + "\nMessage: " +
					ex.what(), !ex.wantStackTrace()
					|| isValidationModeEnabled);
			} catch (const exception & e) {
				SystemFlags::OutputDebug(SystemFlags::debugError,
					"In [%s::%s Line: %d] Error [%s]\n",
					extractFileFromDirectoryPath(__FILE__).
					c_str(), __FUNCTION__, __LINE__,
					e.what());
				throw megaglest_runtime_error("Error loading Faction Types: " +
					currentPath + "\nMessage: " +
					e.what(), isValidationModeEnabled);
			}

			if (techtreeChecksum != NULL) {
				*techtreeChecksum = checksumValue;
			}

			if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
				enabled)
				SystemFlags::OutputDebug(SystemFlags::debugSystem,
					"In [%s::%s Line: %d]\n",
					extractFileFromDirectoryPath(__FILE__).
					c_str(), __FUNCTION__, __LINE__);
		}

		TechTree::~TechTree() {
			if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
				enabled)
				SystemFlags::OutputDebug(SystemFlags::debugSystem,
					"In [%s::%s Line: %d]\n",
					extractFileFromDirectoryPath(__FILE__).
					c_str(), __FUNCTION__, __LINE__);
			Logger::getInstance().add(Lang::getInstance().
				getString("LogScreenGameUnLoadingTechtree",
					"", true), true);
			resourceTypes.clear();
			factionTypes.clear();
			armorTypes.clear();
			attackTypes.clear();
		}

		std::vector < std::string > TechTree::validateFactionTypes() {
			std::vector < std::string > results;
			for (int i = 0; i < (int) factionTypes.size(); ++i) {
				std::vector < std::string > factionResults =
					factionTypes[i].validateFactionType();
				if (factionResults.empty() == false) {
					results.insert(results.end(), factionResults.begin(),
						factionResults.end());
				}

				factionResults = factionTypes[i].validateFactionTypeUpgradeTypes();
				if (factionResults.empty() == false) {
					results.insert(results.end(), factionResults.begin(),
						factionResults.end());
				}
			}

			return results;
		}

		std::vector < std::string > TechTree::validateResourceTypes() {
			std::vector < std::string > results;
			ResourceTypes resourceTypesNotUsed = resourceTypes;
			for (unsigned int i = 0; i < resourceTypesNotUsed.size(); ++i) {
				ResourceType & rt = resourceTypesNotUsed[i];
				rt.setCleanupMemory(false);
			}
			for (unsigned int i = 0; i < factionTypes.size(); ++i) {
				//printf("Validating [%d / %d] faction [%s]\n",i,(int)factionTypes.size(),factionTypes[i].getName().c_str());

				std::vector < std::string > factionResults =
					factionTypes[i].validateFactionTypeResourceTypes(resourceTypes);
				if (factionResults.empty() == false) {
					results.insert(results.end(), factionResults.begin(),
						factionResults.end());
				}

				// Check if the faction uses the resources in this techtree
				// Now lets find a matching faction resource type for the unit
				for (int j = (int) resourceTypesNotUsed.size() - 1; j >= 0; --j) {
					const ResourceType & rt = resourceTypesNotUsed[j];
					//printf("Validating [%d / %d] resourcetype [%s]\n",j,(int)resourceTypesNotUsed.size(),rt.getName().c_str());

					if (factionTypes[i].factionUsesResourceType(&rt) == true) {
						//printf("FOUND FACTION CONSUMER FOR RESOURCE - [%d / %d] faction [%s] resource [%d / %d] resourcetype [%s]\n",i,(int)factionTypes.size(),factionTypes[i].getName().c_str(),j,(int)resourceTypesNotUsed.size(),rt.getName().c_str());
						resourceTypesNotUsed.erase(resourceTypesNotUsed.begin() + j);
					}
				}
			}

			if (resourceTypesNotUsed.empty() == false) {
				//printf("FOUND unused resource Types [%d]\n",(int)resourceTypesNotUsed.size());

				for (unsigned int i = 0; i < resourceTypesNotUsed.size(); ++i) {
					const ResourceType & rt = resourceTypesNotUsed[i];
					char szBuf[8096] = "";
					snprintf(szBuf, 8096,
						"The Resource type [%s] is not used by any units in this techtree!",
						rt.getName().c_str());
					results.push_back(szBuf);
				}
			}
			return results;
		}

		// ==================== get ====================

		FactionType *TechTree::getTypeByName(const string & name) {
			for (int i = 0; i < (int) factionTypes.size(); ++i) {
				if (factionTypes[i].getName(false) == name) {
					return &factionTypes[i];
				}
			}
			if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
				enabled)
				SystemFlags::OutputDebug(SystemFlags::debugSystem,
					"In [%s::%s Line: %d]\n",
					extractFileFromDirectoryPath(__FILE__).
					c_str(), __FUNCTION__, __LINE__);
			throw megaglest_runtime_error("Faction not found: " + name, true);
		}

		const FactionType *TechTree::getType(const string & name) const {
			for (int i = 0; i < (int) factionTypes.size(); ++i) {
				if (factionTypes[i].getName(false) == name) {
					return &factionTypes[i];
				}
			}
			if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
				enabled)
				SystemFlags::OutputDebug(SystemFlags::debugSystem,
					"In [%s::%s Line: %d]\n",
					extractFileFromDirectoryPath(__FILE__).
					c_str(), __FUNCTION__, __LINE__);
			throw megaglest_runtime_error("Faction not found: " + name, true);
		}

		const ResourceType *TechTree::getTechResourceType(int i) const {
			for (int j = 0; j < getResourceTypeCount(); ++j) {
				const ResourceType *rt = getResourceType(j);
				assert(rt != NULL);
				if (rt == NULL) {
					throw megaglest_runtime_error("rt == NULL");
				}
				if (rt->getResourceNumber() == i && rt->getClass() == rcTech)
					return getResourceType(j);
			}

			return getFirstTechResourceType();
		}

		const ResourceType *TechTree::getFirstTechResourceType() const {
			for (int i = 0; i < getResourceTypeCount(); ++i) {
				const ResourceType *rt = getResourceType(i);
				assert(rt != NULL);
				if (rt->getResourceNumber() == 1 && rt->getClass() == rcTech)
					return getResourceType(i);
			}

			char szBuf[8096] = "";
			snprintf(szBuf, 8096,
				"The referenced tech tree [%s] is either missing or has no resources defined but at least one resource is required.",
				this->name.c_str());
			throw megaglest_runtime_error(szBuf, true);
		}

		const ResourceType *TechTree::getResourceType(const string & name) const {

			for (int i = 0; i < (int) resourceTypes.size(); ++i) {
				if (resourceTypes[i].getName() == name) {
					return &resourceTypes[i];
				}
			}

			throw megaglest_runtime_error("Resource Type not found: " + name,
				true);
		}

		const ArmorType *TechTree::getArmorType(const string & name) const {
			for (int i = 0; i < (int) armorTypes.size(); ++i) {
				if (armorTypes[i].getName(false) == name) {
					return &armorTypes[i];
				}
			}

			throw megaglest_runtime_error("Armor Type not found: " + name, true);
		}

		const AttackType *TechTree::getAttackType(const string & name) const {
			for (int i = 0; i < (int) attackTypes.size(); ++i) {
				if (attackTypes[i].getName(false) == name) {
					return &attackTypes[i];
				}
			}

			throw megaglest_runtime_error("Attack Type not found: " + name, true);
		}

		double TechTree::getDamageMultiplier(const AttackType * att,
			const ArmorType * art) const {
			return damageMultiplierTable.getDamageMultiplier(att, art);
		}

		void TechTree::saveGame(XmlNode * rootNode) {
			std::map < string, string > mapTagReplacements;
			XmlNode *techTreeNode = rootNode->addChild("TechTree");

			//      string name;
			techTreeNode->addAttribute("name", name, mapTagReplacements);
			//    //string desc;
			//      string treePath;
				  //techTreeNode->addAttribute("treePath",treePath, mapTagReplacements);
			//      vector<string> pathList;
			//      for(unsigned int i = 0; i < pathList.size(); ++i) {
			//              XmlNode *pathListNode = techTreeNode->addChild("pathList");
			//              pathListNode->addAttribute("value",pathList[i], mapTagReplacements);
			//      }
			//    ResourceTypes resourceTypes;
			//      for(unsigned int i = 0; i < resourceTypes.size(); ++i) {
			//              ResourceType &rt = resourceTypes[i];
			//              rt.saveGame(techTreeNode);
			//      }
			//    FactionTypes factionTypes;
			//      for(unsigned int i = 0; i < factionTypes.size(); ++i) {
			//              FactionType &ft = factionTypes[i];
			//              ft.saveGame(techTreeNode);
			//      }

			//      ArmorTypes armorTypes;
			//      for(unsigned int i = 0; i < armorTypes.size(); ++i) {
			//              ArmorType &at = armorTypes[i];
			//              at.saveGame(techTreeNode);
			//      }

			//      AttackTypes attackTypes;
			//      for(unsigned int i = 0; i < attackTypes.size(); ++i) {
			//              AttackType &at = attackTypes[i];
			//              at.saveGame(techTreeNode);
			//      }

			//      DamageMultiplierTable damageMultiplierTable;
			//      damageMultiplierTable.saveGame(techTreeNode);

			//      Checksum checksumValue;
			techTreeNode->addAttribute("checksumValue",
				intToStr(checksumValue.getSum()),
				mapTagReplacements);
		}

	}
}                              //end namespace
