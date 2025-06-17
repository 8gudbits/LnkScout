#include <iostream>
#include <windows.h>
#include <shobjidl.h>
#include <filesystem>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

// Get the current user's AppData Microsoft directory
fs::path get_protected_path()
{
    const char *userProfile = getenv("USERPROFILE");
    if (userProfile)
    {
        return fs::path(userProfile) / "AppData" / "Roaming" / "Microsoft";
    }
    return "";
}

// Function to check if a .lnk file is valid
bool is_lnk_file_valid(const fs::path &lnk_path)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    bool isValid = false;
    IShellLinkW *psl;
    IPersistFile *ppf;
    WCHAR szGotPath[MAX_PATH];
    WIN32_FIND_DATAW wfd;

    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, reinterpret_cast<void **>(&psl));
        if (SUCCEEDED(hr))
        {
            hr = psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void **>(&ppf));
            if (SUCCEEDED(hr))
            {
                hr = ppf->Load(lnk_path.c_str(), STGM_READ);
                if (SUCCEEDED(hr))
                {
                    hr = psl->Resolve(NULL, SLR_NO_UI);
                    if (SUCCEEDED(hr))
                    {
                        hr = psl->GetPath(szGotPath, MAX_PATH, &wfd, SLGP_SHORTPATH);
                        if (SUCCEEDED(hr))
                        {
                            isValid = fs::exists(szGotPath);
                        }
                    }
                }
                ppf->Release();
            }
            psl->Release();
        }
        CoUninitialize();
    }
    return isValid;
}

// Function to list and optionally remove invalid .lnk files
void list_lnk_files(const fs::path &path, bool show_valid, bool show_invalid, bool remove_invalid, bool sort_results)
{
    fs::path protectedPath = get_protected_path();
    bool protected_skipped = false;

    std::vector<fs::path> directories_to_scan = {path};                // Queue of directories to scan
    std::vector<std::string> valid_links, invalid_links, skipped_dirs; // Sorting containers

    while (!directories_to_scan.empty())
    {
        fs::path current_dir = directories_to_scan.back();
        directories_to_scan.pop_back();

        std::error_code ec;
        fs::directory_iterator dir_it(current_dir, fs::directory_options::skip_permission_denied, ec);

        if (ec)
        {
            std::string skipped_msg = "Skipped - " + current_dir.string();
            if (sort_results)
                skipped_dirs.push_back(skipped_msg);
            else
                std::cout << skipped_msg << std::endl; // Print immediately if sorting is disabled
            continue;
        }

        for (const auto &entry : dir_it)
        {
            if (entry.is_directory())
            {
                directories_to_scan.push_back(entry.path()); // Add subdirectory to queue
            }
            else if (entry.is_regular_file() && entry.path().extension() == ".lnk")
            {
                bool valid = is_lnk_file_valid(entry.path());
                std::string result = (valid ? "Valid" : "Invalid") + std::string(" - ") + entry.path().string();

                // Check if file is inside the protected directory
                if (remove_invalid && entry.path().string().find(protectedPath.string()) == 0)
                {
                    protected_skipped = true;
                    std::cout << "Protected - " << entry.path() << " (Skipped for system stability)" << std::endl;
                    continue;
                }

                if (sort_results)
                {
                    if (valid && show_valid)
                        valid_links.push_back(result);
                    else if (!valid && show_invalid)
                        invalid_links.push_back(result);
                }
                else
                {
                    if (valid && show_valid)
                        std::cout << result << std::endl;
                    else if (!valid && show_invalid)
                        std::cout << result << std::endl;
                }

                if (!valid && remove_invalid)
                {
                    fs::remove(entry.path());
                }
            }
        }
    }

    // Display sorted results only if sorting is enabled
    if (sort_results)
    {
        for (const auto &dir : skipped_dirs)
            std::cout << dir << std::endl;
        for (const auto &link : valid_links)
            std::cout << link << std::endl;
        for (const auto &link : invalid_links)
            std::cout << link << std::endl;
    }

    // Notify user if protected directory was skipped
    if (protected_skipped)
    {
        std::cout << "\nNote: Files inside \"" << protectedPath.string() << "\" were skipped for system stability.\n"
                  << "Removing certain invalid shortcuts (e.g., File Explorer.lnk, Task Manager.lnk) can break Windows UI features.\n";
    }
}

// Entry point of the program
int main(int argc, char *argv[])
{
    bool show_banner = true;
    bool show_valid = true;
    bool show_invalid = true;
    bool remove_invalid = false;
    bool sort_results = false;

    fs::path path;
    bool path_set = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-n" || arg == "--nobanner")
        {
            show_banner = false;
        }
        else if (arg == "-h" || arg == "--help")
        {
            std::cout << "\nLnkScout 1.1 (x64) : (c) 8gudbits - All rights reserved.\n"
                      << "Source - \"https://github.com/8gudbits/8gudbitsKit\"\n"
                      << "\nUsage: " << argv[0] << " <directory path> [--valid/-v | --invalid/-i [--remove/-r]] [--sort/-s] [--nobanner/-n] [--help/-h]\n"
                      << "\nOptions:\n"
                      << " -h, --help         Show this help message.\n"
                      << " -n, --nobanner     Suppresses the banner.\n"
                      << " -v, --valid        Display only valid links.\n"
                      << " -i, --invalid      Display only invalid links.\n"
                      << " -r, --remove       Remove invalid links (except protected system files).\n"
                      << " -s, --sort         Sort output: Skipped first, Valid second, Invalid last.\n";
            return 0;
        }
        else if (arg == "-v" || arg == "--valid")
        {
            show_invalid = false;
        }
        else if (arg == "-i" || arg == "--invalid")
        {
            show_valid = false;
        }
        else if (arg == "-r" || arg == "--remove")
        {
            remove_invalid = true;
        }
        else if (arg == "-s" || arg == "--sort")
        {
            sort_results = true;
        }
        else
        {
            if (!path_set)
            {
                path = arg;
                path_set = true;
            }
        }
    }

    if (!path_set || !fs::exists(path) || !fs::is_directory(path))
    {
        std::cerr << "\nError: Path does not exist, path is not a directory, or was not specified.\n"
                  << "Reminder: Make sure to not use backslash at the end of path. For instance, \"E:\\New Folder\\Example\\\".\n"
                  << "Use --help/-h to see the help menu.\n";
        return 1;
    }

    if (show_banner)
    {
        std::cout << "\nLnkScout 1.1 (x64) : (c) 8gudbits - All rights reserved.\n"
                  << "Source - \"https://github.com/8gudbits/8gudbitsKit\"\n";
    }

    std::cout << "\nLooking for shortcut files . . ." << std::endl;
    if (remove_invalid)
    {
        std::cout << "Any invalid 'lnk' files will be removed (except protected system files) . . ." << std::endl;
    }

    list_lnk_files(path, show_valid, show_invalid, remove_invalid, sort_results);
    return 0;
}
