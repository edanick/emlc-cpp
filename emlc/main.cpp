#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <regex>
#include <set>

using namespace std;

// ======================
// Constants
// ======================
const set<string> SELF_CLOSING_TAGS = {
    "area", "base", "br", "col", "embed", "hr", "img",
    "input", "link", "meta", "param", "source", "track", "wbr"
};
// Optional closing tags for HTML (omitted for strictness, but good for parsing)
const set<string> OPTIONAL_CLOSE_TAGS = {
    "li", "dt", "dd", "p", "rt", "rp", "optgroup", "option",
    "thead", "tbody", "tfoot", "tr", "td", "th"
};

const string VERSION = "1.0";

// ======================
// Helper Functions
// ======================
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

bool ends_with(const string& str, const string& suffix) {
    if (suffix.size() > str.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool is_ident_start(char c) {
    return isalpha(c) || c == '_';
}

bool is_ident_part(char c) {
    return isalnum(c) || c == '-' || c == '_' || c == '.' || c == ':';
}

// ======================
// AST Structure
// ======================
struct Attribute {
    string key;
    string value;
    string separator; // e.g., " ", ", ", "\n "
};

enum NodeType {
    ELEMENT,
    TEXT,
    COMMENT,
    COMMENT_BLOCK,
    PI, // Processing Instruction
    IMPORT, // Special for FXML imports
    WHITESPACE
};

struct Node {
    NodeType type;
    string tag; // Element tag name
    vector<Attribute> attrs;
    string content; // Text, Comment content, PI content
    vector<Node*> children;
    bool explicit_empty_block = false; // true if {} was explicitly present but empty

    Node(NodeType t) : type(t) {}
    virtual ~Node() {
        for (auto c : children) delete c;
    }

    void add_child(Node* child) {
        children.push_back(child);
    }
};

// ======================
// Formatter Interface
// ======================
class Formatter {
public:
    virtual string format(Node* node, int indent_level = 0) = 0;
    virtual ~Formatter() {}

protected:
    string get_indent(int level) {
        return string(level * 4, ' ');
    }
    
    string format_attrs(const vector<Attribute>& attrs) {
        string s = "";
        for (const auto& attr : attrs) {
            string sep = attr.separator.empty() ? " " : attr.separator;
            if (s.empty()) {
                // For the first attribute, if the separator is just a space, 
                // we usually want to prepend it only if there are attributes.
                // But the separator stored includes the leading whitespace.
            }
            s += sep + attr.key + "=\"" + attr.value + "\"";
        }
        return s;
    }
};

// ======================
// EML Formatter
// ======================
class EmlFormatter : public Formatter {
public:
    string format(Node* node, int indent_level = 0) override {
        string out = "";
        string ind = get_indent(indent_level);

        if (node->type == WHITESPACE) {
             // Replicate newlines
             size_t n = std::count(node->content.begin(), node->content.end(), '\n');
             if (n > 1) {
                 for(size_t k = 0; k < n - 1; ++k) out += "\n";
             }
             return out;
        }

        if (node->type == COMMENT) {
            return ind + "// " + node->content + "\n";
        }
        if (node->type == COMMENT_BLOCK) {
            return ind + "/*" + node->content + "*/\n";
        }
        if (node->type == IMPORT) {
            return "import " + node->content + ";\n";
        }
        if (node->type == PI) {
            if (node->tag == "php") {
                out += ind + "php {\n";
                out += format_children_raw(node, indent_level + 1);
                out += ind + "}\n";
                return out;
            }
            // Other PIs or unexpected ones
            return ind + "php /* " + node->content + " */\n"; 
        }

        if (node->type == TEXT) {
             // Text in EML is usually inline or wrapped in {} if implicitly part of a parent
             // But here we are formatting a node. 
             // Pure text nodes at top level shouldn't happen in valid EML usually, unless inside an element.
             return ind + node->content + "\n";
        }

        if (node->type == ELEMENT) {
            if (node->tag == "ROOT") {
                for (auto c : node->children) {
                    out += format(c, indent_level);
                }
                return out;
            }

            out += ind + node->tag;
            
            // Attributes
            if (!node->attrs.empty()) {
                out += " (";
                bool first = true;
                for (const auto& attr : node->attrs) {
                    if (!first) out += ", ";
                    out += attr.key + " = \"" + attr.value + "\"";
                    first = false;
                }
                out += ")";
            }

            if (node->children.empty()) {
                if (node->explicit_empty_block) {
                    out += " {}\n";
                } else {
                    out += "\n";
                }
            } else {
                // Check if single text child (inline)
                if (node->children.size() == 1 && node->children[0]->type == TEXT) {
                    string text = trim(node->children[0]->content);
                    if (text.find('\n') == string::npos) {
                        out += " { " + text + " }\n";
                        return out;
                    }
                }

                out += " {\n";
                for (auto c : node->children) {
                    out += format(c, indent_level + 1);
                }
                out += ind + "}\n";
            }
        }
        return out;
    }
    
private:
    string format_children_raw(Node* node, int indent_level) {
        // For raw blocks like php, we just want content lines indented
        string out = "";
        string ind = get_indent(indent_level);
        stringstream ss(node->content);
        string line;
        while (getline(ss, line)) {
            out += ind + trim(line) + "\n";
        }
        return out;
    }
};

// ======================
// HTML/XML Formatter
// ======================
class MarkupFormatter : public Formatter {
    bool is_xml; // true: XAML/XML (strict), false: HTML/PHP (loose, void tags)

public:
    MarkupFormatter(bool xml_mode) : is_xml(xml_mode) {}

    string format(Node* node, int indent_level = 0) override {
        if (!node) return "";
        string out = "";
        string ind = get_indent(indent_level);

        if (node->type == WHITESPACE) {
             size_t n = std::count(node->content.begin(), node->content.end(), '\n');
             if (n > 1) {
                 for(size_t k = 0; k < n - 1; ++k) out += "\n";
             }
             return out;
        }

        if (node->type == COMMENT) {
            // Multi-line comment fidelity
            if (node->content.find('\n') != string::npos) {
                string c = node->content;
                // If it looks like a block comment, maybe preserve lines?
                // Standard indentation approach:
                // <!--
                //    line1
                //    line2
                // -->
                // But we should just preserve strict lines if provided?
                /*
                stringstream ss(c);
                string line;
                string out = ind + "<!--\n";
                while(getline(ss, line)) {
                    out += ind + line + "\n"; // Adding indent might double it if captured?
                }
                out += ind + "-->\n";
                return out;
                */
                // For fidelity, assume content has indentation? 
                // Just wrap in <!-- --> without flattening.
                return ind + "<!-- " + trim(node->content) + " -->\n"; 
            }
            return ind + "<!-- " + trim(node->content) + " -->\n";
        }
        if (node->type == COMMENT_BLOCK) {
            // /* ... */ style (block)
            // Preserve raw content?
            return ind + "<!--" + node->content + "-->\n";
        }
        if (node->type == IMPORT) {
            return ind + "<?import " + node->content + "?>\n";
        }
        if (node->type == PI) {
            if (node->tag == "php") {
                // Do NOT trim content to preserve indentation
                string php_content = node->content;
                
                // If content starts with newline, we can rely on it.
                // But generally users want:
                // <?php
                //     code...
                // ?>
                // If we output raw `node->content`, it includes everything.
                // We just need to wrap it.
                
                // Issue: If we just dump content, we might miss the first newline if it wasn't there?
                // EML: php { ... }
                // Content includes leading newline if present.
                
                // If we output: ind + "<?php" + php_content + ind + "?>\n"
                // It should work IF php_content has leading/trailing newlines.
                // If not, we might want to ensure them?
                
                // Let's assume fidelity: output exactly what was captured.
                // BUT we need to indent the closing tag?
                // Or closing tag is part of content? No, parser strips }
                
                // Safe approach matching Script/Style fix:
                // Check if starts with \n. If not, add one?
                bool starts_newline = !php_content.empty() && php_content[0] == '\n';
                string out_content = php_content;
                if (!starts_newline) out_content = "\n" + out_content;
                
                // Ensure ends with newline for closing tag?
                if (!out_content.empty() && out_content.back() != '\n') {
                    out_content += "\n";
                }

                 if (is_xml) {
                     return ind + "<?php" + out_content + ind + "?>\n";
                 } else {
                     return ind + "<?php" + out_content + ind + "?>\n";
                 }
            }
            return ind + "<?" + node->tag + " " + node->content + "?>\n";
        }

        if (node->type == TEXT) {
            // If it's pure whitespace content, we might want to respect it?
            // trimming usually safest for pretty print
            return ind + trim(node->content) + "\n";
        }

        if (node->type == ELEMENT) {
            if (node->tag == "ROOT") {
                for (auto c : node->children) {
                    out += format(c, indent_level);
                }
                return out;
            }

            string attr_str = "";
            for (const auto& attr : node->attrs) {
                // Reconstruct separator logic roughly or strictly
                // For simplified formatter, just use space
                attr_str += " " + attr.key + "=\"" + attr.value + "\"";
            }

            string open_tag = "<" + node->tag + attr_str;
            
            // Self-closing check
            bool self_close = false;
            if (is_xml) {
                // In XML/XAML, if empty AND not explicitly forced to have block (although optimization usually valid),
                // User requirement: "try to turn non self closing to self closing" -> No, user complaint was OPPOSITE.
                // User complaint: "turns a non self closing tag to a self closing".
                // So we should ONLY self-close if logic demands it, OR if it's strictly empty and allowed.
                // FIX: If it has children, never self close. If no children:
                // If it was explicit empty block in EML `Tag {}`, we shouldn't self close? 
                // But AST doesn't know origin if converted from XML.
                // Let's rely on children.empty(). 
                // AND checking if the user INTENDED an empty block.
                // We added `explicit_empty_block`.
                if (node->children.empty() && !node->explicit_empty_block) {
                    self_close = true;
                }
            } else {
                // HTML: Only void tags are self-closing (void elements)
                if (SELF_CLOSING_TAGS.count(node->tag)) {
                    self_close = true; // Output <br> or <br /> depending on style?
                }
            }
            
            // Override: If strict XML, explicit empty block means <T></T>.
            if (is_xml && node->explicit_empty_block) self_close = false;
            // Override: If HTML, explicit empty block `div {}` -> <div></div>. Correct.
            
            if (self_close) {
                 if (is_xml) out += ind + open_tag + " />\n";
                 else out += ind + open_tag + ">\n"; // HTML void tags usually don't have />
            } else {
                out += ind + open_tag + ">";
                
                // Content
                if (node->children.empty()) {
                    out += "</" + node->tag + ">\n";
                } else {
                    // Optimized single text line
                    if (node->children.size() == 1 && node->children[0]->type == TEXT) {
                        string t = node->children[0]->content;
                        string trimmed = trim(t);

                        // Try to inline if no newlines and not empty
                        // Use untrimmed 't' to preserve internal spaces if user provided them `h1 { Hello }`
                        if (t.find('\n') == string::npos && !trimmed.empty()) {
                            out += t + "</" + node->tag + ">\n";
                            return out;
                        }
                        
                        // Multi-line or whitespace-only preservation
                        
                        // Trim trailing horizontal whitespace (indentation of the closing brace in EML)
                        // to prevent extra blank line/indent before output closing tag
                        size_t last_char = t.find_last_not_of(" \t");
                        if (last_char != string::npos) {
                            t.erase(last_char + 1);
                        } else {
                            t.clear(); // string validation?
                        }

                        // If content doesn't start with newline, add one for block separation
                        bool starts_newline = !t.empty() && t[0] == '\n';
                        if (!starts_newline) out += "\n";
                        
                        out += t;
                        
                        // Ensure closing tag starts on a new line
                        if (!t.empty() && t.back() != '\n') out += "\n";
                        
                        out += ind + "</" + node->tag + ">\n";
                        return out;
                    }

                    // Fallback for multiple/mixed children (recursive)
                    out += "\n";
                    for (auto c : node->children) {
                        out += format(c, indent_level + 1);
                    }
                    out += ind + "</" + node->tag + ">\n";
                }
            }
        }
        return out;
    }
};

// ======================
// Parser Class
// ======================
class Parser {
    string input;
    size_t pos;
    size_t len;

public:
    Node* parse(const string& in, bool is_eml_format) {
        input = in;
        pos = 0;
        len = input.length();
        
        Node* root = new Node(ELEMENT);
        root->tag = "ROOT";
        root->explicit_empty_block = false;

        if (is_eml_format) {
            parse_eml_nodes(root);
        } else {
            parse_markup_nodes(root);
        }
        return root;
    }

private:
    char peek() { return pos < len ? input[pos] : 0; }
    char advance() { return pos < len ? input[pos++] : 0; }
    bool eof() { return pos >= len; }
    
    void skip_whitespace() {
        while (!eof() && isspace(peek())) advance();
    }
    
    string read_while(bool (*predicate)(char)) {
        size_t start = pos;
        while (!eof() && predicate(peek())) advance();
        return input.substr(start, pos - start);
    }

    // --- EML Parsing ---
    
    void parse_eml_nodes(Node* parent) {
        while (!eof()) {
            size_t start_ws = pos;
            while (!eof() && isspace(peek())) advance();
            if (pos > start_ws) {
                // capture pure vertical whitespace
                string ws = input.substr(start_ws, pos - start_ws);
                if (std::count(ws.begin(), ws.end(), '\n') > 1) {
                     Node* ws_node = new Node(WHITESPACE);
                     ws_node->content = ws;
                     parent->add_child(ws_node);
                }
            }
            if (eof()) break;

            if (peek() == '/' && pos + 1 < len) {
                if (input[pos+1] == '/') {
                    // Line Comment
                    pos += 2;
                    size_t cstart = pos;
                    while (!eof() && peek() != '\n') advance();
                    Node* c = new Node(COMMENT);
                    c->content = trim(input.substr(cstart, pos - cstart));
                    parent->add_child(c);
                    continue;
                } else if (input[pos+1] == '*') {
                    // Block Comment
                    pos += 2;
                    size_t cstart = pos;
                    size_t cend = input.find("*/", pos);
                    if (cend == string::npos) cend = len;
                    Node* c = new Node(COMMENT_BLOCK);
                    c->content = input.substr(cstart, cend - cstart);
                    parent->add_child(c);
                    pos = (cend == len) ? len : cend + 2;
                    continue;
                }
            }

            // Import special
            if (input.substr(pos, 6) == "import" && (pos+6 >= len || isspace(input[pos+6]))) {
                pos += 6;
                size_t istart = pos;
                size_t iend = input.find(';', pos);
                if (iend != string::npos) {
                    Node* imp = new Node(IMPORT);
                    imp->content = trim(input.substr(istart, iend - istart));
                    parent->add_child(imp);
                    pos = iend + 1;
                    continue;
                }
            }
            
            if (!is_ident_start(peek())) {
                // Unexpected char, advance to avoid infinite loop
                advance(); 
                continue; 
            }

            // Tag Name
            string tag = read_while(is_ident_part);
            Node* el = new Node(ELEMENT);
            el->tag = tag;
            
            skip_whitespace();
            
            // Attributes
            if (!eof() && peek() == '(') {
                advance(); // (
                parse_eml_attrs(el);
            }
            
            skip_whitespace();
            
            // Content
            if (!eof() && peek() == '{') {

                advance(); // {
                el->explicit_empty_block = true; // Just by virtue of having {}
                
                // Mode detection
                int mode = 0; // 0=Markup, 1=Code(php/script), 2=Naive
                if (tag == "script" || tag == "style") mode = 1;
                else if (tag == "php") mode = 1; 
                else if (tag == "pre" || tag == "code") mode = 2; // Naive text capture
                
                
                if (mode == 0) {
                     // Recursive parsing
                     // BUT, we need to handle "inline text" vs "nested elements"
                     // EML allows "div { Some Text }" or "div { span { } }"
                     // We recursively call parse_eml_nodes BUT restriction is } stops it.
                     // Actually, detecting if content is purely text or nested elements is tricky.
                     // Heuristic: If we encounter what looks like a tag start, it's elements.
                     // Otherwise text?
                     // Let's scan content first to see?
                     // No, let's parse recursively. If we hit text that isn't a tag, add TEXT node.

                     parse_eml_block_content(el); 
 
                } else {
                     // Capture raw content balancing braces
                     el->type = (tag == "php") ? PI : ELEMENT; // Treat python as PI for formatting
                     el->content = read_balanced_braces();
                     if (el->type == PI) el->tag = "php"; // Special PI
                     else {
                         // raw content as single text child
                         Node* txt = new Node(TEXT);
                         txt->content = el->content;
                         el->add_child(txt);
                         el->content = "";
                     }
                }
                
                if (el->children.empty() && el->content.empty()) el->explicit_empty_block = true;
                else el->explicit_empty_block = false; // Has content, so flag logic is irrelevant/implicit

            } else {
                 // No content block -> "tag" or "tag (attrs)"
                 el->explicit_empty_block = false;
            }
            
            parent->add_child(el);
        }
    }

    void parse_eml_attrs(Node* node) {
        while (!eof() && peek() != ')') {
            skip_whitespace();
            if (peek() == ')') break;
            
            // Key
            size_t kstart = pos;
            while(!eof() && is_ident_part(peek())) advance();
            string key = input.substr(kstart, pos - kstart);
            
            // =
            skip_whitespace();
            if (peek() == '=') {
                advance();
                skip_whitespace();
                char q = peek();
                if (q == '"' || q == '\'') {
                    advance();
                    size_t vstart = pos;
                    while (!eof() && peek() != q) advance();
                    string val = input.substr(vstart, pos - vstart);
                    if (!eof()) advance(); // close quote
                    node->attrs.push_back({key, val, " "});
                } else {
                    // naked value? not standard EML but maybe supported
                    size_t vstart = pos;
                     while (!eof() && !isspace(peek()) && peek() != ')' && peek() != ',') advance();
                     string val = input.substr(vstart, pos - vstart);
                     node->attrs.push_back({key, val, " "});
                }
            } else {
                // Boolean attr
                node->attrs.push_back({key, "", " "});
            }
            
            skip_whitespace();
            if (peek() == ',') advance();
        }
        if (peek() == ')') advance();
    }
    
    void parse_eml_block_content(Node* parent) {
        // Read ALL content inside braces, preserving whitespace
        string block_inner = read_balanced_braces();

        
        // Check if it contains EML syntax (tags/nested elements)
        if (contains_eml_syntax(block_inner)) {

            // Recurse parser on string
            Parser sub;
            Node* sub_root = sub.parse(block_inner, true);
            for(auto c : sub_root->children) {
                parent->add_child(c);
            }
            sub_root->children.clear();
            delete sub_root;
        } else {

            // Pure text content (may be whitespace-only or actual text)
            // ALWAYS add TEXT node if block has anything (even just whitespace)
            if (!block_inner.empty()) {

                Node* txt = new Node(TEXT);
                txt->content = block_inner;
                parent->add_child(txt);
            }
        }
    }
    
    // Quick helper to duplicate original logic
    bool contains_eml_syntax(const string& text) {
        regex pattern(R"(\b[a-zA-Z_][a-zA-Z0-9_.-]*\s*[({])");
        return regex_search(text, pattern);
    }
    
    // Just parse one node sequence or comment
    void parse_eml_nodes_single_step(Node* parent) {
        // ... handled by the bulk logic usually. 
    }
    
    string read_balanced_braces() {
        string content = ""; // content inside braces
        int depth = 1;
        // We assume we just consumed '{' before calling, OR we are at content start.
        // Actually `parse_eml_nodes` called `advance()` for `{`.
        
        while (!eof() && depth > 0) {
            char c = peek();
            if (c == '{') depth++;
            if (c == '}') {
                depth--;
                if (depth == 0) {
                    advance(); // consume closing
                    break; 
                }
            }
            content += advance();
        }
        return content;
    }

    // --- Markup (HTML/XML) Parsing ---
    
    void parse_markup_nodes(Node* parent) {
        while (!eof()) {
             size_t lt = input.find('<', pos);
             if (lt == string::npos) {
                 // Remaining text
                 if (lt > pos) {
                     string txt = input.substr(pos);
                     if (trim(txt).empty()) {
                         if (std::count(txt.begin(), txt.end(), '\n') > 0) {
                             Node* ws = new Node(WHITESPACE);
                             ws->content = txt;
                             parent->add_child(ws);
                         }
                     } else {
                         Node* n = new Node(TEXT);
                         n->content = txt;
                         parent->add_child(n);
                     }
                 }
                 pos = len; 
                 break;
             }
             
             if (lt > pos) {
                 string txt = input.substr(pos, lt - pos);
                 if (trim(txt).empty()) {
                     if (std::count(txt.begin(), txt.end(), '\n') > 0) {
                         Node* ws = new Node(WHITESPACE);
                         ws->content = txt;
                         parent->add_child(ws);
                     }
                 } else {
                     Node* n = new Node(TEXT);
                     n->content = txt; 
                     parent->add_child(n);
                 }
             }
             pos = lt;
             
             if (pos + 4 <= len && input.substr(pos, 4) == "<!--") {
                 // Comment
                 size_t end = input.find("-->", pos);
                 if (end == string::npos) end = len;
                 Node* c = new Node(COMMENT);
                 c->content = trim(input.substr(pos + 4, end - (pos + 4)));
                 parent->add_child(c);
                 pos = (end == len) ? len : end + 3;
                 continue;
             }
             
             if (pos + 2 <= len && input.substr(pos, 2) == "<?") {
                 // PI
                 size_t end = input.find("?>", pos);
                 if (end == string::npos) end = len;
                 Node* pi = new Node(PI);
                 string raw = input.substr(pos + 2, end - (pos + 2));
                 
                 // Detect php or import
                 if (raw.substr(0, 3) == "php") {
                     pi->tag = "php";
                     pi->content = raw.substr(3);
                 } else if (raw.substr(0, 7) == "import ") {
                     pi->type = IMPORT;
                     pi->content = raw.substr(7);
                 } else {
                     pi->tag = "xml"; // generic
                     pi->content = raw;
                 }
                 parent->add_child(pi);
                 pos = (end == len) ? len : end + 2;
                 continue;
             }
             
             // Tag
             if (pos + 1 < len && input[pos+1] == '/') {
                 // Closing tag - Should not conceptually be reached if recursive parser works right,
                 // UNLESS we have extra closing tags or messed up hierarchy.
                 // We stop here to let caller handle it.
                 return;
             }
             
             // Open Tag
             pos++; // <
             string tag_name = read_while(is_ident_part);
             Node* el = new Node(ELEMENT);
             el->tag = tag_name;
             
             // Attrs
             while (!eof() && peek() != '>' && peek() != '/') {
                 skip_whitespace();
                 if (!is_ident_start(peek())) { 
                     // Handle weird chars or end of tag
                     if(peek() == '>' || peek() == '/') break;
                     advance(); continue; 
                 }
                 
                 string key = read_while(is_ident_part);
                 skip_whitespace();
                 string val = "";
                 
                 if (peek() == '=') {
                     advance();
                     skip_whitespace();
                     char q = peek();
                     if (q == '"' || q == '\'') {
                         advance();
                         size_t vstart = pos;
                         while(!eof() && peek() != q) advance();
                         val = input.substr(vstart, pos - vstart);
                         if(!eof()) advance();
                     } else {
                         size_t vstart = pos;
                         while(!eof() && !isspace(peek()) && peek()!='>' && peek()!='/') advance();
                         val = input.substr(vstart, pos - vstart);
                     }
                 }
                 el->attrs.push_back({key, val, " "});
             }
             
             bool self_closing = false;
             if (peek() == '/') {
                 self_closing = true;
                 advance();
             }
             if (peek() == '>') advance();
             
             parent->add_child(el);
             
             if (!self_closing && !SELF_CLOSING_TAGS.count(tag_name)) {
                 // Recurse for children
                 parse_markup_nodes(el);
                 
                 // consume closing tag
                 if (pos + 2 <= len && input.substr(pos, 2) == "</") {
                     size_t close_start = pos;
                     pos += 2;
                     string ctag = read_while(is_ident_part);
                     if (ctag == tag_name) {
                         while(!eof() && peek() != '>') advance();
                         if(!eof()) advance();
                     } else {
                         // Mismatched tag, backtrack to not consume it?
                         // Or just assume it implies closing of current?
                         // For now, reset pos to close_start so parent can see it
                         pos = close_start;
                     }
                 }
                 
                 // Logic for EML fidelity:
                 // <tag></tag> -> tag {} (explicit empty)
                 // <tag>..</tag> -> tag { .. }
                 if (el->children.empty()) {
                     el->explicit_empty_block = true;
                 } else {
                     el->explicit_empty_block = false; 
                 }
             } else {
                 // <tag /> -> tag (no braces)
                 el->explicit_empty_block = false; 
             }
        }
    }
};

// ======================
// Main
// ======================
// ======================
// Main
// ======================
void print_help() {
    cout << "Copyright (c) 2025 Edanick" << endl;
    cout << endl;
    cout << "EMLC v" << VERSION << endl;
    cout << endl;
    cout << "Usage: emlc <input> <output> [options]" << endl;
    cout << endl;
    cout << "Arguments:" << endl;
    cout << "  <input>      Input file path (.eml, .xml, .html, .php, .xaml, .fxml)" << endl;
    cout << "  <output>     Output file path" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "  -h, --help, /?   Show this help message" << endl;
    cout << "  -v, --version    Show version information" << endl;
    cout << endl;
    cout << "Examples:" << endl;
    cout << "  emlc index.eml index.html       Convert EML to HTML" << endl;
    cout << "  emlc site.eml index.php         Convert EML to PHP" << endl;
    cout << "  emlc view.eml view.xaml         Convert EML to XAML" << endl;
    cout << "  emlc layout.eml layout.fxml     Convert EML to FXML" << endl;
    cout << "  emlc input.eml output.xml       Convert EML to XML" << endl;
    cout << endl;
    cout << "  emlc index.html index.eml       Convert HTML to EML" << endl;
    cout << "  emlc index.php site.eml         Convert PHP to EML" << endl;
    cout << "  emlc view.xaml view.eml         Convert XAML to EML" << endl;
    cout << "  emlc layout.fxml layout.eml     Convert FXML to EML" << endl;
    cout << "  emlc input.xml output.eml       Convert XML to EML" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help();
        return 0;
    }

    string arg1 = argv[1];
    if (arg1 == "-h" || arg1 == "--help" || arg1 == "/?") {
        print_help();
        return 0;
    }
    if (arg1 == "-v" || arg1 == "--version") {
        cout << "emlc version " << VERSION << endl;
        cout << "Copyright (c) 2025 Edanick" << endl;
        return 0;
    }

    if (argc < 3) {
        cerr << "Error: Missing output file path." << endl;
        print_help();
        return 1;
    }

    string input_path = argv[1];
    string output_path = argv[2];

    ifstream infile(input_path);
    if (!infile.is_open()) {
        cerr << "Error: Could not open " << input_path << endl;
        return 1;
    }
    stringstream buffer;
    buffer << infile.rdbuf();
    string content = buffer.str();
    infile.close();

    bool input_is_eml = ends_with(input_path, ".eml");
    bool output_is_xml = ends_with(output_path, ".xml") || ends_with(output_path, ".xaml") || ends_with(output_path, ".fxml");

    Parser parser;
    Node* root = parser.parse(content, input_is_eml);

    Formatter* formatter;
    if (ends_with(output_path, ".eml")) {
        formatter = new EmlFormatter();
    } else {
        formatter = new MarkupFormatter(output_is_xml);
    }

    string result = formatter->format(root);
    
    ofstream outfile(output_path);
    if (!outfile.is_open()) {
        cerr << "Error: Could not open output " << output_path << endl;
        return 1;
    }
    outfile << result;
    outfile.close();

    delete formatter;
    delete root;

    cout << "Converted " << input_path << " -> " << output_path << endl;
    return 0;
}