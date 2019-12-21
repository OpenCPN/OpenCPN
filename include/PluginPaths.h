#include   <string>
#include   <vector>
#include   <wx/filename.h>
#include   <wx/platinfo.h>
#include   <wx/string.h>

/** Join a list of directories to a string. */
std::string join(std::vector<std::string> v, char c);

class PluginPaths {

public:

    /**  Return the singleton instance. */
    static PluginPaths* getInstance();

    /** The single, user-writable directory for installing .dll files. */
    std::string UserLibdir() { return m_userLibdir; };

    /** The single, user-writable directory for installing helper binaries. */
    std::string UserBindir() { return m_userBindir; }

    /**
     * The single, user-writable common parent for plugin data
     * directories, typically ending in 'plugins'.
     */
    std::string UserDatadir() { return m_userDatadir; }

    /** Location used to store files with unknown prefix.  */
    std::string UserUnknownPrefixDir() { return m_unknownPathDir; }

    /** List of directories from which we load plugins.*/
    std::vector<std::string> Libdirs()  { return m_libdirs; }

    /** 'List of directories for plugin binary helpers.*/
    std::vector<std::string> Bindirs() { return m_bindirs; };

    /**
     *  List of directories containing plugin data, each
     *  element typically ends in 'plugins'.
     */
    std::vector<std::string> Datadirs() { return m_datadirs; }

    /** home directory, convenience stuff. */
    std::string Homedir() const { return m_home; }

private:
    PluginPaths();
    void initWindowsPaths();
    void initFlatpackPaths();
    void initLinuxPaths();
    void initApplePaths();

    std::string m_home;
    std::string m_userDatadir;
    std::string m_userBindir;
    std::string m_userLibdir;
    std::string m_unknownPathDir;
    std::vector<std::string> m_libdirs;
    std::vector<std::string> m_bindirs;
    std::vector<std::string> m_datadirs;
};
