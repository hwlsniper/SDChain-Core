//------------------------------------------------------------------------------
/*
    This file is part of Beast: https://github.com/vinniefalco/Beast
    Copyright 2013, Vinnie Falco <vinnie.falco@gmail.com>



*/
//==============================================================================

#include <sdchain/beast/utility/weak_fn.h>
#include <sdchain/beast/unit_test.h>

namespace beast {

class weak_fn_test : public beast::unit_test::suite
{
public:
    struct T
    {
        bool& called_;

        explicit
        T (bool& called)
            : called_(called)
        {
        }

        void
        fv()
        {
            called_ = true;
        }

        void
        fi(int i)
        {
            called_ = true;
        }

        void
        fis(int, std::string)
        {
            called_ = true;
        }

        int
        fri()
        {
            called_ = true;
            return 2;
        }
    };

    void
    run()
    {
        {
            bool called = false;
            auto const p = std::make_shared<T>(called);
            std::bind(weak_fn(&T::fv, p))();
            BEAST_EXPECT(called);
        }

        {
            bool called = false;
            auto p = std::make_shared<T>(called);
            auto call = std::bind(weak_fn(&T::fv, p));
            p.reset();
            call();
            BEAST_EXPECT(! called);
        }

        {
            bool called = false;
            auto p = std::make_shared<T>(called);
            std::bind(weak_fn(&T::fi, p), 1)();
            BEAST_EXPECT(called);
        }

        {
            bool called = false;
            auto p = std::make_shared<T>(called);
            std::bind(weak_fn(&T::fi, p),
                std::placeholders::_1)(1);
            BEAST_EXPECT(called);
        }

        {
            bool called = false;
            auto p = std::make_shared<T>(called);
            std::bind(weak_fn(&T::fis, p),
                1, std::placeholders::_1)("foo");
            BEAST_EXPECT(called);
        }

        {
            bool called = false;
            auto p = std::make_shared<T>(called);
            try
            {
                auto call = std::bind(weak_fn(&T::fis, p, throw_if_invalid<>()),
                    1, std::placeholders::_1);
                p.reset();
                call("foo");
                fail();
            }
            catch(std::bad_weak_ptr const&)
            {
                BEAST_EXPECT(! called);
            }
        }

        {
            bool called = false;
            auto p = std::make_shared<T>(called);
            BEAST_EXPECT(std::bind(weak_fn(&T::fri, p))() == 2);
            BEAST_EXPECT(called);
        }

        {
            bool called = false;
            auto p = std::make_shared<T>(called);
            auto call = std::bind(weak_fn(&T::fv, p,
                [&called]()
                {
                    called = true;
                }));
            p.reset();
            call();
            BEAST_EXPECT(called);
        }
    }
};

BEAST_DEFINE_TESTSUITE(weak_fn,asio,beast);

}
