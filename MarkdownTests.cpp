// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tests.hpp"

using namespace tcob::gfx::html;

TEST_CASE("GFX.Markdown.Headings")
{
    SUBCASE("ATX h1-h6")
    {
        REQUIRE(md_to_html("# H1") == "<h1>H1</h1>");
        REQUIRE(md_to_html("## H2") == "<h2>H2</h2>");
        REQUIRE(md_to_html("### H3") == "<h3>H3</h3>");
        REQUIRE(md_to_html("#### H4") == "<h4>H4</h4>");
        REQUIRE(md_to_html("##### H5") == "<h5>H5</h5>");
        REQUIRE(md_to_html("###### H6") == "<h6>H6</h6>");
    }

    SUBCASE("ATX trailing hashes stripped")
    {
        REQUIRE(md_to_html("## Foo ##") == "<h2>Foo</h2>");
        REQUIRE(md_to_html("### Foo ###") == "<h3>Foo</h3>");
    }

    SUBCASE("ATX not a heading without space")
    {
        REQUIRE(md_to_html("#NoSpace") == "<p>#NoSpace</p>");
    }

    SUBCASE("setext h1")
    {
        REQUIRE(md_to_html("Title\n=====") == "<h1>Title</h1>");
    }

    SUBCASE("setext h2")
    {
        REQUIRE(md_to_html("Title\n-----") == "<h2>Title</h2>");
    }

    SUBCASE("heading with inline formatting")
    {
        REQUIRE(md_to_html("## **bold** heading") == "<h2><strong>bold</strong> heading</h2>");
    }
}

TEST_CASE("GFX.Markdown.Paragraphs")
{
    SUBCASE("simple paragraph")
    {
        REQUIRE(md_to_html("Hello world") == "<p>Hello world</p>");
    }

    SUBCASE("two paragraphs separated by blank line")
    {
        REQUIRE(md_to_html("First\n\nSecond") == "<p>First</p><p>Second</p>");
    }

    SUBCASE("consecutive lines join into one paragraph")
    {
        REQUIRE(md_to_html("line one\nline two") == "<p>line one line two</p>");
    }
}

TEST_CASE("GFX.Markdown.Inline")
{
    SUBCASE("bold asterisk")
    {
        REQUIRE(md_to_html("**bold**") == "<p><strong>bold</strong></p>");
    }

    SUBCASE("bold underscore")
    {
        REQUIRE(md_to_html("__bold__") == "<p><strong>bold</strong></p>");
    }

    SUBCASE("italic asterisk")
    {
        REQUIRE(md_to_html("*italic*") == "<p><em>italic</em></p>");
    }

    SUBCASE("italic underscore")
    {
        REQUIRE(md_to_html("_italic_") == "<p><em>italic</em></p>");
    }

    SUBCASE("underscore mid-word not italic")
    {
        REQUIRE(md_to_html("foo_bar_baz") == "<p>foo_bar_baz</p>");
    }

    SUBCASE("strikethrough")
    {
        REQUIRE(md_to_html("~strike~") == "<p><del>strike</del></p>");
        REQUIRE(md_to_html("~~strike~~") == "<p><del>strike</del></p>");
    }

    SUBCASE("inline code single backtick")
    {
        REQUIRE(md_to_html("`code`") == "<p><code>code</code></p>");
    }

    SUBCASE("inline code double backtick")
    {
        REQUIRE(md_to_html("``co`de``") == "<p><code>co`de</code></p>");
    }

    SUBCASE("bold inside text")
    {
        REQUIRE(md_to_html("a **b** c") == "<p>a <strong>b</strong> c</p>");
    }

    SUBCASE("nested bold in italic")
    {
        REQUIRE(md_to_html("*a **b** c*") == "<p><em>a <strong>b</strong> c</em></p>");
    }

    SUBCASE("bold italic ***")
    {
        REQUIRE(md_to_html("***bold italic***") == "<p><strong><em>bold italic</em></strong></p>");
    }

    SUBCASE("backslash escape")
    {
        REQUIRE(md_to_html("\\*not italic\\*") == "<p>*not italic*</p>");
    }

    SUBCASE("link")
    {
        REQUIRE(md_to_html("[text](https://example.com)") == "<p><a href=\"https://example.com\">text</a></p>");
    }

    SUBCASE("image")
    {
        REQUIRE(md_to_html("![alt](img.png)") == "<p><img src=\"img.png\" alt=\"alt\"></p>");
    }

    SUBCASE("HTML escaping in text")
    {
        REQUIRE(md_to_html("a & b < c > d") == "<p>a &amp; b &lt; c &gt; d</p>");
    }

    SUBCASE("HTML escaping in code")
    {
        REQUIRE(md_to_html("`a < b`") == "<p><code>a &lt; b</code></p>");
    }

    SUBCASE("HTML escaping in link url")
    {
        REQUIRE(md_to_html("[x](a&b)") == "<p><a href=\"a&amp;b\">x</a></p>");
    }

    SUBCASE("inline HTML")
    {
        REQUIRE(md_to_html("inline <p>HTML</p>") == "<p>inline <p>HTML</p></p>");
        REQUIRE(md_to_html("<a><bab><c2c>") == "<p><a><bab><c2c></p>");
    }

    SUBCASE("color")
    {
        REQUIRE(md_to_html("{red}(color)") == "<p><span style=\"color:red\">color</span></p>");
    }
}

