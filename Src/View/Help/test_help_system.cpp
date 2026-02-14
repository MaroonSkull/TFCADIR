/**
 * @file test_help_system.cpp
 * @brief Unit tests for HelpSystem class
 */

#include "View/Help/HelpSystem.hpp"
#include <gtest/gtest.h>

using namespace view;

/// Test fixture for HelpSystem tests
class HelpSystemTest : public ::testing::Test {
protected:
  std::shared_ptr<HelpSystem> helpSystem_;

  void SetUp() override {
    helpSystem_ = std::make_shared<HelpSystem>();
    helpSystem_->initialize();
  }

  void TearDown() override { helpSystem_.reset(); }
};

/// Tests initialization creates default topics
TEST_F(HelpSystemTest, InitializeCreatesDefaultTopics) {
  EXPECT_GT(helpSystem_->getAllTopics().size(), 0);
}

/// Tests initialization is idempotent
TEST_F(HelpSystemTest, InitializeIdempotent) {
  EXPECT_TRUE(helpSystem_->initialize());
  EXPECT_TRUE(helpSystem_->initialize());
}

/// Tests search finds matching topics by title
TEST_F(HelpSystemTest, SearchFindsMatchingTopicsByTitle) {
  auto results = helpSystem_->searchTopics("Getting Started");

  EXPECT_GT(results.size(), 0);
  bool found = false;
  for (const auto &topic : results) {
    if (topic.id == "getting_started") {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found);
}

/// Tests search finds matching topics by content
TEST_F(HelpSystemTest, SearchFindsMatchingTopicsByContent) {
  auto results = helpSystem_->searchTopics("TFCADIR");

  EXPECT_GT(results.size(), 0);
}

/// Tests search finds matching topics by keyword
TEST_F(HelpSystemTest, SearchFindsMatchingTopicsByKeyword) {
  auto results = helpSystem_->searchTopics("shortcuts");

  EXPECT_GT(results.size(), 0);
}

/// Tests search returns empty for no matches
TEST_F(HelpSystemTest, SearchReturnsEmptyForNoMatches) {
  auto results = helpSystem_->searchTopics("nonexistent_topic_xyz123");

  EXPECT_TRUE(results.empty());
}

/// Tests search is case-insensitive
TEST_F(HelpSystemTest, SearchCaseInsensitive) {
  auto resultsLower = helpSystem_->searchTopics("getting");
  auto resultsUpper = helpSystem_->searchTopics("GETTING");
  auto resultsMixed = helpSystem_->searchTopics("GeTtInG");

  EXPECT_EQ(resultsLower.size(), resultsUpper.size());
  EXPECT_EQ(resultsLower.size(), resultsMixed.size());
}

/// Tests getHelpText returns content for valid context
TEST_F(HelpSystemTest, GetHelpTextValidContext) {
  std::string text = helpSystem_->getHelpText("getting_started");

  EXPECT_FALSE(text.empty());
  EXPECT_NE(text.find("Welcome"), std::string::npos);
}

/// Tests getHelpText returns content for plane_selection context
TEST_F(HelpSystemTest, GetHelpTextPlaneSelectionContext) {
  std::string text = helpSystem_->getHelpText("plane_selection");

  EXPECT_FALSE(text.empty());
  EXPECT_NE(text.find("Drawing Tools"), std::string::npos);
}

/// Tests getHelpText returns error for invalid context
TEST_F(HelpSystemTest, GetHelpTextInvalidContext) {
  std::string text = helpSystem_->getHelpText("nonexistent_context");

  EXPECT_FALSE(text.empty());
  EXPECT_NE(text.find("not found"), std::string::npos);
}

/// Tests hasHelp returns true for valid context
TEST_F(HelpSystemTest, HasHelpValidContext) {
  EXPECT_TRUE(helpSystem_->hasHelp("getting_started"));
  EXPECT_TRUE(helpSystem_->hasHelp("tool.line"));
  EXPECT_TRUE(helpSystem_->hasHelp("plane_selection"));
}

/// Tests hasHelp returns false for invalid context
TEST_F(HelpSystemTest, HasHelpInvalidContext) {
  EXPECT_FALSE(helpSystem_->hasHelp("nonexistent_context"));
}

/// Tests getAllTopics returns all topics
TEST_F(HelpSystemTest, GetAllTopicsReturnsAll) {
  const auto &topics = helpSystem_->getAllTopics();

  EXPECT_GT(topics.size(), 0);

  // Verify topics have required fields
  for (const auto &topic : topics) {
    EXPECT_FALSE(topic.id.empty());
    EXPECT_FALSE(topic.title.empty());
    EXPECT_FALSE(topic.content.empty());
  }
}

/// Tests getTopicsByCategory filters correctly
TEST_F(HelpSystemTest, GetTopicsByCategory) {
  auto topics = helpSystem_->getTopicsByCategory("Reference");

  EXPECT_GT(topics.size(), 0);
  for (const auto &topic : topics) {
    EXPECT_EQ(topic.category, "Reference");
  }
}

/// Tests getTopicsByCategory returns empty for nonexistent category
TEST_F(HelpSystemTest, GetTopicsByCategoryNonexistent) {
  auto topics = helpSystem_->getTopicsByCategory("nonexistent_category");

  EXPECT_TRUE(topics.empty());
}

/// Tests getAllShortcuts returns all shortcuts
TEST_F(HelpSystemTest, GetAllShortcutsReturnsAll) {
  const auto &shortcuts = helpSystem_->getAllShortcuts();

  EXPECT_GT(shortcuts.size(), 0);

  // Verify shortcuts have required fields
  for (const auto &shortcut : shortcuts) {
    EXPECT_FALSE(shortcut.key.empty());
    EXPECT_FALSE(shortcut.action.empty());
    EXPECT_FALSE(shortcut.description.empty());
    EXPECT_FALSE(shortcut.category.empty());
  }
}

/// Tests getShortcutsByCategory filters correctly
TEST_F(HelpSystemTest, GetShortcutsByCategory) {
  auto shortcuts = helpSystem_->getShortcutsByCategory("File Operations");

  EXPECT_GT(shortcuts.size(), 0);
  for (const auto &shortcut : shortcuts) {
    EXPECT_EQ(shortcut.category, "File Operations");
  }
}

/// Tests getTutorialIds returns tutorial IDs
TEST_F(HelpSystemTest, GetTutorialIdsReturnsIds) {
  auto ids = helpSystem_->getTutorialIds();

  EXPECT_GT(ids.size(), 0);
  for (const auto &id : ids) {
    EXPECT_FALSE(id.empty());
  }
}

/// Tests getTutorial returns valid tutorial
TEST_F(HelpSystemTest, GetTutorialValid) {
  const HelpTutorial *tutorial = helpSystem_->getTutorial("getting_started");

  ASSERT_NE(tutorial, nullptr);
  EXPECT_EQ(tutorial->id, "getting_started");
  EXPECT_FALSE(tutorial->title.empty());
  EXPECT_FALSE(tutorial->description.empty());
  EXPECT_GT(tutorial->steps.size(), 0);
}

/// Tests getTutorial returns nullptr for invalid ID
TEST_F(HelpSystemTest, GetTutorialInvalid) {
  const HelpTutorial *tutorial =
      helpSystem_->getTutorial("nonexistent_tutorial");

  EXPECT_EQ(tutorial, nullptr);
}

/// Tests startTutorial starts tutorial
TEST_F(HelpSystemTest, StartTutorial) {
  EXPECT_TRUE(helpSystem_->startTutorial("getting_started"));
  EXPECT_TRUE(helpSystem_->isTutorialActive());

  const HelpTutorial *tutorial = helpSystem_->getCurrentTutorial();
  ASSERT_NE(tutorial, nullptr);
  EXPECT_EQ(tutorial->id, "getting_started");
  EXPECT_EQ(tutorial->currentStep, 0);
}

/// Tests startTutorial fails for invalid ID
TEST_F(HelpSystemTest, StartTutorialInvalid) {
  EXPECT_FALSE(helpSystem_->startTutorial("nonexistent_tutorial"));
  EXPECT_FALSE(helpSystem_->isTutorialActive());
}

/// Tests advanceTutorial advances steps
TEST_F(HelpSystemTest, AdvanceTutorial) {
  helpSystem_->startTutorial("getting_started");

  size_t initialStep = helpSystem_->getCurrentTutorial()->currentStep;
  EXPECT_TRUE(helpSystem_->advanceTutorial());
  EXPECT_GT(helpSystem_->getCurrentTutorial()->currentStep, initialStep);
}

/// Tests advanceTutorial returns false when complete
TEST_F(HelpSystemTest, AdvanceTutorialComplete) {
  helpSystem_->startTutorial("getting_started");

  // Advance through all steps
  while (helpSystem_->advanceTutorial()) {
    // Continue until complete
  }

  EXPECT_FALSE(helpSystem_->isTutorialActive());
}

/// Tests cancelTutorial cancels active tutorial
TEST_F(HelpSystemTest, CancelTutorial) {
  helpSystem_->startTutorial("getting_started");
  EXPECT_TRUE(helpSystem_->isTutorialActive());

  helpSystem_->cancelTutorial();
  EXPECT_FALSE(helpSystem_->isTutorialActive());
  EXPECT_EQ(helpSystem_->getCurrentTutorial(), nullptr);
}

/// Tests isTutorialActive returns correct state
TEST_F(HelpSystemTest, IsTutorialActive) {
  EXPECT_FALSE(helpSystem_->isTutorialActive());

  helpSystem_->startTutorial("getting_started");
  EXPECT_TRUE(helpSystem_->isTutorialActive());

  helpSystem_->cancelTutorial();
  EXPECT_FALSE(helpSystem_->isTutorialActive());
}

/// Tests getCurrentTutorial returns nullptr when no tutorial active
TEST_F(HelpSystemTest, GetCurrentTutorialNone) {
  EXPECT_EQ(helpSystem_->getCurrentTutorial(), nullptr);
}

/// Tests registerTooltip registers tooltip
TEST_F(HelpSystemTest, RegisterTooltip) {
  helpSystem_->registerTooltip("test_element", "Test tooltip text");

  std::string tooltip = helpSystem_->getTooltip("test_element");
  EXPECT_EQ(tooltip, "Test tooltip text");
}

/// Tests getTooltip returns empty for unregistered element
TEST_F(HelpSystemTest, GetTooltipUnregistered) {
  std::string tooltip = helpSystem_->getTooltip("unregistered_element");

  EXPECT_TRUE(tooltip.empty());
}

/// Tests getTooltip returns registered tooltips
TEST_F(HelpSystemTest, GetTooltipRegistered) {
  // Default tooltips should be available
  std::string tooltip = helpSystem_->getTooltip("tool_line");

  EXPECT_FALSE(tooltip.empty());
  EXPECT_NE(tooltip.find("Line"), std::string::npos);
}

/// Tests getCategories returns unique categories
TEST_F(HelpSystemTest, GetCategoriesReturnsUnique) {
  auto categories = helpSystem_->getCategories();

  EXPECT_GT(categories.size(), 0);

  // Verify all categories are unique
  std::set<std::string> uniqueCategories(categories.begin(), categories.end());
  EXPECT_EQ(categories.size(), uniqueCategories.size());
}

/// Tests setHelpDialogCallback stores callback
TEST_F(HelpSystemTest, SetHelpDialogCallback) {
  bool callbackCalled = false;
  helpSystem_->setHelpDialogCallback(
      [&callbackCalled](bool open) { callbackCalled = true; });

  helpSystem_->showHelp("test_context");
  EXPECT_TRUE(callbackCalled);
}

/// Tests toLower converts to lowercase
TEST_F(HelpSystemTest, ToLowerConvertsToLowercase) {
  EXPECT_EQ(HelpSystem::toLower("HELLO"), "hello");
  EXPECT_EQ(HelpSystem::toLower("Hello World"), "hello world");
  EXPECT_EQ(HelpSystem::toLower("already lower"), "already lower");
  EXPECT_EQ(HelpSystem::toLower(""), "");
  EXPECT_EQ(HelpSystem::toLower("123ABC"), "123abc");
}

/// Tests toLower handles special characters
TEST_F(HelpSystemTest, ToLowerSpecialCharacters) {
  EXPECT_EQ(HelpSystem::toLower("Test-Case_123"), "test-case_123");
  EXPECT_EQ(HelpSystem::toLower("UPPER!@#$%"), "upper!@#$%");
}

/// Tests context mapping includes all required contexts
TEST_F(HelpSystemTest, ContextMappingComplete) {
  // Test required contexts from the task
  EXPECT_TRUE(helpSystem_->hasHelp("plane_selection"));
  EXPECT_TRUE(helpSystem_->hasHelp("tool.line"));
  EXPECT_TRUE(helpSystem_->hasHelp("tool.circle"));
  EXPECT_TRUE(helpSystem_->hasHelp("mode.sketch"));
  EXPECT_TRUE(helpSystem_->hasHelp("default"));
}

/// Tests multiple tooltip registrations
TEST_F(HelpSystemTest, MultipleTooltipRegistrations) {
  helpSystem_->registerTooltip("element1", "Tooltip 1");
  helpSystem_->registerTooltip("element2", "Tooltip 2");
  helpSystem_->registerTooltip("element3", "Tooltip 3");

  EXPECT_EQ(helpSystem_->getTooltip("element1"), "Tooltip 1");
  EXPECT_EQ(helpSystem_->getTooltip("element2"), "Tooltip 2");
  EXPECT_EQ(helpSystem_->getTooltip("element3"), "Tooltip 3");
}

/// Tests tooltip overwrite
TEST_F(HelpSystemTest, TooltipOverwrite) {
  helpSystem_->registerTooltip("element", "Original tooltip");
  EXPECT_EQ(helpSystem_->getTooltip("element"), "Original tooltip");

  helpSystem_->registerTooltip("element", "New tooltip");
  EXPECT_EQ(helpSystem_->getTooltip("element"), "New tooltip");
}

/// Tests empty search returns all topics
TEST_F(HelpSystemTest, EmptySearchReturnsAll) {
  auto allTopics = helpSystem_->getAllTopics();
  auto searchResults = helpSystem_->searchTopics("");

  EXPECT_EQ(searchResults.size(), allTopics.size());
}

/// Tests tutorial step content
TEST_F(HelpSystemTest, TutorialStepContent) {
  const HelpTutorial *tutorial = helpSystem_->getTutorial("getting_started");
  ASSERT_NE(tutorial, nullptr);

  for (const auto &step : tutorial->steps) {
    EXPECT_FALSE(step.title.empty());
    EXPECT_FALSE(step.instructions.empty());
  }
}
