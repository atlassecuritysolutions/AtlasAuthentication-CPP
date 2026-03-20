// Atlas Authentication Library - Example Usage
// Build as Release/x64 | Set your API key in Atlas.h (Atlas::API_KEY)

#include <Windows.h>
#include <iostream>
#include <fstream>
#include "Atlas.h"

int main()
{
    // Must be called once at startup before any other Atlas functions
    Atlas::Startup();

    std::cout << "Atlas Authentication Example\n" << std::endl;

    // Prompt user for license key and authenticate with the server
    std::cout << "Enter license: ";
    std::string license;
    std::getline(std::cin >> std::ws, license);
    std::cout << "Attempting to connect to server..." << std::endl;

    if (!Atlas::Login(license)) {
        // Check the Logs tab in your dashboard for full details
        std::string errorMsg = Atlas::Data::GetErrorMessage();
        std::cout << "\n[!] Authentication failed!" << std::endl;
        if (!errorMsg.empty())
            std::cout << "[!] Reason: " << errorMsg << std::endl;
        std::cout << "\nPress Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    // Call periodically to verify the session is still valid — terminates if not
    Atlas::Network::CheckAuthentication();

    // Access user data after successful authentication
    std::cout << "\n--- User Information ---" << std::endl;
    std::cout << "License: "      << Atlas::Data::GetLicense()         << std::endl;
    std::cout << "Expiry: "       << Atlas::Data::GetExpiry()          << std::endl;
    std::cout << "IP: "           << Atlas::Data::GetIP()              << std::endl;
    std::cout << "HWID: "         << Atlas::Data::GetHWID()            << std::endl;
    std::cout << "Level: "        << Atlas::Data::GetLevel()           << std::endl;
    std::cout << "Note: "         << Atlas::Data::GetNote()            << std::endl;
    std::cout << "Active Users: " << Atlas::Data::GetActiveUserCount() << std::endl;
    std::cout << "Total Users: "  << Atlas::Data::GetUserCount()       << std::endl;

    // Send a custom log message — appears in your dashboard Logs tab
    Atlas::Network::SubmitLog("User successfully completed example");

    // Download a file uploaded via the Atlas Dashboard, this usage example also writes it to disk!
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
