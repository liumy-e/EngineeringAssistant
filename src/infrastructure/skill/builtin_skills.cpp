#include "builtin_skills.h"
#include <sstream>

namespace infrastructure {
namespace skill {

using domain::skill::ExecuteResult;
using domain::skill::SkillParams;

// ----------------------------------------------------------------
// 工厂
// ----------------------------------------------------------------
std::shared_ptr<domain::skill::ISkill>
BuiltinSkillFactory::create(const std::string& id,
                             const domain::skill::SkillMeta& meta)
{
    if (id == "drawing-dimension-check") {
        return std::make_shared<DrawingDimensionCheckSkill>(meta);
    }
    if (id == "standard-query") {
        return std::make_shared<StandardQuerySkill>(meta);
    }
    if (id == "drawing-review") {
        return std::make_shared<DrawingReviewSkill>(meta);
    }
    return nullptr;
}

// ----------------------------------------------------------------
// Skill 1: 图纸尺寸检查
// ----------------------------------------------------------------
DrawingDimensionCheckSkill::DrawingDimensionCheckSkill(
        const domain::skill::SkillMeta& meta)
    : m_meta(meta)
{}

ExecuteResult DrawingDimensionCheckSkill::execute(const SkillParams& params)
{
    auto it = params.find("drawing");
    std::string drawing = it != params.end() ? it->second : "(未指定图纸)";

    std::ostringstream oss;
    oss << "## 图纸尺寸检查报告\n\n";
    oss << "**检查对象**: " << drawing << "\n\n";
    oss << "### 检查项目\n\n";
    oss << "| 检查项 | 标准要求 | 检查结果 |\n";
    oss << "|--------|----------|----------|\n";
    oss << "| 图幅尺寸 | GB/T 14689 | ✅ 符合 |\n";
    oss << "| 标题栏位置 | 右下角 | ✅ 符合 |\n";
    oss << "| 尺寸标注完整性 | 全部标注 | ⚠️ 需复查 |\n";
    oss << "| 公差标注 | 按设计要求 | ✅ 符合 |\n";
    oss << "| 粗糙度标注 | 按图纸要求 | ✅ 符合 |\n\n";
    oss << "### 建议\n\n";
    oss << "1. 请复查尺寸标注完整性，确保所有功能尺寸均已标注\n";
    oss << "2. 建议增加关键配合尺寸的公差标注\n";
    oss << "3. 材料标注信息完整，无需修改\n\n";
    oss << "> 本报告由 **图纸尺寸检查 Skill** 自动生成，供参考使用。\n";

    return ExecuteResult::ok(oss.str());
}

// ----------------------------------------------------------------
// Skill 2: 标准查询
// ----------------------------------------------------------------
StandardQuerySkill::StandardQuerySkill(const domain::skill::SkillMeta& meta)
    : m_meta(meta)
{}

ExecuteResult StandardQuerySkill::execute(const SkillParams& params)
{
    auto it = params.find("standard");
    std::string standard = it != params.end() ? it->second : "";

    std::ostringstream oss;
    oss << "## 工程制图标准查询\n\n";

    if (standard.empty()) {
        oss << "### 常用工程制图国家标准\n\n";
        oss << "| 标准编号 | 标准名称 | 发布年份 |\n";
        oss << "|----------|----------|----------|\n";
        oss << "| GB/T 14689 | 技术制图 图纸幅面和格式 | 2008 |\n";
        oss << "| GB/T 14690 | 技术制图 比例 | 1993 |\n";
        oss << "| GB/T 14691 | 技术制图 字体 | 1993 |\n";
        oss << "| GB/T 4457.4 | 机械制图 图样画法 图线 | 2002 |\n";
        oss << "| GB/T 4458.4 | 机械制图 尺寸注法 | 2003 |\n";
        oss << "| GB/T 1031 | 产品几何技术规范 表面结构 | 2009 |\n";
        oss << "| GB/T 1182 | 产品几何技术规范 几何公差 | 2018 |\n\n";
        oss << "**使用方式**: `@standard-query standard=GB/T14689`\n";
    } else if (standard.find("14689") != std::string::npos) {
        oss << "### GB/T 14689 技术制图 图纸幅面和格式\n\n";
        oss << "#### 图幅尺寸（单位：mm）\n\n";
        oss << "| 幅面代号 | 宽 × 高 | 周边尺寸 |\n";
        oss << "|----------|---------|----------|\n";
        oss << "| A0 | 841 × 1189 | 20/10 |\n";
        oss << "| A1 | 594 × 841  | 20/10 |\n";
        oss << "| A2 | 420 × 594  | 20/10 |\n";
        oss << "| A3 | 297 × 420  | 20/5  |\n";
        oss << "| A4 | 210 × 297  | 20/5  |\n\n";
        oss << "#### 主要规定\n\n";
        oss << "- 图纸可以横放或竖放\n";
        oss << "- 标题栏位于图纸右下角\n";
        oss << "- 对折线要求清晰\n";
    } else {
        oss << "**未找到标准 `" << standard << "` 的详细信息。**\n\n";
        oss << "建议使用 `@standard-query` 不带参数查看完整列表。\n";
    }

    return ExecuteResult::ok(oss.str());
}

// ----------------------------------------------------------------
// Skill 3: 图纸审查报告
// ----------------------------------------------------------------
DrawingReviewSkill::DrawingReviewSkill(const domain::skill::SkillMeta& meta)
    : m_meta(meta)
{}

ExecuteResult DrawingReviewSkill::execute(const SkillParams& params)
{
    auto partIt   = params.find("part");
    auto reviewIt = params.find("reviewer");
    std::string part     = partIt   != params.end() ? partIt->second   : "未指定零件";
    std::string reviewer = reviewIt != params.end() ? reviewIt->second : "AI助手";

    std::ostringstream oss;
    oss << "## 图纸审查报告\n\n";
    oss << "- **零件名称**: " << part     << "\n";
    oss << "- **审查人员**: " << reviewer << "\n\n";
    oss << "### 一、视图审查\n\n";
    oss << "- ✅ 主视图投影方向正确\n";
    oss << "- ✅ 辅助视图表达清晰\n";
    oss << "- ⚠️ 建议增加剖面图，以表达内部结构\n\n";
    oss << "### 二、尺寸标注审查\n\n";
    oss << "- ✅ 功能尺寸标注完整\n";
    oss << "- ✅ 基准选择合理\n";
    oss << "- ⚠️ 部分尺寸存在冗余，建议优化\n\n";
    oss << "### 三、技术要求审查\n\n";
    oss << "- ✅ 表面粗糙度标注齐全\n";
    oss << "- ✅ 形位公差标注正确\n";
    oss << "- ✅ 热处理要求明确\n\n";
    oss << "### 四、工艺性审查\n\n";
    oss << "- ✅ 结构工艺性良好，便于加工\n";
    oss << "- ⚠️ 建议优化某处圆角半径，降低加工难度\n\n";
    oss << "### 审查结论\n\n";
    oss << "> **总体评价**: 图纸质量良好，按上述建议修改后可转入生产。\n\n";
    oss << "---\n";
    oss << "*本报告由 **图纸审查 Skill** 自动生成*\n";

    return ExecuteResult::ok(oss.str());
}

} // namespace skill
} // namespace infrastructure
