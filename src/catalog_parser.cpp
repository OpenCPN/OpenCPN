/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2019 Alec Leamas                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include <cstring>

#include <expat.h>

#include <wx/log.h>

#if defined(__MINGW32__) && defined(Yield)
#undef Yield                 // from win.h, conflicts with mingw headers
#endif

#include "catalog_parser.h"
#include "ocpn_utils.h"

#ifdef XML_LARGE_SIZE
#  if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#    define XML_FMT_INT_MOD "I64"
#  else
#    define XML_FMT_INT_MOD "ll"
#  endif
#else
#  define XML_FMT_INT_MOD "l"
#endif

#ifdef XML_UNICODE_WCHAR_T
# include <wchar.h>
# define XML_FMT_STR "ls"
#else
# define XML_FMT_STR "s"
#endif


static void XMLCALL elementData(void* userData, const XML_Char* s, int len)
{
    catalog_ctx* ctx = static_cast<catalog_ctx*>(userData);
    ctx->buff.append(s, len);
}


static void XMLCALL
startElement(void* userData, const XML_Char* name, const XML_Char** atts)
{
    catalog_ctx* ctx = static_cast<catalog_ctx*>(userData);
    ctx->buff = "";
    if (strcmp(name, "plugin") == 0) {
        ctx->plugin = std::unique_ptr<PluginMetadata>(new PluginMetadata);
        ctx->depth += 1;
    }
}


static void XMLCALL endElement(void* userData, const XML_Char* name)
{
    catalog_ctx* ctx = static_cast<catalog_ctx*>(userData);
    std::string buff = ctx->buff;
    if (ctx->depth <= 0)  {
        if (strcmp(name, "version") == 0) {
            ctx->version = ocpn::trim(buff);
        }
        else if (strcmp(name, "date") == 0) {
            ctx->date = ocpn::trim(buff);
        }
    }
    else if (strcmp(name, "plugin") == 0) {
        ctx->plugins.push_back(*ctx->plugin);
        ctx->depth -= 1;
    } else if (strcmp(name, "name") == 0) {
        ctx->plugin->name = ocpn::trim(buff);
    } else if (strcmp(name, "version") == 0) {
        ctx->plugin->version = ocpn::trim(buff);
    } else if (strcmp(name, "release") == 0) {
        ctx->plugin->release = ocpn::trim(buff);
    } else if (strcmp(name, "summary") == 0) {
        ctx->plugin->summary = ocpn::trim(buff);
    } else if (strcmp(name, "api_version") == 0) {
        ctx->plugin->api_version = ocpn::trim(buff);
    } else if (strcmp(name, "author") == 0) {
        ctx->plugin->author = ocpn::trim(buff);
    } else if (strcmp(name, "description") == 0) {
        ctx->plugin->description = ocpn::trim(buff);
    } else if (strcmp(name, "git-commit") == 0) {
        ctx->plugin->git_commit = ocpn::trim(buff);
    } else if (strcmp(name, "git-date") == 0) {
        ctx->plugin->git_date = ocpn::trim(buff);
    } else if (strcmp(name, "source") == 0) {
        ctx->plugin->source = ocpn::trim(buff);
    } else if (strcmp(name, "tarball-url") == 0) {
        ctx->plugin->tarball_url = ocpn::trim(buff);
    } else if (strcmp(name, "info-url") == 0) {
        ctx->plugin->info_url = ocpn::trim(buff);
    } else if (strcmp(name, "target") == 0) {
        ctx->plugin->target = ocpn::trim(buff);
    } else if (strcmp(name, "target-version") == 0) {
        ctx->plugin->target_version = ocpn::trim(buff);
    } else if (strcmp(name, "open-source") == 0) {
        ctx->plugin->openSource = ocpn::trim(buff) == "yes";
    }
}


bool ParseCatalog(const std::string xml, catalog_ctx* ctx)
{
    bool ok = true;
    XML_Parser parser = XML_ParserCreate(NULL);
    ctx->buff.clear();
    ctx->depth = 0;

    XML_SetUserData(parser, ctx);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser, elementData);

    if (XML_Parse(parser, xml.c_str(), xml.size(), true) == XML_STATUS_ERROR) {
        wxLogWarning("%" XML_FMT_STR " at line %" XML_FMT_INT_MOD "u\n",
                     XML_ErrorString(XML_GetErrorCode(parser)),
                     XML_GetCurrentLineNumber(parser));
        ok = false;
    }
    XML_ParserFree(parser);
    if (ctx->plugins.size() == 0) {
        wxLogWarning("ParseCatalog: No plugins found.");
        ok = false;
    }
    return ok;
}
