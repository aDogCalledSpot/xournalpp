#include "LanguageConfigGui.h"

#include <algorithm>

#include "control/settings/Settings.h"

#include "StringUtils.h"
#include "XojMsgBox.h"
#include "config-paths.h"
#include "config.h"
#include "filesystem.h"
#include "i18n.h"

#ifdef _WIN32
#undef PACKAGE_LOCALE_DIR
#define PACKAGE_LOCALE_DIR "../share/locale/"
#endif

#ifdef __APPLE__
#include "Stacktrace.h"
#undef PACKAGE_LOCALE_DIR
const char* PACKAGE_LOCALE_DIR = (Stacktrace::getExePath() / "../Resources/share/locale").c_str();
#endif

LanguageConfigGui::LanguageConfigGui(GladeSearchpath* gladeSearchPath, GtkWidget* w, Settings* settings):
        GladeGui(gladeSearchPath, "settingsLanguageConfig.glade", "offscreenwindow"), settings(settings) {

    std::locale loc("de_DE.UTF-8");
    auto& facet = std::use_facet<std::messages<char>>(loc);
    auto cat = facet.open("xournalpp", loc);
    auto dropdown = get("languageSettingsDropdown");
    gtk_container_remove(GTK_CONTAINER(getWindow()), dropdown);
    gtk_container_add(GTK_CONTAINER(w), dropdown);
    gtk_widget_show_all(dropdown);

    // Fetch available locales
    try {
        fs::path baseLocaleDir(PACKAGE_LOCALE_DIR);
        for (auto const& d: fs::directory_iterator(baseLocaleDir)) {
            if (fs::exists(d.path() / "LC_MESSAGES" / (std::string(GETTEXT_PACKAGE) + ".mo"))) {
                availableLocales.push_back(d.path().filename().u8string());
            }
        }
    } catch (fs::filesystem_error const& e) {
        XojMsgBox::showErrorToUser(nullptr, e.what());
    }
    std::sort(availableLocales.begin(), availableLocales.end());

    // No pot file for English
    if (auto enPos = std::lower_bound(availableLocales.begin(), availableLocales.end(), "en");
        enPos != availableLocales.end() && !StringUtils::startsWith(*enPos, "en")) {
        availableLocales.insert(enPos, "en");
    }

    // Default
    availableLocales.insert(availableLocales.begin(), facet.get(cat, 0, 0, "System Default"));

    auto gtkAvailableLocales = gtk_list_store_new(1, G_TYPE_STRING);
    for (auto const& i: availableLocales) {
        GtkTreeIter iter;
        gtk_list_store_append(gtkAvailableLocales, &iter);
        gtk_list_store_set(gtkAvailableLocales, &iter, 0, i.c_str(), -1);
    }
    gtk_combo_box_set_model(GTK_COMBO_BOX(dropdown), GTK_TREE_MODEL(gtkAvailableLocales));


    // Set the current locale if previously selected
    auto prefPos = availableLocales.begin();
    if (auto preferred = settings->getPreferredLocale(); !preferred.empty()) {
        prefPos = std::lower_bound(availableLocales.begin(), availableLocales.end(), preferred);
        if (*prefPos != preferred) {
            XojMsgBox::showErrorToUser(nullptr, facet.get(cat, 0, 0, "Previously selected language not available anymore!"));

            // Use system default
            prefPos = availableLocales.begin();
        }
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(dropdown), prefPos - availableLocales.begin());
    facet.close(cat);
}


void LanguageConfigGui::saveSettings() {
    gint pos = gtk_combo_box_get_active(GTK_COMBO_BOX(get("languageSettingsDropdown")));
    auto pref = (pos == 0) ? "" : availableLocales[pos];

    settings->setPreferredLocale(pref);
    settings->customSettingsChanged();
}
