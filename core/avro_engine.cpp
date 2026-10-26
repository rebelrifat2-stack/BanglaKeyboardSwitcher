// Independent Avro-style phonetic engine (NOT a copy of OmicronLab's Avro).
// Rules written from the publicly known Avro phonetic conventions; see
// docs/ARCHITECTURE.md for known differences (no dictionary / auto-correct).
#include "engine.h"
#include <cstring>
#include <cctype>

namespace bks {
namespace {

enum Kind { VOWEL, INHERENT, CONS, CONJ, YA, WA, MARK, BREAK };
struct Rule { const char* roman; Kind kind; const wchar_t* a; const wchar_t* b; };

// a = independent vowel / consonant text ; b = vowel sign (kar)
const Rule kRules[] = {
  // ---- vowels ----
  {"rri", VOWEL, L"\u098B", L"\u09C3"},          // ঋ  ৃ
  {"oi",  VOWEL, L"\u0990", L"\u09C8"},          // ঐ  ৈ
  {"OI",  VOWEL, L"\u0990", L"\u09C8"},
  {"ou",  VOWEL, L"\u0994", L"\u09CC"},          // ঔ  ৌ
  {"OU",  VOWEL, L"\u0994", L"\u09CC"},
  {"aa",  VOWEL, L"\u0986", L"\u09BE"},          // আ  া
  {"ii",  VOWEL, L"\u0988", L"\u09C0"},          // ঈ  ী
  {"ee",  VOWEL, L"\u0988", L"\u09C0"},
  {"uu",  VOWEL, L"\u098A", L"\u09C2"},          // ঊ  ূ
  {"oo",  VOWEL, L"\u0989", L"\u09C1"},          // উ  ু
  {"a",   VOWEL, L"\u0986", L"\u09BE"},          // আ  া
  {"i",   VOWEL, L"\u0987", L"\u09BF"},          // ই  ি
  {"I",   VOWEL, L"\u0988", L"\u09C0"},          // ঈ  ী
  {"u",   VOWEL, L"\u0989", L"\u09C1"},          // উ  ু
  {"U",   VOWEL, L"\u098A", L"\u09C2"},          // ঊ  ূ
  {"e",   VOWEL, L"\u098F", L"\u09C7"},          // এ  ে
  {"O",   VOWEL, L"\u0993", L"\u09CB"},          // ও  ো
  {"o",   INHERENT, L"\u0985", L""},             // অ (no sign after consonant)
  // ---- special conjuncts ----
  {"kkh", CONJ, L"\u0995\u09CD\u09B7", nullptr}, // ক্ষ
  {"kSh", CONJ, L"\u0995\u09CD\u09B7", nullptr},
  {"jNG", CONJ, L"\u099C\u09CD\u099E", nullptr}, // জ্ঞ
  {"nj",  CONJ, L"\u099E\u09CD\u099C", nullptr}, // ঞ্জ
  {"nc",  CONJ, L"\u099E\u09CD\u099A", nullptr}, // ঞ্চ
  {"x",   CONJ, L"\u0995\u09CD\u09B8", nullptr}, // ক্স
  // ---- signs ----
  {"t``", MARK, L"\u09CE", nullptr},             // ৎ
  {"ng",  MARK, L"\u0982", nullptr},             // ং
  {"^",   MARK, L"\u0981", nullptr},             // ঁ
  {"`",   BREAK, L"", nullptr},                  // breaks a conjunct / vowel
  // ---- consonants ----
  {"NG", CONS, L"\u099E", nullptr},              // ঞ
  {"Ng", CONS, L"\u0999", nullptr},              // ঙ
  {"kh", CONS, L"\u0996", nullptr}, {"gh", CONS, L"\u0998", nullptr},
  {"ch", CONS, L"\u099B", nullptr}, {"jh", CONS, L"\u099D", nullptr},
  {"Th", CONS, L"\u09A0", nullptr}, {"Dh", CONS, L"\u09A2", nullptr},
  {"th", CONS, L"\u09A5", nullptr}, {"dh", CONS, L"\u09A7", nullptr},
  {"ph", CONS, L"\u09AB", nullptr}, {"bh", CONS, L"\u09AD", nullptr},
  {"sh", CONS, L"\u09B6", nullptr}, {"Sh", CONS, L"\u09B7", nullptr},
  {"Rh", CONS, L"\u09A2\u09BC", nullptr},
  {"k", CONS, L"\u0995", nullptr}, {"q", CONS, L"\u0995", nullptr},
  {"g", CONS, L"\u0997", nullptr}, {"c", CONS, L"\u099A", nullptr},
  {"j", CONS, L"\u099C", nullptr}, {"T", CONS, L"\u099F", nullptr},
  {"D", CONS, L"\u09A1", nullptr}, {"N", CONS, L"\u09A3", nullptr},
  {"t", CONS, L"\u09A4", nullptr}, {"d", CONS, L"\u09A6", nullptr},
  {"n", CONS, L"\u09A8", nullptr}, {"p", CONS, L"\u09AA", nullptr},
  {"f", CONS, L"\u09AB", nullptr}, {"b", CONS, L"\u09AC", nullptr},
  {"v", CONS, L"\u09AD", nullptr}, {"m", CONS, L"\u09AE", nullptr},
  {"z", CONS, L"\u09AF", nullptr}, {"r", CONS, L"\u09B0", nullptr},
  {"l", CONS, L"\u09B2", nullptr}, {"S", CONS, L"\u09B6", nullptr},
  {"s", CONS, L"\u09B8", nullptr}, {"h", CONS, L"\u09B9", nullptr},
  {"R", CONS, L"\u09A1\u09BC", nullptr},
  {"y", YA, L"", nullptr},                       // য় alone, ্য after consonant
  {"w", WA, L"", nullptr},                       // ও alone, ্ব after consonant
};

const wchar_t HASANTA = L'\u09CD';

class Avro : public Engine {
public:
    bool IsComposable(char c) const override {
        return std::isalpha(static_cast<unsigned char>(c)) || c == '`' || c == '^';
    }

