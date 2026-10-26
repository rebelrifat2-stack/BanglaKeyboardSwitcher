// Independent Bijoy-52-style fixed-layout engine producing Unicode.
//
// Layout source: official "বিজয় বাংলা কীবোর্ড – তৃতীয় সংস্করণ" chart supplied by the project
// owner (4 layers: normal, shift, link+normal, link+shift; the "link" key is G = hasanta).
// Digit-row link layers (fraction numerals etc.) and a few rare signs are NOT implemented.
// Legacy SutonnyMJ (ASCII font-encoded) output is NOT implemented - Unicode output only.
// Bijoy is a prefix-vowel layout: i-kar / e-kar / oi-kar are typed BEFORE the consonant.
// Unicode needs them AFTER, so Convert() re-orders them.
#include "engine.h"
#include <map>

namespace bks {
namespace {

enum Kind {
    K_CONS,     // consonant
    K_PHALA,    // ্র / ্য  (attaches to the current consonant)
    K_HASANTA,  // ্  (or "independent vowel" prefix when not after a consonant)
    K_SUFFIX,   // vowel sign that follows the consonant:  া ু ূ ৃ ী ৗ
    K_PREFIX,   // vowel sign typed before the consonant:  ি ে ৈ
    K_OTHER     // independent vowels অ ও ঔ ঋ, signs ং ঁ ঃ ৎ, digits, । ৳ ...
};
struct Entry { Kind kind; const wchar_t* text; const wchar_t* indep; };

const std::map<char, Entry>& Table() {
    static const std::map<char, Entry> t = {
        // ---- consonants (normal / shifted) ----
        {'q',{K_CONS,L"\u0999",0}}, {'Q',{K_OTHER,L"\u0982",0}},   // ঙ  ং
        {'w',{K_CONS,L"\u09AF",0}}, {'W',{K_CONS,L"\u09AF\u09BC",0}},    // য  য়
        {'e',{K_CONS,L"\u09A1",0}}, {'E',{K_CONS,L"\u09A2",0}},    // ড  ঢ
        {'r',{K_CONS,L"\u09AA",0}}, {'R',{K_CONS,L"\u09AB",0}},    // প  ফ
        {'t',{K_CONS,L"\u099F",0}}, {'T',{K_CONS,L"\u09A0",0}},    // ট  ঠ
        {'y',{K_CONS,L"\u099A",0}}, {'Y',{K_CONS,L"\u099B",0}},    // চ  ছ
        {'u',{K_CONS,L"\u099C",0}}, {'U',{K_CONS,L"\u099D",0}},    // জ  ঝ
        {'i',{K_CONS,L"\u09B9",0}}, {'I',{K_CONS,L"\u099E",0}},    // হ  ঞ
        {'o',{K_CONS,L"\u0997",0}}, {'O',{K_CONS,L"\u0998",0}},    // গ  ঘ
        {'p',{K_CONS,L"\u09A1\u09BC",0}}, {'P',{K_CONS,L"\u09A2\u09BC",0}},    // ড়  ঢ়
        {'h',{K_CONS,L"\u09AC",0}}, {'H',{K_CONS,L"\u09AD",0}},    // ব  ভ
        {'j',{K_CONS,L"\u0995",0}}, {'J',{K_CONS,L"\u0996",0}},    // ক  খ
        {'k',{K_CONS,L"\u09A4",0}}, {'K',{K_CONS,L"\u09A5",0}},    // ত  থ
        {'l',{K_CONS,L"\u09A6",0}}, {'L',{K_CONS,L"\u09A7",0}},    // দ  ধ
        {'v',{K_CONS,L"\u09B0",0}}, {'V',{K_CONS,L"\u09B2",0}},    // র  ল
        {'b',{K_CONS,L"\u09A8",0}}, {'B',{K_CONS,L"\u09A3",0}},    // ন  ণ
        {'n',{K_CONS,L"\u09B8",0}}, {'N',{K_CONS,L"\u09B7",0}},    // স  ষ
        {'m',{K_CONS,L"\u09AE",0}}, {'M',{K_CONS,L"\u09B6",0}},    // ম  শ
        // ---- vowel keys ----
        {'F',{K_OTHER,L"\u0985",0}},                                // অ
        {'x',{K_OTHER,L"\u0993",L"\u0994"}},                       // ও  (link+x = ঔ)
        // ---- vowel signs (indep = form produced after the hasanta key) ----
        {'f',{K_SUFFIX,L"\u09BE",L"\u0986"}},                       // া  (আ)
        {'s',{K_SUFFIX,L"\u09C1",L"\u0989"}},                       // ু  (উ)
        {'S',{K_SUFFIX,L"\u09C2",L"\u098A"}},                       // ূ  (ঊ)
        {'a',{K_SUFFIX,L"\u09C3",L"\u098B"}},                       // ৃ  (ঋ)
        {'D',{K_SUFFIX,L"\u09C0",L"\u0988"}},                       // ী  (ঈ)
        {'X',{K_SUFFIX,L"\u09D7",0}},                               // ৗ  (shift+X; e-kar + ৗ = ৌ)
        {'A',{K_SUFFIX,L"\u09C4",0}},                               // ৄ  (shift+A)
        {'d',{K_PREFIX,L"\u09BF",L"\u0987"}},                       // ি  (ই)
        {'c',{K_PREFIX,L"\u09C7",L"\u098F"}},                       // ে  (এ)
        {'C',{K_PREFIX,L"\u09C8",L"\u0990"}},                       // ৈ  (ঐ)
        // ---- hasanta, phala, punctuation ----
        {'g',{K_HASANTA,L"\u09CD",L"\u0965"}},   // link key; link+g = ॥
        {'G',{K_OTHER,L"\u0964",0}},                                // ।
        {'z',{K_PHALA,L"\u09CD\u09B0",0}},                          // ্র
        {'Z',{K_PHALA,L"\u09CD\u09AF",0}},                          // ্য
        // ---- signs ----
        {'&',{K_OTHER,L"\u0981",0}},                                // ঁ  (shift+7, per chart)
        {'\\',{K_OTHER,L"\u09CE",0}},                              // ৎ  (\ key)
        {'|',{K_OTHER,L"\u0983",0}},                                // ঃ  (shift+\)
        {'$',{K_OTHER,L"\u09F3",0}},                                // ৳  (shift+4; glyph hard to read on chart)
        {',',{K_OTHER,L",",L"\u09F0"}},                             // ,  (link+, = ৰ Assamese ra)
        {'.',{K_OTHER,L".",L"\u09F1"}},                             // .  (link+. = ৱ Assamese wa)
        // ---- digits ----
        {'0',{K_OTHER,L"\u09E6",0}}, {'1',{K_OTHER,L"\u09E7",0}},
        {'2',{K_OTHER,L"\u09E8",0}}, {'3',{K_OTHER,L"\u09E9",0}},
        {'4',{K_OTHER,L"\u09EA",0}}, {'5',{K_OTHER,L"\u09EB",0}},
        {'6',{K_OTHER,L"\u09EC",0}}, {'7',{K_OTHER,L"\u09ED",0}},
        {'8',{K_OTHER,L"\u09EE",0}}, {'9',{K_OTHER,L"\u09EF",0}},
    };
    return t;
}

enum Last { L_NONE, L_CONS, L_HAS, L_OTHER };

class Bijoy : public Engine {
public:
    bool IsComposable(char c) const override { return Table().count(c) != 0; }

