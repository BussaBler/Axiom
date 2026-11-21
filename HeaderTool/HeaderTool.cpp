// AxiomHeaderTool
// Scans component headers for AX_COMPONENT() / AX_PROPERTY(...) markers and generates
// ComponentRegistry::registerReflectedComponents().
//
// Usage: AxiomHeaderTool <InputDir> <OutputFile> <IncludePrefix>
//   e.g. AxiomHeaderTool Source/ECS/Components build/Generated/ReflectedComponents.cpp ECS/Components
//
// Supported markers:
//   struct AX_COMPONENT() Name { ... };
//   AX_PROPERTY() Type name = default;
//   AX_PROPERTY(AssetType::Texture) UUID textureId;
//   AX_PROPERTY(Options("A", "B", "C")) MyEnum mode = MyEnum::A;
//
// The tool only needs names: field types are deduced by the compiler (fieldTypeOf<T>()).
// It depends on nothing from the engine on purpose.

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {
    struct ParsedField {
        std::string name;
        std::vector<std::string> modifiers;
    };

    struct ParsedComponent {
        std::string name;
        std::string headerFile;
        std::vector<ParsedField> fields;
    };

    struct ToolError {
        std::string message;
        size_t position;
    };

    bool isIdentChar(char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    }

    std::string trim(std::string_view s) {
        size_t begin = 0;
        size_t end = s.size();
        while (begin < end && std::isspace(static_cast<unsigned char>(s[begin]))) {
            begin++;
        }
        while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
            end--;
        }
        return std::string(s.substr(begin, end - begin));
    }

    std::optional<std::string> readFile(const std::filesystem::path& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            return std::nullopt;
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    std::string stripComments(const std::string& src) {
        std::string out = src;
        size_t i = 0;
        while (i < out.size()) {
            const char c = out[i];
            if (c == '"' || c == '\'') {
                const char quote = c;
                i++;
                while (i < out.size() && out[i] != quote) {
                    if (out[i] == '\\') {
                        i++;
                    }
                    i++;
                }
                i++;
            } else if (c == '/' && i + 1 < out.size() && out[i + 1] == '/') {
                while (i < out.size() && out[i] != '\n') {
                    out[i++] = ' ';
                }
            } else if (c == '/' && i + 1 < out.size() && out[i + 1] == '*') {
                while (i < out.size() && !(out[i] == '*' && i + 1 < out.size() && out[i + 1] == '/')) {
                    if (out[i] != '\n') {
                        out[i] = ' ';
                    }
                    i++;
                }
                if (i < out.size()) {
                    out[i] = ' ';
                    out[i + 1] = ' ';
                    i += 2;
                }
            } else {
                i++;
            }
        }
        return out;
    }

    size_t lineOf(const std::string& src, size_t position) {
        return 1 + static_cast<size_t>(std::count(src.begin(), src.begin() + std::min(position, src.size()), '\n'));
    }

    size_t skipWhitespace(const std::string& src, size_t i) {
        while (i < src.size() && std::isspace(static_cast<unsigned char>(src[i]))) {
            i++;
        }
        return i;
    }

    size_t skipStringLiteral(const std::string& src, size_t i) {
        const char quote = src[i++];
        while (i < src.size() && src[i] != quote) {
            if (src[i] == '\\') {
                i++;
            }
            i++;
        }
        return i + 1;
    }

    size_t findMatching(const std::string& src, size_t open, char openChar, char closeChar) {
        int depth = 0;
        size_t i = open;
        while (i < src.size()) {
            const char c = src[i];
            if (c == '"' || c == '\'') {
                i = skipStringLiteral(src, i);
                continue;
            }
            if (c == openChar) {
                depth++;
            } else if (c == closeChar) {
                depth--;
                if (depth == 0) {
                    return i;
                }
            }
            i++;
        }
        throw ToolError{std::string("Missing closing '") + closeChar + "'", open};
    }

    size_t findMarker(const std::string& src, std::string_view marker, size_t from, size_t to) {
        size_t pos = src.find(marker, from);
        while (pos != std::string::npos && pos < to) {
            const bool startOk = pos == 0 || !isIdentChar(src[pos - 1]);
            const bool endOk = pos + marker.size() >= src.size() || !isIdentChar(src[pos + marker.size()]);
            if (startOk && endOk) {
                return pos;
            }
            pos = src.find(marker, pos + 1);
        }
        return std::string::npos;
    }

    std::string readIdentifier(const std::string& src, size_t& i) {
        i = skipWhitespace(src, i);
        const size_t start = i;
        while (i < src.size() && isIdentChar(src[i])) {
            i++;
        }
        return src.substr(start, i - start);
    }

    std::vector<std::string> splitArguments(std::string_view args) {
        std::vector<std::string> result;
        std::string current;
        int depth = 0;
        for (size_t i = 0; i < args.size(); i++) {
            const char c = args[i];
            if (c == '"' || c == '\'') {
                const char quote = c;
                current += c;
                i++;
                while (i < args.size() && args[i] != quote) {
                    if (args[i] == '\\' && i + 1 < args.size()) {
                        current += args[i++];
                    }
                    current += args[i++];
                }
                if (i < args.size()) {
                    current += args[i];
                }
                continue;
            }
            if (c == '(' || c == '{' || c == '[') {
                depth++;
            } else if (c == ')' || c == '}' || c == ']') {
                depth--;
            } else if (c == ',' && depth == 0) {
                result.push_back(trim(current));
                current.clear();
                continue;
            }
            current += c;
        }
        if (!trim(current).empty()) {
            result.push_back(trim(current));
        }
        return result;
    }

    std::string toModifier(const std::string& arg, size_t position) {
        if (arg.starts_with("AssetType::")) {
            return ".asset(" + arg + ")";
        }
        if (arg.starts_with("Options(") && arg.ends_with(")")) {
            const std::string inner = trim(std::string_view(arg).substr(8, arg.size() - 9));
            for (const auto& option : splitArguments(inner)) {
                if (option.size() < 2 || option.front() != '"' || option.back() != '"') {
                    throw ToolError{"Options(...) only accepts string literals, got: " + option, position};
                }
            }
            return ".options({" + inner + "})";
        }
        throw ToolError{"Unknown AX_PROPERTY argument: '" + arg + "' (expected AssetType::X or Options(\"...\"))", position};
    }

    std::string parseFieldName(const std::string& src, size_t& i, size_t bodyEnd) {
        const size_t start = i;
        size_t cut = std::string::npos;
        int depth = 0;
        while (i < bodyEnd) {
            const char c = src[i];
            if (c == '"' || c == '\'') {
                i = skipStringLiteral(src, i);
                continue;
            }
            if (depth == 0 && cut == std::string::npos && (c == '=' || c == '{')) {
                cut = i;
            }
            if (c == '(' || c == '{' || c == '[') {
                depth++;
            } else if (c == ')' || c == '}' || c == ']') {
                depth--;
            } else if (c == ';' && depth == 0) {
                break;
            }
            i++;
        }
        if (i >= bodyEnd) {
            throw ToolError{"AX_PROPERTY declaration has no terminating ';'", start};
        }

        const size_t declEnd = cut == std::string::npos ? i : cut;
        i++;

        size_t nameEnd = declEnd;
        while (nameEnd > start && std::isspace(static_cast<unsigned char>(src[nameEnd - 1]))) {
            nameEnd--;
        }
        size_t nameStart = nameEnd;
        while (nameStart > start && isIdentChar(src[nameStart - 1])) {
            nameStart--;
        }
        const std::string name = src.substr(nameStart, nameEnd - nameStart);
        const std::string type = trim(std::string_view(src).substr(start, nameStart - start));

        if (name.empty() || type.empty() || std::isdigit(static_cast<unsigned char>(name[0]))) {
            throw ToolError{"Could not read the field declaration after AX_PROPERTY(...)", start};
        }
        return name;
    }

    std::vector<ParsedComponent> parseHeader(const std::string& original, const std::string& fileName) {
        const std::string src = stripComments(original);
        std::vector<ParsedComponent> components;

        size_t searchFrom = 0;
        while (true) {
            const size_t marker = findMarker(src, "AX_COMPONENT", searchFrom, src.size());
            if (marker == std::string::npos) {
                break;
            }

            size_t i = skipWhitespace(src, marker + std::string_view("AX_COMPONENT").size());
            if (i >= src.size() || src[i] != '(') {
                throw ToolError{"Expected '(' after AX_COMPONENT", marker};
            }
            i = findMatching(src, i, '(', ')') + 1;

            std::string name = readIdentifier(src, i);
            if (name == "struct" || name == "class") {
                name = readIdentifier(src, i);
            }
            if (name.empty()) {
                throw ToolError{"Expected a component name after AX_COMPONENT()", marker};
            }

            const size_t bodyOpen = src.find('{', i);
            if (bodyOpen == std::string::npos) {
                throw ToolError{"Expected '{' after component " + name, marker};
            }
            const size_t bodyClose = findMatching(src, bodyOpen, '{', '}');

            ParsedComponent component{name, fileName, {}};

            size_t fieldSearch = bodyOpen + 1;
            while (true) {
                const size_t prop = findMarker(src, "AX_PROPERTY", fieldSearch, bodyClose);
                if (prop == std::string::npos) {
                    break;
                }

                size_t j = skipWhitespace(src, prop + std::string_view("AX_PROPERTY").size());
                if (j >= bodyClose || src[j] != '(') {
                    throw ToolError{"Expected '(' after AX_PROPERTY", prop};
                }
                const size_t argsClose = findMatching(src, j, '(', ')');
                const std::string args = src.substr(j + 1, argsClose - j - 1);
                j = argsClose + 1;

                ParsedField field;
                field.name = parseFieldName(src, j, bodyClose);
                for (const auto& arg : splitArguments(args)) {
                    field.modifiers.push_back(toModifier(arg, prop));
                }
                component.fields.push_back(std::move(field));
                fieldSearch = j;
            }

            components.push_back(std::move(component));
            searchFrom = bodyClose + 1;
        }
        return components;
    }

    std::string generate(std::vector<ParsedComponent> components, const std::string& includePrefix) {
        std::sort(components.begin(), components.end(), [](const auto& a, const auto& b) { return a.name < b.name; });

        std::vector<std::string> headers;
        for (const auto& c : components) {
            headers.push_back(c.headerFile);
        }
        std::sort(headers.begin(), headers.end());
        headers.erase(std::unique(headers.begin(), headers.end()), headers.end());

        std::ostringstream out;
        out << "// AUTO-GENERATED BY AXIOM HEADER TOOL. DO NOT EDIT.\n"
            << "#include \"axpch.h\"\n\n"
            << "#include \"ECS/Reflection.h\"\n\n";
        for (const auto& header : headers) {
            out << "#include \"" << includePrefix << "/" << header << "\"\n";
        }

        out << "\nnamespace Axiom {\n"
            << "    void ComponentRegistry::registerReflectedComponents() {\n";
        for (size_t c = 0; c < components.size(); c++) {
            const auto& component = components[c];
            if (c > 0) {
                out << "\n";
            }
            out << "        add<" << component.name << ">(\"" << component.name << "\", {\n";
            for (const auto& field : component.fields) {
                out << "            Field::make(\"" << field.name << "\", &" << component.name << "::" << field.name << ")";
                for (const auto& modifier : field.modifiers) {
                    out << modifier;
                }
                out << ",\n";
            }
            out << "        });\n";
        }
        out << "    }\n"
            << "} // namespace Axiom\n";
        return out.str();
    }
} // namespace

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: AxiomHeaderTool <InputDir> <OutputFile> <IncludePrefix>\n";
        return 1;
    }

    const std::filesystem::path inputDir = argv[1];
    const std::filesystem::path outputFile = argv[2];
    const std::string includePrefix = argv[3];

    std::vector<std::filesystem::path> headerPaths;
    for (const auto& entry : std::filesystem::directory_iterator(inputDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".h") {
            headerPaths.push_back(entry.path());
        }
    }
    std::sort(headerPaths.begin(), headerPaths.end());

    std::vector<ParsedComponent> components;
    for (const auto& path : headerPaths) {
        const auto content = readFile(path);
        if (!content) {
            std::cerr << path.generic_string() << ": error: could not read file\n";
            return 1;
        }
        try {
            for (auto& component : parseHeader(*content, path.filename().string())) {
                components.push_back(std::move(component));
            }
        } catch (const ToolError& e) {
            std::cerr << path.generic_string() << ":" << lineOf(*content, e.position) << ": error: " << e.message << "\n";
            return 1;
        }
    }

    for (size_t i = 0; i < components.size(); i++) {
        for (size_t j = i + 1; j < components.size(); j++) {
            if (components[i].name == components[j].name) {
                std::cerr << "error: component '" << components[i].name << "' is declared in both " << components[i].headerFile << " and "
                          << components[j].headerFile << "\n";
                return 1;
            }
        }
    }

    const std::string generated = generate(std::move(components), includePrefix);

    if (const auto existing = readFile(outputFile); existing && *existing == generated) {
        std::cout << "[HeaderTool] Reflection up to date\n";
        return 0;
    }

    std::filesystem::create_directories(outputFile.parent_path());
    std::ofstream out(outputFile, std::ios::binary);
    if (!out) {
        std::cerr << outputFile.generic_string() << ": error: could not write file\n";
        return 1;
    }
    out << generated;
    std::cout << "[HeaderTool] Generated reflection for " << headerPaths.size() << " headers\n";
    return 0;
}
