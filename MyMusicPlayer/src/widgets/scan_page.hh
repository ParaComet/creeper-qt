#pragma once

#include "services/library_service.hh"

#include "creeper-qt/utility/theme/theme.hh"
#include "creeper-qt/widget/text.hh"
#include "creeper-qt/widget/widget.hh"

class QVBoxLayout;

struct ScanPage : public creeper::Widget {
    explicit ScanPage(creeper::ThemeManager& manager, LibraryService& library);

private:
    creeper::ThemeManager& m_theme_manager;
    LibraryService& m_library;
    creeper::Text* m_library_summary_text = nullptr;
    creeper::Text* m_scan_status_text     = nullptr;
    QVBoxLayout* m_sources_layout         = nullptr;

    auto build_content() -> creeper::Widget*;
    auto build_header_card() -> QWidget*;
    auto build_sources_card() -> QWidget*;
    auto build_result_card() -> QWidget*;

    void refresh_view();
};
