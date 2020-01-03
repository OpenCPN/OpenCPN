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
#include "catalog_handler.h"
#include "Downloader.h"

#include <fstream>
#include <sstream>
#include <iterator>

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


//      PluginMetadata Implementation
bool PluginMetadata::IsSameAs( PluginMetadata *other ){
    bool result = false;
    if(strlen(other->name.c_str()) && !strcmp(name.c_str(), other->name.c_str())){
        if(strlen(other->target.c_str()) && !strcmp(target.c_str(), other->target.c_str())){
            if(strlen(other->target_version.c_str()) && !strcmp(target_version.c_str(), other->target_version.c_str())){
                if(strlen(other->version.c_str()) && !strcmp(version.c_str(), other->version.c_str())){
                    if(strlen(other->release.c_str()) && !strcmp(release.c_str(), other->release.c_str())){
                        result = true;
                    }
                }
            }
        }
    }
    return result;
}


void PluginMetadata::MergeFrom( std::unique_ptr<PluginMetadata> &other ){
    version = other->version;
    release = other->release;
    summary = other->summary;
    api_version = other->api_version;
    author = other->author;
    description = other->description;
    git_commit = other->git_commit;
    git_date = other->git_date;
    source = other->source;
    tarball_url = other->tarball_url;
    info_url = other->info_url;
    openSource = other->openSource;
    target = other->target;
    target_version = other->target_version;
    meta_url = other->meta_url;
}


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
    
    else if (strcmp(name, "name") == 0) {
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
    } else if (strcmp(name, "meta-url") == 0) {
        ctx->plugin->meta_url = ocpn::trim(buff);
    }

    // Done with "plugin" clause
    if (strcmp(name, "plugin") == 0) {
        if(!ctx->plugin)
            return;
    
        //  Add a new plugin if required, otherwise merge the tentative plugin metadata
        bool bmerged = false;
        for(unsigned int i=0 ; i < ctx->plugins.size() ; i++){
            PluginMetadata candidatePlugin = ctx->plugins[i];
            if(ctx->plugin->IsSameAs(&candidatePlugin)){
                candidatePlugin.MergeFrom(ctx->plugin);
                // clear the merged plugin meta-info
                candidatePlugin.meta_url.clear();
                bmerged =true;
                break;
            }
        }
        
        // No match found, must be a "new" plugin, so add this one
        if(!bmerged){
            ctx->plugins.push_back(*ctx->plugin);
            ctx->depth -= 1;
        }
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
    
    // Now look for embedded meta info redirection tags, semi-recursively
    unsigned int index = 0;
    while(index < ctx->plugins.size()){
        PluginMetadata plugin = ctx->plugins[index];
        if(strlen(plugin.meta_url.c_str())){
            auto handler = CatalogHandler::getInstance();
            std::string filePath = "";
            CatalogHandler::ServerStatus stat = handler->DownloadCatalog(filePath, plugin.meta_url);
            if (stat == CatalogHandler::ServerStatus::OK) {
                std::ifstream ifpath(filePath);
                std::string xml((std::istreambuf_iterator<char>(ifpath)), std::istreambuf_iterator<char>());
                
                //TODO  Should validate the XML against the XSD schema here....
                
                XML_Parser parser = XML_ParserCreate(NULL);
                ctx->buff.clear();
                ctx->depth = 0;

                XML_SetUserData(parser, ctx);
                XML_SetElementHandler(parser, startElement, endElement);
                XML_SetCharacterDataHandler(parser, elementData);

                if (XML_Parse(parser, xml.c_str(), xml.size(), true) == XML_STATUS_ERROR) {
                    wxLogWarning(_T("While processing redirected metaURL..."));
                    wxLogWarning("%" XML_FMT_STR " at line %" XML_FMT_INT_MOD "u\n",
                     XML_ErrorString(XML_GetErrorCode(parser)),
                     XML_GetCurrentLineNumber(parser));
                    ok = false;
                }
                XML_ParserFree(parser);

            }
            ctx->plugins[index].meta_url.clear();   // tag as processed
                                                    // This may leave a "blank" plugin in the vector, no real harm?
                                                    // TODO scrub the vector when all done parsing, removing blanks.
            index = 0;          // restart the vector traverse
        }
        else
            index++;
    }
        
    if (ctx->plugins.size() == 0) {
        wxLogWarning("ParseCatalog: No plugins found.");
        ok = false;
    }
    return ok;
}
