#include "scan_page.hh"

#include "style.hh"

#include "creeper-qt/layout/linear.hh"
#include "creeper-qt/utility/material-icon.hh"
#include "creeper-qt/utility/wrapper/layout.hh"
#include "creeper-qt/utility/wrapper/widget.hh"
#include "creeper-qt/widget/buttons/icon-button.hh"
#include "creeper-qt/widget/cards/filled-card.hh"

#include <QFileDialog>
#include <QLayoutItem>
#include <QVBoxLayout>

using namespace creeper;
namespace fc    = filled_card::pro;
namespace ib    = icon_button::pro;
namespace lnpro = linear::pro;
namespace sty   = mymusic::style;

namespace {

void clear_layout(QVBoxLayout* layout) {
    if (layout == nullptr) {
        return;
    }
    while (true) {
        auto* item = layout->takeAt(0);
        if (item == nullptr) {
            break;
        }
        if (auto* widget = item->widget(); widget != nullptr) {
            widget->deleteLater();
        }
        delete item;
    }
}

}

ScanPage::ScanPage(ThemeManager& manager, LibraryService& library)
    : m_theme_manager(manager)
    , m_library(library) {
    apply(widget::pro::Layout<Col> {
        lnpro::Margin { 0 },
        lnpro::Item<Widget> {
            lnpro::Item<Widget>::LayoutMethod { 1 },
            build_content(),
        },
    });

    QObject::connect(&m_library, &LibraryService::library_changed, this, [this] { refresh_view(); });
    QObject::connect(&m_library, &LibraryService::scan_roots_changed, this,
        [this] { refresh_view(); });
    QObject::connect(&m_library, &LibraryService::scan_status_changed, this,
        [this] { refresh_view(); });

    refresh_view();
}

auto ScanPage::build_content() -> creeper::Widget* {
    return new creeper::Widget {
        widget::pro::Layout<Col> {
            lnpro::ContentsMargin { { 0, 0, 0, 0 } },
            lnpro::Spacing { 8 },
            lnpro::Item { build_header_card() },
            lnpro::Item { build_sources_card() },
            lnpro::Item { build_result_card() },
            lnpro::Stretch { 1 },
        },
    };
}

auto ScanPage::build_header_card() -> QWidget* {
    const auto button_features = std::tuple {
        ib::ThemeManager { m_theme_manager },
        ib::Font { material::kRoundSmallFont },
        ib::FixedSize { 44, 44 },
        ib::ShapeRound,
        ib::ColorFilled,
    };

    return new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 24 },
        fc::Layout<Row> {
            lnpro::ContentsMargin { { 22, 20, 22, 20 } },
            lnpro::Spacing { 18 },
            lnpro::Item { sty::make_cover_label(m_theme_manager, "SCAN", QSize { 88, 88 },
                m_theme_manager.color_scheme().secondary_container) },
            lnpro::Item<Col> {
                lnpro::Item<Col>::LayoutMethod { 1 },
                lnpro::Margin { 0 },
                lnpro::Spacing { 6 },
                lnpro::Item<Text> {
                    text::pro::ThemeManager { m_theme_manager },
                    text::pro::Text { "扫描与曲库" },
                },
                lnpro::Item<Text> {
                    widget::pro::Bind { m_library_summary_text },
                    text::pro::ThemeManager { m_theme_manager },
                    text::pro::Text { "正在读取曲库状态..." },
                    sty::meta_text_color(m_theme_manager.color_scheme()),
                },
                lnpro::Item<Text> {
                    text::pro::Text { "添加目录后会递归扫描本地音频文件，并把结果写回曲库。" },
                    sty::meta_text_color(m_theme_manager.color_scheme()),
                },
            },
            lnpro::Item<IconButton> {
                button_features,
                ib::FontIcon { material::icon::kAdd },
                ib::Clickable { [this] {
                    const auto path = QFileDialog::getExistingDirectory(this, QStringLiteral("选择音乐目录"));
                    if (path.isEmpty()) {
                        return;
                    }
                    if (m_library.add_scan_root(path)) {
                        m_library.scan_library();
                    }
                } },
            },
            lnpro::Item<IconButton> {
                button_features,
                ib::FontIcon { material::icon::kRefresh },
                ib::Clickable { [this] { m_library.scan_library(); } },
            },
        },
    };
}

