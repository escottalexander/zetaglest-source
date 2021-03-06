// ==============================================================
//      This file is part of Glest (www.glest.org)
//
//      Copyright (C) 2011-  by Titus Tscharntke
//
//      You can redistribute this code and/or modify it under
//      the terms of the GNU General Public License as published
//      by the Free Software Foundation; either version 2 of the
//      License, or (at your option) any later version
// ==============================================================

#include "menu_state_keysetup.h"

#include "renderer.h"
#include "sound_renderer.h"
#include "core_data.h"
#include "config.h"
#include "menu_state_options.h"
#include "menu_state_root.h"
#include "menu_state_keysetup.h"
#include "menu_state_options_graphics.h"
#include "menu_state_options_sound.h"
#include "menu_state_options_network.h"
#include "menu_state_options_sound.h"
#include "metrics.h"
#include "string_utils.h"

#include "leak_dumper.h"

namespace Glest {
	namespace Game {


		// =====================================================
		//      class MenuStateKeysetup
		// =====================================================

		MenuStateKeysetup::MenuStateKeysetup(Program * program,
			MainMenu * mainMenu,
			ProgramState **
			parentUI) :MenuState(program,
				mainMenu,
				"config"),
			buttonOk("KeySetup", "buttonOk"), buttonDefaults("KeySetup",
				"buttonDefaults"),
			buttonReturn("KeySetup", "buttonReturn"),
			buttonKeyboardSetup("KeySetup", "buttonKeyboardSetup"),
			buttonVideoSection("KeySetup", "buttonVideoSection"),
			buttonAudioSection("KeySetup", "buttonAudioSection"),
			buttonMiscSection("KeySetup", "buttonMiscSection"),
			buttonNetworkSettings("KeySetup", "buttonNetworkSettings"),
			labelTitle("KeySetup", "labelTitle"), keyScrollBar("KeySetup",
				"keyScrollBar"),
			mainMessageBox("KeySetup", "mainMessageBox"),
			labelTestTitle("KeySetup", "labelTestTitle"),
			labelTestValue("KeySetup", "labelTestValue") {
			try {
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);
				containerName = "KeySetup";

				keyButtonsLineHeight = 30;
				keyButtonsHeight = 25;
				keyButtonsWidth = 400;
				keyButtonsXBase = 200;
				keyButtonsYBase = 200 + 400 - keyButtonsLineHeight;
				keyButtonsToRender = 400 / keyButtonsLineHeight;
				int labelWidth = 100;

				this->parentUI = parentUI;
				this->console.setOnlyChatMessagesInStoredLines(false);
				hotkeyIndex = -1;
				hotkeyChar = SDLK_UNKNOWN;

				Lang & lang = Lang::getInstance();
				int buttonStartPos = 170;
				int buttonRowPos = 50;
				if (this->parentUI == NULL) {
					int tabButtonWidth = 200;
					int tabButtonHeight = 30;

					buttonAudioSection.init(0, 720, tabButtonWidth, tabButtonHeight);
					buttonAudioSection.setFont(CoreData::getInstance().
						getMenuFontVeryBig());
					buttonAudioSection.setFont3D(CoreData::getInstance().
						getMenuFontVeryBig3D());
					buttonAudioSection.setText(lang.getString("Audio"));
					// Video Section
					buttonVideoSection.init(200, 720, tabButtonWidth,
						tabButtonHeight);
					buttonVideoSection.setFont(CoreData::getInstance().
						getMenuFontVeryBig());
					buttonVideoSection.setFont3D(CoreData::getInstance().
						getMenuFontVeryBig3D());
					buttonVideoSection.setText(lang.getString("Video"));
					//currentLine-=lineOffset;
					//MiscSection
					buttonMiscSection.init(400, 720, tabButtonWidth,
						tabButtonHeight);
					buttonMiscSection.setFont(CoreData::getInstance().
						getMenuFontVeryBig());
					buttonMiscSection.setFont3D(CoreData::getInstance().
						getMenuFontVeryBig3D());
					buttonMiscSection.setText(lang.getString("Misc"));
					//NetworkSettings
					buttonNetworkSettings.init(600, 720, tabButtonWidth,
						tabButtonHeight);
					buttonNetworkSettings.setFont(CoreData::getInstance().
						getMenuFontVeryBig());
					buttonNetworkSettings.setFont3D(CoreData::getInstance().
						getMenuFontVeryBig3D());
					buttonNetworkSettings.setText(lang.getString("Network"));

					//KeyboardSetup
					buttonKeyboardSetup.init(800, 700, tabButtonWidth,
						tabButtonHeight + 20);
					buttonKeyboardSetup.setFont(CoreData::getInstance().
						getMenuFontVeryBig());
					buttonKeyboardSetup.setFont3D(CoreData::getInstance().
						getMenuFontVeryBig3D());
					buttonKeyboardSetup.setText(lang.getString("Keyboardsetup"));
				}
				// header
				labelTitle.init(375, 650);
				labelTitle.setFont(CoreData::getInstance().getMenuFontVeryBig());
				labelTitle.setFont3D(CoreData::getInstance().
					getMenuFontVeryBig3D());
				labelTitle.setText(lang.getString("KeyboardsetupL"));

				labelTestTitle.init(keyButtonsXBase, 155);
				labelTestTitle.setFont(CoreData::getInstance().
					getMenuFontNormal());
				labelTestTitle.setFont3D(CoreData::getInstance().
					getMenuFontNormal3D());
				labelTestTitle.setText(lang.getString("KeyboardsetupTest"));

				labelTestValue.init(keyButtonsXBase, 155 - 28);
				labelTestValue.setFont(CoreData::getInstance().getMenuFontBig());
				labelTestValue.setFont3D(CoreData::getInstance().
					getMenuFontBig3D());
				labelTestValue.setRenderBackground(true);
				labelTestValue.setMaxEditRenderWidth(keyButtonsWidth);
				labelTestValue.setText("");

				// mainMassegeBox
				mainMessageBox.init(lang.getString("Ok"));
				mainMessageBox.setEnabled(false);
				mainMessageBoxState = 0;

				keyScrollBar.init(800, 200, false, 200, 20);
				keyScrollBar.setLength(400);
				keyScrollBar.setElementCount(0);
				keyScrollBar.setVisibleSize(keyButtonsToRender);
				keyScrollBar.setVisibleStart(0);

				// buttons
				buttonOk.init(buttonStartPos, buttonRowPos, 100);
				buttonOk.setText(lang.getString("Save"));

				buttonReturn.init(buttonStartPos + 110, buttonRowPos, 100);
				buttonReturn.setText(lang.getString("Return"));

				buttonDefaults.init(buttonStartPos + 230, buttonRowPos, 125);
				buttonDefaults.setText(lang.getString("Defaults"));

				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);

