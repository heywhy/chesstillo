#include <format>

#include <ftxui/component/event.hpp>
#include <gtest/gtest.h>

#include <tui/constants.hpp>
#include <tui/mapping.hpp>

using namespace tui;

#define SCAN(tokens, input)          \
  {                                  \
    mapping::Scanner scanner(input); \
    tokens = scanner.Scan();         \
  }

TEST(TUIMappingTestSuite, TestReplaceSpecialKeys) {
  Mapping mapping;

  ASSERT_EQ(mapping::ReplaceTermcodes("<UP>"), ftxui::Event::ArrowUp.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<down>"),
            ftxui::Event::ArrowDown.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<left>"),
            ftxui::Event::ArrowLeft.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<Right>"),
            ftxui::Event::ArrowRight.input());

  ASSERT_EQ(mapping::ReplaceTermcodes("<esc>"), ftxui::Event::Escape.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<space>"), kSpaceChar);

  ASSERT_EQ(mapping::ReplaceTermcodes("<s-a>"), ftxui::Event::A.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-b>"), ftxui::Event::B.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-c>"), ftxui::Event::C.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-d>"), ftxui::Event::D.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-e>"), ftxui::Event::E.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-f>"), ftxui::Event::F.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-g>"), ftxui::Event::G.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-h>"), ftxui::Event::H.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-i>"), ftxui::Event::I.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-j>"), ftxui::Event::J.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-k>"), ftxui::Event::K.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-l>"), ftxui::Event::L.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-m>"), ftxui::Event::M.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-n>"), ftxui::Event::N.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-o>"), ftxui::Event::O.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-p>"), ftxui::Event::P.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-q>"), ftxui::Event::Q.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-r>"), ftxui::Event::R.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-s>"), ftxui::Event::S.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-t>"), ftxui::Event::T.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-u>"), ftxui::Event::U.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-v>"), ftxui::Event::V.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-w>"), ftxui::Event::W.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-x>"), ftxui::Event::X.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-y>"), ftxui::Event::Y.character());
  ASSERT_EQ(mapping::ReplaceTermcodes("<s-z>"), ftxui::Event::Z.character());

  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a>"), ftxui::Event::CtrlA.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-b>"), ftxui::Event::CtrlB.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-c>"), ftxui::Event::CtrlC.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-d>"), ftxui::Event::CtrlD.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-e>"), ftxui::Event::CtrlE.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-f>"), ftxui::Event::CtrlF.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-g>"), ftxui::Event::CtrlG.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-h>"), ftxui::Event::CtrlH.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-i>"), ftxui::Event::CtrlI.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-j>"), ftxui::Event::CtrlJ.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-k>"), ftxui::Event::CtrlK.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-l>"), ftxui::Event::CtrlL.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-m>"), ftxui::Event::CtrlM.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-n>"), ftxui::Event::CtrlN.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-o>"), ftxui::Event::CtrlO.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-p>"), ftxui::Event::CtrlP.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-q>"), ftxui::Event::CtrlQ.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-r>"), ftxui::Event::CtrlR.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s>"), ftxui::Event::CtrlS.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-t>"), ftxui::Event::CtrlT.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-u>"), ftxui::Event::CtrlU.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-v>"), ftxui::Event::CtrlV.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-w>"), ftxui::Event::CtrlW.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-x>"), ftxui::Event::CtrlX.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-y>"), ftxui::Event::CtrlY.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-z>"), ftxui::Event::CtrlZ.input());

  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-a>"), ftxui::Event::CtrlA.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-b>"), ftxui::Event::CtrlB.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-c>"), ftxui::Event::CtrlC.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-d>"), ftxui::Event::CtrlD.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-e>"), ftxui::Event::CtrlE.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-f>"), ftxui::Event::CtrlF.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-g>"), ftxui::Event::CtrlG.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-h>"), ftxui::Event::CtrlH.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-i>"), ftxui::Event::CtrlI.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-j>"), ftxui::Event::CtrlJ.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-k>"), ftxui::Event::CtrlK.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-l>"), ftxui::Event::CtrlL.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-m>"), ftxui::Event::CtrlM.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-n>"), ftxui::Event::CtrlN.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-o>"), ftxui::Event::CtrlO.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-p>"), ftxui::Event::CtrlP.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-q>"), ftxui::Event::CtrlQ.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-r>"), ftxui::Event::CtrlR.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-s>"), ftxui::Event::CtrlS.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-t>"), ftxui::Event::CtrlT.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-u>"), ftxui::Event::CtrlU.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-v>"), ftxui::Event::CtrlV.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-w>"), ftxui::Event::CtrlW.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-x>"), ftxui::Event::CtrlX.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-y>"), ftxui::Event::CtrlY.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-s-z>"), ftxui::Event::CtrlZ.input());

  ASSERT_EQ(mapping::ReplaceTermcodes("<a-a>"), ftxui::Event::AltA.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-b>"), ftxui::Event::AltB.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-c>"), ftxui::Event::AltC.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-d>"), ftxui::Event::AltD.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-e>"), ftxui::Event::AltE.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-f>"), ftxui::Event::AltF.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-g>"), ftxui::Event::AltG.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-h>"), ftxui::Event::AltH.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-i>"), ftxui::Event::AltI.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-j>"), ftxui::Event::AltJ.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-k>"), ftxui::Event::AltK.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-l>"), ftxui::Event::AltL.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-m>"), ftxui::Event::AltM.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-n>"), ftxui::Event::AltN.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-o>"), ftxui::Event::AltO.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-p>"), ftxui::Event::AltP.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-q>"), ftxui::Event::AltQ.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-r>"), ftxui::Event::AltR.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-s>"), ftxui::Event::AltS.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-t>"), ftxui::Event::AltT.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-u>"), ftxui::Event::AltU.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-v>"), ftxui::Event::AltV.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-w>"), ftxui::Event::AltW.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-x>"), ftxui::Event::AltX.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-y>"), ftxui::Event::AltY.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-z>"), ftxui::Event::AltZ.input());

  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-a>"),
            ftxui::Event::CtrlAltA.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-b>"),
            ftxui::Event::CtrlAltB.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-c>"),
            ftxui::Event::CtrlAltC.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-d>"),
            ftxui::Event::CtrlAltD.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-e>"),
            ftxui::Event::CtrlAltE.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-f>"),
            ftxui::Event::CtrlAltF.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-g>"),
            ftxui::Event::CtrlAltG.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-h>"),
            ftxui::Event::CtrlAltH.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-i>"),
            ftxui::Event::CtrlAltI.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-j>"),
            ftxui::Event::CtrlAltJ.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-k>"),
            ftxui::Event::CtrlAltK.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-l>"),
            ftxui::Event::CtrlAltL.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-m>"),
            ftxui::Event::CtrlAltM.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-n>"),
            ftxui::Event::CtrlAltN.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-o>"),
            ftxui::Event::CtrlAltO.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-p>"),
            ftxui::Event::CtrlAltP.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-q>"),
            ftxui::Event::CtrlAltQ.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-r>"),
            ftxui::Event::CtrlAltR.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-s>"),
            ftxui::Event::CtrlAltS.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-t>"),
            ftxui::Event::CtrlAltT.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-u>"),
            ftxui::Event::CtrlAltU.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-v>"),
            ftxui::Event::CtrlAltV.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-w>"),
            ftxui::Event::CtrlAltW.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-x>"),
            ftxui::Event::CtrlAltX.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-y>"),
            ftxui::Event::CtrlAltY.input());
  ASSERT_EQ(mapping::ReplaceTermcodes("<c-a-z>"),
            ftxui::Event::CtrlAltZ.input());

  ASSERT_EQ(mapping::ReplaceTermcodes("<c-b><space>"),
            std::format("{}{}", ftxui::Event::CtrlB.input(), kSpaceChar));

  ASSERT_EQ(mapping::ReplaceTermcodes("<space>b"), " b");
  ASSERT_EQ(mapping::ReplaceTermcodes("<a-b>g"),
            std::format("{}{}", ftxui::Event::AltB.input(), "g"));
}

