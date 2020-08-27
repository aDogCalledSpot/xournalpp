#include "LanguageConfigGui.h"
#include "i18n.h"
#include "control/settings/Settings.h"
#include "config-paths.h"
#include "filesystem.h"
#include "config.h"
#include <iostream>
#include <algorithm>

LanguageConfigGui::LanguageConfigGui(GladeSearchpath* gladeSearchPath, GtkWidget* w, Settings* settings) :
        GladeGui(gladeSearchPath, "settingsLanguageConfig.glade", "offscreenwindow"), settings(settings) {
    auto dropdown = get("languageSettingsDropdown");
    gtk_container_remove(GTK_CONTAINER(getWindow()), dropdown);
    gtk_container_add(GTK_CONTAINER(w), dropdown);
    gtk_widget_show_all(dropdown);

    // Fetch available locales
    fs::path baseLocaleDir(PACKAGE_LOCALE_DIR);
    for (auto const& d: fs::directory_iterator(baseLocaleDir)) {
        if (fs::exists(d.path() / "LC_MESSAGES" / (std::string(GETTEXT_PACKAGE) + ".mo"))) {
            availableLocales.push_back(d.path().filename().u8string());
        }
    }
    std::sort(availableLocales.begin(), availableLocales.end());

    auto gtkAvailableLocales = gtk_list_store_new(1, G_TYPE_STRING);
    for (auto const& i : availableLocales) {
            GtkTreeIter iter;
            gtk_list_store_append(gtkAvailableLocales, &iter);
            gtk_list_store_set(gtkAvailableLocales, &iter, 0, i.c_str(), -1);

    }
    gtk_combo_box_set_model(GTK_COMBO_BOX(dropdown), GTK_TREE_MODEL(gtkAvailableLocales));
}


void LanguageConfigGui::saveSettings() {
    gint pos = gtk_combo_box_get_active(GTK_COMBO_BOX(get("languageSettingsDropdown")));

    settings->setPreferredLocale(availableLocales[pos]);
    settings->customSettingsChanged();
}
