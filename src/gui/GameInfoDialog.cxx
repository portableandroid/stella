//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2020 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include "Bankswitch.hxx"
#include "Console.hxx"
#include "Cart.hxx"
#include "MouseControl.hxx"
#include "SaveKey.hxx"
#include "EditTextWidget.hxx"
#include "RadioButtonWidget.hxx"
#include "Launcher.hxx"
#include "OSystem.hxx"
#include "CartDetector.hxx"
#include "ControllerDetector.hxx"
#include "Paddles.hxx"
#include "PopUpWidget.hxx"
#include "PropsSet.hxx"
#include "BrowserDialog.hxx"
#include "QuadTariDialog.hxx"
#include "TabWidget.hxx"
#include "TIAConstants.hxx"
#include "Widget.hxx"
#include "Font.hxx"

#include "FrameBuffer.hxx"
#include "TIASurface.hxx"
#include "TIA.hxx"
#include "Switches.hxx"
#include "AudioSettings.hxx"
#include "bspf.hxx"

#include "GameInfoDialog.hxx"

using namespace std;
using namespace BSPF;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GameInfoDialog::GameInfoDialog(
      OSystem& osystem, DialogContainer& parent, const GUI::Font& font,
      GuiObject* boss, int max_w, int max_h)
  : Dialog(osystem, parent, font, "Game properties"),
    CommandSender(boss)
{
  const GUI::Font& ifont = instance().frameBuffer().infoFont();

  const int lineHeight = font.getLineHeight(),
            fontHeight = font.getFontHeight(),
            fontWidth = font.getMaxCharWidth(),
            buttonHeight = font.getLineHeight() * 1.25,
            infoLineHeight = ifont.getLineHeight();
  const int VBORDER = fontHeight / 2;
  const int HBORDER = fontWidth * 1.25;
  const int VGAP = fontHeight / 4;

  WidgetArray wid;

  // Set real dimensions
  setSize(56 * fontWidth + HBORDER * 2,
          _th + VGAP * 3 + lineHeight + 8 * (lineHeight + VGAP) + 1 * (infoLineHeight + VGAP) +
            ifont.getLineHeight() + VGAP + buttonHeight + VBORDER * 2,
          max_w, max_h);

  // The tab widget
  myTab = new TabWidget(this, font, 2, 4 + _th,
                        _w - 2 * 2,
                        _h - _th - VGAP - buttonHeight - VBORDER * 2);

  addTabWidget(myTab);

  //////////////////////////////////////////////////////////////////////////////
  addEmulationTab();
  addConsoleTab();
  addControllersTab();
  addCartridgeTab();
  addHighScoresTab();
  //////////////////////////////////////////////////////////////////////////////

  // Activate the first tab
  myTab->setActiveTab(0);

  // Add Defaults, OK and Cancel buttons
  addDefaultsExtraOKCancelBGroup(wid, font, "Export" + ELLIPSIS, kExportPressed);
  _extraWidget->setToolTip("Export the current ROM's properties\n"
                           "into the default directory.");
  addBGroupToFocusList(wid);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::addEmulationTab()
{
  const GUI::Font& ifont = instance().frameBuffer().infoFont();
  const int lineHeight = _font.getLineHeight(),
    fontHeight = _font.getFontHeight(),
    fontWidth = _font.getMaxCharWidth();
  const int VBORDER = fontHeight / 2;
  const int HBORDER = fontWidth * 1.25;
  const int VGAP = fontHeight / 4;

  int ypos, pwidth, tabID;
  WidgetArray wid;
  VariantList items;
  StaticTextWidget* t;

  // 1) Emulation properties
  tabID = myTab->addTab("Emulation", TabWidget::AUTO_WIDTH);

  ypos = VBORDER;

  t = new StaticTextWidget(myTab, _font, HBORDER, ypos + 1, "Type (*)      ");
  pwidth = _font.getStringWidth("CM (SpectraVideo CompuMate)");
  items.clear();
  for(uInt32 i = 0; i < uInt32(Bankswitch::Type::NumSchemes); ++i)
    VarList::push_back(items, Bankswitch::BSList[i].desc, Bankswitch::BSList[i].name);
  myBSType = new PopUpWidget(myTab, _font, t->getRight() + fontWidth, ypos,
                             pwidth, lineHeight, items);
  wid.push_back(myBSType);
  ypos += lineHeight + VGAP;

  myTypeDetected = new StaticTextWidget(myTab, ifont, t->getRight() + fontWidth, ypos,
                                        "CM (SpectraVideo CompuMate) detected");
  ypos += ifont.getLineHeight() + VGAP;

  // Start bank
  myStartBankLabel = new StaticTextWidget(myTab, _font, HBORDER, ypos + 1, "Start bank (*) ");
  items.clear();
  myStartBank = new PopUpWidget(myTab, _font, myStartBankLabel->getRight(), ypos,
                                _font.getStringWidth("AUTO"), lineHeight, items);
  wid.push_back(myStartBank);
  ypos += lineHeight + VGAP * 4;

  pwidth = _font.getStringWidth("Auto-detect");
  t = new StaticTextWidget(myTab, _font, HBORDER, ypos + 1, "TV format      ");
  items.clear();
  VarList::push_back(items, "Auto-detect", "AUTO");
  VarList::push_back(items, "NTSC", "NTSC");
  VarList::push_back(items, "PAL", "PAL");
  VarList::push_back(items, "SECAM", "SECAM");
  VarList::push_back(items, "NTSC50", "NTSC50");
  VarList::push_back(items, "PAL60", "PAL60");
  VarList::push_back(items, "SECAM60", "SECAM60");
  myFormat = new PopUpWidget(myTab, _font, t->getRight(), ypos,
                             pwidth, lineHeight, items);
  wid.push_back(myFormat);

  myFormatDetected = new StaticTextWidget(myTab, ifont, myFormat->getRight() + fontWidth, ypos + 4,
                                          "SECAM60 detected");

  // Phosphor
  ypos += lineHeight + VGAP;
  myPhosphor = new CheckboxWidget(myTab, _font, HBORDER, ypos + 1,
                                  "Phosphor (enabled for all ROMs)", kPhosphorChanged);
  wid.push_back(myPhosphor);

  ypos += lineHeight + VGAP * 0;
  myPPBlend = new SliderWidget(myTab, _font,
                               HBORDER + fontWidth * 2, ypos,
                               "Blend  ", 0, kPPBlendChanged, 4 * fontWidth, "%");
  myPPBlend->setMinValue(0); myPPBlend->setMaxValue(100);
  myPPBlend->setTickmarkIntervals(2);
  wid.push_back(myPPBlend);

  ypos += lineHeight + VGAP;
  t = new StaticTextWidget(myTab, _font, HBORDER, ypos + 1, "V-Center ");
  myVCenter = new SliderWidget(myTab, _font, t->getRight(), ypos, "",
                               0, kVCenterChanged, 7 * fontWidth, "px", 0, true);

  myVCenter->setMinValue(TIAConstants::minVcenter);
  myVCenter->setMaxValue(TIAConstants::maxVcenter);
  myVCenter->setTickmarkIntervals(4);
  wid.push_back(myVCenter);

  ypos += lineHeight + VGAP * 3;
  mySound = new CheckboxWidget(myTab, _font, HBORDER, ypos + 1, "Stereo sound");
  wid.push_back(mySound);

  // Add message concerning usage
  ypos = myTab->getHeight() - fontHeight - ifont.getFontHeight() - VGAP - VBORDER;
  new StaticTextWidget(myTab, ifont, HBORDER, ypos,
                       "(*) Change requires a ROM reload");

  // Add items for tab 0
  addToFocusList(wid, myTab, tabID);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::addConsoleTab()
{
  const int lineHeight = _font.getLineHeight(),
    fontHeight = _font.getFontHeight(),
    fontWidth = _font.getMaxCharWidth();
  const int VBORDER = fontHeight / 2;
  const int HBORDER = fontWidth * 1.25;
  const int VGAP = fontHeight / 4;

  int xpos, ypos, lwidth, tabID;
  WidgetArray wid;

  // 2) Console properties
  tabID = myTab->addTab(" Console ", TabWidget::AUTO_WIDTH);

  xpos = HBORDER; ypos = VBORDER;
  lwidth = _font.getStringWidth(GUI::RIGHT_DIFFICULTY + " ");

  new StaticTextWidget(myTab, _font, xpos, ypos + 1, "TV type");
  myTVTypeGroup = new RadioButtonGroup();
  RadioButtonWidget* r = new RadioButtonWidget(myTab, _font, xpos + lwidth, ypos + 1,
                                               "Color", myTVTypeGroup);
  wid.push_back(r);
  ypos += lineHeight;
  r = new RadioButtonWidget(myTab, _font, xpos + lwidth, ypos + 1,
                            "B/W", myTVTypeGroup);
  wid.push_back(r);
  ypos += lineHeight + VGAP * 2;

  new StaticTextWidget(myTab, _font, xpos, ypos+1, GUI::LEFT_DIFFICULTY);
  myLeftDiffGroup = new RadioButtonGroup();
  r = new RadioButtonWidget(myTab, _font, xpos + lwidth, ypos + 1,
                            "A (Expert)", myLeftDiffGroup);
  wid.push_back(r);
  ypos += lineHeight;
  r = new RadioButtonWidget(myTab, _font, xpos + lwidth, ypos + 1,
                            "B (Novice)", myLeftDiffGroup);
  wid.push_back(r);
  ypos += lineHeight + VGAP * 2;

  new StaticTextWidget(myTab, _font, xpos, ypos+1, GUI::RIGHT_DIFFICULTY);
  myRightDiffGroup = new RadioButtonGroup();
  r = new RadioButtonWidget(myTab, _font, xpos + lwidth, ypos + 1,
                            "A (Expert)", myRightDiffGroup);
  wid.push_back(r);
  ypos += lineHeight;
  r = new RadioButtonWidget(myTab, _font, xpos + lwidth, ypos + 1,
                            "B (Novice)", myRightDiffGroup);
  wid.push_back(r);

  // Add items for tab 1
  addToFocusList(wid, myTab, tabID);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::addControllersTab()
{
  const GUI::Font& ifont = instance().frameBuffer().infoFont();
  const int lineHeight = _font.getLineHeight(),
    fontHeight = _font.getFontHeight(),
    fontWidth = _font.getMaxCharWidth(),
    buttonHeight = _font.getLineHeight() * 1.25;
  const int VBORDER = fontHeight / 2;
  const int HBORDER = fontWidth * 1.25;
  const int INDENT = fontWidth * 2;
  const int VGAP = fontHeight / 4;

  int xpos, ypos, pwidth, tabID;
  VariantList items, ctrls;
  WidgetArray wid;

  // 3) Controller properties
  wid.clear();
  tabID = myTab->addTab("Controllers", TabWidget::AUTO_WIDTH);

  items.clear();
  VarList::push_back(items, "Auto-detect", "AUTO");
  VarList::push_back(items, "Joystick", "JOYSTICK");
  VarList::push_back(items, "Paddles", "PADDLES");
  VarList::push_back(items, "Paddles_IAxis", "PADDLES_IAXIS");
  VarList::push_back(items, "Paddles_IAxDr", "PADDLES_IAXDR");
  VarList::push_back(items, "BoosterGrip", "BOOSTERGRIP");
  VarList::push_back(items, "Driving", "DRIVING");
  VarList::push_back(items, "Keyboard", "KEYBOARD");
  VarList::push_back(items, "AmigaMouse", "AMIGAMOUSE");
  VarList::push_back(items, "AtariMouse", "ATARIMOUSE");
  VarList::push_back(items, "Trakball", "TRAKBALL");
  VarList::push_back(items, "AtariVox", "ATARIVOX");
  VarList::push_back(items, "SaveKey", "SAVEKEY");
  VarList::push_back(items, "Sega Genesis", "GENESIS");
  VarList::push_back(items, "KidVid", "KIDVID");
  VarList::push_back(items, "Lightgun", "LIGHTGUN");
  VarList::push_back(items, "MindLink", "MINDLINK");
  VarList::push_back(ctrls, "QuadTari", "QUADTARI");

  xpos = HBORDER; ypos = VBORDER;
  pwidth = _font.getStringWidth("Paddles_IAxis");
  myLeftPortLabel = new StaticTextWidget(myTab, _font, HBORDER, ypos+1, "Left port        ");
  myLeftPort = new PopUpWidget(myTab, _font, myLeftPortLabel->getRight(),
                               myLeftPortLabel->getTop()-1,
                               pwidth, lineHeight, items, "", 0, kLeftCChanged);
  wid.push_back(myLeftPort);
  ypos += lineHeight + VGAP;

  myLeftPortDetected = new StaticTextWidget(myTab, ifont, myLeftPort->getLeft(), ypos,
                                            "Sega Genesis detected");
  ypos += ifont.getLineHeight() + VGAP;

  myRightPortLabel = new StaticTextWidget(myTab, _font, HBORDER, ypos+1, "Right port       ");
  myRightPort = new PopUpWidget(myTab, _font, myRightPortLabel->getRight(),
                                myRightPortLabel->getTop()-1,
                                pwidth, lineHeight, items, "", 0, kRightCChanged);
  wid.push_back(myRightPort);

  ypos += lineHeight + VGAP;
  myRightPortDetected = new StaticTextWidget(myTab, ifont, myRightPort->getLeft(), ypos,
                                             "Sega Genesis detected");

  mySwapPorts = new CheckboxWidget(myTab, _font, myLeftPort->getRight() + fontWidth * 4,
                                   myLeftPort->getTop() + 1, "Swap ports");
  wid.push_back(mySwapPorts);

  myQuadTariButton = new ButtonWidget(myTab, _font, myRightPort->getRight() + fontWidth * 4, myRightPort->getTop() - 2,
                                      " QuadTari" + ELLIPSIS + " ", kQuadTariPressed);
  wid.push_back(myQuadTariButton);

  // EEPROM erase button for left/right controller
  ypos += ifont.getLineHeight() + VGAP + 4;
  pwidth = myRightPort->getWidth();   //_font.getStringWidth("Erase EEPROM ") + 23;
  myEraseEEPROMLabel = new StaticTextWidget(myTab, _font, HBORDER, ypos, "AtariVox/SaveKey ");
  myEraseEEPROMButton = new ButtonWidget(myTab, _font, myEraseEEPROMLabel->getRight(), ypos - 4,
                                         pwidth, buttonHeight, "Erase EEPROM", kEEButtonPressed);

  wid.push_back(myEraseEEPROMButton);
  myEraseEEPROMInfo = new StaticTextWidget(myTab, ifont, myEraseEEPROMButton->getRight() + 4,
                                           myEraseEEPROMLabel->getTop() + 3,
                                           "(for this game only)");
  ypos += lineHeight + VGAP * 4;

  mySwapPaddles = new CheckboxWidget(myTab, _font, xpos, ypos, "Swap paddles");
  wid.push_back(mySwapPaddles);
  ypos += lineHeight + VGAP;

  // Paddles
  myPaddlesCenter = new StaticTextWidget(myTab, _font, xpos, ypos, "Paddles center:");
  ypos += lineHeight + VGAP;

  xpos += INDENT;
  myPaddleXCenter = new SliderWidget(myTab, _font, xpos, ypos - 1, "X ", 0, kPXCenterChanged,
                                     fontWidth * 6, "px", 0 ,true);
  myPaddleXCenter->setMinValue(Paddles::MIN_ANALOG_CENTER);
  myPaddleXCenter->setMaxValue(Paddles::MAX_ANALOG_CENTER);
  myPaddleXCenter->setTickmarkIntervals(4);
  wid.push_back(myPaddleXCenter);
  ypos += lineHeight + VGAP;

  myPaddleYCenter = new SliderWidget(myTab, _font, xpos, ypos - 1, "Y ", 0, kPYCenterChanged,
                                     fontWidth * 6, "px", 0 ,true);
  myPaddleYCenter->setMinValue(Paddles::MIN_ANALOG_CENTER);
  myPaddleYCenter->setMaxValue(Paddles::MAX_ANALOG_CENTER);
  myPaddleYCenter->setTickmarkIntervals(4);
  wid.push_back(myPaddleYCenter);

  // Mouse
  xpos = HBORDER + fontWidth * 24 - INDENT;
  ypos = mySwapPaddles->getTop() - 1;
  myMouseControl = new CheckboxWidget(myTab, _font, xpos, ypos + 1, "Specific mouse axes",
                                      kMCtrlChanged);
  wid.push_back(myMouseControl);

  // Mouse controller specific axis
  pwidth = _font.getStringWidth("MindLink 0");
  ctrls.clear();
  VarList::push_back(ctrls, "None",       static_cast<uInt32>(MouseControl::Type::NoControl));
  VarList::push_back(ctrls, "Paddle 0",   static_cast<uInt32>(MouseControl::Type::Paddle0));
  VarList::push_back(ctrls, "Paddle 1",   static_cast<uInt32>(MouseControl::Type::Paddle1));
  VarList::push_back(ctrls, "Paddle 2",   static_cast<uInt32>(MouseControl::Type::Paddle2));
  VarList::push_back(ctrls, "Paddle 3",   static_cast<uInt32>(MouseControl::Type::Paddle3));
  VarList::push_back(ctrls, "Driving 0",  static_cast<uInt32>(MouseControl::Type::Driving0));
  VarList::push_back(ctrls, "Driving 1",  static_cast<uInt32>(MouseControl::Type::Driving1));
  VarList::push_back(ctrls, "MindLink 0", static_cast<uInt32>(MouseControl::Type::MindLink0));
  VarList::push_back(ctrls, "MindLink 1", static_cast<uInt32>(MouseControl::Type::MindLink1));

  xpos += CheckboxWidget::prefixSize(_font);
  ypos += lineHeight + VGAP;
  myMouseX = new PopUpWidget(myTab, _font, xpos, ypos, pwidth, lineHeight, ctrls,
                             "X-Axis is ");
  wid.push_back(myMouseX);

  ypos += lineHeight + VGAP;
  myMouseY = new PopUpWidget(myTab, _font, myMouseX->getLeft(), ypos, pwidth, lineHeight, ctrls,
                             "Y-Axis is ");
  wid.push_back(myMouseY);

  xpos -= CheckboxWidget::prefixSize(_font); ypos += lineHeight + VGAP;
  myMouseRange = new SliderWidget(myTab, _font, xpos, ypos,
                                  "Mouse axes range ", 0, 0, fontWidth * 4, "%");
  myMouseRange->setMinValue(1); myMouseRange->setMaxValue(100);
  myMouseRange->setTickmarkIntervals(4);
  myMouseRange->setToolTip("Adjust paddle range emulated by the mouse.");
  wid.push_back(myMouseRange);

  // Add items for tab 2
  addToFocusList(wid, myTab, tabID);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::addCartridgeTab()
{
  // 4) Cartridge properties
  const int lineHeight = _font.getLineHeight(),
    fontHeight = _font.getFontHeight(),
    fontWidth = _font.getMaxCharWidth();
  const int VBORDER = fontHeight / 2;
  const int HBORDER = fontWidth * 1.25;
  const int VGAP = fontHeight / 4;

  int xpos, ypos, lwidth, fwidth, tabID;
  WidgetArray wid;
  VariantList items;

  wid.clear();
  tabID = myTab->addTab("Cartridge", TabWidget::AUTO_WIDTH);

  xpos = HBORDER; ypos = VBORDER;
  lwidth = _font.getStringWidth("Manufacturer ");
  fwidth = _w - lwidth - HBORDER * 2 - 2;
  new StaticTextWidget(myTab, _font, xpos, ypos + 1, lwidth, fontHeight, "Name");
  myName = new EditTextWidget(myTab, _font, xpos + lwidth, ypos - 1,
                              fwidth, lineHeight, "");
  wid.push_back(myName);

  ypos += lineHeight + VGAP;
  new StaticTextWidget(myTab, _font, xpos, ypos + 1, lwidth, fontHeight, "MD5");
  myMD5 = new EditTextWidget(myTab, _font, xpos + lwidth, ypos - 1,
                             fwidth, lineHeight, "");
  myMD5->setEditable(false);

  ypos += lineHeight + VGAP;
  new StaticTextWidget(myTab, _font, xpos, ypos + 1, lwidth, fontHeight, "Manufacturer");
  myManufacturer = new EditTextWidget(myTab, _font, xpos + lwidth, ypos - 1,
                                      fwidth, lineHeight, "");
  wid.push_back(myManufacturer);

  ypos += lineHeight + VGAP;
  new StaticTextWidget(myTab, _font, xpos, ypos + 1, lwidth, fontHeight,
                       "Model", TextAlign::Left);
  myModelNo = new EditTextWidget(myTab, _font, xpos + lwidth, ypos - 1,
                                 fwidth, lineHeight, "");
  wid.push_back(myModelNo);

  ypos += lineHeight + VGAP;
  new StaticTextWidget(myTab, _font, xpos, ypos + 1, lwidth, fontHeight, "Rarity");
  myRarity = new EditTextWidget(myTab, _font, xpos + lwidth, ypos - 1,
                                fwidth, lineHeight, "");
  wid.push_back(myRarity);

  ypos += lineHeight + VGAP;
  new StaticTextWidget(myTab, _font, xpos, ypos + 1, lwidth, fontHeight, "Note");
  myNote = new EditTextWidget(myTab, _font, xpos + lwidth, ypos - 1,
                              fwidth, lineHeight, "");
  wid.push_back(myNote);

  // Add items for tab 3
  addToFocusList(wid, myTab, tabID);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::addHighScoresTab()
{
  // 4) High Scores properties
  const int lineHeight = _font.getLineHeight(),
    fontHeight = _font.getFontHeight(),
    fontWidth = _font.getMaxCharWidth();
  const int VBORDER = fontHeight / 2;
  const int HBORDER = fontWidth * 1.25;
  const int INDENT = fontWidth * 2;
  const int VGAP = fontHeight / 4;

  int xpos, ypos, lwidth, pwidth, tabID;
  WidgetArray wid;
  VariantList items;

  tabID = myTab->addTab("High Scores", TabWidget::AUTO_WIDTH);

  EditableWidget::TextFilter fAddr = [](char c) {
    return (c >= 'a' && c <= 'f') || (c >= '0' && c <= '9');
  };
  EditableWidget::TextFilter fVars = [](char c) {
    return (c >= '0' && c <= '9');
  };

  EditableWidget::TextFilter fText = [](char c) {
    return (c >= 'a' && c <= 'z') || (c >= ' ' && c < ',') || (c > ',' && c < '@');
  };

  xpos = HBORDER; ypos = VBORDER;
  lwidth = _font.getStringWidth("Variations ");

  myHighScores = new CheckboxWidget(myTab, _font, xpos, ypos + 1, "Enable High Scores",
                                    kHiScoresChanged);

  xpos += CheckboxWidget::prefixSize(_font); ypos += lineHeight + VGAP * 2;

  /*myARMGame = new CheckboxWidget(myTab, _font, xpos, ypos + 1, "read ARM cartridge RAM",
                                 kHiScoresChanged);

  ypos += lineHeight + VGAP;*/

  pwidth = _font.getStringWidth("4"); // popup

  int awidth = EditTextWidget::calcWidth(_font, 4); // addresses
  int vwidth = EditTextWidget::calcWidth(_font, 3); // values
  int swidth = EditTextWidget::calcWidth(_font, HSM::MAX_SPECIAL_NAME); // special
  int fwidth = EditTextWidget::calcWidth(_font, 3); // variants

  myVariationsLabel = new StaticTextWidget(myTab, _font, xpos, ypos + 1, lwidth, fontHeight,
                                           "Variations");
  myVariations = new EditTextWidget(myTab, _font, xpos + lwidth, ypos - 1, fwidth, lineHeight);
  myVariations->setTextFilter(fVars);
  myVariations->setMaxLen(3);
  myVariations->setToolTip("Define the number of game variations.");
  wid.push_back(myVariations);

  myVarAddressLabel = new StaticTextWidget(myTab, _font, myVariations->getRight() + fontWidth * 2,
                                           ypos + 1, "Address ");
  myVarAddress = new EditTextWidget(myTab, _font, myVarAddressLabel->getRight(), ypos - 1, awidth,
                                    lineHeight);
  myVarAddress->setTextFilter(fAddr);
  myVarAddress->setMaxLen(4);
  myVarAddress->setToolTip("Define the address (in hex format) where the variation number "
                           "is stored.");
  wid.push_back(myVarAddress);
  myVarAddressVal = new EditTextWidget(myTab, _font, myVarAddress->getRight() + 2, ypos - 1,
                                       vwidth, lineHeight);
  myVarAddressVal->setEditable(false);

  myVarsBCD = new CheckboxWidget(myTab, _font, myVarAddressVal->getRight() + fontWidth * 2,
                                 ypos + 1, "BCD", kHiScoresChanged);
  myVarsBCD->setToolTip("Check when the variation number is stored as BCD.");
  wid.push_back(myVarsBCD);

  myVarsZeroBased = new CheckboxWidget(myTab, _font, myVarsBCD->getRight() + fontWidth * 2,
                                       ypos + 1, "0-based", kHiScoresChanged);
  myVarsZeroBased->setToolTip("Check when the variation number is stored zero-based.");
  wid.push_back(myVarsZeroBased);

  ypos += lineHeight + VGAP * 3;

  myScoreLabel = new StaticTextWidget(myTab, _font, xpos, ypos + 1, "Score");

  xpos += INDENT; ypos += lineHeight + VGAP;

  vwidth = EditTextWidget::calcWidth(_font, 2); // address values
  items.clear();
  for (uInt32 i = 1; i <= HSM::MAX_SCORE_DIGITS; ++i)
    VarList::push_back(items, std::to_string(i), std::to_string(i));

  myScoreDigitsLabel = new StaticTextWidget(myTab, _font, xpos, ypos + 1, "Digits    ");
  myScoreDigits = new PopUpWidget(myTab, _font, myScoreDigitsLabel->getRight(), ypos, pwidth,
                                  lineHeight, items, "", 0, kHiScoresChanged);
  myScoreDigits->setToolTip("Select the number of score digits displayed.");
  wid.push_back(myScoreDigits);

  items.clear();
  for (uInt32 i = 0; i <= HSM::MAX_SCORE_DIGITS - 3; ++i)
    VarList::push_back(items, std::to_string(i), std::to_string(i));
  pwidth = _font.getStringWidth("0");

  myTrailingZeroesLabel = new StaticTextWidget(myTab, _font, myScoreDigits->getRight() + 30,
                                               ypos + 1, "0-digits ");
  myTrailingZeroes = new PopUpWidget(myTab, _font, myTrailingZeroesLabel->getRight(), ypos,
                                     pwidth, lineHeight,
                                     items, "", 0, kHiScoresChanged);
  myTrailingZeroes->setToolTip("Select the number of trailing score digits which are fixed to 0.");
  wid.push_back(myTrailingZeroes);

  myScoreBCD = new CheckboxWidget(myTab, _font, myVarsBCD->getLeft(), ypos + 1, "BCD",
                                  kHiScoresChanged);
  myScoreBCD->setToolTip("Check when the score is stored as BCD.");
  wid.push_back(myScoreBCD);

  myScoreInvert = new CheckboxWidget(myTab, _font, myScoreBCD->getRight() + fontWidth * 2,
                                     ypos + 1, "Invert");
  myScoreInvert->setToolTip("Check when a lower score (e.g. a timer) is better.");
  wid.push_back(myScoreInvert);

  uInt32 s_xpos = xpos;
  ypos += lineHeight + VGAP;

  myScoreAddressesLabel = new StaticTextWidget(myTab, _font, s_xpos, ypos + 1, "Addresses ");
  s_xpos += myScoreAddressesLabel->getWidth();
  for (uInt32 a = 0; a < HSM::MAX_SCORE_ADDR; ++a)
  {
    myScoreAddress[a] = new EditTextWidget(myTab, _font, s_xpos, ypos - 1, awidth, lineHeight);
    myScoreAddress[a]->setTextFilter(fAddr);
    myScoreAddress[a]->setMaxLen(4);
    myScoreAddress[a]->setToolTip("Define the addresses (in hex format, highest byte first) "
                                  "where the score is stored.");
    wid.push_back(myScoreAddress[a]);
    s_xpos += myScoreAddress[a]->getWidth() + 2;

    myScoreAddressVal[a] = new EditTextWidget(myTab, _font, myScoreAddress[a]->getRight() + 2,
                                              ypos - 1, vwidth, lineHeight);
    myScoreAddressVal[a]->setEditable(false);
    s_xpos += myScoreAddressVal[a]->getWidth() + 16;
  }

  ypos += lineHeight + VGAP * 1;

  myCurrentScoreLabel = new StaticTextWidget(myTab, _font, xpos, ypos + 1, "Current   ");
  myCurrentScore = new StaticTextWidget(myTab, _font, myCurrentScoreLabel->getRight(), ypos + 1,
                                        "12345678");
  myCurrentScore->setToolTip("The score read using the current definitions.");

  xpos -= INDENT; ypos += lineHeight + VGAP * 3;

  vwidth = EditTextWidget::calcWidth(_font, 3); // score values
  mySpecialLabel = new StaticTextWidget(myTab, _font, xpos, ypos + 1, "Special");
  mySpecialName = new EditTextWidget(myTab, _font, mySpecialLabel->getRight() + fontWidth,
                                     ypos - 1, swidth, lineHeight);
  mySpecialName->setTextFilter(fText);
  mySpecialName->setMaxLen(HSM::MAX_SPECIAL_NAME);
  mySpecialName->setToolTip("Define a short label (up to 5 chars) for the optional,\ngame's "
                            "special value (e.g. 'Level', 'Wave', 'Round'" + ELLIPSIS + ")");
  wid.push_back(mySpecialName);

  mySpecialAddressLabel = new StaticTextWidget(myTab, _font, myVarAddressLabel->getLeft(),
                                               ypos + 1, "Address ");
  mySpecialAddress = new EditTextWidget(myTab, _font, mySpecialAddressLabel->getRight(),
                                        ypos - 1, awidth, lineHeight);
  mySpecialAddress->setTextFilter(fAddr);
  mySpecialAddress->setMaxLen(4);
  mySpecialAddress->setToolTip("Define the address (in hex format) where the special "
                               "number is stored.");
  wid.push_back(mySpecialAddress);
  mySpecialAddressVal = new EditTextWidget(myTab, _font, mySpecialAddress->getRight() + 2,
                                           ypos - 1, vwidth, lineHeight);
  mySpecialAddressVal->setEditable(false);

  mySpecialBCD = new CheckboxWidget(myTab, _font, myVarsBCD->getLeft(), ypos + 1, "BCD",
                                    kHiScoresChanged);
  mySpecialBCD->setToolTip("Check when the special number is stored as BCD.");
  wid.push_back(mySpecialBCD);

  mySpecialZeroBased = new CheckboxWidget(myTab, _font, mySpecialBCD->getRight() + fontWidth * 2,
                                          ypos + 1, "0-based", kHiScoresChanged);
  mySpecialZeroBased->setToolTip("Check when the special number is stored zero-based.");
  wid.push_back(mySpecialZeroBased);

  ypos += lineHeight + VGAP * 3;

  myHighScoreNotesLabel = new StaticTextWidget(myTab, _font, xpos, ypos + 1, "Note");
  myHighScoreNotes = new EditTextWidget(myTab, _font, mySpecialName->getLeft(), ypos - 1,
                                        _w - HBORDER - mySpecialName->getLeft() - 2 , lineHeight);
  myHighScoreNotes->setTextFilter(fText);
  myHighScoreNotes->setToolTip("Define some free text which explains the high scores properties.");
  wid.push_back(myHighScoreNotes);

  // Add items for tab 4
  addToFocusList(wid, myTab, tabID);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
GameInfoDialog::~GameInfoDialog()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::createBrowser(const string& title)
{
  uInt32 w = 0, h = 0;
  getDynamicBounds(w, h);
  if(w > uInt32(_font.getMaxCharWidth() * 80))
    w = _font.getMaxCharWidth() * 80;

  // Create file browser dialog
  if(!myBrowser || uInt32(myBrowser->getWidth()) != w ||
     uInt32(myBrowser->getHeight()) != h)
    myBrowser = make_unique<BrowserDialog>(this, _font, w, h, title);
  else
    myBrowser->setTitle(title);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::loadConfig()
{
  if(instance().hasConsole())
  {
    myGameProperties = instance().console().properties();
    myGameFile = instance().romFile();
  }
  else
  {
    const string& md5 = instance().launcher().selectedRomMD5();
    instance().propSet().getMD5(md5, myGameProperties);
    myGameFile = FilesystemNode(instance().launcher().selectedRom());
  }

  loadEmulationProperties(myGameProperties);
  loadConsoleProperties(myGameProperties);
  loadControllerProperties(myGameProperties);
  loadCartridgeProperties(myGameProperties);
  loadHighScoresProperties(myGameProperties);

  myTab->loadConfig();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::loadEmulationProperties(const Properties& props)
{
  string bsDetected = "";

  myBSType->setSelected(props.get(PropType::Cart_Type), "AUTO");
  if(myBSType->getSelectedTag().toString() == "AUTO")
  {
    if(instance().hasConsole())
    {
      string bs = instance().console().about().BankSwitch;
      size_t pos = bs.find_first_of('*');
      // remove '*':
      if(pos != string::npos)
        bs = bs.substr(0, pos) + bs.substr(pos + 1);
      bsDetected = bs + "detected";
    }
    else
    {
      const FilesystemNode& node = FilesystemNode(instance().launcher().selectedRom());
      ByteBuffer image;
      string md5 = props.get(PropType::Cart_MD5);
      size_t size = 0;

      // try to load the image for auto detection
      if(!instance().hasConsole() &&
        node.exists() && !node.isDirectory() && (image = instance().openROM(node, md5, size)) != nullptr)
      {
        bsDetected = Bankswitch::typeToDesc(CartDetector::autodetectType(image, size)) + " detected";
      }
    }
  }
  myTypeDetected->setLabel(bsDetected);

  // Start bank
  VariantList items;

  VarList::push_back(items, "Auto", "AUTO");
  if(instance().hasConsole())
  {
    uInt16 numBanks = instance().console().cartridge().romBankCount();

    for(uInt16 i = 0; i < numBanks; ++i)
      VarList::push_back(items, i, i);
    myStartBank->setEnabled(true);
  }
  else
  {
    const string& startBank = props.get(PropType::Cart_StartBank);

    VarList::push_back(items, startBank, startBank);
    myStartBank->setEnabled(false);
  }
  myStartBank->addItems(items);
  myStartBank->setSelected(props.get(PropType::Cart_StartBank), "AUTO");

  myFormat->setSelected(props.get(PropType::Display_Format), "AUTO");
  if(instance().hasConsole() && myFormat->getSelectedTag().toString() == "AUTO")
  {
    const string& format = instance().console().about().DisplayFormat;
    string label;
    if (format.at(format.length() - 1) == '*')
      label = format.substr(0, format.length() - 1);
    else
      label = format;
    myFormatDetected->setLabel(label + " detected");
  }
  else
    myFormatDetected->setLabel("");

  // if phosphor is always enabled, disable game specific phosphor settings
  bool alwaysPhosphor = instance().settings().getString("tv.phosphor") == "always";
  bool usePhosphor = props.get(PropType::Display_Phosphor) == "YES";
  myPhosphor->setState(usePhosphor);
  myPhosphor->setEnabled(!alwaysPhosphor);
  if (alwaysPhosphor)
    myPhosphor->setLabel("Phosphor (enabled for all ROMs)");
  else
    myPhosphor->setLabel("Phosphor");
  myPPBlend->setEnabled(!alwaysPhosphor && usePhosphor);

  const string& blend = props.get(PropType::Display_PPBlend);
  myPPBlend->setValue(stringToInt(blend));

  // set vertical center
  Int32 vcenter = stringToInt(props.get(PropType::Display_VCenter));
  myVCenter->setValueLabel(vcenter);
  myVCenter->setValue(vcenter);
  myVCenter->setValueUnit(vcenter ? "px" : "");

  mySound->setState(props.get(PropType::Cart_Sound) == "STEREO");
  // if stereo is always enabled, disable game specific stereo setting
  mySound->setEnabled(!instance().audioSettings().stereo());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::loadConsoleProperties(const Properties& props)
{
  myLeftDiffGroup->setSelected(props.get(PropType::Console_LeftDiff) == "A" ? 0 : 1);
  myRightDiffGroup->setSelected(props.get(PropType::Console_RightDiff) == "A" ? 0 : 1);
  myTVTypeGroup->setSelected(props.get(PropType::Console_TVType) == "BW" ? 1 : 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::loadControllerProperties(const Properties& props)
{
  string controller = props.get(PropType::Controller_Left);
  myLeftPort->setSelected(controller, "AUTO");
  controller = props.get(PropType::Controller_Right);
  myRightPort->setSelected(controller, "AUTO");

  mySwapPorts->setState(props.get(PropType::Console_SwapPorts) == "YES");
  mySwapPaddles->setState(props.get(PropType::Controller_SwapPaddles) == "YES");

  // Paddle centers
  myPaddleXCenter->setValue(BSPF::stringToInt(props.get(PropType::Controller_PaddlesXCenter)));
  myPaddleYCenter->setValue(BSPF::stringToInt(props.get(PropType::Controller_PaddlesYCenter)));

  // MouseAxis property (potentially contains 'range' information)
  istringstream m_axis(props.get(PropType::Controller_MouseAxis));
  string m_control, m_range;
  m_axis >> m_control;
  bool autoAxis = equalsIgnoreCase(m_control, "AUTO");
  myMouseControl->setState(!autoAxis);
  if(autoAxis)
  {
    myMouseX->setSelectedIndex(0);
    myMouseY->setSelectedIndex(0);
  }
  else
  {
    myMouseX->setSelected(m_control[0] - '0');
    myMouseY->setSelected(m_control[1] - '0');
  }
  myMouseX->setEnabled(!autoAxis);
  myMouseY->setEnabled(!autoAxis);
  if(m_axis >> m_range)
  {
    myMouseRange->setValue(stringToInt(m_range));
  }
  else
  {
    myMouseRange->setValue(100);
  }

  updateControllerStates();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::loadCartridgeProperties(const Properties& props)
{
  myName->setText(props.get(PropType::Cart_Name));
  myMD5->setText(props.get(PropType::Cart_MD5));
  myManufacturer->setText(props.get(PropType::Cart_Manufacturer));
  myModelNo->setText(props.get(PropType::Cart_ModelNo));
  myRarity->setText(props.get(PropType::Cart_Rarity));
  myNote->setText(props.get(PropType::Cart_Note));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::loadHighScoresProperties(const Properties& props)
{
  HSM::ScoresProps info;
  uInt32 numVariations;
  bool enable = instance().highScores().get(props, numVariations, info);

  myHighScores->setState(enable);

  myVariations->setText(to_string(numVariations));

  ostringstream ss;

  myScoreDigits->setSelected(info.numDigits);
  myTrailingZeroes->setSelected(info.trailingZeroes);
  myScoreBCD->setState(info.scoreBCD);
  myScoreInvert->setState(info.scoreInvert);
  myVarsBCD->setState(info.varsBCD);
  myVarsZeroBased->setState(info.varsZeroBased);
  mySpecialName->setText(info.special);
  mySpecialBCD->setState(info.specialBCD);
  mySpecialZeroBased->setState(info.specialZeroBased);

  myHighScoreNotes->setText(info.notes);

  ss.str("");
  ss << hex << right // << setw(HSM::MAX_ADDR_CHARS) << setfill(' ')
    << uppercase << info.varsAddr;
  myVarAddress->setText(ss.str());

  ss.str("");
  ss << hex << right // << setw(HSM::MAX_ADDR_CHARS) << setfill(' ')
    << uppercase << info.specialAddr;
  mySpecialAddress->setText(ss.str());


  for (uInt32 a = 0; a < HSM::MAX_SCORE_ADDR; ++a)
  {
    ss.str("");
    if(a < instance().highScores().numAddrBytes(info.numDigits, info.trailingZeroes))
    {
      ss << hex << right // << setw(HSM::MAX_ADDR_CHARS) << setfill(' ')
        << uppercase << info.scoreAddr[a];
    }
    myScoreAddress[a]->setText(ss.str());
  }
  updateHighScoresWidgets();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::saveProperties()
{
  // Emulation properties
  myGameProperties.set(PropType::Cart_Type, myBSType->getSelectedTag().toString());
  myGameProperties.set(PropType::Cart_StartBank, myStartBank->getSelectedTag().toString());
  myGameProperties.set(PropType::Display_Format, myFormat->getSelectedTag().toString());
  myGameProperties.set(PropType::Display_Phosphor, myPhosphor->getState() ? "YES" : "NO");
  myGameProperties.set(PropType::Display_PPBlend, myPPBlend->getValueLabel() == "Off" ? "0" :
                       myPPBlend->getValueLabel());
  Int32 vcenter = myVCenter->getValue();

  myGameProperties.set(PropType::Display_VCenter, to_string(vcenter));
  myGameProperties.set(PropType::Cart_Sound, mySound->getState() ? "STEREO" : "MONO");

  // Console properties
  myGameProperties.set(PropType::Console_TVType, myTVTypeGroup->getSelected() ? "BW" : "COLOR");
  myGameProperties.set(PropType::Console_LeftDiff, myLeftDiffGroup->getSelected() ? "B" : "A");
  myGameProperties.set(PropType::Console_RightDiff, myRightDiffGroup->getSelected() ? "B" : "A");

  // Controller properties
  string controller = myLeftPort->getSelectedTag().toString();
  myGameProperties.set(PropType::Controller_Left, controller);
  if(controller != "AUTO" && controller != "QUADTARI")
  {
    myGameProperties.set(PropType::Controller_Left1, "");
    myGameProperties.set(PropType::Controller_Left2, "");
  }

  controller = myRightPort->getSelectedTag().toString();
  myGameProperties.set(PropType::Controller_Right, controller);
  if(controller != "AUTO" && controller != "QUADTARI")
  {
    myGameProperties.set(PropType::Controller_Right1, "");
    myGameProperties.set(PropType::Controller_Right2, "");
  }

  myGameProperties.set(PropType::Console_SwapPorts, (mySwapPorts->isEnabled() && mySwapPorts->getState()) ? "YES" : "NO");
  myGameProperties.set(PropType::Controller_SwapPaddles, mySwapPaddles->getState() ? "YES" : "NO");

  // Paddle center
  myGameProperties.set(PropType::Controller_PaddlesXCenter, std::to_string(myPaddleXCenter->getValue()));
  myGameProperties.set(PropType::Controller_PaddlesYCenter, std::to_string(myPaddleYCenter->getValue()));

  // MouseAxis property (potentially contains 'range' information)
  string mcontrol = "AUTO";
  if(myMouseControl->getState())
    mcontrol = myMouseX->getSelectedTag().toString() +
               myMouseY->getSelectedTag().toString();
  string range = myMouseRange->getValueLabel();
  if(range != "100")
    mcontrol += " " + range;
  myGameProperties.set(PropType::Controller_MouseAxis, mcontrol);

  // Cartridge properties
  myGameProperties.set(PropType::Cart_Name, myName->getText());
  myGameProperties.set(PropType::Cart_Manufacturer, myManufacturer->getText());
  myGameProperties.set(PropType::Cart_ModelNo, myModelNo->getText());
  myGameProperties.set(PropType::Cart_Rarity, myRarity->getText());
  myGameProperties.set(PropType::Cart_Note, myNote->getText());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::saveConfig()
{
  saveProperties();

  saveHighScoresProperties();

  // Always insert; if the properties are already present, nothing will happen
  instance().propSet().insert(myGameProperties);
  instance().saveConfig();

  // In any event, inform the Console
  if(instance().hasConsole())
  {
    instance().console().setProperties(myGameProperties);

    // update 'Emulation' tab settings immediately
    instance().console().setFormat(myFormat->getSelected());
    instance().frameBuffer().tiaSurface().enablePhosphor(myPhosphor->getState(), myPPBlend->getValue());
    instance().console().updateVcenter(myVCenter->getValue());
    instance().console().initializeAudio();

    // update 'Console' tab settings immediately
    instance().console().switches().setTvColor(myTVTypeGroup->getSelected() == 0);
    instance().console().switches().setLeftDifficultyA(myLeftDiffGroup->getSelected() == 0);
    instance().console().switches().setRightDifficultyA(myRightDiffGroup->getSelected() == 0);

    // update 'Controllers' tab settings immediately
    instance().console().setControllers(myGameProperties.get(PropType::Cart_MD5));

    Paddles::setAnalogXCenter(myPaddleXCenter->getValue());
    Paddles::setAnalogYCenter(myPaddleYCenter->getValue());
    Paddles::setDigitalPaddleRange(myMouseRange->getValue());
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::saveHighScoresProperties()
{
  HSM::ScoresProps info;

  if (myHighScores->getState())
  {
    string strText;

    // limit variants and special size
    strText = myVariations->getText();
    strText = strText.substr(0, 3);
    myVariations->setText(strText);

    strText = mySpecialName->getText();
    strText = strText.substr(0, HSM::MAX_SPECIAL_NAME);
    mySpecialName->setText(strText);

    // fill format
    info.varsZeroBased = myVarsZeroBased->getState();
    info.varsBCD = myVarsBCD->getState();

    info.numDigits = myScoreDigits->getSelected() + 1;
    info.trailingZeroes = myTrailingZeroes->getSelected();
    info.scoreBCD = myScoreBCD->getState();
    info.scoreInvert = myScoreInvert->getState();

    info.special = mySpecialName->getText();
    info.specialZeroBased = mySpecialZeroBased->getState();
    info.specialBCD = mySpecialBCD->getState();

    info.notes = myHighScoreNotes->getText();

    // fill addresses
    string strAddr;

    strAddr = myVarAddress->getText();
    info.varsAddr = stringToIntBase16(strAddr, HSM::DEFAULT_ADDRESS);
    strAddr = mySpecialAddress->getText();
    info.specialAddr = stringToIntBase16(strAddr, HSM::DEFAULT_ADDRESS);

    for (uInt32 a = 0; a < HSM::MAX_SCORE_ADDR; ++a)
    {
      strAddr = myScoreAddress[a]->getText();
      info.scoreAddr[a] = stringToIntBase16(strAddr, HSM::DEFAULT_ADDRESS);
    }

    string strVars = myVariations->getText();

    instance().highScores().set(myGameProperties, stringToInt(strVars, HSM::DEFAULT_VARIATION),
                                info);
  }
  else
  {
    myGameProperties.reset(PropType::Cart_Highscore);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::setDefaults()
{
  // Load the default properties
  Properties defaultProperties;
  const string& md5 = myGameProperties.get(PropType::Cart_MD5);

  instance().propSet().getMD5(md5, defaultProperties, true);

  switch(myTab->getActiveTab())
  {
    case 0: // Emulation properties
      loadEmulationProperties(defaultProperties);
      break;

    case 1: // Console properties
      loadConsoleProperties(defaultProperties);
      break;

    case 2: // Controller properties
      loadControllerProperties(defaultProperties);
      break;

    case 3: // Cartridge properties
      loadCartridgeProperties(defaultProperties);
      break;

    case 4: // High Scores properties
      loadHighScoresProperties(defaultProperties);
      break;

    default: // make the compiler happy
      break;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::updateControllerStates()
{
  bool swapPorts = mySwapPorts->getState();
  bool autoDetect = false;
  ByteBuffer image;
  string md5 = myGameProperties.get(PropType::Cart_MD5);
  size_t size = 0;

  // try to load the image for auto detection
  if(!instance().hasConsole())
  {
    const FilesystemNode& node = FilesystemNode(instance().launcher().selectedRom());

    autoDetect = node.exists() && !node.isDirectory() && (image = instance().openROM(node, md5, size)) != nullptr;
  }
  string label = "";
  Controller::Type type = Controller::getType(myLeftPort->getSelectedTag().toString());

  if(type == Controller::Type::Unknown)
  {
    if(instance().hasConsole())
    {
      label = (!swapPorts ? instance().console().leftController().name()
               : instance().console().rightController().name()) + " detected";
      if(BSPF::startsWithIgnoreCase(label, "QUADTARI"))
        label = "QuadTari detected"; // remove plugged-in controller names
    }
    else if(autoDetect)
      label = ControllerDetector::detectName(image, size, type,
                                             !swapPorts ? Controller::Jack::Left : Controller::Jack::Right,
                                             instance().settings()) + " detected";
  }
  myLeftPortDetected->setLabel(label);

  label = "";
  type = Controller::getType(myRightPort->getSelectedTag().toString());

  if(type == Controller::Type::Unknown)
  {
    if(instance().hasConsole())
    {
      label = (!swapPorts ? instance().console().rightController().name()
               : instance().console().leftController().name()) + " detected";
      if(BSPF::startsWithIgnoreCase(label, "QUADTARI"))
        label = "QuadTari detected"; // remove plugged-in controller names
    }
    else if(autoDetect)
      label = ControllerDetector::detectName(image, size, type,
                                             !swapPorts ? Controller::Jack::Right : Controller::Jack::Left,
                                             instance().settings()) + " detected";
  }
  myRightPortDetected->setLabel(label);

  const string& contrLeft = myLeftPort->getSelectedTag().toString();
  const string& contrRight = myRightPort->getSelectedTag().toString();
  bool enableEEEraseButton = false;

  // Compumate bankswitching scheme doesn't allow to select controllers
  bool enableSelectControl = myBSType->getSelectedTag() != "CM";
  // Enable Swap Paddles checkbox only for paddle games
  bool enablePaddles = BSPF::startsWithIgnoreCase(contrLeft, "PADDLES") ||
    BSPF::startsWithIgnoreCase(contrRight, "PADDLES") ||
    BSPF::startsWithIgnoreCase(myLeftPortDetected->getLabel(), "Paddles") ||
    BSPF::startsWithIgnoreCase(myRightPortDetected->getLabel(), "Paddles");

  if(instance().hasConsole())
  {
    const Controller& lport = instance().console().leftController();
    const Controller& rport = instance().console().rightController();

    // we only enable the button if we have a valid previous and new controller.
    bool enableBtnForLeft =
      (contrLeft == "AUTO" || contrLeft == "SAVEKEY" || contrLeft == "ATARIVOX") &&
      (lport.type() == Controller::Type::SaveKey || lport.type() == Controller::Type::AtariVox);
    bool enableBtnForRight =
      (contrRight == "AUTO" || contrRight == "SAVEKEY" || contrRight == "ATARIVOX") &&
      (rport.type() == Controller::Type::SaveKey || rport.type() == Controller::Type::AtariVox);
    enableEEEraseButton = enableBtnForLeft || enableBtnForRight;
  }

  myLeftPortLabel->setEnabled(enableSelectControl);
  myRightPortLabel->setEnabled(enableSelectControl);
  myLeftPort->setEnabled(enableSelectControl);
  myRightPort->setEnabled(enableSelectControl);
  myQuadTariButton->setEnabled(BSPF::startsWithIgnoreCase(contrLeft, "QUADTARI") ||
                               BSPF::startsWithIgnoreCase(contrRight, "QUADTARI") ||
                               BSPF::startsWithIgnoreCase(myLeftPortDetected->getLabel(), "QUADTARI") ||
                               BSPF::startsWithIgnoreCase(myRightPortDetected->getLabel(), "QUADTARI"));

  mySwapPorts->setEnabled(enableSelectControl);
  mySwapPaddles->setEnabled(enablePaddles);

  myEraseEEPROMLabel->setEnabled(enableEEEraseButton);
  myEraseEEPROMButton->setEnabled(enableEEEraseButton);
  myEraseEEPROMInfo->setEnabled(enableEEEraseButton);

  myPaddlesCenter->setEnabled(enablePaddles);
  myPaddleXCenter->setEnabled(enablePaddles);
  myPaddleYCenter->setEnabled(enablePaddles);

  bool enableMouse = enablePaddles ||
    BSPF::startsWithIgnoreCase(contrLeft, "Driving") ||
    BSPF::startsWithIgnoreCase(contrRight, "Driving") ||
    BSPF::startsWithIgnoreCase(contrLeft, "MindLink") ||
    BSPF::startsWithIgnoreCase(contrRight, "MindLink");

  myMouseControl->setEnabled(enableMouse);
  myMouseX->setEnabled(enableMouse && myMouseControl->getState());
  myMouseY->setEnabled(enableMouse && myMouseControl->getState());

  myMouseRange->setEnabled(enablePaddles);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::eraseEEPROM()
{
  Controller& lport = instance().console().leftController();
  Controller& rport = instance().console().rightController();

  if(lport.type() == Controller::Type::SaveKey ||
     lport.type() == Controller::Type::AtariVox)
  {
    SaveKey& skey = static_cast<SaveKey&>(lport);
    skey.eraseCurrent();
  }

  if(rport.type() == Controller::Type::SaveKey ||
     rport.type() == Controller::Type::AtariVox)
  {
    SaveKey& skey = static_cast<SaveKey&>(rport);
    skey.eraseCurrent();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::updateHighScoresWidgets()
{
  bool enable = myHighScores->getState();
  bool enableVars = enable && myVariations->getText() > "1";
  bool enableSpecial = enable && !mySpecialName->getText().empty();
  bool enableConsole = instance().hasConsole();
  uInt32 numAddr = instance().highScores().numAddrBytes(myScoreDigits->getSelected() + 1,
                                                        myTrailingZeroes->getSelected());

  // enable widgets
  //myARMGame->setEnabled(enable);
  myVariationsLabel->setEnabled(enable);
  myVariations->setEnabled(enable);
  myVariations->setEditable(enable);
  myVarAddressLabel->setEnabled(enableVars);
  myVarAddress->setEnabled(enableVars);
  myVarAddress->setEditable(enableVars);
  myVarAddressVal->setEnabled(enableVars && enableConsole);
  myVarsBCD->setEnabled(enableVars && stringToInt(myVariations->getText(), 1) >= 10);
  myVarsZeroBased->setEnabled(enableVars);

  myScoreLabel->setEnabled(enable);
  myScoreDigitsLabel->setEnabled(enable);
  myScoreDigits->setEnabled(enable);
  myScoreBCD->setEnabled(enable);
  myTrailingZeroesLabel->setEnabled(enable);
  myTrailingZeroes->setEnabled(enable);
  myScoreInvert->setEnabled(enable);

  myScoreAddressesLabel->setEnabled(enable);

  for(uInt32 a = 0; a < HSM::MAX_SCORE_ADDR; ++a)
  {
    myScoreAddress[a]->setEnabled(enable && numAddr > a);
    myScoreAddressVal[a]->setEnabled(enable && numAddr > a&& enableConsole);
  }

  myCurrentScoreLabel->setEnabled(enable && enableConsole);
  myCurrentScore->setEnabled(enable && enableConsole);

  mySpecialLabel->setEnabled(enable);
  mySpecialName->setEnabled(enable);
  mySpecialName->setEditable(enable);
  mySpecialAddressLabel->setEnabled(enableSpecial);
  mySpecialAddress->setEnabled(enableSpecial);
  mySpecialAddress->setEditable(enableSpecial);
  mySpecialAddressVal->setEnabled(enableSpecial && enableConsole);
  mySpecialBCD->setEnabled(enableSpecial);
  mySpecialZeroBased->setEnabled(enableSpecial);

  myHighScoreNotesLabel->setEnabled(enable);
  myHighScoreNotes->setEnabled(enable);

  // verify and update widget data

  // update variations RAM value
  setAddressVal(myVarAddress, myVarAddressVal, myVarsBCD->getState(),
                myVarsZeroBased->getState(), stringToInt(myVariations->getText(), 1));

  setAddressVal(mySpecialAddress, mySpecialAddressVal, mySpecialBCD->getState(),
                mySpecialZeroBased->getState());

  // update score RAM values and resulting scores
  HSM::ScoreAddresses scoreAddr;

  for(uInt32 a = 0; a < HSM::MAX_SCORE_ADDR; ++a)
  {
    if(a < numAddr)
    {
      setAddressVal(myScoreAddress[a], myScoreAddressVal[a]);
      string strAddr = myScoreAddress[a]->getText();
      scoreAddr[a] = stringToIntBase16(strAddr, HSM::DEFAULT_ADDRESS);
    }
    else
      myScoreAddressVal[a]->setText("");
  }

  Int32 score = instance().highScores().score(numAddr, myTrailingZeroes->getSelected(),
                                              myScoreBCD->getState(), scoreAddr);

  myCurrentScore->setLabel(instance().highScores().formattedScore(score));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::setAddressVal(EditTextWidget* addressWidget, EditTextWidget* valWidget,
                                   bool isBCD, bool zeroBased, uInt8 maxVal)
{
  string strAddr;

  // limit address size
  strAddr = addressWidget->getText();
  strAddr = strAddr.substr(0, HSM::MAX_ADDR_CHARS);

  if (instance().hasConsole() && valWidget->isEnabled())
  {
    uInt16 addr;
    uInt8 val;
    ostringstream ss;

    // convert to number and read from memory
    addr = stringToIntBase16(strAddr, HSM::DEFAULT_ADDRESS);
    val = instance().highScores().peek(addr);
    val = instance().highScores().convert(val, maxVal, isBCD, zeroBased);

    // format output and display in value widget
    // if (isBCD)
    //  ss << hex;
    ss << right // << setw(2) << setfill(' ')
      << uppercase << uInt16(val);
    valWidget->setText(ss.str());
  }
  else
    valWidget->setText("");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::exportCurrentPropertiesToDisk()
{
  saveProperties();
  stringstream out;
  out << myGameProperties;

  try
  {
    FilesystemNode propfile(myBrowser->getResult().getShortPath());

    propfile.write(out);
    instance().frameBuffer().showTextMessage("ROM properties exported");
  }
  catch(...)
  {
    instance().frameBuffer().showTextMessage("Error exporting ROM properties");
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void GameInfoDialog::handleCommand(CommandSender* sender, int cmd,
                                   int data, int id)
{
  switch(cmd)
  {
    case GuiObject::kOKCmd:
      saveConfig();
      close();
      break;

    case GuiObject::kDefaultsCmd:
      setDefaults();
      break;

    case kExportPressed:
      // This dialog is resizable under certain conditions, so we need
      // to re-create it as necessary
      createBrowser("Export properties as");

      myBrowser->show(instance().userDir().getPath() + myGameFile.getNameWithExt(".pro"),
        BrowserDialog::FileSave, kExportChosen);
      break;

    case kExportChosen:
      exportCurrentPropertiesToDisk();
      break;

    case TabWidget::kTabChangedCmd:
      if(data == 2)  // 'Controllers' tab selected
        updateControllerStates();

      // The underlying dialog still needs access to this command
      Dialog::handleCommand(sender, cmd, data, 0);
      break;

    case kLeftCChanged:
    case kRightCChanged:
      updateControllerStates();
      break;

    case kQuadTariPressed:
    {
      bool enableLeft =
        BSPF::startsWithIgnoreCase(myLeftPort->getSelectedTag().toString(), "QUADTARI") ||
        BSPF::startsWithIgnoreCase(myLeftPortDetected->getLabel(), "QUADTARI");
      bool enableRight =
        BSPF::startsWithIgnoreCase(myRightPort->getSelectedTag().toString(), "QUADTARI") ||
        BSPF::startsWithIgnoreCase(myRightPortDetected->getLabel(), "QUADTARI");

      if(!myQuadTariDialog)
        myQuadTariDialog = make_unique<QuadTariDialog>
          (this, _font, _font.getMaxCharWidth() * 37, _font.getFontHeight() * 8,
           myGameProperties);
      myQuadTariDialog->show(enableLeft, enableRight);
      break;
    }
    case kEEButtonPressed:
      eraseEEPROM();
      break;

    case kPhosphorChanged:
    {
      bool status = myPhosphor->getState();
      myPPBlend->setEnabled(status);
      break;
    }

    case kPPBlendChanged:
      if(myPPBlend->getValue() == 0)
      {
        myPPBlend->setValueLabel("Off");
        myPPBlend->setValueUnit("");
      }
      else
        myPPBlend->setValueUnit("%");
      break;

    case kVCenterChanged:
      if (myVCenter->getValue() == 0)
      {
        myVCenter->setValueLabel("Default");
        myVCenter->setValueUnit("");
      }
      else
        myVCenter->setValueUnit("px");
      break;

    case kPXCenterChanged:
      myPaddleXCenter->setValueLabel(myPaddleXCenter->getValue() * 5);
      break;

    case kPYCenterChanged:
      myPaddleYCenter->setValueLabel(myPaddleYCenter->getValue() * 5);
      break;

    case kMCtrlChanged:
    {
      bool state = myMouseControl->getState();
      myMouseX->setEnabled(state);
      myMouseY->setEnabled(state);
      break;
    }

    case EditTextWidget::kChangedCmd:
    case kHiScoresChanged:
      updateHighScoresWidgets();
      break;

    default:
      Dialog::handleCommand(sender, cmd, data, 0);
      break;
  }
}
