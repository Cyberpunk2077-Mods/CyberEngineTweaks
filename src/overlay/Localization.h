#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct Paths;

// UI localization for CET overlay chrome. Does not change any Lua/public mod APIs.
struct Localization
{
    static constexpr const char* kAuto = "auto";

    struct LanguageInfo
    {
        const char* code; // game-style code, e.g. en-us
        const char* nativeName;
    };

    explicit Localization(Paths& aPaths);

    void Load();
    void SetLanguage(const std::string& aCode);
    [[nodiscard]] const std::string& GetLanguage() const { return m_language; }
    [[nodiscard]] const std::string& GetResolvedLanguage() const { return m_resolved; }
    [[nodiscard]] bool IsAuto() const { return m_language == kAuto || m_language.empty(); }

    [[nodiscard]] const char* Get(const char* aKey) const;

    [[nodiscard]] static const std::vector<LanguageInfo>& SupportedLanguages();
    [[nodiscard]] static std::string NormalizeLanguageCode(std::string aCode);
    [[nodiscard]] static std::string DetectGameOrSystemLanguage();

private:
    void LoadFile(const std::string& aCode, std::unordered_map<std::string, std::string>& aOut) const;
    void EnsureEnglishFallback();

    Paths& m_paths;
    std::string m_language{kAuto};
    std::string m_resolved{"en-us"};
    std::unordered_map<std::string, std::string> m_strings;
    std::unordered_map<std::string, std::string> m_english;
};
