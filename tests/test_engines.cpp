// Unit tests for both engines. Build: see CMakeLists.txt or run tests/run_tests.sh
#include "../core/engine.h"
#include <cstdio>
#include <string>
using namespace bks;

static std::string U8(const std::wstring& w) {
    std::string o;
    for (wchar_t c : w) {
        unsigned x = (unsigned)c;
        if (x < 0x80) o += (char)x;
        else if (x < 0x800) { o += (char)(0xC0 | x >> 6); o += (char)(0x80 | (x & 0x3F)); }
        else { o += (char)(0xE0 | x >> 12); o += (char)(0x80 | ((x >> 6) & 0x3F)); o += (char)(0x80 | (x & 0x3F)); }
    }
    return o;
}

// Simulates what the TSF layer does: composable keys fill the buffer,
// any other key commits the buffer and is either mapped or passed through.
static std::wstring Type(const Engine& e, const std::string& keys) {
    std::wstring out; std::string buf;
    for (char c : keys) {
        if (e.IsComposable(c)) { buf += c; continue; }
        out += e.Convert(buf); buf.clear();
        std::wstring s = e.Standalone(c);
        out += s.empty() ? std::wstring(1, (wchar_t)c) : s;
    }
    out += e.Convert(buf);
    return out;
}

static int failures = 0, total = 0;
static void Check(const char* name, const Engine& e, const char* keys, const std::wstring& want) {
    ++total;
    std::wstring got = Type(e, keys);
    bool ok = got == want;
    if (!ok) ++failures;
    std::printf("[%s] %-7s %-22s -> %s%s\n", ok ? "PASS" : "FAIL", name, keys, U8(got).c_str(),
                ok ? "" : ("   (expected " + U8(want) + ")").c_str());
}

int main() {
    const Engine& A = AvroEngine(); const Engine& B = BijoyEngine();
    // ---------------- Avro ----------------
    Check("Avro", A, "ami banglay likhi", L"আমি বাংলায় লিখি");
    Check("Avro", A, "kolom", L"কলম");
    Check("Avro", A, "bondhu", L"বন্ধু");
    Check("Avro", A, "shikkha", L"শিক্ষা");
    Check("Avro", A, "bangladesh", L"বাংলাদেশ");
    Check("Avro", A, "prem", L"প্রেম");
    Check("Avro", A, "bakyo", L"বাক্য");
    Check("Avro", A, "bijNGan", L"বিজ্ঞান");
    Check("Avro", A, "ghor", L"ঘর");
    Check("Avro", A, "tumi kemon achO?", L"তুমি কেমন আছো?");
    Check("Avro", A, "1971.", L"১৯৭১।");
    Check("Avro", A, "ca^d", L"চাঁদ");
    Check("Avro", A, "oi", L"ঐ");
    Check("Avro", A, "swopno", L"স্বপ্ন");
    // ---------------- Bijoy ----------------
    Check("Bijoy", B, "Ffdm", L"আমি");
    Check("Bijoy", B, "hfQVf", L"বাংলা");
    Check("Bijoy", B, "dVdJ", L"লিখি");
    Check("Bijoy", B, "jgN", L"ক্ষ");
    Check("Bijoy", B, "gdngjsV", L"ইস্কুল");
    Check("Bijoy", B, "crXN", L"পৌষ");
    Check("Bijoy", B, "crf", L"পো");
    Check("Bijoy", B, "vgm", L"র্ম");
    Check("Bijoy", B, "hz", L"ব্র");
    Check("Bijoy", B, "hZ", L"ব্য");
    Check("Bijoy", B, "gc", L"এ");
    Check("Bijoy", B, "Ww", L"য়য");
    Check("Bijoy", B, "gd", L"ই");  Check("Bijoy", B, "gD", L"ঈ");
    Check("Bijoy", B, "gs", L"উ");  Check("Bijoy", B, "gS", L"ঊ");
    Check("Bijoy", B, "ga", L"ঋ");  Check("Bijoy", B, "gx", L"ঔ");
    Check("Bijoy", B, "gf", L"আ");  Check("Bijoy", B, "gC", L"ঐ");
    Check("Bijoy", B, "gg", L"॥");  Check("Bijoy", B, "g,", L"ৰ"); Check("Bijoy", B, "g.", L"ৱ");
    Check("Bijoy", B, "xFG", L"ওঅ।"); Check("Bijoy", B, "&\\|", L"ঁৎঃ");
    Check("Bijoy", B, "jX", L"কৗ"); Check("Bijoy", B, "jA", L"কৄ");
    Check("Bijoy", B, "g", L"্"); Check("Bijoy", B, "gj", L"্ক");
    Check("Bijoy", B, "1971 hfQVf", L"১৯৭১ বাংলা");
    // backspace = drop last typed char and re-convert (no corruption)
    ++total;
    std::wstring bs = B.Convert(std::string("dVdJ").substr(0, 3));  // second i-kar still waiting for its consonant
    bool ok = (bs == L"লিি");
    if (!ok) ++failures;
    std::printf("[%s] Bijoy   backspace(dVdJ->dVd)   -> %s\n", ok ? "PASS" : "FAIL", U8(bs).c_str());
    ++total;
    std::wstring ba = A.Convert(std::string("kkhm").substr(0, 3));
    ok = (ba == L"ক্ষ");
    if (!ok) ++failures;
    std::printf("[%s] Avro    backspace(kkhm->kkh)  -> %s\n", ok ? "PASS" : "FAIL", U8(ba).c_str());

    std::printf("\n%d/%d passed\n", total - failures, total);
    return failures ? 1 : 0;
}
