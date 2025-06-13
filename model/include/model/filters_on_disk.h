/**
 * \file
 * Filter storage routines. Handles both user defined filters and the
 * readonly system ones shipped with application.
 *
 * All methods uses plain names as arguments and return values, actual
 * filenames and paths are hidden by the interface.
 */

#include <string>
#include <vector>

#include "model/navmsg_filter.h"

namespace filters_on_disk {

/** Return list of filters, possibly including also the system ones. */
std::vector<std::string> List(bool include_system = false);

/**
 *  Return true iff filter with given name exists, either system or user
 *  defined.
 */
bool Exists(const std::string& name);

/** Remove a filter, return ok if no errors. */
bool Remove(const std::string& name);

/** Write contents for given filter to disk. */
bool Write(const NavmsgFilter& filter, const std::string& name);

/** Read filter with given name from disk. */
NavmsgFilter Read(const std::string& name);

}  // namespace filters_on_disk