    std::wstring Convert(const std::string& s) const override {
        std::wstring out;
        bool afterCons = false;   // previous token was a consonant (conjunct / kar context)
        size_t i = 0;
        while (i < s.size()) {
            const Rule* best = nullptr; size_t bestLen = 0;
            for (const Rule& r : kRules) {
                size_t n = std::strlen(r.roman);
                if (n > bestLen && s.compare(i, n, r.roman) == 0) { best = &r; bestLen = n; }
            }
            if (!best) {                       // unknown letter: keep it visible
                out += static_cast<wchar_t>(static_cast<unsigned char>(s[i]));
                afterCons = false; ++i; continue;
            }
            i += bestLen;
            switch (best->kind) {
            case VOWEL:
                out += afterCons ? best->b : best->a; afterCons = false; break;
            case INHERENT:
                if (!afterCons) out += best->a;      // 'o' at start / after vowel = অ
                afterCons = false; break;            // after consonant: only breaks conjunct
            case CONS: case CONJ:
                if (afterCons) out += HASANTA;
                out += best->a; afterCons = true; break;
            case YA:
                if (afterCons) { out += HASANTA; out += L'\u09AF'; }   // ্য
                else out += L"\u09AF\u09BC";                                  // য়
                afterCons = true; break;
            case WA:
                if (afterCons) { out += HASANTA; out += L'\u09AC'; afterCons = true; }  // ্ব
                else { out += L'\u0993'; afterCons = false; }                           // ও
                break;
            case MARK:  out += best->a; afterCons = false; break;
            case BREAK: afterCons = false; break;
            }
        }
        return out;
    }

    std::wstring Standalone(char c) const override {
        if (c >= '0' && c <= '9') return std::wstring(1, static_cast<wchar_t>(L'\u09E6' + (c - '0')));
        if (c == '.') return L"\u0964";   // ।
        if (c == '$') return L"\u09F3";   // ৳
        return std::wstring();
    }
};

} // namespace

const Engine& AvroEngine() { static Avro e; return e; }

} // namespace bks