				Config & configKeys =
					Config::getInstance(std::pair < ConfigType,
						ConfigType >(cfgMainKeys, cfgUserKeys));
				mergedProperties = configKeys.getMergedProperties();
				masterProperties = configKeys.getMasterProperties();
				//userProperties=configKeys.getUserProperties();
				userProperties.clear();

				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);

				//throw megaglest_runtime_error("Test!");

				for (int i = 0; i < (int) mergedProperties.size(); ++i) {

					string keyName = mergedProperties[i].second;
					if (keyName.length() > 0) {
						//char c = configKeys.translateStringToCharKey(keyName);
						SDL_Keycode c = configKeys.translateStringToSDLKey(keyName);
						if (c > SDLK_UNKNOWN && c < SDL_NUM_SCANCODES) {
							SDL_Keycode keysym = static_cast <SDL_Keycode> (c);
							// SDL skips capital letters
							if (keysym >= 65 && keysym <= 90) {
								keysym = (SDL_Keycode) ((int) keysym + 32);
							}
							keyName = SDL_GetKeyName(keysym);
						} else {
							keyName = "";
						}
						if (keyName == "unknown key" || keyName == "") {
							keyName = mergedProperties[i].second;
						}
					}

					GraphicButton *button =
						new GraphicButton(containerName,
							string("ScrollButton") + intToStr(i));
					button->init(keyButtonsXBase, keyButtonsYBase, keyButtonsWidth,
						keyButtonsHeight);
					button->setText(mergedProperties[i].first);
					keyButtons.push_back(button);
					GraphicLabel *label =
						new GraphicLabel(containerName,
							string("ScrollLabel") + intToStr(i));
					label->init(keyButtonsXBase + keyButtonsWidth + 5, keyButtonsYBase,
						labelWidth, 20);
					label->setRenderBackground(true);
					label->setMaxEditRenderWidth(105);
					label->setText("  " + keyName);
					labels.push_back(label);
				}

				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);

				keyScrollBar.init(keyButtonsXBase + keyButtonsWidth + labelWidth +
					20, 200, false, 200, 20);
				keyScrollBar.setLength(400);
				keyScrollBar.setElementCount((int) keyButtons.size());
				keyScrollBar.setVisibleSize(keyButtonsToRender);
				keyScrollBar.setVisibleStart(0);
			} catch (const std::exception & ex) {
				char szBuf[8096] = "";
				snprintf(szBuf, 8096, "In [%s::%s %d] Error detected:\n%s\n",
					__FILE__, __FUNCTION__, __LINE__, ex.what());
				SystemFlags::OutputDebug(SystemFlags::debugError, szBuf);
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem, "%s", szBuf);

				mainMessageBoxState = 1;
				showMessageBox("Error: " + string(ex.what()), "Error detected",
					false);
			}
		}

		void MenuStateKeysetup::reloadUI() {
			Lang & lang = Lang::getInstance();

			labelTitle.setText(lang.getString("KeyboardsetupL"));
			labelTestTitle.setText(lang.getString("KeyboardsetupTest"));
			labelTestValue.setText("");

			// mainMassegeBox
			mainMessageBox.init(lang.getString("Ok"));

			buttonOk.setText(lang.getString("Save"));
			buttonReturn.setText(lang.getString("Return"));
			buttonDefaults.setText(lang.getString("Defaults"));
		}

		void MenuStateKeysetup::cleanup() {
			if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
				enabled)
				SystemFlags::OutputDebug(SystemFlags::debugSystem,
					"In [%s::%s Line: %d]\n", __FILE__,
					__FUNCTION__, __LINE__);
			clearUserButtons();
			if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
				enabled)
				SystemFlags::OutputDebug(SystemFlags::debugSystem,
					"In [%s::%s Line: %d] END\n", __FILE__,
					__FUNCTION__, __LINE__);
		}

		MenuStateKeysetup::~MenuStateKeysetup() {
			if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
				enabled)
				SystemFlags::OutputDebug(SystemFlags::debugSystem,
					"In [%s::%s Line: %d]\n", __FILE__,
					__FUNCTION__, __LINE__);
			cleanup();
			if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
				enabled)
				SystemFlags::OutputDebug(SystemFlags::debugSystem,
					"In [%s::%s Line: %d] END\n", __FILE__,
					__FUNCTION__, __LINE__);
		}

		void MenuStateKeysetup::clearUserButtons() {
			while (!keyButtons.empty()) {
				delete keyButtons.back();
				keyButtons.pop_back();
			}
			while (!labels.empty()) {
				delete labels.back();
				labels.pop_back();
			}
		}

		void MenuStateKeysetup::mouseClick(int x, int y, MouseButton mouseButton) {
			if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
				enabled)
				SystemFlags::OutputDebug(SystemFlags::debugSystem,
					"In [%s::%s Line: %d]\n", __FILE__,
					__FUNCTION__, __LINE__);

			CoreData & coreData = CoreData::getInstance();
			SoundRenderer & soundRenderer = SoundRenderer::getInstance();

			if (mainMessageBox.getEnabled()) {
				int button = 0;
				if (mainMessageBox.mouseClick(x, y, button)) {
					soundRenderer.playFx(coreData.getClickSoundA());
					if (button == 0) {
						mainMessageBox.setEnabled(false);
					}
				}
			} else if (keyScrollBar.mouseClick(x, y)) {
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);
				soundRenderer.playFx(coreData.getClickSoundB());
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);
			} else if (buttonReturn.mouseClick(x, y)) {
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);
				soundRenderer.playFx(coreData.getClickSoundB());
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);

				if (this->parentUI != NULL) {
					// Set the parent pointer to NULL so the owner knows it was deleted
					*this->parentUI = NULL;
					// Delete the main menu
					delete mainMenu;
					return;
				}
				mainMenu->setState(new MenuStateRoot(program, mainMenu));
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);
			} else if (buttonDefaults.mouseClick(x, y)) {
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);
				soundRenderer.playFx(coreData.getClickSoundB());
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);

				Config & configKeys =
					Config::getInstance(std::pair < ConfigType,
						ConfigType >(cfgMainKeys, cfgUserKeys));
				string userKeysFile = configKeys.getFileName(true);

				bool result = removeFile(userKeysFile.c_str());
				if (SystemFlags::VERBOSE_MODE_ENABLED)
					printf("In [%s::%s Line: %d] delete file [%s] returned %d\n",
						__FILE__, __FUNCTION__, __LINE__, userKeysFile.c_str(),
						result);
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d] delete file [%s] returned %d\n",
						__FILE__, __FUNCTION__, __LINE__,
						userKeysFile.c_str(), result);
				configKeys.reload();

				if (this->parentUI != NULL) {
					// Set the parent pointer to NULL so the owner knows it was deleted
					*this->parentUI = NULL;
					// Delete the main menu
					delete mainMenu;
					return;
				}

				mainMenu->setState(new MenuStateKeysetup(program, mainMenu));
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);
			} else if (buttonOk.mouseClick(x, y)) {
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);
				soundRenderer.playFx(coreData.getClickSoundB());
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);

				if (userProperties.empty() == false) {
					Config & config = Config::getInstance();
					Config & configKeys =
						Config::getInstance(std::pair < ConfigType,
							ConfigType >(cfgMainKeys, cfgUserKeys),
							std::pair < string,
							string >(Config::glestkeys_ini_filename,
								Config::
								glestuserkeys_ini_filename),
							std::pair < bool, bool >(true, false),
							config.getString("GlestKeysIniPath", ""));
					string userKeysFile = configKeys.getFileName(true);
					if (SystemFlags::VERBOSE_MODE_ENABLED)
						printf
						("In [%s::%s Line: %d] save file [%s] userProperties.size() = "
							MG_SIZE_T_SPECIFIER "\n", __FILE__, __FUNCTION__, __LINE__,
							userKeysFile.c_str(), userProperties.size());
					if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
						enabled)
						SystemFlags::OutputDebug(SystemFlags::debugSystem,
							"In [%s::%s Line: %d] save file [%s] userProperties.size() = "
							MG_SIZE_T_SPECIFIER "\n", __FILE__,
							__FUNCTION__, __LINE__,
							userKeysFile.c_str(),
							userProperties.size());

					configKeys.setUserProperties(userProperties);
					configKeys.save();
					configKeys.reload();
				}

				Lang & lang = Lang::getInstance();
				console.addLine(lang.getString("SettingsSaved"));
				if (SystemFlags::getSystemSettingType(SystemFlags::debugSystem).
					enabled)
					SystemFlags::OutputDebug(SystemFlags::debugSystem,
						"In [%s::%s Line: %d]\n", __FILE__,
						__FUNCTION__, __LINE__);
			} else if (keyScrollBar.getElementCount() != 0) {
				for (int i = keyScrollBar.getVisibleStart(); i
					<= keyScrollBar.getVisibleEnd(); ++i) {
					if (keyButtons[i]->mouseClick(x, y)) {
						hotkeyIndex = i;
						hotkeyChar = SDLK_UNKNOWN;
						break;
					}
				}
			}

			if (this->parentUI == NULL) {
				if (buttonKeyboardSetup.mouseClick(x, y)) {
					soundRenderer.playFx(coreData.getClickSoundA());
					return;
				} else if (buttonAudioSection.mouseClick(x, y)) {
					soundRenderer.playFx(coreData.getClickSoundA());
					mainMenu->setState(new MenuStateOptionsSound(program, mainMenu, this->parentUI));   // open keyboard shortcuts setup screen
					return;
				} else if (buttonNetworkSettings.mouseClick(x, y)) {
					soundRenderer.playFx(coreData.getClickSoundA());
					mainMenu->setState(new MenuStateOptionsNetwork(program, mainMenu, this->parentUI)); // open keyboard shortcuts setup screen
					return;
				} else if (buttonMiscSection.mouseClick(x, y)) {
					soundRenderer.playFx(coreData.getClickSoundA());
					mainMenu->setState(new MenuStateOptions(program, mainMenu, this->parentUI));        // open keyboard shortcuts setup screen
					return;
				} else if (buttonVideoSection.mouseClick(x, y)) {
					soundRenderer.playFx(coreData.getClickSoundA());
					mainMenu->setState(new MenuStateOptionsGraphics(program, mainMenu, this->parentUI));        // open keyboard shortcuts setup screen
					return;
				}
			}
		}

		void MenuStateKeysetup::mouseUp(int x, int y,
			const MouseButton mouseButton) {
			if (mouseButton == mbLeft) {
				keyScrollBar.mouseUp(x, y);
			}
		}

		void MenuStateKeysetup::mouseMove(int x, int y, const MouseState * ms) {
			buttonReturn.mouseMove(x, y);
			buttonOk.mouseMove(x, y);
			if (this->parentUI == NULL) {
				buttonKeyboardSetup.mouseMove(x, y);
				buttonAudioSection.mouseMove(x, y);
				buttonNetworkSettings.mouseMove(x, y);
				buttonMiscSection.mouseMove(x, y);
				buttonVideoSection.mouseMove(x, y);
			}
			if (ms->get(mbLeft)) {
				keyScrollBar.mouseDown(x, y);
			} else {
				keyScrollBar.mouseMove(x, y);
			}

			if (keyScrollBar.getElementCount() != 0) {
				for (int i = keyScrollBar.getVisibleStart();
					i <= keyScrollBar.getVisibleEnd(); ++i) {
					keyButtons[i]->mouseMove(x, y);
				}
			}

		}

		void MenuStateKeysetup::render() {
			Renderer & renderer = Renderer::getInstance();

			//printf("MenuStateKeysetup::render A\n");

			if (mainMessageBox.getEnabled()) {
				//printf("MenuStateKeysetup::render B\n");
				renderer.renderMessageBox(&mainMessageBox);
			} else {
				//printf("MenuStateKeysetup::render C\n");
				renderer.renderButton(&buttonReturn);
				renderer.renderButton(&buttonDefaults);
				renderer.renderButton(&buttonOk);
				if (this->parentUI == NULL) {
					renderer.renderButton(&buttonKeyboardSetup);
					renderer.renderButton(&buttonVideoSection);
					renderer.renderButton(&buttonAudioSection);
					renderer.renderButton(&buttonMiscSection);
					renderer.renderButton(&buttonNetworkSettings);
				}
				renderer.renderLabel(&labelTitle);
				renderer.renderLabel(&labelTestTitle);
				renderer.renderLabel(&labelTestValue);

				if (keyScrollBar.getElementCount() != 0) {
					for (int i = keyScrollBar.getVisibleStart();
						i <= keyScrollBar.getVisibleEnd(); ++i) {
						if (hotkeyIndex == i) {
							renderer.renderButton(keyButtons[i], &YELLOW);
						} else {
							renderer.renderButton(keyButtons[i]);
						}
						renderer.renderLabel(labels[i]);
					}
				}
				renderer.renderScrollBar(&keyScrollBar);
			}

			renderer.renderConsole(&console);
			if (program != NULL)
				program->renderProgramMsgBox();
		}

		void MenuStateKeysetup::update() {
			//printf("MenuStateKeysetup::update A\n");

			if (keyScrollBar.getElementCount() != 0) {
				for (int i = keyScrollBar.getVisibleStart(); i
					<= keyScrollBar.getVisibleEnd(); ++i) {
					keyButtons[i]->setY(keyButtonsYBase - keyButtonsLineHeight * (i
						-
						keyScrollBar.
						getVisibleStart
						()));
					labels[i]->setY(keyButtonsYBase -
						keyButtonsLineHeight * (i -
							keyScrollBar.
							getVisibleStart()));
				}
			}

			console.update();
		}



		void MenuStateKeysetup::showMessageBox(const string & text,
			const string & header,
			bool toggle) {
			if (!toggle) {
				mainMessageBox.setEnabled(false);
			}

			if (!mainMessageBox.getEnabled()) {
				mainMessageBox.setText(text);
				mainMessageBox.setHeader(header);
				mainMessageBox.setEnabled(true);
			} else {
				mainMessageBox.setEnabled(false);
			}
		}


		void MenuStateKeysetup::keyDown(SDL_KeyboardEvent key) {
			hotkeyChar = extractKeyPressed(key);
			//printf("\nkeyDown [%d]\n",hotkeyChar);

			string keyName = "";
			//if(hotkeyChar > SDLK_UNKNOWN && hotkeyChar < SDL_NUM_SCANCODES) {
			if (SystemFlags::VERBOSE_MODE_ENABLED)
				printf("In [%s::%s Line: %d] keyName [%s] char [%d][%d]\n", __FILE__,
					__FUNCTION__, __LINE__, keyName.c_str(), hotkeyChar,
					key.keysym.sym);
			keyName = SDL_GetKeyName(hotkeyChar);
			//printf ("In [%s::%s Line: %d] keyName [%s] char [%d][%d]\n",__FILE__,__FUNCTION__,__LINE__,keyName.c_str(),hotkeyChar,key.keysym.sym);
			//}
			//key = hotkeyChar;

			if (SystemFlags::VERBOSE_MODE_ENABLED)
				printf("In [%s::%s Line: %d] keyName [%s] char [%d][%d]\n", __FILE__,
					__FUNCTION__, __LINE__, keyName.c_str(), hotkeyChar,
					key.keysym.sym);

			//      SDLKey keysym = SDLK_UNKNOWN;
			//      if(keyName == "unknown key" || keyName == "") {
			//              Config &configKeys = Config::getInstance(std::pair<ConfigType,ConfigType>(cfgMainKeys,cfgUserKeys));
			//              keysym = configKeys.translateSpecialStringToSDLKey(hotkeyChar);
			//
			//              if(SystemFlags::VERBOSE_MODE_ENABLED) printf ("In [%s::%s Line: %d] keysym [%d]\n",__FILE__,__FUNCTION__,__LINE__,keysym);
			//
			//              // SDL skips capital letters
			//              if(keysym >= 65 && keysym <= 90) {
			//                      keysym = (SDLKey)((int)keysym + 32);
			//              }
			//              //if(keysym < 255) {
			//              //      key = keysym;
			//              //}
			//              keyName = SDL_GetKeyName(keysym);
			//      }

			char szCharText[20] = "";
			snprintf(szCharText, 20, "%c", hotkeyChar);
			char *utfStr = ConvertToUTF8(&szCharText[0]);

			char szBuf[8096] = "";
			snprintf(szBuf, 8096, "  %s [%s][%d][%d][%d]", keyName.c_str(),
				utfStr, key.keysym.sym, hotkeyChar, key.keysym.mod);
			labelTestValue.setText(szBuf);
			//printf ("In [%s::%s Line: %d] szBuf [%s]\n",__FILE__,__FUNCTION__,__LINE__,szBuf);

			delete[]utfStr;

			if (SystemFlags::VERBOSE_MODE_ENABLED)
				printf("In [%s::%s Line: %d] hotkeyChar [%d]\n", __FILE__,
					__FUNCTION__, __LINE__, hotkeyChar);
		}

		void MenuStateKeysetup::keyPress(SDL_KeyboardEvent c) {
		}

		void MenuStateKeysetup::keyUp(SDL_KeyboardEvent key) {
			//Config &configKeys = Config::getInstance(std::pair<ConfigType,ConfigType>(cfgMainKeys,cfgUserKeys));

			if (hotkeyIndex >= 0) {
				if (hotkeyChar != 0) {
					if (SystemFlags::VERBOSE_MODE_ENABLED)
						printf("In [%s::%s Line: %d] char [%d][%d]\n", __FILE__,
							__FUNCTION__, __LINE__, hotkeyChar, key.keysym.sym);

					//string keyName = "";
					//if(hotkeyChar > SDLK_UNKNOWN && hotkeyChar < SDL_NUM_SCANCODES) {

					string keyName = SDL_GetKeyName(key.keysym.sym);
					if (StartsWith(keyName, "Keypad ") == false) {
						keyName = SDL_GetKeyName(hotkeyChar);
						key.keysym.sym = hotkeyChar;
					}
					//}
					//key.keysym.sym = hotkeyChar;

					if (SystemFlags::VERBOSE_MODE_ENABLED)
						printf("In [%s::%s Line: %d] keyName [%s] char [%d][%d]\n",
							__FILE__, __FUNCTION__, __LINE__, keyName.c_str(),
							hotkeyChar, key.keysym.sym);

					//SDLKey keysym = SDLK_UNKNOWN;
		  //                      if(keyName == "unknown key" || keyName == "") {
		  //                              Config &configKeys = Config::getInstance(std::pair<ConfigType,ConfigType>(cfgMainKeys,cfgUserKeys));
		  //                              keysym = configKeys.translateSpecialStringToSDLKey(hotkeyChar);
		  //
		  //                              if(SystemFlags::VERBOSE_MODE_ENABLED) printf ("In [%s::%s Line: %d] keysym [%d]\n",__FILE__,__FUNCTION__,__LINE__,keysym);
		  //
		  //                              // SDL skips capital letters
		  //                              if(keysym >= 65 && keysym <= 90) {
		  //                                      keysym = (SDLKey)((int)keysym + 32);
		  //                              }
		  //                              if(keysym < 255) {
		  //                                      key = keysym;
		  //                              }
		  //                              keyName = SDL_GetKeyName(keysym);
		  //                      }

					if (SystemFlags::VERBOSE_MODE_ENABLED)
						printf("In [%s::%s Line: %d] keyName [%s] char [%d][%d]\n",
							__FILE__, __FUNCTION__, __LINE__, keyName.c_str(),
							hotkeyChar, key.keysym.sym);

					if (keyName != "unknown key") {
						GraphicLabel *label = labels[hotkeyIndex];
						label->setText(keyName);

						pair < string, string > &nameValuePair =
							mergedProperties[hotkeyIndex];

						// Need to distinguish numeric keys to be translated to real keys
						// from these ACTUAL sdl keys so surround in quotes.
						//printf("KeyUp #1 keyName [%s]\n", keyName.c_str());

						if (keyName.size() == 1 && keyName[0] >= '0'
							&& keyName[0] <= '9') {
							keyName = "'" + keyName + "'";
						}
						//printf("KeyUp #2 keyName [%s]\n", keyName.c_str());

						bool isNewUserKeyEntry = true;
						for (int i = 0; i < (int) userProperties.size(); ++i) {
							string hotKeyName = userProperties[i].first;
							if (nameValuePair.first == hotKeyName) {
								//                                              if(keysym <= SDLK_ESCAPE || keysym > 255) {
								//                                                      if(keysym <= SDLK_ESCAPE) {
								//                                                              userProperties[i].second = intToStr(extractKeyPressed(key));
								//                                                      }
								//                                                      else {
								//                                                              userProperties[i].second = keyName;
								//                                                      }
								//                                              }
								//                                              else {
								//                                                      userProperties[i].second = "";
								//                                                      userProperties[i].second.push_back(extractKeyPressed(key));
								//                                              }
								userProperties[i].second = keyName;
								isNewUserKeyEntry = false;
								break;
							}
						}
						if (isNewUserKeyEntry == true) {
							pair < string, string > newNameValuePair = nameValuePair;
							//                                      if(keysym <= SDLK_ESCAPE || keysym > 255) {
							//                                              if(keysym <= SDLK_ESCAPE) {
							//                                                      newNameValuePair.second = intToStr(extractKeyPressed(key));
							//                                              }
							//                                              else {
							//                                                      newNameValuePair.second = keyName;
							//                                              }
							//                                      }
							//                                      else {
							//                                              newNameValuePair.second = extractKeyPressed(key);
							//                                      }
							newNameValuePair.second = keyName;
							userProperties.push_back(newNameValuePair);
						}
					}
				}
				hotkeyIndex = -1;
				hotkeyChar = SDLK_UNKNOWN;
			}
		}


	}
}                               //end namespace
