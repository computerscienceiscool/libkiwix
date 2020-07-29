/*
 * Copyright 2019 Matthieu Gautier<mgautier@kymeria.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */


#ifndef KIWIXLIB_SERVER_RESPONSE_H
#define KIWIXLIB_SERVER_RESPONSE_H

#include <string>

#include <mustache.hpp>
#include "byte_range.h"
#include "entry.h"
#include "etag.h"

extern "C" {
#include "microhttpd_wrapper.h"
}

namespace kiwix {

enum class ResponseMode {
  ERROR_RESPONSE,
  RAW_CONTENT,
};

class InternalServer;
class RequestContext;

class EntryResponse;

class Response {
  public:
    Response(const std::string& root, bool verbose, bool withTaskbar, bool withLibraryButton, bool blockExternalLinks);
    virtual ~Response() = default;

    static std::unique_ptr<Response> build(const InternalServer& server);
    static std::unique_ptr<Response> build_304(const InternalServer& server, const ETag& etag);
    static std::unique_ptr<Response> build_404(const InternalServer& server, const RequestContext& request, const std::string& bookName);
    static std::unique_ptr<Response> build_500(const InternalServer& server, const std::string& msg);

    MHD_Result send(const RequestContext& request, MHD_Connection* connection);

    void set_code(int code) { m_returnCode = code; }
    void set_cacheable() { m_etag.set_option(ETag::CACHEABLE_ENTITY); }
    void set_server_id(const std::string& id) { m_etag.set_server_id(id); }
    void set_compress(bool compress) { m_compress = compress; }
    void set_taskbar(const std::string& bookName, const std::string& bookTitle);

    int getReturnCode() const { return m_returnCode; }
    std::string get_mimeType() const { return m_mimeType; }

    void introduce_taskbar();
    void inject_externallinks_blocker();

    bool can_compress(const RequestContext& request) const;
    bool contentDecorationAllowed() const;

  private: // functions
    virtual MHD_Response* create_mhd_response(const RequestContext& request);
    MHD_Response* create_error_response(const RequestContext& request) const;
    MHD_Response* create_raw_content_mhd_response(const RequestContext& request);

  protected: // data
    bool m_verbose;
    ResponseMode m_mode;
    std::string m_root;
    std::string m_content;
    std::string m_mimeType;
    int m_returnCode;
    bool m_withTaskbar;
    bool m_withLibraryButton;
    bool m_blockExternalLinks;
    bool m_compress;
    std::string m_bookName;
    std::string m_bookTitle;
    ByteRange m_byteRange;
    ETag m_etag;

    friend class EntryResponse; // temporary to allow the builder to change m_mode
};


class RedirectionResponse : public Response {
  public:
    RedirectionResponse(const std::string& root, bool verbose, bool withTaskbar, bool withLibraryButton, bool blockExternalLinks, const std::string& redirectionUrl);
    static std::unique_ptr<Response> build(const InternalServer& server, const std::string& redirectionUrl);


  private:
    MHD_Response* create_mhd_response(const RequestContext& request);

    std::string m_redirectionUrl;
};

class ContentResponse : public Response {
  public:
    ContentResponse(const std::string& root, bool verbose, bool withTaskbar, bool withLibraryButton, bool blockExternalLinks, const std::string& content, const std::string& mimetype);
    static std::unique_ptr<Response> build(const InternalServer& server, const std::string& content, const std::string& mimetype);
    static std::unique_ptr<Response> build(const InternalServer& server, const std::string& template_str, kainjow::mustache::data data, const std::string& mimetype);
};

class EntryResponse : public Response {
  public:
    EntryResponse(const std::string& root, bool verbose, bool withTaskbar, bool withLibraryButton, bool blockExternalLinks, const Entry& entry, const std::string& mimetype, const ByteRange& byterange);
    static std::unique_ptr<Response> build(const InternalServer& server, const RequestContext& request, const Entry& entry);

  private:
    MHD_Response* create_mhd_response(const RequestContext& request);

    Entry m_entry;

};

}

#endif //KIWIXLIB_SERVER_RESPONSE_H
