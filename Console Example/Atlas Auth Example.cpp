// Atlas Authentication Library — Example Usage (Console)
// Build as Release/x64 | Set your API key in Atlas.h (Atlas::API_KEY)
//
// This example walks through EVERY auth path Atlas supports so you can test
// each one against your dashboard without editing the code:
//
//     [1] License key      — the classic single-user, HWID-bound flow.
//     [2] Username/password — sign in to a password account you created earlier.
//     [3] Register          — create a username/password account against a
//                             license key. On success we auto-login into it
//                             so you can see the full authenticated session.
//
// Everything downstream — Data::*, Network::CheckAuthentication,
// Network::SubmitLog, Network::ChangePassword — works identically regardless of
// which path you chose.

#include <Windows.h>
#include <iostream>
#include <fstream>
#include "Atlas.h"

// Small helpers to keep main() readable. Nothing Atlas-specific here.
static std::string prompt(const char* label) {
    std::cout << label;
    std::string s;
    std::getline(std::cin >> std::ws, s);
    return s;
}
static void print_error(const char* headline) {
    // Every failure — bad credentials, banned, expired, network — surfaces
    // through GetErrorMessage(). The dashboard Logs tab has the full record.
    std::string err = Atlas::Data::GetErrorMessage();
    std::cout << "\n[!] " << headline << std::endl;
    if (!err.empty()) std::cout << "[!] Reason: " << err << std::endl;
}

int main()
{
    // Must be called once at startup before any other Atlas functions.
    Atlas::Startup();

    std::cout << "Atlas Authentication Example\n" << std::endl;
    std::cout << "Choose an auth path:" << std::endl;
    std::cout << "  [1] License key       (classic, HWID-bound)" << std::endl;
    std::cout << "  [2] Username/password (existing password account)" << std::endl;
    std::cout << "  [3] Register          (bind a license to a new username/password)" << std::endl;
    std::string choice = prompt("\nChoice [1/2/3]: ");

    bool authed = false;

    if (choice == "1") {
        // License-key login — the classic path. HWID + IP binding are set on
        // the dashboard side; the client just presents the key.
        std::string license = prompt("Enter license key: ");
        std::cout << "Connecting to server..." << std::endl;
        authed = Atlas::Login(license);
        if (!authed) print_error("Authentication failed!");
    }
    else if (choice == "2") {
        // Username/password login — for accounts previously created with
        // Register (below) or the dashboard's Users tab.
        std::string username = prompt("Enter username: ");
        std::string password = prompt("Enter password: ");
        std::cout << "Connecting to server..." << std::endl;
        authed = Atlas::Login(username, password);
        if (!authed) print_error("Authentication failed!");
    }
    else if (choice == "3") {
        // Register — one-shot: bind a license key to a new username/password
        // account, then log into it. Register alone does NOT open a session,
        // so we call Login(user, pass) right after to keep the flow going.
        std::string license  = prompt("Enter license key to bind: ");
        std::string username = prompt("Pick a username (3-80 chars): ");
        std::string password = prompt("Pick a password (6-128 chars): ");
        std::cout << "Registering account..." << std::endl;
        if (!Atlas::Register(license, username, password)) {
            print_error("Registration failed!");
        } else {
            // Server auto-prints the success message via GetErrorMessage().
            std::string info = Atlas::Data::GetErrorMessage();
            if (!info.empty()) std::cout << "[+] " << info << std::endl;
            std::cout << "Signing in with the new account..." << std::endl;
            authed = Atlas::Login(username, password);
            if (!authed) print_error("Sign-in after registration failed!");
        }
    }
    else {
        std::cout << "\nUnknown choice — exiting." << std::endl;
        std::cin.get();
        return 1;
    }

    if (!authed) {
        std::cout << "\nPress Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    // Call periodically to verify the session is still valid — the SDK
    // terminates the process if the server has revoked the session.
    Atlas::Network::CheckAuthentication();

    // Access user data after successful authentication. GetUsername() returns
    // the account name for password logins; it's empty for license-only.
    const std::string username = Atlas::Data::GetUsername();
    std::cout << "\n--- User Information ---" << std::endl;
    if (!username.empty())
        std::cout << "Username: " << username << std::endl;
    std::cout << "License:  " << Atlas::Data::GetLicense() << std::endl;
    std::cout << "Expiry:   " << Atlas::Data::GetExpiry() << std::endl;
    std::cout << "IP:       " << Atlas::Data::GetIP() << std::endl;
    std::cout << "HWID:     " << Atlas::Data::GetHWID() << std::endl;
    std::cout << "Level:    " << Atlas::Data::GetLevel() << std::endl;  // int
    std::cout << "Note:     " << Atlas::Data::GetNote() << std::endl;
    std::cout << "Active Users: " << Atlas::Data::GetActiveUserCount() << std::endl;
    std::cout << "Total Users:  " << Atlas::Data::GetUserCount() << std::endl;

    // Send a custom log message — appears in your dashboard Logs tab.
    Atlas::Network::SubmitLog("User successfully completed the example");

    // ChangePassword is only meaningful when you're logged in as a password
    // account. On a license-only session there's nothing to change, so we
    // don't even offer the prompt — mirrors what real apps would do.
    if (!username.empty()) {
        std::cout << "\nChange password? [y/N]: ";
        std::string yn;
        std::getline(std::cin, yn);
        if (yn == "y" || yn == "Y") {
            std::string oldp = prompt("Current password: ");
            std::string newp = prompt("New password (6-128 chars): ");
            if (Atlas::Network::ChangePassword(oldp, newp)) {
                std::cout << "[+] Password changed. Use the new password on your next Login." << std::endl;
            } else {
                print_error("Password change failed!");
            }
        }
    }

    // Download a file uploaded via the Atlas Panel
    //auto fileData = Atlas::Network::Download(1);
    //if (!fileData.empty()) {
    //    std::ofstream file("downloaded_file.bin", std::ios::binary);
    //    file.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());
    //    std::cout << "\nFile downloaded (" << fileData.size() << " bytes)" << std::endl;
    //}

    // Your application code continues here
    std::cout << "\nPress Enter to exit program fully..." << std::endl;
    std::cin.get();
    return 0;
}