TEST_CASE("GFX.Markdown.Tables")
{
    SUBCASE("simple table")
    {
        std::string const input {
            "| Header1 | Header2 |\n"
            "| --- | --- |\n"
            "| Cell1 | Cell2 |"};
        std::string const expected {
            "<table><thead><tr><th>Header1</th><th>Header2</th></tr></thead>"
            "<tbody><tr><td>Cell1</td><td>Cell2</td></tr></tbody></table>"};
        REQUIRE(md_to_html(input) == expected);
    }

    SUBCASE("table with inline formatting")
    {
        std::string const input {
            "| **Bold** | *Italic* |\n"
            "| --- | --- |\n"
            "| ~~strike~~ | `code` |"};
        std::string const expected {
            "<table><thead><tr><th><strong>Bold</strong></th><th><em>Italic</em></th></tr></thead>"
            "<tbody><tr><td><del>strike</del></td><td><code>code</code></td></tr></tbody></table>"};
        REQUIRE(md_to_html(input) == expected);
    }

    SUBCASE("table with multiple rows")
    {
        std::string const input {
            "| H1 | H2 |\n"
            "| --- | --- |\n"
            "| R1C1 | R1C2 |\n"
            "| R2C1 | R2C2 |"};
        std::string const expected {
            "<table><thead><tr><th>H1</th><th>H2</th></tr></thead>"
            "<tbody><tr><td>R1C1</td><td>R1C2</td></tr>"
            "<tr><td>R2C1</td><td>R2C2</td></tr></tbody></table>"};
        REQUIRE(md_to_html(input) == expected);
    }
}

TEST_CASE("GFX.Markdown.ThematicBreak")
{
    SUBCASE("dashes")
    {
        REQUIRE(md_to_html("---") == "<hr>");
    }

    SUBCASE("asterisks")
    {
        REQUIRE(md_to_html("***") == "<hr>");
    }

    SUBCASE("underscores")
    {
        REQUIRE(md_to_html("___") == "<hr>");
    }

    SUBCASE("with spaces")
    {
        REQUIRE(md_to_html("- - -") == "<hr>");
    }

    SUBCASE("not a break: only two")
    {
        REQUIRE(md_to_html("--") == "<p>--</p>");
    }
}