    std::wstring Convert(const std::string& typed) const override {
        std::wstring out, pend;   // pend = prefix vowel sign waiting for its consonant cluster
        Last last = L_NONE;
        bool indep = false;       // hasanta typed outside a cluster: next sign becomes an independent vowel

        auto flush = [&]() { out += pend; pend.clear(); };

        for (char ch : typed) {
            auto it = Table().find(ch);
            if (it == Table().end()) continue;
            const Entry& e = it->second;

            // A pending "independent vowel" hasanta that is not followed by a
            // vowel-sign key is just a plain hasanta.
            if (indep && !e.indep && e.kind != K_HASANTA) { flush(); out += L'\u09CD'; indep = false; last = L_OTHER; }

            switch (e.kind) {
            case K_CONS:
                if (!pend.empty() && last == L_CONS) flush();   // previous cluster complete
                out += e.text; last = L_CONS; break;
            case K_PHALA:
                out += e.text; last = (last == L_CONS) ? L_CONS : L_OTHER; break;
            case K_HASANTA:
                if (last == L_CONS) { out += e.text; last = L_HAS; }
                else if (indep) { out += e.indep; indep = false; last = L_OTHER; }   // link+link = ॥
                else indep = true;
                break;
            case K_SUFFIX:
                if (indep) { out += e.indep; indep = false; last = L_OTHER; break; }
                if (pend == L"\u09C7" && e.text[0] == L'\u09BE') { out += L"\u09CB"; pend.clear(); }       // ে + া = ো
                else if (pend == L"\u09C7" && e.text[0] == L'\u09D7') { out += L"\u09CC"; pend.clear(); }  // ে + ৗ = ৌ
                else if (e.text[0] == L'\u09BE' && pend.empty() && !out.empty() && out.back() == L'\u0985') {
                    out.back() = L'\u0986';                                                                // অ + া = আ
                } else { flush(); out += e.text; }
                last = L_OTHER; break;
            case K_PREFIX:
                if (indep) { out += e.indep; indep = false; last = L_OTHER; break; }
                flush(); pend = e.text; last = L_OTHER; break;
            case K_OTHER:
                flush(); out += (indep && e.indep) ? e.indep : e.text; indep = false; last = L_OTHER; break;
            }
        }
        if (indep) { flush(); out += L'\u09CD'; }
        flush();
        return out;
    }
};

} // namespace

const Engine& BijoyEngine() { static Bijoy e; return e; }

} // namespace bks
