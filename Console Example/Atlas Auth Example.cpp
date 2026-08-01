// Atlas SDK — Console example (C++).
// Build Release / x64. Set your API key in Atlas.h (Atlas::API_KEY = "...").
//
//   Dashboard: https://atlassecurity.site/dashboard
//   Docs:      https://atlassecurity.site/docs
//   Legal:     https://atlassecurity.site/legal

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Atlas.h"

int main()
{
    // Must be called once at startup before any other Atlas functions.
    Atlas::Startup();

    std::cout << "Atlas Authentication Example\n\n"
              << "Choose an auth path:\n"
              << "  [1] License key       (classic, HWID-bound)\n"
              << "  [2] Account sign-in   (username + password + email verification)\n"
              << "  [3] Register account  (creates a new account, optional email (Configured in dashboard))\n\n"
              << "Choice [1/2/3]: ";
    std::string choice; std::getline(std::cin >> std::ws, choice);

    bool authed = false;

    if (choice == "1") {
        std::cout << "Enter license key: ";
        std::string license; std::getline(std::cin >> std::ws, license);
        authed = Atlas::License::Login(license);
    }
    else if (choice == "2") {
        // ================================================================
        // ACCOUNT SIGN-IN - headless flow, every prompt rendered inline.
        //
        // account.Login() returns a status. On NeedsVerification the server
        // emailed an 8-digit code; the SDK is holding the challenge until
        // SubmitVerification(code) is called.
        //
        // You can replace this entire block with a single call:
        //
        //     authed = Atlas::Account::Dialog::Login();
        //
        // - the built-in Win32 sign-in dialog collects credentials, opens
        // the verification-code dialog when the server asks, and closes
        // when the session is live. Same protocol, zero UI code.
        // ================================================================
        std::cout << "Enter username: ";
        std::string username; std::getline(std::cin >> std::ws, username);
        std::cout << "Enter password: ";
        std::string password; std::getline(std::cin >> std::ws, password);

        auto r = Atlas::Account::Login(username, password);
        using S = Atlas::Account::LoginResult::Status;

        if (r.status == S::Ok) {
            authed = true;
        }
        else if (r.status == S::NeedsVerification) {
            std::cout << "\nWe emailed an 8-digit code to " << r.masked_email
                      << " (from " << r.sign_in_ip;
            if (!r.sign_in_country.empty()) std::cout << " / " << r.sign_in_country;
            std::cout << ")\nEnter the code: ";
            std::string code; std::getline(std::cin >> std::ws, code);
            authed = Atlas::Account::SubmitVerification(code);
        }
    }
    else if (choice == "3") {
        // ================================================================
        // REGISTER - create the account and confirm the email if supplied.
        // Register does NOT sign you in - run this example again and pick
        // [2] to sign in whenever you're ready.
        //
        // You can replace this entire block with a single call:
        //
        //     Atlas::Account::Dialog::Register();
        //
        // - the built-in Win32 register dialog collects credentials, opens
        // the email-confirmation dialog automatically when an email was
        // supplied, and reports success. Same protocol, zero UI code.
        // ================================================================
        std::cout << "Pick a username: ";
        std::string username; std::getline(std::cin >> std::ws, username);
        std::cout << "Pick a password: ";
        std::string password; std::getline(std::cin >> std::ws, password);
        std::cout << "Email (optional - enter to skip): ";
        std::string email; std::getline(std::cin, email);

        if (!Atlas::Account::Register(username, password, email)) {
            std::cout << "\n[!] " << Atlas::Data::GetErrorMessage() << "\n";
            std::cout << "\nPress Enter to exit...\n";
            std::cin.get();
            return 1;
        }
        if (Atlas::Account::HasPendingEmailConfirm()) {
            std::cout << "\nWe emailed an 8-digit confirmation code to "
                      << email << ".\nEnter the code: ";
            std::string code; std::getline(std::cin >> std::ws, code);
            if (!Atlas::Account::ConfirmEmail(code)) {
                std::cout << "\n[!] " << Atlas::Data::GetErrorMessage() << "\n";
                std::cout << "\nPress Enter to exit...\n";
                std::cin.get();
                return 1;
            }
        }
        std::cout << "\n[+] Account '" << username << "' is ready. "
                  << "Run this example again and pick [2] to sign in.\n"
                  << "\nPress Enter to exit...\n";
        std::cin.get();
        return 0;
    }
    else {
        std::cout << "\nUnknown choice - exiting.\n";
        return 1;
    }

    if (!authed) {
        std::cout << "\n[!] Authentication failed. "
                  << Atlas::Data::GetErrorMessage() << "\n";
        std::cout << "\nPress Enter to exit...\n";
        std::cin.get();
        return 1;
    }

    // Session data - every field is populated the moment authed is true.
    // GetUsername() is empty on license-only sessions.
    //
    // Note: the SDK's heartbeat thread validates the session every 3-7s on
    // its own. Calling Atlas::Network::CheckAuthentication() here is only
    // needed if your app wants a synchronous "is my session still valid
    // right now" check on demand (e.g. before performing a sensitive
    // action). We skip it in this example because the SDK's own heartbeat
    // is authoritative.
    // On account sessions GetLicense() returns a synthetic "user:<name>" —
    // hide it and print Username instead. On license-only sessions Username
    // is empty and License is the real key.
    std::cout << "\n--- User Information ---\n";
    const bool is_account = !Atlas::Data::GetUsername().empty();
    if (is_account)
        std::cout << "Username:     " << Atlas::Data::GetUsername() << "\n";
    else
        std::cout << "License:      " << Atlas::Data::GetLicense() << "\n";
    std::cout << "Expiry:       " << Atlas::Data::GetExpiry() << "\n"
              << "IP:           " << Atlas::Data::GetIP()     << "\n"
              << "HWID:         " << Atlas::Data::GetHWID()   << "\n"
              << "Level:        " << Atlas::Data::GetLevel()  << "\n"
              << "Note:         " << Atlas::Data::GetNote()   << "\n"
              << "Active Users: " << Atlas::Data::GetActiveUserCount() << "\n"
              << "Total Users:  " << Atlas::Data::GetUserCount()       << "\n";

    // Send a custom log message - appears in your dashboard Logs tab.
    Atlas::Network::SubmitLog("User successfully completed the example");

    // ChangePassword is only meaningful on a password-mode session.
    if (!Atlas::Data::GetUsername().empty()) {
        std::cout << "\nChange password? [y/N]: ";
        std::string yn; std::getline(std::cin, yn);
        if (yn == "y" || yn == "Y") {
            std::cout << "Current password: ";
            std::string oldp; std::getline(std::cin >> std::ws, oldp);
            std::cout << "New password: ";
            std::string newp; std::getline(std::cin >> std::ws, newp);
            if (Atlas::Network::ChangePassword(oldp, newp))
                std::cout << "[+] Password changed. Use the new password on your next sign-in.\n";
            else
                std::cout << "[!] " << Atlas::Data::GetErrorMessage() << "\n";
        }
    }

    // ================================================================
    // OPTIONAL - password reset flow. Not run inline (would interrupt
    // the session we just opened). Two calls, ready to lift:
    //
    //     Atlas::Account::RequestPasswordReset("username-or-email");
    //     // ... user reads the 8-digit code from their email ...
    //     Atlas::Account::CompletePasswordReset(code, new_password);
    //
    // Or, with the built-in UI:
    //
    //     Atlas::Account::Dialog::ResetPassword();
    // ================================================================

    // Download a file uploaded via the Atlas Panel
    //auto fileData = Atlas::Network::Download(1);
    //if (!fileData.empty()) {
    //    std::ofstream file("downloaded_file.bin", std::ios::binary);
    //    file.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());
    //    std::cout << "\nFile downloaded (" << fileData.size() << " bytes)\n";
    //}

    std::cout << "\nPress Enter to exit program fully...\n";
    std::cin.get();
    return 0;
}
