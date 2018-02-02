//
// Copyright (c) 2013-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Test that header file is self-contained.
#include <beast/http/file_body.hpp>

#include <beast/core/file_stdio.hpp>
#include <beast/core/flat_buffer.hpp>
#include <beast/http/parser.hpp>
#include <beast/http/serializer.hpp>
#include <beast/unit_test/suite.hpp>
#include <boost/filesystem.hpp>

namespace beast {
namespace http {

class file_body_test : public beast::unit_test::suite
{
public:
    struct lambda
    {
        flat_buffer buffer;

        template<class ConstBufferSequence>
        void
        operator()(error_code&, ConstBufferSequence const& buffers)
        {
            buffer.commit(boost::asio::buffer_copy(
                buffer.prepare(boost::asio::buffer_size(buffers)),
                buffers));
        }
    };

    template<class File>
    void
    doTestFileBody()
    {
        error_code ec;
        string_view const s =
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 3\r\n"
            "\r\n"
            "xyz";
        auto const temp = boost::filesystem::unique_path();
        {
            response_parser<basic_file_body<File>> p;
            p.eager(true);

            p.get().body.open(
                temp.string<std::string>().c_str(), file_mode::write, ec);
            BEAST_EXPECTS(! ec, ec.message());

            p.put(boost::asio::buffer(s.data(), s.size()), ec);
            BEAST_EXPECTS(! ec, ec.message());
        }
        {
            File f;
            f.open(temp.string<std::string>().c_str(), file_mode::read, ec);
            auto size = f.size(ec);
            BEAST_EXPECTS(! ec, ec.message());
            BEAST_EXPECT(size == 3);
            std::string s1;
            s1.resize(3);
            f.read(&s1[0], s1.size(), ec);
            BEAST_EXPECTS(! ec, ec.message());
            BEAST_EXPECTS(s1 == "xyz", s);
        }
        {
            lambda visit;
            {
                response<basic_file_body<File>> res{status::ok, 11};
                res.set(field::server, "test");
                res.body.open(temp.string<std::string>().c_str(),
                    file_mode::scan, ec);
                BEAST_EXPECTS(! ec, ec.message());
                res.prepare_payload();

                serializer<false, basic_file_body<File>, fields> sr{res};
                sr.next(ec, visit);
                BEAST_EXPECTS(! ec, ec.message());
                auto const cb = *visit.buffer.data().begin();
                string_view const s1{
                    boost::asio::buffer_cast<char const*>(cb),
                    boost::asio::buffer_size(cb)};
                BEAST_EXPECTS(s1 == s, s1);
            }
        }
        boost::filesystem::remove(temp, ec);
        BEAST_EXPECTS(! ec, ec.message());
    }
    void
    run() override
    {
        doTestFileBody<file_stdio>();
    #if BEAST_USE_WIN32_FILE
        doTestFileBody<file_win32>();
    #endif
    #if BEAST_USE_POSIX_FILE
        doTestFileBody<file_posix>();
    #endif
    }
};

BEAST_DEFINE_TESTSUITE(file_body,http,beast);

} // http
} // beast