TEST(TUIMappingTestSuite, TestFailsBindValidation) {
  Mapping mapping;

  ASSERT_THROW(mapping.SetKeymap(tui::INTERACT, "", "<NOP>"),
               std::runtime_error);
}

TEST(TUIMappingTestSuite, TestBindSimpleCombo) {
  Mapping mapping;

  mapping.SetKeymap(tui::VISUAL, "gra", "<NOP>");
  mapping.SetKeymap(tui::VISUAL, "<UP>", "<NOP>");
  mapping.SetKeymap(tui::VISUAL, "<C-A>", "<NOP>");

  auto entries = mapping.GetKeymaps(tui::VISUAL);

  ASSERT_EQ(entries.size(), 3);
  ASSERT_EQ(entries[0]->lhs, ftxui::Event::CtrlA.input());
  ASSERT_EQ(entries[1]->lhs, ftxui::Event::ArrowUp.input());
  ASSERT_EQ(entries[2]->lhs, "gra");
}

TEST(TUIMappingTestSuite, TestUnbindSimpleCombo) {
  Mapping mapping;

  mapping.SetKeymap(tui::VISUAL, "gra", "<NOP>");
  mapping.SetKeymap(tui::VISUAL, "<UP>", "<NOP>");
  mapping.SetKeymap(tui::VISUAL, "<C-A>", "<NOP>");

  auto entries = mapping.GetKeymaps(tui::VISUAL);

  ASSERT_EQ(entries.size(), 3);

  mapping.DelKeymap(tui::VISUAL, "gra");

  entries = mapping.GetKeymaps(tui::VISUAL);

  ASSERT_EQ(entries.size(), 2);

  ASSERT_EQ(entries[0]->lhs, ftxui::Event::CtrlA.input());
  ASSERT_EQ(entries[1]->lhs, ftxui::Event::ArrowUp.input());
}