auto ScanPage::build_sources_card() -> QWidget* {
    auto* panel = new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelHigh,
        fc::Radius { 24 },
        fc::Layout<Col> {
            lnpro::ContentsMargin { { 18, 18, 18, 18 } },
            lnpro::Spacing { 10 },
            lnpro::Item<Text> {
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { "已添加的扫描目录" },
            },
        },
    };

    // 提取出该窗口的layout，以便后续动态更新内容
    auto* layout = qobject_cast<QVBoxLayout*>(panel->layout());
    m_sources_layout = new QVBoxLayout {};
    m_sources_layout->setContentsMargins(0, 0, 0, 0);
    m_sources_layout->setSpacing(10);

    // 添加一个占位的空widget，以便在没有扫描目录时显示提示文本
    auto* container = new QWidget {};
    container->setLayout(m_sources_layout);
    layout->addWidget(container);
    return panel;
}

auto ScanPage::build_result_card() -> QWidget* {
    return new FilledCard {
        fc::ThemeManager { m_theme_manager },
        fc::LevelLow,
        fc::Radius { 24 },
        fc::Layout<Col> {
            lnpro::ContentsMargin { { 18, 18, 18, 18 } },
            lnpro::Spacing { 10 },
            lnpro::Item<Text> {
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { "扫描结果与日志" },
            },
            lnpro::Item<Text> {
                widget::pro::Bind { m_scan_status_text },
                text::pro::ThemeManager { m_theme_manager },
                text::pro::Text { "等待扫描" },
                sty::meta_text_color(m_theme_manager.color_scheme()),
            },
        },
    };
}

void ScanPage::refresh_view() {
    if (m_library_summary_text != nullptr) {
        m_library_summary_text->setText(QStringLiteral("%1 首歌曲 · %2 位歌手 · %3 张专辑 · %4 个扫描目录")
                                            .arg(m_library.songs().size())
                                            .arg(m_library.artists().size())
                                            .arg(m_library.albums().size())
                                            .arg(m_library.scan_roots().size()));
    }

    if (m_scan_status_text != nullptr) {
        QString status = m_library.last_scan_message();
        if (status.isEmpty()) {
            status = QStringLiteral("等待扫描");
        }
        if (m_library.last_scan_at().isValid()) {
            status += QStringLiteral(" · 上次更新 %1").arg(
                m_library.last_scan_at().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")));
        }
        if (m_library.is_scanning()) {
            status = QStringLiteral("扫描中... %1").arg(status);
        }
        m_scan_status_text->setText(status);
    }

    clear_layout(m_sources_layout);
    if (m_sources_layout == nullptr) {
        return;
    }

    if (m_library.scan_roots().empty()) {
        m_sources_layout->addWidget(new Text {
            text::pro::ThemeManager { m_theme_manager },
            text::pro::Text { "还没有添加扫描目录。点击右上角加号选择一个本地音乐文件夹。" },
            sty::meta_text_color(m_theme_manager.color_scheme()),
        });
        return;
    }

    for (const auto& root : m_library.scan_roots()) {
        QString meta = QStringLiteral("%1 首音频文件").arg(root.discovered_count);
        if (root.last_scan_at.isValid()) {
            meta += QStringLiteral(" · 最近扫描 %1")
                        .arg(root.last_scan_at.toString(QStringLiteral("yyyy-MM-dd hh:mm")));
        }

        m_sources_layout->addWidget(new FilledCard {
            fc::ThemeManager { m_theme_manager },
            fc::LevelLow,
            fc::Radius { 18 },
            fc::Layout<Row> {
                lnpro::ContentsMargin { { 14, 14, 14, 14 } },
                lnpro::Spacing { 12 },
                lnpro::Item { sty::make_cover_label(m_theme_manager, "DIR",
                    QSize { 52, 52 }, m_theme_manager.color_scheme().primary_container) },
                lnpro::Item<Col> {
                    lnpro::Item<Col>::LayoutMethod { 1 },
                    lnpro::Margin { 0 },
                    lnpro::Spacing { 4 },
                    lnpro::Item<Text> {
                        text::pro::ThemeManager { m_theme_manager },
                        text::pro::Text { root.path },
                    },
                    lnpro::Item<Text> {
                        text::pro::Text { meta },
                        sty::meta_text_color(m_theme_manager.color_scheme()),
                    },
                },
            },
        });
    }

    m_sources_layout->addStretch(1);
    m_theme_manager.apply_theme();
}
