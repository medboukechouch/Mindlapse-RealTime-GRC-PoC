#include <chrono>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace {

constexpr std::size_t IO_BUFFER_SIZE = 1u << 20;  // 1 MB

char to_lower_ascii(char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

bool equals_ci(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (to_lower_ascii(a[i]) != to_lower_ascii(b[i])) {
            return false;
        }
    }
    return true;
}

bool contains_ci(std::string_view text, std::string_view needle) {
    if (needle.empty() || needle.size() > text.size()) {
        return false;
    }

    for (std::size_t i = 0; i + needle.size() <= text.size(); ++i) {
        bool ok = true;
        for (std::size_t j = 0; j < needle.size(); ++j) {
            if (to_lower_ascii(text[i + j]) != to_lower_ascii(needle[j])) {
                ok = false;
                break;
            }
        }
        if (ok) {
            return true;
        }
    }
    return false;
}

std::string_view get_csv_field(std::string_view line, int target_index) {
    int idx = 0;
    std::size_t start = 0;

    for (std::size_t i = 0; i <= line.size(); ++i) {
        if (i == line.size() || line[i] == ',') {
            if (idx == target_index) {
                return line.substr(start, i - start);
            }
            ++idx;
            start = i + 1;
        }
    }

    return {};
}

int find_column_index(std::string_view header, std::string_view name) {
    int idx = 0;
    std::size_t start = 0;

    for (std::size_t i = 0; i <= header.size(); ++i) {
        if (i == header.size() || header[i] == ',') {
            std::string_view col = header.substr(start, i - start);
            if (equals_ci(col, name)) {
                return idx;
            }
            ++idx;
            start = i + 1;
        }
    }

    return -1;
}

int parse_int_fast(std::string_view value) {
    int out = 0;
    bool has_digits = false;

    for (char c : value) {
        if (c >= '0' && c <= '9') {
            has_digits = true;
            out = out * 10 + (c - '0');
        } else if (has_digits) {
            break;
        }
    }

    return has_digits ? out : 0;
}

bool is_suspicious(std::string_view line,
                   int status_idx,
                   int anomaly_label_idx,
                   int attempts_idx,
                   int comment_idx) {
    const std::string_view status = (status_idx >= 0) ? get_csv_field(line, status_idx) : std::string_view{};
    if (equals_ci(status, "Failed")) {
        return true;
    }

    if (anomaly_label_idx >= 0) {
        const std::string_view label = get_csv_field(line, anomaly_label_idx);
        if (!label.empty() && !equals_ci(label, "normal")) {
            return true;
        }
    }

    if (attempts_idx >= 0) {
        const int attempts = parse_int_fast(get_csv_field(line, attempts_idx));
        if (attempts >= 5) {
            return true;
        }
    }

    if (comment_idx >= 0) {
        const std::string_view comment = get_csv_field(line, comment_idx);
        if (contains_ci(comment, "brute") ||
            contains_ci(comment, "scan") ||
            contains_ci(comment, "geo") ||
            contains_ci(comment, "privilege")) {
            return true;
        }
    }

    return false;
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string input_path = (argc > 1) ? argv[1] : "data/linux_auth_logs_labeled.csv";
    const std::string output_path = (argc > 2) ? argv[2] : "data/alerts_filtered.csv";

    std::ifstream in(input_path, std::ios::in | std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Erreur: impossible d'ouvrir le fichier d'entree: " << input_path << "\n";
        return 1;
    }

    std::ofstream out(output_path, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Erreur: impossible d'ouvrir le fichier de sortie: " << output_path << "\n";
        return 2;
    }

    std::string in_buffer(IO_BUFFER_SIZE, '\0');
    std::string out_buffer(IO_BUFFER_SIZE, '\0');
    in.rdbuf()->pubsetbuf(in_buffer.data(), static_cast<std::streamsize>(in_buffer.size()));
    out.rdbuf()->pubsetbuf(out_buffer.data(), static_cast<std::streamsize>(out_buffer.size()));

    auto t0 = std::chrono::high_resolution_clock::now();

    std::string header;
    if (!std::getline(in, header)) {
        std::cerr << "Erreur: fichier CSV vide.\n";
        return 3;
    }

    const int status_idx = find_column_index(header, "status");
    const int anomaly_label_idx = find_column_index(header, "anomaly_label");
    const int attempts_idx = find_column_index(header, "attempts");
    const int comment_idx = find_column_index(header, "comment");

    if (status_idx < 0) {
        std::cerr << "Erreur: colonne 'status' introuvable dans l'en-tete.\n";
        return 4;
    }

    out << header << '\n';

    std::string line;
    std::uint64_t total_lines = 0;
    std::uint64_t suspicious_lines = 0;

    while (std::getline(in, line)) {
        ++total_lines;

        if (line.empty()) {
            continue;
        }

        if (is_suspicious(line, status_idx, anomaly_label_idx, attempts_idx, comment_idx)) {
            out << line << '\n';
            ++suspicious_lines;
        }
    }

    out.flush();

    auto t1 = std::chrono::high_resolution_clock::now();
    const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    const double elapsed_s = static_cast<double>(elapsed_ms) / 1000.0;
    const double throughput = (elapsed_s > 0.0)
                                  ? static_cast<double>(total_lines) / elapsed_s
                                  : 0.0;

    std::cout << "Input: " << input_path << "\n"
              << "Output: " << output_path << "\n"
              << "Total lines read (without header): " << total_lines << "\n"
              << "Suspicious lines extracted: " << suspicious_lines << "\n"
              << "Execution time: " << elapsed_ms << " ms\n"
              << "Throughput: " << static_cast<std::uint64_t>(throughput) << " lines/s\n";

    return 0;
}