TEST(TUIMappingTestSuite, TestHandleMapping) {
  Mapping mapping;
  bool called = false;

  mapping.SetKeymap(tui::VISUAL, "gra", [&] { called = true; });

  mapping.SetKeymap(tui::VISUAL, "<UP>", "<NOP>");
  mapping.SetKeymap(tui::VISUAL, "<C-A>", "<NOP>");

  ASSERT_FALSE(mapping.Handle(tui::INTERACT, "gra"));
  ASSERT_TRUE(mapping.Handle(tui::VISUAL, "gra"));
  ASSERT_TRUE(called);
}

TEST(TUIMappingTestSuite, TestMultiModeKeymap) {
  int calls = 0;
  Mapping mapping;

  mapping.SetKeymap(tui::NORMAL | tui::VISUAL, "d", [&] { ++calls; });

  mapping.Handle(tui::NORMAL, "d");
  mapping.Handle(tui::VISUAL, "d");

  ASSERT_EQ(calls, 2);
}

TEST(TUIMappingTestSuite, TestMultiModeKeymapCanBeOverriden) {
  Mapping mapping;
  bool override_called = false;

  mapping.SetKeymap(tui::NORMAL | tui::VISUAL, "q", [] {});

  ASSERT_TRUE(mapping.Handle(tui::VISUAL, "q"));
  ASSERT_FALSE(override_called);

  mapping.SetKeymap(tui::VISUAL, "q", [&] { override_called = true; });

  ASSERT_TRUE(mapping.Handle(tui::VISUAL, "q"));

  ASSERT_TRUE(override_called);
}