TEST_CASE("GFX.Markdown.Blockquote")
{
    SUBCASE("single line")
    {
        REQUIRE(md_to_html("> quote") == "<blockquote><p>quote</p></blockquote>");
    }

    SUBCASE("multiple lines")
    {
        REQUIRE(md_to_html("> line1\n> line2")
                == "<blockquote><p>line1 line2</p></blockquote>");
    }

    SUBCASE("with inline formatting")
    {
        REQUIRE(md_to_html("> **bold**") == "<blockquote><p><strong>bold</strong></p></blockquote>");
    }
}

TEST_CASE("GFX.Markdown.Lists")
{
    SUBCASE("unordered dash")
    {
        REQUIRE(md_to_html("- a\n- b\n- c") == "<ul><li>a</li><li>b</li><li>c</li></ul>");
    }

    SUBCASE("unordered asterisk")
    {
        REQUIRE(md_to_html("* a\n* b") == "<ul><li>a</li><li>b</li></ul>");
    }

    SUBCASE("unordered plus")
    {
        REQUIRE(md_to_html("+ a\n+ b") == "<ul><li>a</li><li>b</li></ul>");
    }

    SUBCASE("ordered")
    {
        REQUIRE(md_to_html("1. first\n2. second\n3. third")
                == "<ol><li>first</li><li>second</li><li>third</li></ol>");
    }

    SUBCASE("ordered non-1 start")
    {
        REQUIRE(md_to_html("3. first\n4. second")
                == "<ol start=\"3\"><li>first</li><li>second</li></ol>");
    }

    SUBCASE("ordered with closing paren")
    {
        REQUIRE(md_to_html("1) a\n2) b") == "<ol><li>a</li><li>b</li></ol>");
    }

    SUBCASE("nested unordered")
    {
        std::string const input {"- a\n  - b\n  - c\n- d"};
        std::string const expected {
            "<ul>"
            "<li>a"
            "<ul><li>b</li><li>c</li></ul>"
            "</li>"
            "<li>d</li>"
            "</ul>"};
        REQUIRE(md_to_html(input) == expected);
    }

    SUBCASE("list item with inline formatting")
    {
        REQUIRE(md_to_html("- **bold** item") == "<ul><li><strong>bold</strong> item</li></ul>");
    }
}

TEST_CASE("GFX.Markdown.Code")
{
    SUBCASE("fenced no language")
    {
        REQUIRE(md_to_html("```\ncode\n```") == "<pre><code>code\n</code></pre>");
    }

    SUBCASE("fenced with language")
    {
        REQUIRE(md_to_html("```cpp\nint x;\n```") == "<pre><code class=\"language-cpp\">int x;\n</code></pre>");
    }

    SUBCASE("fenced tilde")
    {
        REQUIRE(md_to_html("~~~\ncode\n~~~") == "<pre><code>code\n</code></pre>");
    }

    SUBCASE("fenced HTML escaped")
    {
        REQUIRE(md_to_html("```\na < b\n```") == "<pre><code>a &lt; b\n</code></pre>");
    }

    SUBCASE("fenced multiline")
    {
        REQUIRE(md_to_html("```\nline1\nline2\n```") == "<pre><code>line1\nline2\n</code></pre>");
    }

    SUBCASE("indented code block")
    {
        REQUIRE(md_to_html("    code line") == "<pre><code>code line\n</code></pre>");
    }
}

TEST_CASE("GFX.Markdown.Mixed")
{
    SUBCASE("heading then paragraph")
    {
        REQUIRE(md_to_html("# Title\n\nParagraph.") == "<h1>Title</h1><p>Paragraph.</p>");
    }

    SUBCASE("paragraph then list")
    {
        REQUIRE(md_to_html("Intro\n\n- a\n- b") == "<p>Intro</p><ul><li>a</li><li>b</li></ul>");
    }

    SUBCASE("empty input")
    {
        REQUIRE(md_to_html("") == "");
    }

    SUBCASE("blank lines only")
    {
        REQUIRE(md_to_html("\n\n") == "");
    }

    SUBCASE("windows line endings")
    {
        REQUIRE(md_to_html("# Title\r\n\r\nParagraph.") == "<h1>Title</h1><p>Paragraph.</p>");
    }
}
