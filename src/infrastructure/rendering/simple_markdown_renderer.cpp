#include "simple_markdown_renderer.h"
#include <sstream>
#include <regex>
#include <algorithm>

namespace infrastructure {
namespace rendering {

using RenderResult = domain::rendering::RenderResult;

// ----------------------------------------------------------------
// 公共 CSS，嵌入到 HTML 中，让 QTextBrowser 显示效果更好
// ----------------------------------------------------------------
static const char* k_css = R"(
<style>
body  { font-family: 'Microsoft YaHei', 'Helvetica Neue', Arial, sans-serif;
        font-size: 14px; line-height: 1.7; color: #333; margin: 0; padding: 8px; }
h1    { font-size: 1.8em; border-bottom: 2px solid #4A90D9; padding-bottom: 6px; color: #2c3e50; }
h2    { font-size: 1.4em; border-bottom: 1px solid #ddd;   padding-bottom: 4px; color: #2c3e50; }
h3    { font-size: 1.2em; color: #2c3e50; }
h4,h5,h6 { color: #2c3e50; }
code  { background: #f4f4f4; border-radius: 3px; padding: 1px 4px;
        font-family: Consolas, monospace; font-size: 0.9em; color: #c7254e; }
pre   { background: #282c34; color: #abb2bf; border-radius: 6px;
        padding: 12px 16px; overflow-x: auto; font-size: 0.85em; line-height: 1.5; }
pre code { background: none; color: inherit; padding: 0; }
blockquote { border-left: 4px solid #4A90D9; margin: 0; padding: 4px 12px; color: #666; }
ul, ol { padding-left: 1.6em; }
li    { margin: 3px 0; }
a     { color: #4A90D9; text-decoration: none; }
a:hover { text-decoration: underline; }
hr    { border: none; border-top: 1px solid #ddd; margin: 12px 0; }
strong { color: #222; }
</style>
)";

// ----------------------------------------------------------------
// render() 创建局部 RenderState，所有状态通过引用在栈上传递，
// 不保存到成员变量，render() 可安全重入 / 并发调用。
// ----------------------------------------------------------------
RenderResult SimpleMarkdownRenderer::render(const std::string& markdown)
{
    RenderState state;  // 全部状态都是局部变量，每次调用独立

    std::istringstream stream(markdown);
    std::string line;
    std::string html;
    html.reserve(markdown.size() * 2);

    html += "<html><head><meta charset='UTF-8'>";
    html += k_css;
    html += "</head><body>";

    while (std::getline(stream, line)) {
        // 去掉行尾 \r（Windows 换行符）
        if (!line.empty() && line.back() == '\r') line.pop_back();

        html += processLine(line, state);
    }

    // 关闭未关闭的块
    closeOpenBlocks(html, state);

    html += "</body></html>";
    return RenderResult::ok(html);
}

// ----------------------------------------------------------------
std::string SimpleMarkdownRenderer::processLine(const std::string& line, RenderState& s)
{
    std::string out;

    // ---- 代码块 ----
    if (line.substr(0, 3) == "```") {
        if (s.inCodeBlock) {
            s.inCodeBlock = false;
            return "</code></pre>\n";
        } else {
            // 关闭段落
            if (s.inParagraph) {
                out += "</p>\n";
                s.inParagraph = false;
            }
            s.inCodeBlock = true;
            std::string lang = line.size() > 3 ? escapeHtml(line.substr(3)) : "";
            return out + "<pre><code class='language-" + lang + "'>";
        }
    }
    if (s.inCodeBlock) {
        return escapeHtml(line) + "\n";
    }

    // ---- 空行 ----
    if (line.empty()) {
        if (s.inUl)        { s.inUl = false; out += "</ul>\n"; }
        if (s.inOl)        { s.inOl = false; out += "</ol>\n"; }
        if (s.inParagraph) { s.inParagraph = false; out += "</p>\n"; }
        return out;
    }

    // ---- 分隔线 ----
    if (line == "---" || line == "***" || line == "___") {
        if (s.inParagraph) { out += "</p>\n"; s.inParagraph = false; }
        return out + "<hr/>\n";
    }

    // ---- 标题 ----
    if (line[0] == '#') {
        if (s.inParagraph) { out += "</p>\n"; s.inParagraph = false; }
        if (s.inUl) { out += "</ul>\n"; s.inUl = false; }
        if (s.inOl) { out += "</ol>\n"; s.inOl = false; }

        int level = 0;
        while (level < (int)line.size() && line[level] == '#') ++level;
        if (level > 6) level = 6;
        std::string content = line.size() > (size_t)level + 1
                              ? line.substr(level + 1) : "";
        return out + "<h" + std::to_string(level) + ">"
               + processInline(content)
               + "</h" + std::to_string(level) + ">\n";
    }

    // ---- 引用块 ----
    if (line[0] == '>') {
        if (s.inParagraph) { out += "</p>\n"; s.inParagraph = false; }
        std::string content = line.size() > 1 ? line.substr(2) : "";
        return out + "<blockquote>" + processInline(content) + "</blockquote>\n";
    }

    // ---- 无序列表 ----
    if ((line[0] == '-' || line[0] == '*') && line.size() > 1 && line[1] == ' ') {
        if (s.inParagraph) { out += "</p>\n"; s.inParagraph = false; }
        if (s.inOl)        { out += "</ol>\n"; s.inOl = false; }
        if (!s.inUl)       { out += "<ul>\n"; s.inUl = true; }
        return out + "<li>" + processInline(line.substr(2)) + "</li>\n";
    }

    // ---- 有序列表 ----
    {
        size_t dotPos = line.find(". ");
        if (dotPos != std::string::npos && dotPos > 0 && dotPos < 4) {
            std::string num = line.substr(0, dotPos);
            bool isNum = std::all_of(num.begin(), num.end(), ::isdigit);
            if (isNum) {
                if (s.inParagraph) { out += "</p>\n"; s.inParagraph = false; }
                if (s.inUl)        { out += "</ul>\n"; s.inUl = false; }
                if (!s.inOl)       { out += "<ol>\n"; s.inOl = true; }
                return out + "<li>" + processInline(line.substr(dotPos + 2)) + "</li>\n";
            }
        }
    }

    // ---- 普通段落 ----
    if (s.inUl) { out += "</ul>\n"; s.inUl = false; }
    if (s.inOl) { out += "</ol>\n"; s.inOl = false; }
    if (!s.inParagraph) {
        out += "<p>";
        s.inParagraph = true;
    } else {
        out += "<br/>";
    }
    out += processInline(line);
    return out;
}

// ----------------------------------------------------------------
// 行内格式：粗体、斜体、行内代码、链接
// ----------------------------------------------------------------
std::string SimpleMarkdownRenderer::processInline(const std::string& text)
{
    std::string result;
    result.reserve(text.size() + 32);

    size_t i = 0;
    while (i < text.size()) {
        // 行内代码 `...`
        if (text[i] == '`') {
            size_t end = text.find('`', i + 1);
            if (end != std::string::npos) {
                result += "<code>" + escapeHtml(text.substr(i + 1, end - i - 1)) + "</code>";
                i = end + 1;
                continue;
            }
        }

        // 链接 [text](url)
        if (text[i] == '[') {
            size_t closeBracket = text.find(']', i + 1);
            if (closeBracket != std::string::npos &&
                closeBracket + 1 < text.size() && text[closeBracket + 1] == '(') {
                size_t closeParen = text.find(')', closeBracket + 2);
                if (closeParen != std::string::npos) {
                    std::string linkText = text.substr(i + 1, closeBracket - i - 1);
                    std::string url      = text.substr(closeBracket + 2,
                                                       closeParen - closeBracket - 2);
                    result += "<a href='" + escapeHtml(url) + "'>"
                              + processInline(linkText) + "</a>";
                    i = closeParen + 1;
                    continue;
                }
            }
        }

        // 粗体 **...**
        if (i + 1 < text.size() && text[i] == '*' && text[i+1] == '*') {
            size_t end = text.find("**", i + 2);
            if (end != std::string::npos) {
                result += "<strong>" + processInline(text.substr(i + 2, end - i - 2)) + "</strong>";
                i = end + 2;
                continue;
            }
        }

        // 斜体 *...*
        if (text[i] == '*') {
            size_t end = text.find('*', i + 1);
            if (end != std::string::npos) {
                result += "<em>" + processInline(text.substr(i + 1, end - i - 1)) + "</em>";
                i = end + 1;
                continue;
            }
        }

        // 普通字符（转义 HTML 特殊字符）
        char c = text[i++];
        switch (c) {
            case '&':  result += "&amp;";  break;
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            case '"':  result += "&quot;"; break;
            default:   result += c;        break;
        }
    }
    return result;
}

// ----------------------------------------------------------------
std::string SimpleMarkdownRenderer::escapeHtml(const std::string& text)
{
    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            case '"':  out += "&quot;"; break;
            default:   out += c;        break;
        }
    }
    return out;
}

// ----------------------------------------------------------------
void SimpleMarkdownRenderer::closeOpenBlocks(std::string& html, RenderState& s)
{
    if (s.inCodeBlock)  { html += "</code></pre>\n"; s.inCodeBlock = false; }
    if (s.inUl)         { html += "</ul>\n";          s.inUl        = false; }
    if (s.inOl)         { html += "</ol>\n";          s.inOl        = false; }
    if (s.inParagraph)  { html += "</p>\n";           s.inParagraph = false; }
}

} // namespace rendering
} // namespace infrastructure
