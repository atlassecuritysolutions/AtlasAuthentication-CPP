#pragma once
#include <string>
#include <vector>
#include <cstdint>

// ============================================================================
// ATLAS AUTHENTICATION LIBRARY - API REFERENCE
// ============================================================================
// This header provides all the functions you need to integrate Atlas Auth
// into your application. See the example below for usage.
// ============================================================================

namespace Atlas {

    // ========================================================================
    // CONFIGURATION - Set your API key here
    // ========================================================================
    // Get your API key from atlassecurity.site/dashboard after creating an application
    inline std::string API_KEY = "YOUR_API_KEY";

    // ========================================================================
    // CORE AUTHENTICATION FUNCTIONS
    // ========================================================================

    // Initialize the authentication system (call once at top of program start or main)
    void Startup();

    // Authenticate a user with their license key. Returns true on success, false otherwise.
    // The classic single-user, hardware-bound flow.
    bool Login(const std::string& license_key);

    // Authenticate a user by username + password (an account previously created via Register).
    // Same session shape after success; every Data::* / Network::* call works identically.
    bool Login(const std::string& username, const std::string& password);

    // One-shot registration: bind a license key to a new username/password account.
    // Returns true on success. On success the caller can immediately call
    // Login(username, password) to open a session — Register itself does not
    // open one. Fails if the license already has an account, the username is
    // taken, the license is invalid, or the credentials shape is rejected.
    bool Register(const std::string& license_key,
                  const std::string& username,
                  const std::string& password);

    // Logout and terminate the current session and clear authentication state
    void Logout();

    // ========================================================================
    // NETWORK - Server Communication Functions
    // ========================================================================
    namespace Network {
        // Verify the user's session is still valid with the server
        // Call periodically to ensure user hasn't been banned/expired
        // Useful also to check against tampering or modifying within the programs logic
        bool CheckAuthentication();

        // Download a file from the server by file ID
        // Returns file data as byte vector, empty on failure
        std::vector<uint8_t> Download(int file_id);

        // Report a user for banning (requires appropriate permissions)
        // ban_reason: Reason for the ban (appears in dashboard)
        // duration_minutes: Ban duration in minutes (0 = permanent)
        bool BanUser(const std::string& ban_reason, int duration_minutes);

        // Submit a custom log entry to appear in your applications logs (max 512 characters)
        bool SubmitLog(const char* log_text);

        // Change the current password account's password.
        // Only valid after Atlas::Login(username, password) — license-only sessions
        // return false with an explanatory message in Data::GetErrorMessage().
        // new_password must be 6–128 chars and differ from the old one.
        bool ChangePassword(const std::string& old_password, const std::string& new_password);
    }

    // ========================================================================
    // DATA - User Information and Statistics
    // ========================================================================
    namespace Data {
        // User license information (available in dataset after login)
        std::string GetLicense();               // License key used during login
        std::string GetUsername();              // Password-account username (empty for license-only logins)
        std::string GetIP();                    // IP address detected by server
        std::string GetHWID();                  // Hardware ID for device binding
        std::string GetDevice();                // Client-supplied ComputerName/Username from last auth
        std::string GetExpiry();                // Expiration date (DD-MM-YYYY HH:MM:SS) or "Lifetime"
        std::string GetNote();                  // Custom note set by admin (empty if none)
        int GetLevel();                         // Numeric access level (0 when unknown)
        std::string GetFirstSeenDate();         // First authentication date (DD-MM-YYYY HH:MM:SS)
        std::string GetLastSeenDate();          // Last authentication date (DD-MM-YYYY HH:MM:SS)

        // Live server statistics
        std::string GetActiveUserCount();       // Currently authenticated users
        std::string GetUserCount();             // Total registered users

        // Error handling
        std::string GetErrorMessage();          // Last error description (empty if no error)
        void ClearError();                      // Reset error state
        bool HasError();                        // Check if error occurred

        // License expiry helpers
        int GetDaysRemaining();                 // Days until expiry (-1 if lifetime, 0 if expired)
        bool IsLifetime();                      // True if license never expires
        bool IsExpiringSoon(int days_threshold = 7); // True if expiring within threshold, set value as you wish

        // Authentication status
        bool IsAuthenticated();                 // True if logged in and session active
        bool IsBanned();                        // True if user account is banned

        // Device info
    }

    // ========================================================================
    // VARIABLES - Server-Side Configuration
    // ========================================================================
    namespace Variables {
        // Fetch server-side variables (set through Atlas dashboard)
        std::string Fetch(const std::string& key);   // Get text value (empty if not found)
        bool FetchBool(const std::string& key);      // Get boolean ("true"/"1"/"yes" = true)
        int FetchInt(const std::string& key);        // Get integer (0 if not found/invalid)
    }

    // ========================================================================
    // HELPER - Utility Functions
    // ========================================================================
    namespace Helper {
        // NOTICE: All these functions are unrelated to Atlas!
        
        // Send Discord webhook message
        bool SendDiscord(const std::string& webhook_url, const std::string& message);

        // Send Discord embed (rich formatted message)
        // color: Hexadecimal color (default: 0x3498db - blue)
        bool SendDiscordEmbed(const std::string& webhook_url, const std::string& title, const std::string& description, int color = 0x3498db);

        // Send custom HTTP webhook with JSON payload (Slack, custom endpoints, etc.)
        bool SendWebhook(const std::string& url, const std::string& json_payload);

        // Force exit the application process (100% secure & untamperable termination)
        void Exit();

        // Lightweight connectivity check — returns round-trip time in milliseconds, -1 if unreachable
        int Ping();
    }

} // namespace Atlas
